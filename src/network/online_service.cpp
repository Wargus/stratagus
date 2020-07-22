#include <clocale>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

#ifdef USE_WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif

#include "assert.h"
#include "network.h"
#include "network/netsockets.h"
#include "util.h"
#include "version.h"

#include "./xsha1.h"

class BNCSInputStream {
public:
    BNCSInputStream(CTCPSocket *socket) {
        this->sock = socket;
        this->bufsize = 1024;
        this->buffer = (char*)calloc(sizeof(char), bufsize);
        this->avail = 0;
        this->pos = 0;
    };
    ~BNCSInputStream() {};

    std::string readString() {
        if (avail == 0) {
            return NULL;
        }
        std::stringstream strstr;
        int i = pos;
        char c;
        while ((c = buffer[i]) != '\0' && i < avail) {
            strstr.put(c);
            i += 1;
        }
        consumeData(i);
        return strstr.str();
    };

    std::vector<std::string> readStringlist() {
        std::vector<std::string> stringlist;
        while (true) {
            std::string nxt = readString();
            if (nxt.empty()) {
                break;
            } else {
                stringlist.push_back(nxt);
            }
        }
        return stringlist;
    };

    uint8_t read8() {
        uint8_t byte = buffer[pos];
        consumeData(1);
        return byte;
    }

    uint16_t read16() {
        uint16_t byte = ntohs(reinterpret_cast<uint16_t *>(buffer + pos)[0]);
        consumeData(2);
        return byte;
    }

    uint32_t read32() {
        uint32_t byte = ntohs(reinterpret_cast<uint32_t *>(buffer + pos)[0]);
        consumeData(4);
        return byte;
    }

    uint64_t read64() {
        uint64_t byte = ntohs(reinterpret_cast<uint64_t *>(buffer + pos)[0]);
        consumeData(8);
        return byte;
    }

    bool readBool8() {
        return read8() != 0;
    }

    bool readBool32() {
        return read32() != 0;
    }

    uint64_t readFiletime() {
        return read64();
    }

    std::string string32() {
        // uint32 encoded (4-byte) string
        uint32_t data = read32();
        char dt[5];
        strncpy(dt, (const char*)&data, 4);
        dt[4] = '\0';
        return std::string(dt);
    };

    /**
     * To be called at the start of a message, gets the entire data into memory or returns -1.
     */
    uint8_t readMessageId() {
        // Every BNCS message has the same header:
        //  (UINT8) Always 0xFF
        //  (UINT8) Message ID
        // (UINT16) Message length, including this header
        //   (VOID) Message data
        avail += this->sock->Recv(buffer + avail, 4 - avail);
        if (avail < 4) {
            return -1;
        }
        assert(read8() == 0xff);
        uint8_t msgId = read8();
        uint16_t len = read16();
        avail += this->sock->Recv(buffer + avail, len - 4);
        if (avail < len) {
            // Didn't receive full message on the socket, yet. Reset position so
            // this method can be used to try again
            pos = 0;
            return -1;
        } else {
            return 0;
        }
    };

private:
    void consumeData(int bytes) {
        pos += bytes;
    }

    CTCPSocket *sock;
    char *buffer;
    int avail;
    int pos;
    int bufsize;
};

class BNCSOutputStream {
public:
    BNCSOutputStream(uint8_t id) {
        // Every BNCS message has the same header:
        //  (UINT8) Always 0xFF
        //  (UINT8) Message ID
        // (UINT16) Message length, including this header
        //   (VOID) Message data
        this->sz = 16;
        this->buf = (uint8_t*) calloc(sizeof(uint8_t), this->sz);
        this->pos = 0;
        serialize8(0xff);
        serialize8(id);
        this->length_pos = pos;
        serialize16((uint16_t)0);
    };
    ~BNCSOutputStream() {
        free(buf);
    };

    void serialize32(uint32_t data) {
        ensureSpace(sizeof(data));
        uint32_t *view = reinterpret_cast<uint32_t *>(buf + pos);
        *view = htonl(data);
        pos += sizeof(data);
    };
    void serialize32(int32_t data) {
        ensureSpace(sizeof(data));
        int32_t *view = reinterpret_cast<int32_t *>(buf + pos);
        *view = htonl(data);
        pos += sizeof(data);
    };
    void serialize16(uint16_t data) {
        ensureSpace(sizeof(data));
        uint16_t *view = reinterpret_cast<uint16_t *>(buf + pos);
        *view = htons(data);
        pos += sizeof(data);
    };
    void serialize16(int16_t data) {
        ensureSpace(sizeof(data));
        uint16_t *view = reinterpret_cast<uint16_t *>(buf + pos);
        *view = htons(data);
        pos += sizeof(data);
    };
    void serialize8(uint8_t data) {
        ensureSpace(sizeof(data));
        *(buf + pos) = data;
        pos++;
    };
    void serialize(const char* str, int len) {
        ensureSpace(len);
        memcpy(buf + pos, str, len);
        pos += len;
    };
    void serialize(const char* str) {
        int len = strlen(str);
        ensureSpace(len);
        memcpy(buf + pos, str, len);
        pos += len;
    };

    int flush(CTCPSocket *sock) {
        return sock->Send(getBuffer(), pos);
    };

    void flush(CUDPSocket *sock, CHost *host) {
        sock->Send(*host, getBuffer(), pos);
    };

private:
    uint8_t *getBuffer() {
        // insert length to make it a valid buffer
        uint16_t *view = reinterpret_cast<uint16_t *>(buf + length_pos);
        *view = htons(pos);
        return buf;
    };

    void ensureSpace(size_t required) {
        if (pos + required < sz) {
            sz = sz * 2;
            buf = (uint8_t*) realloc(buf, sz);
            assert(buf != NULL);
        }
    }

    uint8_t *buf;
    int sz;
    int pos;
    int length_pos;
};

class Context;
class NetworkState {
public:
    virtual ~NetworkState() {};
    virtual void doOneStep(Context *ctx) = 0;

protected:
    int send(Context *ctx, BNCSOutputStream *buf);
};

class Context {
public:
    Context() {
        this->udpSocket = new CUDPSocket();
        this->tcpSocket = new CTCPSocket();
        this->istream = new BNCSInputStream(tcpSocket);
        this->state = NULL;
        this->host = new CHost("127.0.0.1", 6112); // TODO: parameterize
        this->clientToken = MyRand();
        this->username = "";
        this->password = "";
        this->info = "";
    }

    ~Context() {
        if (state != NULL) {
            delete state;
        }
        delete udpSocket;
        delete tcpSocket;
        delete host;
    }

    std::string getInfo() { return info; }

    void setInfo(std::string arg) { info = arg; }

    std::string getUsername() { return username; }

    void setUsername(std::string arg) { username = arg; }

    std::string getPassword() { return password; }

    void setPassword(std::string arg) { password = arg; }

    CHost *getHost() { return host; }

    void setHost(CHost *arg) {
        if (host != NULL) {
            delete host;
        }
        host = arg;
    }

    CUDPSocket *getUDPSocket() { return udpSocket; }

    CTCPSocket *getTCPSocket() { return tcpSocket; }

    BNCSInputStream *getMsgIStream() { return istream; }

    void doOneStep() { this->state->doOneStep(this); }

    void setState(NetworkState* newState) {
        assert (newState != this->state);
        if (this->state != NULL) {
            delete this->state;
        }
        this->state = newState;
    }

    uint32_t clientToken;
    uint32_t serverToken;

private:
    NetworkState *state;
    CHost *host;
    CUDPSocket *udpSocket;
    CTCPSocket *tcpSocket;
    BNCSInputStream *istream;

    std::string info;
    std::string username;
    std::string password;
};

int NetworkState::send(Context *ctx, BNCSOutputStream *buf) {
    return buf->flush(ctx->getTCPSocket());
}

class DisconnectedState : public NetworkState {
public:
    DisconnectedState(std::string message) {
        this->message = message;
        this->hasPrinted = false;
    };

    virtual void doOneStep(Context *ctx) {
        if (!hasPrinted) {
            std::cout << message << std::endl;
            hasPrinted = true;
        }
        // the end
    }

private:
    bool hasPrinted;
    std::string message;
};

class IN_CHAT : public NetworkState {
    virtual void doOneStep(Context *ctx);
};

class S2C_GETCHANNELLIST : public NetworkState {
    virtual void doOneStep(Context *ctx) {
        if (ctx->getTCPSocket()->HasDataToRead(0)) {
            uint8_t msg = ctx->getMsgIStream()->readMessageId();
            if (msg == 0xff) {
                // try again next time
                return;
            }
            if (msg != 0x0b) {
                std::string error = std::string("Expected SID_GETCHANNELLIST, got msg id ");
                error += std::to_string(msg);
                ctx->setState(new DisconnectedState(error));
            }

            std::vector<std::string> channels = ctx->getMsgIStream()->readStringlist();

            ctx->setState(new IN_CHAT());
        }
    }
};

class S2C_ENTERCHAT : public NetworkState {
    virtual void doOneStep(Context *ctx) {
        if (ctx->getTCPSocket()->HasDataToRead(0)) {
            uint8_t msg = ctx->getMsgIStream()->readMessageId();
            if (msg == 0xff) {
                // try again next time
                return;
            }
            if (msg != 0x0a) {
                std::string error = std::string("Expected SID_ENTERCHAT, got msg id ");
                error += std::to_string(msg);
                ctx->setState(new DisconnectedState(error));
            }

            std::string uniqueName = ctx->getMsgIStream()->readString();
            std::string statString = ctx->getMsgIStream()->readString();
            std::string accountName = ctx->getMsgIStream()->readString();

            ctx->setUsername(uniqueName);
            if (!statString.empty()) {
                ctx->setInfo("Statstring after logon: " + statString);
            }

            ctx->setState(new S2C_GETCHANNELLIST());
        }
    }
};

class C2S_ENTERCHAT : public NetworkState {
    virtual void doOneStep(Context *ctx) {
        // does all of enterchar, getchannellist, and first-join joinchannel
        BNCSOutputStream enterchat(0x0a);
        enterchat.serialize(ctx->getUsername().c_str());
        enterchat.serialize("");
        enterchat.flush(ctx->getTCPSocket());

        BNCSOutputStream getlist(0x0b);
        getlist.serialize32(0x00);
        getlist.flush(ctx->getTCPSocket());

        BNCSOutputStream join(0x0c);
        join.serialize32(0x01); // first-join
        join.serialize("ignored");
        join.flush(ctx->getTCPSocket());

        ctx->setState(new S2C_ENTERCHAT());
    }
};

class S2C_PKT_SERVERPING : public NetworkState {
public:
    S2C_PKT_SERVERPING(std::string message) {
        this->retries = 0;
    };

    virtual void doOneStep(Context *ctx) {
        if (ctx->getUDPSocket()->HasDataToRead(1)) {
            // PKT_SERVERPING
            //  (UINT8) 0xFF
            //  (UINT8) 0x05
            // (UINT16) 8
            // (UINT32) UDP Code
            char buf[8];
            int received = ctx->getUDPSocket()->Recv(buf, 8, ctx->getHost());
            if (received == 8) {
                uint32_t udpCode = reinterpret_cast<uint32_t*>(buf)[1];
                BNCSOutputStream udppingresponse(0x14);
                udppingresponse.serialize32(udpCode);
                udppingresponse.flush(ctx->getTCPSocket());
            }
        } else {
            retries++;
            if (retries < 5000) {
                return;
            }
            // we're using a timeout of 1ms, so now we've been waiting at
            // the very least for 5 seconds... let's skip UDP then
        }
        ctx->setState(new C2S_ENTERCHAT());
    }

private:
    int retries;
};

class C2S_LOGONRESPONSE2 : public NetworkState {
    virtual void doOneStep(Context *ctx);
};

class S2C_LOGONRESPONSE2 : public NetworkState {
    virtual void doOneStep(Context *ctx) {
        if (ctx->getTCPSocket()->HasDataToRead(0)) {
            uint8_t msg = ctx->getMsgIStream()->readMessageId();
            if (msg == 0xff) {
                // try again next time
                return;
            }
            if (msg != 0x3a) {
                std::string error = std::string("Expected SID_LOGONRESPONSE2, got msg id ");
                error += std::to_string(msg);
                ctx->setState(new DisconnectedState(error));
            }

            uint32_t status = ctx->getMsgIStream()->read32();

            switch (status) {
            case 0x00:
                // success - TODO - channel list, join chat
                return;
            case 0x01:
            case 0x02:
                ctx->setInfo("Account does not exist or incorrect password");
                ctx->setPassword("");
                ctx->setState(new C2S_LOGONRESPONSE2());
                return;
            case 0x06:
                ctx->setInfo("Account closed: " + ctx->getMsgIStream()->readString());
                ctx->setPassword("");
                ctx->setState(new C2S_LOGONRESPONSE2());
                return;
            default:
                ctx->setState(new DisconnectedState("unknown logon response"));
            }
        }
    }
};

void C2S_LOGONRESPONSE2::doOneStep(Context *ctx) {
    std::string user = ctx->getUsername();
    std::string pw = ctx->getPassword();
    if (!user.empty() && !pw.empty()) {
        // C2S SID_LOGONRESPONSE2
        BNCSOutputStream logon(0x3a);
        logon.serialize32(ctx->clientToken);
        logon.serialize32(ctx->serverToken);

        uint32_t pwhash[5];
        xsha1_calcHashBuf(pw.c_str(), pw.length(), pwhash);
        xsha1_calcHashDat(pwhash, pwhash);
        // Uint8[20] password hash
        for (int i = 0; i < 20; i++) {
            logon.serialize8(reinterpret_cast<uint8_t*>(pwhash)[i]);
        }
        logon.serialize(user.c_str());
        logon.flush(ctx->getTCPSocket());

        ctx->setState(new S2C_LOGONRESPONSE2());
    }
};

class S2C_SID_AUTH_CHECK : public NetworkState {
    virtual void doOneStep(Context *ctx) {
        if (ctx->getTCPSocket()->HasDataToRead(0)) {
            uint8_t msg = ctx->getMsgIStream()->readMessageId();
            if (msg == 0xff) {
                // try again next time
                return;
            }
            if (msg != 0x51) {
                std::string error = std::string("Expected SID_AUTH_CHECK, got msg id ");
                error += std::to_string(msg);
                ctx->setState(new DisconnectedState(error));
            }

            uint32_t result = ctx->getMsgIStream()->read32();
            std::string info = ctx->getMsgIStream()->readString();

            switch (result) {
            case 0x000:
                // Passed challenge
                ctx->setState(new C2S_LOGONRESPONSE2());
                return;
            case 0x100:
                // Old game version
                info = "Old game version: " + info;
                break;
            case 0x101:
                // Invalid version
                info = "Invalid version: " + info;
                break;
            case 0x102:
                // Game version must be downgraded
                info = "Game version must be downgraded: " + info;
                break;
            case 0x200:
                // Invalid CD key
                info = "Invalid CD Key: " + info;
                break;
            case 0x201:
                // CD Key in use
                info = "CD Key in use: " + info;
                break;
            case 0x202:
                // Banned key
                info = "Banned key: " + info;
                break;
            case 0x203:
                // Wrong product
                info = "Wrong product: " + info;
                break;
            default:
                // Invalid version code
                info = "Invalid version code: " + info;
            }
            ctx->setState(new DisconnectedState(info));
        }
    }
};

class S2C_SID_AUTH_INFO : public NetworkState {
    virtual void doOneStep(Context *ctx) {
        if (ctx->getTCPSocket()->HasDataToRead(0)) {
            uint8_t msg = ctx->getMsgIStream()->readMessageId();
            if (msg == 0xff) {
                // try again next time
                return;
            }
            if (msg != 0x50) {
                std::string error = std::string("Expected SID_AUTH_INFO, got msg id ");
                error += std::to_string(msg);
                ctx->setState(new DisconnectedState(error));
            }

            uint32_t logonType = ctx->getMsgIStream()->read32();
            assert(logonType == 0x00); // only support Broken SHA-1 logon for now
            uint32_t serverToken = ctx->getMsgIStream()->read32();
            ctx->serverToken = serverToken;
            uint32_t udpValue = ctx->getMsgIStream()->read32();
            uint64_t mpqFiletime = ctx->getMsgIStream()->readFiletime();
            std::string mpqFilename = ctx->getMsgIStream()->readString();
            std::string formula = ctx->getMsgIStream()->readString();

            // immediately respond with pkt_conntest2 udp msg
            BNCSOutputStream conntest(0x09);
            conntest.serialize32(serverToken);
            conntest.serialize32(udpValue);
            conntest.flush(ctx->getUDPSocket(), ctx->getHost());

            // immediately respond with SID_AUTH_CHECK
            BNCSOutputStream check(0x51);
            check.serialize32(ctx->clientToken);
            // EXE version (one UINT32 value, serialized in network byte order here)
            check.serialize8(StratagusPatchLevel2);
            check.serialize8(StratagusPatchLevel);
            check.serialize8(StratagusMinorVersion);
            check.serialize8(StratagusMajorVersion);
            // EXE hash - we use lua file checksums
            check.serialize32(FileChecksums);
            // no CD key, not a Spawn key
            check.serialize32(0);
            check.serialize32(0);
            // EXE information
            std::string exeInfo("stratagus.exe ");
            exeInfo += StratagusLastModifiedDate;
            exeInfo += " ";
            exeInfo += StratagusLastModifiedTime;
            exeInfo += " ";
            exeInfo += std::to_string(StratagusVersion);
            check.serialize(exeInfo.c_str());
            // Key owner name
            check.serialize(DESCRIPTION);
            check.flush(ctx->getTCPSocket());

            ctx->setState(new S2C_SID_AUTH_CHECK());
        }
    }
};

class S2C_SID_PING : public NetworkState {
    virtual void doOneStep(Context *ctx) {
        if (ctx->getTCPSocket()->HasDataToRead(0)) {
            uint8_t msg = ctx->getMsgIStream()->readMessageId();
            if (msg == 0xff) {
                // try again next time
                return;
            }
            if (msg != 0x25) {
                // not a ping
                std::string error = std::string("Expected SID_PING, got msg id ");
                error += std::to_string(msg);
                ctx->setState(new DisconnectedState(error));
            }
            uint32_t pingValue = ctx->getMsgIStream()->read32();

            // immediately respond
            BNCSOutputStream buffer(0x25);
            buffer.serialize32(pingValue);
            send(ctx, &buffer);

            ctx->setState(new S2C_SID_AUTH_INFO());
        }
    }
};

class C2S_SID_AUTH_INFO : public NetworkState {
    virtual void doOneStep(Context *ctx) {
        BNCSOutputStream buffer(0x50);
        // (UINT32) Protocol ID
        buffer.serialize32(0x00);
        // (UINT32) Platform code
        buffer.serialize("IX86", 4);
        // (UINT32) Product code
        buffer.serialize("WAGS", 4);
        // (UINT32) Version byte
        buffer.serialize32(0x4f);
        // (UINT32) Language code
        buffer.serialize32(0x00);
        // (UINT32) Local IP
        if (CNetworkParameter::Instance.localHost.compare("127.0.0.1")) {
            // user set a custom local ip, use that
            struct in_addr addr;
            inet_aton(CNetworkParameter::Instance.localHost.c_str(), &addr);
            buffer.serialize32(addr.s_addr);
        } else {
            unsigned long ips[20];
            int networkNumInterfaces = NetworkFildes.GetSocketAddresses(ips, 20);
            bool found_one = false;
            if (networkNumInterfaces) {
                // we can only advertise one, so take the first that fits in 32-bit and is thus presumably ipv4
                for (int i = 0; i < networkNumInterfaces; i++) {
                    uint32_t ip = ips[i];
                    if (ip == ips[i]) {
                        found_one = true;
                        buffer.serialize32(ip);
                        break;
                    }
                }
            }
            if (!found_one) {
                // use default
                buffer.serialize32(0x00);
            }
        }
        // (UINT32) Time zone bias
        buffer.serialize32(0x00);
        // (UINT32) MPQ locale ID
        buffer.serialize32(0x00);
        // (UINT32) User language ID
        buffer.serialize32(0x00);
        // (STRING) Country abbreviation
        buffer.serialize("USA");
        // (STRING) Country
        buffer.serialize("United States");

        send(ctx, &buffer);
        ctx->setState(new S2C_SID_PING());
    }
};

class ProtoByteState : public NetworkState {
    virtual void doOneStep(Context *ctx) {
        ctx->getTCPSocket()->Send("\x1", 1); // use game protocol
        ctx->setState(new C2S_SID_AUTH_INFO());
    }
};

class ConnectState : public NetworkState {
    virtual void doOneStep(Context *ctx) {
        ctx->getTCPSocket()->Open(*ctx->getHost());
        if (ctx->getTCPSocket()->IsValid() == false) {
            ctx->setState(new DisconnectedState("TCP open failed"));
            return;
        }
        if (ctx->getTCPSocket()->Connect(*ctx->getHost()) == false) {
            ctx->setState(new DisconnectedState("TCP connect failed"));
            return;
        }
        ctx->setState(new ProtoByteState());
    }
};

static void goOnline() {
    Context *ctx = new Context();
    ctx->setState(new ConnectState());
    while (true) {
        ctx->doOneStep();
        sleep(1);
    }
}
