#include <cstdlib>
#include <cstring>
#include <iostream>
#include <ostream>
#include <unistd.h>

#ifdef USE_WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif

#include "assert.h"

#include "network/netsockets.h"

class BNCSBuf {
public:
    BNCSBuf(uint8_t id) {
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
    ~BNCSBuf() {
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
    uint8_t *getBuffer() {
        // insert length to make it a valid buffer
        uint16_t *view = reinterpret_cast<uint16_t *>(buf + length_pos);
        *view = htons(pos);
        return buf;
    };
    int getLength() {
        return pos;
    };
private:
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
    int send(Context *ctx, BNCSBuf *buf);
};

class Context {
public:
    Context() {
        this->udpSocket = new CUDPSocket();
        this->tcpSocket = new CTCPSocket();
        this->state = NULL;
    }

    ~Context() {
        if (state != NULL) {
            delete state;
        }
        delete udpSocket;
        delete tcpSocket;
    }

    CUDPSocket *getUDPSocket() { return udpSocket; }

    CTCPSocket *getTCPSocket() { return tcpSocket; }

    void doOneStep() { this->state->doOneStep(this); }

    void setState(NetworkState* newState) {
        assert (newState != this->state);
        if (this->state != NULL) {
            delete this->state;
        }
        this->state = newState;
    }

private:
    NetworkState *state;
    CUDPSocket *udpSocket;
    CTCPSocket *tcpSocket;
};

int NetworkState::send(Context *ctx, BNCSBuf *buf) {
    return ctx->getTCPSocket()->Send(buf->getBuffer(), buf->getLength());
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

class C2S_SID_AUTH_INFO : public NetworkState {
    virtual void doOneStep(Context *ctx) {
        BNCSBuf buffer(0x50);
        // (UINT32) Protocol ID
        buffer.serialize32(0x00);
        // (UINT32) Platform code
        buffer.serialize("IX86", 4);
        // (UINT32) Product code
        buffer.serialize("W2BN", 4);
        // (UINT32) Version byte
        buffer.serialize32(0x4f);
        // (UINT32) Language code
        buffer.serialize32(0x00);
        // (UINT32) Local IP
        buffer.serialize32(0x00);
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
        // ctx->setState(new S2C_PING());
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
        CHost host("127.0.0.1", 6112); // TODO: parameterize
        ctx->getTCPSocket()->Open(host);
        if (ctx->getTCPSocket()->IsValid() == false) {
            ctx->setState(new DisconnectedState("TCP open failed"));
            return;
        }
        if (ctx->getTCPSocket()->Connect(host) == false) {
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
