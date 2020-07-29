#include "online_service.h"

#include <arpa/inet.h>
#include <clocale>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <map>
#include <ostream>
#include <queue>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unistd.h>
#include <vector>

#ifdef USE_WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif

#include "cursor.h"
#include "font.h"
#include "input.h"
#include "stratagus.h"
#include "ui.h"
#include "video.h"
#include "widgets.h"
#include "game.h"
#include "parameters.h"
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
        return ntohs(byte);
    }

    uint32_t read32() {
        uint32_t byte = ntohs(reinterpret_cast<uint32_t *>(buffer + pos)[0]);
        consumeData(4);
        return ntohl(byte);
    }

    uint64_t read64() {
        uint64_t byte = ntohs(reinterpret_cast<uint64_t *>(buffer + pos)[0]);
        consumeData(8);
        return ntohl(byte & (uint32_t)-1) | ntohl(byte >> 32);
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
        // we still need to have len in total for this message, so if we have
        // more available than len minus the current position and minus the
        // first 4 bytes that we already consumed, we'll have enough
        long needed = len - avail + pos - 4;
        if (needed > 0) {
            long got = this->sock->Recv(buffer + avail, needed);
            avail += got;
            if (got < needed) {
                // Didn't receive full message on the socket, yet. Reset position so
                // this method can be used to try again
                pos = 0;
                return -1;
            }
        }
        return msgId;
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
    void serialize32(const char* str) {
        assert(strlen(str) == 4);
        uint32_t value;
        memcpy(&value, str, 4);
        serialize32(value);
    };
    void serialize(const char* str) {
        int len = strlen(str) + 1; // include NULL byte
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
        *view = pos;
        return buf;
    };

    void ensureSpace(size_t required) {
        if (pos + required >= sz) {
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

class Friend {
public:
    Friend(std::string name, uint8_t status, uint8_t location, uint32_t product, std::string locationName) {
        this->name = name;
        this->status = status;
        this->location = location;
        this->product = product;
        this->locationName = locationName;
    }

    std::string getStatus() {
        switch (location) {
        case 0:
            return "offline";
        case 1:
            return "not in chat";
        case 2:
            return "in chat";
        case 3:
            return "in public game " + locationName;
        case 4:
            return "in a private game";
        case 5:
            return "in private game " + locationName;
        default:
            return "unknown";
        }
    }

    std::string getName() { return name; }

    std::string getProduct() {
        switch (product) {
        case 0x1:
        case 0x2:
        case 0x6:
        case 0xb:
            return "Starcraft";
        case 0x3:
            return "Warcraft II";
        case 0x4:
        case 0x5:
            return "Diablo II";
        case 0x7:
        case 0x8:
        case 0xc:
            return "Warcraft III";
        case 0x9:
        case 0xa:
            return "Diablo";
        default:
            return "Unknown Game";
        }
    }

private:
    std::string name;
    uint8_t status;
    uint8_t location;
    uint32_t product;
    std::string locationName;
};

class Game {
public:
    Game(uint32_t settings, uint16_t port, uint32_t host, uint32_t status, uint32_t time, std::string name, std::string pw, std::string stats) {
        this->gameSettings = settings;
        this->host = CHost(host, port);
        this->gameStatus = status;
        this->elapsedTime = time;
        this->gameName = name;
        this->gamePassword = pw;
        this->gameStatstring = stats;
        splitStatstring();
    }

    CHost getHost() { return host; }

    bool isSavedGame() {
        return !gameStats[0].empty();
    };

    std::tuple<int, int> mapSize() {
        if (gameStats[1].empty()) {
            return {128, 128};
        }
        char w = gameStats[1].at(0);
        char h = gameStats[1].at(1);
        return {w * 32, h * 32};
    };

    int maxPlayers() {
        if (gameStats[2].empty()) {
            return 8;
        } else {
            return std::stoi(gameStats[2]) - 10;
        }
    };

    std::string getApproval() {
        return "Not approved";
    }

    std::string getGameSettings() {
        if (gameStats[9].empty()) {
            return "Map default";
        }
        long settings = std::stol(gameStats[9]);
        std::string result;
        if (settings & 0x200) {
            result += " 1 worker";
        }
        if (settings & 0x400) {
            result += " fixed placement";
        }
        switch (settings & 0x23000) {
        case 0x01000:
            result += " low resources";
            break;
        case 0x02000:
            result += " medium resources";
            break;
        case 0x03000:
            result += " high resources";
            break;
        case 0x20000:
            result += " random resources";
            break;
        }
        switch (settings & 0x1C000) {
        case 0x04000:
            result += " forest";
            break;
        case 0x08000:
            result += " winter";
            break;
        case 0x0c000:
            result += " wasteland";
            break;
        case 0x14000:
            result += " random";
            break;
        case 0x1c000:
            result += " orc swamp";
            break;
        }
        return result;
    };

    std::string getCreator() {
        int end = gameStats[10].find("\r");
        return gameStats[10].substr(0, end);
    };

    std::string getMap() {
        int begin = gameStats[10].find("\r") + 1;
        return gameStats[10].substr(begin);
    };

    std::string getGameStatus() {
        switch (gameStatus) {
        case 0x0:
            return "OK";
        case 0x1:
            return "No such game";
        case 0x2:
            return "Wrong password";
        case 0x3:
            return "Game full";
        case 0x4:
            return "Game already started";
        case 0x5:
            return "Spawned CD-Key not allowed";
        case 0x6:
            return "Too many server requests";
        }
        return "Unknown status";
    }

    std::string getGameType() {
        std::string sub("");
        switch (gameSettings & 0xff) {
        case 0x02:
            return "Melee";
        case 0x03:
            return "Free 4 All";
        case 0x04:
            return "One vs One";
        case 0x05:
            return "CTF";
        case 0x06:
            switch (gameSettings & 0xffff0000) {
            case 0x00010000:
                return "Greed 2500 resources";
            case 0x00020000:
                return "Greed 5000 resources";
            case 0x00030000:
                return "Greed 7500 resources";
            case 0x00040000:
                return "Greed 10000 resources";
            }
            return "Greed";
        case 0x07:
            switch (gameSettings & 0xffff0000) {
            case 0x00010000:
                return "Slaughter 15 minutes";
            case 0x00020000:
                return "Slaughter 30 minutes";
            case 0x00030000:
                return "Slaughter 45 minutes";
            case 0x00040000:
                return "Slaughter 60 minutes";
            }
            return "Slaughter";
        case 0x08:
            return "Sudden Death";
        case 0x09:
            switch (gameSettings & 0xffff0000) {
            case 0x00000000:
                return "Ladder (Disconnect is not loss)";
            case 0x00010000:
                return "Ladder (Loss on Disconnect)";
            }
            return "Ladder";
        case 0x0A:
            return "Use Map Settings";
        case 0x0B:
        case 0x0C:
        case 0x0D:
            switch (gameSettings & 0xffff0000) {
            case 0x00010000:
                sub += " (2 teams)";
                break;
            case 0x00020000:
                sub += " (3 teams)";
                break;
            case 0x00030000:
                sub += " (4 teams)";
                break;
            }
            switch (gameSettings & 0xff) {
            case 0x0B:
                return "Team Melee" + sub;
            case 0x0C:
                return "Team Free 4 All" + sub;
            case 0x0D:
                return "Team CTF" + sub;
            }
        case 0x0F:
            switch (gameSettings & 0xffff0000) {
            case 0x00010000:
                return "Top vs Bottom (1v7)";
            case 0x00020000:
                return "Top vs Bottom (2v6)";
            case 0x00030000:
                return "Top vs Bottom (3v5)";
            case 0x00040000:
                return "Top vs Bottom (4v4)";
            case 0x00050000:
                return "Top vs Bottom (5v3)";
            case 0x00060000:
                return "Top vs Bottom (6v2)";
            case 0x00070000:
                return "Top vs Bottom (7v1)";
            }
            return "Top vs Bottom";
        case 0x10:
            return "Iron Man Ladder";
        default:
            return "Unknown";
        }
    }

private:
    void splitStatstring() {
        // statstring is a comma-delimited list of values
        int pos = 0;
        while (true) {
            int newpos = gameStatstring.find(",", pos);
            gameStats.push_back(gameStatstring.substr(pos, newpos));
            pos = newpos + 1;
            if (pos == 0) {
                break;
            }
        }
        while (gameStats.size() < 10) {
            gameStats.push_back("");
        }
    }

    uint32_t gameSettings;
    uint32_t languageId;
    CHost host;
    uint32_t gameStatus;
    uint32_t elapsedTime;
    std::string gameName;
    std::string gamePassword;
    std::string gameStatstring;
    std::vector<std::string> gameStats;
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
    }

    ~Context() {
        if (state != NULL) {
            delete state;
        }
        delete udpSocket;
        delete tcpSocket;
        delete host;
    }

    // User and UI actions
    void sendText(std::string txt) {
        // C>S 0x0E SID_CHATCOMMAND
        int pos = 0;
        for (int pos = 0; pos < txt.size(); pos += 220) {
            std::string text = txt.substr(pos, pos + 220);
            if (pos + 220 < txt.size()) {
                text += "...";
            }
            BNCSOutputStream msg(0x0e);
            msg.serialize(text.c_str());
            msg.flush(getTCPSocket());
        }
    }

    void requestExtraUserInfo(std::string username) {
        BNCSOutputStream msg(0x26);
        msg.serialize32(1); // num accounts
        msg.serialize32(5); // num keys
        msg.serialize32((uint32_t) extendedInfoIdx.size());
        msg.serialize(username.c_str());
        msg.serialize("record\\GAME\\0\\wins");
        msg.serialize("record\\GAME\\0\\losses");
        msg.serialize("record\\GAME\\0\\disconnects");
        msg.serialize("record\\GAME\\0\\last game");
        msg.serialize("record\\GAME\\0\\last game result");
        msg.flush(getTCPSocket());
    }

    void refreshGames() {
        // C>S 0x09 SID_GETADVLISTEX
        BNCSOutputStream getadvlistex(0x09);
        getadvlistex.serialize16((uint16_t) 0x00); // all games
        getadvlistex.serialize16((uint16_t) 0x01); // no sub game type
        getadvlistex.serialize32(0xff80); // show all games
        getadvlistex.serialize32(0x00); // reserved field
        getadvlistex.serialize32(0xff); // return all games
        getadvlistex.serialize(""); // no game name
        getadvlistex.serialize(""); // no game pw
        getadvlistex.serialize(""); // no game statstring
        getadvlistex.flush(getTCPSocket());
    }

    void refreshFriends() {
        // C>S 0x65 SID_FRIENDSLIST
        BNCSOutputStream msg(0x65);
        msg.flush(getTCPSocket());
    }

    void joinGame(std::string name, std::string pw) {
        // C>S 0x22 SID_NOTIFYJOIN
        BNCSOutputStream msg(0x09);
        msg.serialize32("W2BN");
        msg.serialize32(0x4f);
        msg.serialize(name.c_str());
        msg.serialize(pw.c_str());
        msg.flush(getTCPSocket());
    }

    // UI information
    void setCurrentChannel(std::string name) {
        this->currentChannel = name;
    }

    std::string getCurrentChannel() { return currentChannel; }

    void setGamelist(std::vector<Game*> games) {
        for (const auto value : this->games) {
            delete value;
        }
        this->games = games;
    }

    std::vector<Game*> getGames() { return games; }

    void setFriendslist(std::vector<Friend*> friends) {
        for (const auto value : this->friends) {
            delete value;
        }
        this->friends = friends;
    }

    std::vector<Friend*> getFriends() { return friends; }

    void reportUserdata(uint32_t id, std::vector<std::string> values) {
        this->extendedInfoValues[id] = values;
    }

    std::queue<std::string> getInfo() { return info; }

    void showInfo(std::string arg) { info.push("*** " + arg + " ***"); }

    void showError(std::string arg) { info.push("!!! " + arg + " !!!"); }

    void showChat(std::string arg) { info.push(arg); }

    void addUser(std::string name) {
        userList.insert(name);
    }

    void removeUser(std::string name) {
        userList.erase(name);
    }

    std::set<std::string> getUsers() { return userList; }

    void setChannels(std::vector<std::string> channels) {
        this->channelList = channels;
    }

    std::vector<std::string> getChannels() { return channelList; }

    // State
    std::string getUsername() { return username; }

    void setUsername(std::string arg) { username = arg; }

    uint32_t* getPassword2() {
        // we assume that any valid password has at least 1 non-null word hash
        for (int i = 0; i < 5; i++) {
            if (password[i] != 0) {
                return password2;
            }
        }
        return NULL;
    }

    uint32_t* getPassword1() {
        // we assume that any valid password has at least 1 non-null word hash
        for (int i = 0; i < 5; i++) {
            if (password[i] != 0) {
                return password;
            }
        }
        return NULL;
    }

    void setPassword(std::string pw) {
        xsha1_calcHashBuf(pw.c_str(), pw.length(), password);
        xsha1_calcHashDat(password, password2);
    }

    // Protocol
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

    std::string username;
    uint32_t password[5]; // xsha1 hash of password
    uint32_t password2[5]; // xsha1 hash of password hash

    std::string currentChannel;
    std::set<std::string> userList;
    std::vector<std::string> channelList;
    std::queue<std::string> info;
    std::vector<Game*> games;
    std::vector<Friend*> friends;
    std::map<std::string, uint32_t> extendedInfoIdx;
    std::map<uint32_t, std::vector<std::string>> extendedInfoValues;
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

class C2S_GAMERESULT_OR_STOPADV : public NetworkState {
    virtual void doOneStep(Context *ctx) {
        // TODO - wait until the game lobby is left or the game is over and then send the result
        // C>S 0x02 SID_STOPADV
        // C>S 0x2C SID_GAMERESULT
    }
};

class S2C_CHATEVENT : public NetworkState {
public:
    S2C_CHATEVENT() {
        this->ticks = 0;
    }

    virtual void doOneStep(Context *ctx) {
        ticks++;
        if ((ticks % 5000) == 0) {
            // C>S 0x07 PKT_KEEPALIVE
            // ~5000 frames @ ~50fps == 100 seconds
            BNCSOutputStream keepalive(0x07);
            keepalive.serialize32(ticks);
            keepalive.flush(ctx->getUDPSocket(), ctx->getHost());
        }

        if (ctx->getTCPSocket()->HasDataToRead(0)) {
            uint8_t msg = ctx->getMsgIStream()->readMessageId();
            if (msg == 0xff) {
                // try again next time
                return;
            }

            switch (msg) {
            case 0x00: // SID_NULL
                break;
            case 0x0f: // CHATEVENT
                handleChatevent(ctx);
                break;
            case 0x09:
                // S>C 0x09 SID_GETADVLISTEX
                handleGamelist(ctx);
                break;
            case 0x1c:
                // S>C 0x1C SID_STARTADVEX3
                if (ctx->getMsgIStream()->read32()) {
                    ctx->showError("Game creation failed");
                } else {
                    ctx->setState(new C2S_GAMERESULT_OR_STOPADV());
                }
                break;
            case 0x65:
                // S>C 0x65 SID_FRIENDSLIST
                handleFriendlist(ctx);
                break;
            case 0x26:
                // S>C 0x26 SID_READUSERDATA
                handleUserdata(ctx);
                break;
            default:
                // TODO:
                // S>C 0x68 SID_FRIENDSREMOVE
                // S>C 0x67 SID_FRIENDSADD
                std::cout << "Unhandled message ID: " << std::hex << msg << std::endl;
            }
        }
    }

private:
    void handleGamelist(Context *ctx) {
        uint32_t cnt = ctx->getMsgIStream()->read32();
        std::vector<Game*> games;
        while (cnt--) {
            uint32_t settings = ctx->getMsgIStream()->read32();
            uint32_t lang = ctx->getMsgIStream()->read32();
            uint16_t addr_fam = ctx->getMsgIStream()->read16();
            uint16_t port = ctx->getMsgIStream()->read16();
            uint32_t ip = ctx->getMsgIStream()->read32();
            uint32_t sinzero1 = ctx->getMsgIStream()->read32();
            uint32_t sinzero2 = ctx->getMsgIStream()->read32();
            uint32_t status = ctx->getMsgIStream()->read32();
            uint32_t time = ctx->getMsgIStream()->read32();
            std::string name = ctx->getMsgIStream()->readString();
            std::string pw = ctx->getMsgIStream()->readString();
            std::string stat = ctx->getMsgIStream()->readString();
            games.push_back(new Game(settings, port, ip, status, time, name, pw, stat));
        }
        ctx->setGamelist(games);
    }


    void handleFriendlist(Context *ctx) {
        uint32_t cnt = ctx->getMsgIStream()->read32();
        std::vector<Friend*> friends;
        while (cnt--) {
            std::string user = ctx->getMsgIStream()->readString();
            uint8_t status = ctx->getMsgIStream()->read8();
            uint8_t location = ctx->getMsgIStream()->read8();
            uint32_t product = ctx->getMsgIStream()->read32();
            std::string locname = ctx->getMsgIStream()->readString();
            friends.push_back(new Friend(user, status, location, product, locname));
        }
        ctx->setFriendslist(friends);
    }

    void handleUserdata(Context *ctx) {
        uint32_t cnt = ctx->getMsgIStream()->read32();
        assert(cnt == 1);
        uint32_t keys = ctx->getMsgIStream()->read32();
        uint32_t reqId = ctx->getMsgIStream()->read32();
        std::vector<std::string> values = ctx->getMsgIStream()->readStringlist();
        ctx->reportUserdata(reqId, values);
    }

    void handleChatevent(Context *ctx) {
        uint32_t eventId = ctx->getMsgIStream()->read32();
        uint32_t userFlags = ctx->getMsgIStream()->read32();
        uint32_t ping = ctx->getMsgIStream()->read32();
        uint32_t ip = ctx->getMsgIStream()->read32();
        uint32_t acn = ctx->getMsgIStream()->read32();
        uint32_t reg = ctx->getMsgIStream()->read32();
        std::string username = ctx->getMsgIStream()->readString();
        std::string text = ctx->getMsgIStream()->readString();
        switch (eventId) {
        case 0x01: // sent for user that is already in channel
            ctx->addUser(username);
            break;
        case 0x02: // user joined channel
            ctx->addUser(username);
            ctx->showInfo(username + " joined");
            break;
        case 0x03: // user left channel
            ctx->removeUser(username);
            ctx->showInfo(username + " left");
        case 0x04: // recv whisper
            ctx->showChat(username + " whispers: " + text);
            break;
        case 0x05: // recv chat
            ctx->showChat(username + ": " + text);
            break;
        case 0x06: // recv broadcast
            ctx->showChat("[BROADCAST]: " + text);
            break;
        case 0x07: // channel info
            ctx->setCurrentChannel(username);
            ctx->showInfo("Joined channel " + username);
            break;
        case 0x09: // user flags update
            break;
        case 0x0a: // sent whisper
            break;
        case 0x0d: // channel full
            ctx->showInfo("Channel full");
            break;
        case 0x0e: // channel does not exist
            ctx->showInfo("Channel does not exist");
            break;
        case 0x0f: // channel is restricted
            ctx->showInfo("Channel restricted");
            break;
        case 0x12: // general info text
            ctx->showInfo("[INFO]: " + text);
            break;
        case 0x13: // error message
            ctx->showError("[ERROR]: " + text);
            break;
        case 0x17: // emote
            break;
        }
    }

    uint32_t ticks;
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
            ctx->setChannels(channels);

            // request our user info and refresh the active games list
            ctx->requestExtraUserInfo(ctx->getUsername());
            ctx->refreshGames();

            ctx->setState(new S2C_CHATEVENT());
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
                ctx->showInfo("Statstring after logon: " + statString);
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
    S2C_PKT_SERVERPING() {
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

class S2C_CREATEACCOUNT2 : public NetworkState {
    virtual void doOneStep(Context *ctx) {
        if (ctx->getTCPSocket()->HasDataToRead(0)) {
            uint8_t msg = ctx->getMsgIStream()->readMessageId();
            if (msg == 0xff) {
                // try again next time
                return;
            }
            if (msg != 0x3d) {
                std::string error = std::string("Expected SID_CREATEACCOUNT2, got msg id ");
                error += std::to_string(msg);
                ctx->setState(new DisconnectedState(error));
            }

            uint32_t status = ctx->getMsgIStream()->read32();
            std::string nameSugg = ctx->getMsgIStream()->readString();

            if (!nameSugg.empty()) {
                nameSugg = " (try username: " + nameSugg + ")";
            }

            switch (status) {
            case 0x00:
                // login into created account
                ctx->setState(new C2S_LOGONRESPONSE2());
                return;
            case 0x01:
                ctx->showError("Name too short" + nameSugg);
                ctx->setUsername("");
                ctx->setState(new C2S_LOGONRESPONSE2());
                return;
            case 0x02:
                ctx->showError("Name contains invalid character(s)" + nameSugg);
                ctx->setUsername("");
                ctx->setState(new C2S_LOGONRESPONSE2());
                return;
            case 0x03:
                ctx->showError("Name contains banned word(s)" + nameSugg);
                ctx->setUsername("");
                ctx->setState(new C2S_LOGONRESPONSE2());
                return;
            case 0x04:
                ctx->showError("Account already exists" + nameSugg);
                ctx->setUsername("");
                ctx->setState(new C2S_LOGONRESPONSE2());
                return;
            case 0x05:
                ctx->showError("Account is still being created" + nameSugg);
                ctx->setUsername("");
                ctx->setState(new C2S_LOGONRESPONSE2());
                return;
            case 0x06:
                ctx->showError("Name does not contain enough alphanumeric characters" + nameSugg);
                ctx->setUsername("");
                ctx->setState(new C2S_LOGONRESPONSE2());
                return;
            case 0x07:
                ctx->showError("Name contained adjacent punctuation characters" + nameSugg);
                ctx->setUsername("");
                ctx->setState(new C2S_LOGONRESPONSE2());
                return;
            case 0x08:
                ctx->showError("Name contained too many punctuation characters" + nameSugg);
                ctx->setUsername("");
                ctx->setState(new C2S_LOGONRESPONSE2());
                return;
            default:
                ctx->showError("Unknown error creating account" + nameSugg);
                ctx->setUsername("");
                ctx->setState(new C2S_LOGONRESPONSE2());
                return;
            }

        }
    }
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
                // success. we need to send SID_UDPPINGRESPONSE before entering chat
                ctx->setState(new S2C_PKT_SERVERPING());
                return;
            case 0x01:
                ctx->showInfo("Account does not exist, creating it...");
                createAccount(ctx);
                return;
            case 0x02:
                ctx->showInfo("Incorrect password");
                ctx->setPassword("");
                ctx->setState(new C2S_LOGONRESPONSE2());
                return;
            case 0x06:
                ctx->showInfo("Account closed: " + ctx->getMsgIStream()->readString());
                ctx->setPassword("");
                ctx->setState(new C2S_LOGONRESPONSE2());
                return;
            default:
                ctx->setState(new DisconnectedState("unknown logon response"));
            }
        }
    }

private:
    void createAccount(Context *ctx) {
        BNCSOutputStream msg(0x3d);
        uint32_t *pw = ctx->getPassword1();
        for (int i = 0; i < 20; i++) {
            msg.serialize8(reinterpret_cast<uint8_t*>(pw)[i]);
        }
        msg.serialize(ctx->getUsername().c_str());
        msg.flush(ctx->getTCPSocket());
        ctx->setState(new S2C_CREATEACCOUNT2());
    }
};

void C2S_LOGONRESPONSE2::doOneStep(Context *ctx) {
    std::string user = ctx->getUsername();
    uint32_t *pw = ctx->getPassword1(); // single-hashed for SID_LOGONRESPONSE2
    if (!user.empty() && pw) {
        BNCSOutputStream logon(0x3a);
        logon.serialize32(ctx->clientToken);
        logon.serialize32(ctx->serverToken);

        // Battle.net password hashes are hashed twice using XSHA-1. First, the
        // password is hashed by itself, then the following data is hashed again
        // and sent to Battle.net:
        // (UINT32) Client Token
        // (UINT32) Server Token
        // (UINT32)[5] First password hash
        uint32_t data[7];
        data[0] = ctx->clientToken;
        data[1] = ctx->serverToken;
        data[2] = pw[0];
        data[3] = pw[1];
        data[4] = pw[2];
        data[5] = pw[3];
        data[6] = pw[4];
        uint32_t sendHash[5];
        xsha1_calcHashDat(data, sendHash);
        for (int i = 0; i < 20; i++) {
            logon.serialize8(reinterpret_cast<uint8_t*>(sendHash)[i]);
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
            std::string exeInfo("");
            if (!FullGameName.empty()) {
                exeInfo += FullGameName;
            } else {
                if (!GameName.empty()) {
                    exeInfo += GameName;
                } else {
                    exeInfo += Parameters::Instance.applicationName;
                }
            }
            exeInfo += " ";
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

            // immediately respond with C2S_SID_PING
            BNCSOutputStream buffer(0x25);
            buffer.serialize32(pingValue);
            send(ctx, &buffer);

            ctx->setState(new S2C_SID_AUTH_INFO());
        }
    }
};

class ConnectState : public NetworkState {
    virtual void doOneStep(Context *ctx) {
        // Connect
        
        if (!ctx->getTCPSocket()->Open(CHost("0.0.0.0", 6113))) { // TODO...
            ctx->setState(new DisconnectedState("TCP open failed"));
            return;
        }
        if (!ctx->getTCPSocket()->IsValid()) {
            ctx->setState(new DisconnectedState("TCP not valid"));
            return;
        }
        if (!ctx->getTCPSocket()->Connect(*ctx->getHost())) {
            ctx->setState(new DisconnectedState("TCP connect failed"));
            return;
        }
        if (!ctx->getUDPSocket()->Open(*ctx->getHost())) {
            std::cerr << "UDP open failed" << std::endl;
            // ctx->setState(new DisconnectedState("UDP open failed"));
            // return;
        }

        // Send proto byte
        ctx->getTCPSocket()->Send("\x1", 1);

        // C>S SID_AUTH_INFO
        BNCSOutputStream buffer(0x50);
        // (UINT32) Protocol ID
        buffer.serialize32(0x00);
        // (UINT32) Platform code
        buffer.serialize32("IX86");
        // (UINT32) Product code - we'll just use W2BN and encode what we are in
        // the version
        buffer.serialize32("W2BN");
        // (UINT32) Version byte - just use the last from W2BN
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
        uint32_t bias = 0;
        std::time_t systemtime = std::time(NULL);
        struct std::tm *utc = std::gmtime(&systemtime);
        if (utc != NULL) {
            std::time_t utctime_since_epoch = std::mktime(utc);
            if (utctime_since_epoch != -1) {
                struct std::tm *localtime = std::localtime(&systemtime);
                if (localtime != 0) {
                    std::time_t localtime_since_epoch = std::mktime(localtime);
                    if (localtime_since_epoch != -1) {
                        bias = (localtime_since_epoch - utctime_since_epoch) / 60;
                    }
                }
            }
        }
        buffer.serialize32(bias);
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

extern gcn::Gui *Gui;
static gcn::Container *onlineServiceContainer;
static gcn::ScrollArea *messageArea;
static gcn::ScrollArea *usersArea;
static gcn::ScrollArea *friendsArea;
static gcn::ScrollArea *channelsArea;
static gcn::ScrollArea *gamelistArea;
static gcn::TextField *chatInput;
static gcn::Window *loginWindow;
static gcn::Container *loginWindowContainer;
static gcn::TextField *username;
static gcn::TextField *password;

static Context *ctx;

class ChatInputListener : public gcn::ActionListener {
    virtual void action(const std::string &) {
        ctx->sendText(chatInput->getText());
        chatInput->setText("");
    }
};

class UsernameInputListener : public gcn::ActionListener {
    virtual void action(const std::string &) {
        ctx->setUsername(username->getText());
    }
};

class PasswordInputListener : public gcn::ActionListener {
    virtual void action(const std::string &) {
        ctx->setPassword(password->getText());
    }
};

void GoOnline() {
    std::string nc, rc;
    GetDefaultTextColors(nc, rc);

    const EventCallback *old_callbacks = GetCallbacks();

    gcn::Widget *oldTop = Gui->getTop();
    Gui->setUseDirtyDrawing(false);

    ctx = new Context();
    ctx->setState(new ConnectState());

    onlineServiceContainer = new gcn::Container();
    onlineServiceContainer->setDimension(gcn::Rectangle(0, 0, Video.Width, Video.Height));
    onlineServiceContainer->setOpaque(false);
    Gui->setTop(onlineServiceContainer);

    static int chatInputHeight = 24;
    messageArea = new gcn::ScrollArea(new gcn::TextBox());
    messageArea->setBackgroundColor(gcn::Color(200, 200, 120));
    messageArea->setSize(Video.Width * 0.7, Video.Height * 0.7 - chatInputHeight);
    static_cast<gcn::TextBox*>(messageArea->getContent())->setEditable(false);
    onlineServiceContainer->add(messageArea, Video.Width * 0.3, Video.Height * 0.3);

    chatInput = new gcn::TextField();
    chatInput->setSize(Video.Width * 0.7, chatInputHeight);
    onlineServiceContainer->add(chatInput, Video.Width * 0.3, Video.Height * 0.7 - chatInputHeight);
    chatInput->addActionListener(new ChatInputListener());

    usersArea = new gcn::ScrollArea(new gcn::TextBox());
    usersArea->setBackgroundColor(gcn::Color(120, 200, 200));
    usersArea->setSize(Video.Width * 0.1, Video.Height * 0.7);
    static_cast<gcn::TextBox*>(usersArea->getContent())->setEditable(false);
    onlineServiceContainer->add(usersArea, Video.Width * 0.2, Video.Height * 0.3);

    friendsArea = new gcn::ScrollArea(new gcn::TextBox());
    friendsArea->setBackgroundColor(gcn::Color(200, 120, 200));
    friendsArea->setSize(Video.Width * 0.1, Video.Height * 0.5);
    static_cast<gcn::TextBox*>(friendsArea->getContent())->setEditable(false);
    onlineServiceContainer->add(friendsArea, 0, 0);

    channelsArea = new gcn::ScrollArea(new gcn::TextBox());
    channelsArea->setBackgroundColor(gcn::Color(255, 255, 255));
    channelsArea->setSize(Video.Width * 0.1, Video.Height * 0.5);
    static_cast<gcn::TextBox*>(channelsArea->getContent())->setEditable(false);
    onlineServiceContainer->add(channelsArea, 0, Video.Height * 0.5);

    gamelistArea = new gcn::ScrollArea(new gcn::TextBox());
    gamelistArea->setBackgroundColor(gcn::Color(120, 120, 120));
    gamelistArea->setSize(Video.Width * 0.8, Video.Height * 0.3);
    static_cast<gcn::TextBox*>(gamelistArea->getContent())->setEditable(false);
    onlineServiceContainer->add(gamelistArea, Video.Width * 0.2, 0);

    loginWindow = new gcn::Window();
    loginWindow->setBaseColor(gcn::Color(120, 120, 120, 120));
    loginWindow->setCaption("Username / Password");
    loginWindow->setWidth(Video.Width / 2);
    loginWindow->setHeight(chatInputHeight * 8);

    loginWindowContainer = new gcn::Container();
    loginWindowContainer->setWidth(loginWindow->getWidth());
    loginWindowContainer->setHeight(loginWindow->getHeight());
    loginWindowContainer->setOpaque(false);
    loginWindow->setContent(loginWindowContainer);

    username = new gcn::TextField();
    username->setSize(Video.Width * 0.3, chatInputHeight);
    username->addActionListener(new UsernameInputListener());
    loginWindowContainer->add(username, Video.Width * 0.1, chatInputHeight);

    password = new gcn::TextField();
    password->setSize(Video.Width * 0.3, chatInputHeight);
    password->addActionListener(new PasswordInputListener());
    loginWindowContainer->add(password, Video.Width * 0.1, chatInputHeight * 4);

    onlineServiceContainer->add(loginWindow, Video.Width / 4, Video.Height / 2 - chatInputHeight * 4);

    SetVideoSync();
    GameCursor = UI.Point.Cursor;
    InterfaceState = IfaceStateNormal;
    UI.SelectedViewport = UI.Viewports;
    while (1) {
        ctx->doOneStep();

        if (!ctx->getCurrentChannel().empty()) {
            loginWindow->setVisible(false);

            if ((FrameCounter % (FRAMES_PER_SECOND * 5)) == 0) {
                ctx->refreshGames();
                ctx->refreshFriends();
            }

            if ((FrameCounter % (FRAMES_PER_SECOND * 1)) == 0) {
                static_cast<gcn::TextBox*>(gamelistArea->getContent())->setText("");
                for (auto g : ctx->getGames()) {
                    static_cast<gcn::TextBox*>(gamelistArea->getContent())->addRow(g->getMap() + " " +
                                                                                   g->getCreator() + " " +
                                                                                   g->getGameType() + " " +
                                                                                   g->getGameSettings() + " " +
                                                                                   std::to_string(g->maxPlayers()));
                }

                static_cast<gcn::TextBox*>(usersArea->getContent())->setText("");
                for (auto u : ctx->getUsers()) {
                    static_cast<gcn::TextBox*>(usersArea->getContent())->addRow(u);
                }

                static_cast<gcn::TextBox*>(channelsArea->getContent())->setText("");
                for (auto u : ctx->getChannels()) {
                    static_cast<gcn::TextBox*>(channelsArea->getContent())->addRow(u);
                }

                static_cast<gcn::TextBox*>(friendsArea->getContent())->setText("");
                for (auto u : ctx->getFriends()) {
                    static_cast<gcn::TextBox*>(friendsArea->getContent())->addRow(u->getName() + ", " + u->getStatus() + ", " + u->getProduct());
                }
            }
        }

        if (!ctx->getInfo().empty()) {
            std::string info = ctx->getInfo().front();
            ctx->getInfo().pop();
            while (!info.empty()) {
                static_cast<gcn::TextBox*>(messageArea->getContent())->addRow(info);
            }
        }

        Gui->draw();
        DrawCursor();
        Invalidate();
        RealizeVideoMemory();

        WaitEventsOneFrame();
    }

    CleanModules();
    LoadCcl(Parameters::Instance.luaStartFilename); // Reload the main config file
    // PreMenuSetup();

    InterfaceState = IfaceStateMenu;
    GameCursor = UI.Point.Cursor;

    SetCallbacks(old_callbacks);
    Gui->setTop(oldTop);
    SetDefaultTextColors(nc, rc);

    Video.ClearScreen();
    Invalidate();

    delete onlineServiceContainer;
    delete messageArea->getContent();
    delete messageArea;
    delete usersArea->getContent();
    delete usersArea;
    delete friendsArea->getContent();
    delete friendsArea;
    delete channelsArea->getContent();
    delete channelsArea;
    delete gamelistArea->getContent();
    delete gamelistArea;
    delete chatInput;
    delete loginWindow;
    delete loginWindowContainer;
    delete username;
    delete password;
}
