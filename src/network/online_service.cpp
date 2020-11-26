#include "online_service.h"
#include "results.h"

#ifdef USE_WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif
#include <clocale>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <ios>
#include <iostream>
#include <map>
#include <ostream>
#include <queue>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#ifndef USE_WIN32
#include <unistd.h>
#endif
#include <vector>

#ifdef USE_WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif

#include "stratagus.h"
#include "script.h"
#include "map.h"
#include "netconnect.h"
#include "script.h"
#include "settings.h"
#include "tileset.h"
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

static void dump(uint8_t* buffer, int received_bytes) {
    std::cout << "Raw contents >>>" << std::endl;
    for (int i = 0; i < received_bytes; i += 8) {
        std::cout << std::hex;
        int j = i;
        for (; j < received_bytes && j < (i + 8); j++) {
            uint8_t byte = buffer[j];
            if (byte < 0x10) {
                std::cout << "0";
            }
            std::cout << (unsigned short)byte << " ";
        }
        for (; j < (i + 9); j++) {
            std::cout << "   "; // room for 2 hex digits and one space
        }
        j = i;
        for (; j < received_bytes && j < (i + 8); j++) {
            char c = buffer[j];
            if (c >= 32) {
                std::cout.write(&c, 1);
            } else {
                std::cout.write(".", 1);
            }
        }
        std::cout << std::endl;
    }
    std::cout << "<<<" << std::endl;
}

class BNCSInputStream {
public:
    BNCSInputStream(CTCPSocket *socket) {
        this->sock = socket;
        this->bufsize = 1024;
        this->buffer = (char*)calloc(sizeof(char), bufsize);
        this->received_bytes = 0;
        this->pos = 0;
    };
    ~BNCSInputStream() {};

    std::string readString() {
        if (received_bytes == 0) {
            return NULL;
        }
        std::stringstream strstr;
        int i = pos;
        char c;
        while ((c = buffer[i]) != '\0' && i < received_bytes) {
            strstr.put(c);
            i += 1;
        }
        consumeData(i + 1 - pos);
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
        uint32_t byte = ntohl(reinterpret_cast<uint32_t *>(buffer + pos)[0]);
        consumeData(4);
        return ntohl(byte);
    }

    uint64_t read64() {
        uint64_t byte = reinterpret_cast<uint64_t *>(buffer + pos)[0];
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

    /**
     * For debugging and development: read the entire thing as a char
     * array. Caller must "free" the out-char
     */
    int readAll(char** out) {
        *out = (char*)calloc(received_bytes, sizeof(char));
        strncpy(*out, buffer, received_bytes);
        return received_bytes;
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
        received_bytes += this->sock->Recv(buffer + received_bytes, 4 - received_bytes);
        if (received_bytes != 4) {
            return -1;
        }
        assert(received_bytes == 4);
        assert(read8() == 0xff);
        uint8_t msgId = read8();
        uint16_t len = read16();
        // we still need to have len in total for this message, so if we have
        // more available than len minus the current position and minus the
        // first 4 bytes that we already consumed, we'll have enough
        assert(pos == 4);
        long needed = len - received_bytes;
        if (needed != 0) {
            assert(needed > 0);
            if (len >= bufsize) {
                buffer = (char*)realloc(buffer, sizeof(char) * len + 1);
                bufsize = len + 1;
            }
            received_bytes += this->sock->Recv(buffer + received_bytes, needed);
            if (received_bytes < len) {
                // Didn't receive full message on the socket, yet. Reset position so
                // this method can be used to try again
                pos = 0;
                return -1;
            }
        }
        return msgId;
    };

    void finishMessage() {
        received_bytes = 0;
        pos = 0;
    }

private:
    void consumeData(int bytes) {
        pos += bytes;
    }

    CTCPSocket *sock;
    char *buffer;
    int received_bytes;
    int pos;
    int bufsize;
};

class BNCSOutputStream {
public:
    BNCSOutputStream(uint8_t id, bool udp = false) {
        // Every BNCS message has the same header:
        //  (UINT8) Always 0xFF
        //  (UINT8) Message ID
        // (UINT16) Message length, including this header
        //   (VOID) Message data
        // The UDP messages are instead 4 bytes for the id, then the content
        this->sz = 16;
        this->buf = (uint8_t*) calloc(sizeof(uint8_t), this->sz);
        this->pos = 0;
        if (udp) {
            serialize8(id);
            serialize8(0);
            this->length_pos = -1;
        } else {
            serialize8(0xff);
            serialize8(id);
            this->length_pos = pos;
        }
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
    void serialize32NativeByteOrder(uint32_t data) {
        ensureSpace(sizeof(data));
        uint32_t *view = reinterpret_cast<uint32_t *>(buf + pos);
        *view = data;
        pos += sizeof(data);
    };
    void serialize16(uint16_t data) {
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
    void serializeC32(const char* str) {
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
        if (sock->IsValid()) {
            sock->Send(*host, getBuffer(), pos);
        }
    };

private:
    uint8_t *getBuffer() {
        // if needed, insert length to make it a valid buffer
        if (length_pos >= 0) {
            uint16_t *view = reinterpret_cast<uint16_t *>(buf + length_pos);
            *view = pos;
        }
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
    unsigned int sz;
    unsigned int pos;
    int length_pos;
};

static std::string gameName() {
    if (!FullGameName.empty()) {
        return FullGameName;
    } else {
        if (!GameName.empty()) {
            return GameName;
        } else {
            return Parameters::Instance.applicationName;
        }
    }
}

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

    int getSpeed() {
        return std::stol(gameStats[3]);
    }

    std::string getApproval() {
        if (std::stol(gameStats[4]) == 0) {
            return "Not approved";
        } else {
            return "Approved";
        }
    }

    std::string getGameSettings() {
        if (gameStats[8].empty()) {
            return "Map default";
        }
        long settings = std::stol(gameStats[8]);
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
        return gameStats[9];
    };

    std::string getMap() {
        return gameStats[10];
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
            return gameStats[5]; // just the code
        }
    }

private:
    void splitStatstring() {
        // statstring is a comma-delimited list of values
        int pos = 0;
        int newpos = 0;
        char sep[2] = {',', '\0'};
        while (true) {
            newpos = gameStatstring.find(sep, pos);
            if (newpos < pos && sep[0] == ',') {
                sep[0] = '\r';
                continue;
            } else if (newpos < pos) {
                break;
            } else {
                gameStats.push_back(gameStatstring.substr(pos, newpos - pos));
            }
            pos = newpos + 1;
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

class Context : public OnlineContext {
public:
    Context() {
        this->tcpSocket = new CTCPSocket();
        this->istream = new BNCSInputStream(tcpSocket);
        this->state = NULL;
        this->host = new CHost("127.0.0.1", 6112);
        this->clientToken = MyRand();
        this->username = "";
        setPassword("");
        defaultUserKeys.push_back("profile\\location");
        defaultUserKeys.push_back("profile\\description");
        defaultUserKeys.push_back("record\\GAME\\0\\wins");
        defaultUserKeys.push_back("record\\GAME\\0\\losses");
        defaultUserKeys.push_back("record\\GAME\\0\\disconnects");
        defaultUserKeys.push_back("record\\GAME\\0\\last game");
        defaultUserKeys.push_back("record\\GAME\\0\\last game result");

    }

    ~Context() {
        if (state != NULL) {
            delete state;
        }
        delete tcpSocket;
        delete host;
    }

    bool isConnected() {
        return !getCurrentChannel().empty();
    }

    bool isConnecting() {
        return !isDisconnected() && !isConnected() && !username.empty() && hasPassword;
    }

    bool isDisconnected() {
        return state == NULL;
    }

    // User and UI actions
    void disconnect() {
        if (isConnected()) {
            // SID_STOPADV: according to bnetdocs.org, this is always sent when
            // clients disconnect, regardless of state
            BNCSOutputStream stop(0x02);
            stop.flush(tcpSocket);
            // SID_LEAVECHAT
            BNCSOutputStream leave(0x10);
            leave.flush(tcpSocket);
        }
        if (canDoUdp == 1) {
            ExitNetwork1();
        }
        tcpSocket->Close();
        state = NULL;
        clientToken = MyRand();
        username = "";
        setPassword("");
    }

    void sendText(std::string txt, bool silent = false) {
        // C>S 0x0E SID_CHATCOMMAND
        int pos = 0;
        for (unsigned int pos = 0; pos < txt.size(); pos += 220) {
            std::string text = txt.substr(pos, pos + 220);
            if (pos + 220 < txt.size()) {
                text += "...";
            }
            BNCSOutputStream msg(0x0e);
            msg.serialize(text.c_str());
            msg.flush(getTCPSocket());
        }
        if (!silent) {
            showChat(username + ": " + txt);
        }
    }

    void requestExternalAddress() {
        // uses the /netinfo command to get which ip:port the server sees from us
        sendText("/netinfo", true);
        requestedAddress = true;
    }

    void requestExtraUserInfo(std::string username) {
        BNCSOutputStream msg(0x26);
        msg.serialize32NativeByteOrder(1); // num accounts
        msg.serialize32NativeByteOrder(defaultUserKeys.size()); // num keys
        msg.serialize32NativeByteOrder((uint32_t) extendedInfoNames.size());
        extendedInfoNames.push_back(username);
        msg.serialize(username.c_str());
        for (const auto key : defaultUserKeys) {
            msg.serialize(key.c_str());
        }
        msg.flush(getTCPSocket());
    }

    void punchNAT(std::string username) {
        if (externalAddress.isValid()) {
            sendText("/whisper " + username + " /udppunch " + externalAddress.toString());
        }
    }

    void refreshChannels() {
        BNCSOutputStream getlist(0x0b);
        // identify as W2BN
        getlist.serialize32(0x4f);
        getlist.flush(getTCPSocket());
    }

    void refreshGames() {
        // C>S 0x09 SID_GETADVLISTEX
        BNCSOutputStream getadvlistex(0x09);
        getadvlistex.serialize16(0x00); // all games
        getadvlistex.serialize16(0x01); // no sub game type
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

    virtual void joinGame(std::string username, std::string pw) {
        if (!isConnected()) {
            return;
        }
        // C>S 0x22 SID_NOTIFYJOIN
        BNCSOutputStream msg(0x09);
        msg.serializeC32("W2BN");
        msg.serialize32(0x4f);
        msg.serialize(gameNameFromUsername(username).c_str());
        msg.serialize(pw.c_str());
        msg.flush(getTCPSocket());
    }

    virtual void leaveGame() {
        // TODO: ?
    }

    virtual void startAdvertising(bool isStarted = false) {
        if (!isConnected()) {
            return;
        }
        BNCSOutputStream msg(0x1c);
        int maxSlots = 0;
        for (int i = 0; i < PlayerMax; i++) {
            if (ServerSetupState.CompOpt[i] == 0) { // available
                maxSlots++;
            }
        }
        int joinedPlayers = 0;
        for (int i = 1; i < PlayerMax; i++) { // skip server host
            if (Hosts[i].PlyNr) {
                joinedPlayers++;
            }
        }
        uint32_t state = 0x10000000; // disconnect always counts as loss
        if (joinedPlayers) {
            state |= 0x04000000; // has players other than creator
        }
        if (joinedPlayers + 1 == maxSlots) {
            state |= 0x02000000; // game is full
        }
        if (isStarted) {
            state |= 0x08000000; // game in progress
        }
        msg.serialize32(state);
        msg.serialize32(0x00); // uptime
        msg.serialize16(0x0300); // game type - map settings not supported on W2BN, so use FFA
        msg.serialize16(0x0100); // sub game type
        msg.serialize32(0xff); // provider version constant
        msg.serialize32(0x00); // not ladder
        msg.serialize((gameNameFromUsername(getUsername())).c_str()); // game name
        msg.serialize(""); // password. TODO: add support

        std::stringstream statstring;
        statstring << ","; // this game is not saved. TODO: add support
        int w = Map.Info.MapWidth;
        int h = Map.Info.MapWidth;
        if (w == 128 && h == 128) {
            statstring << ",";
        } else {
            statstring << std::dec << w / 32 << h / 32 << ",";
        }
        if (maxSlots == 8) {
            statstring << ",";
        } else {
            statstring << std::dec << maxSlots + 10 << ",";
        }
        statstring << "0x04,"; // speed - normal (can be changed in-game anyway)
        statstring << "0x00,"; // not an approved game
        statstring << "0x0a,"; // game type uses map settings
        statstring << "0x01,"; // game settings parameter - none
        statstring << std::hex << FileChecksums << ","; // cd key checksum - we use lua files checksum

        uint32_t game_settings = 0;
        if (GameSettings.NumUnits == 1) {
            game_settings |= 0x200;
        }
        if (NoRandomPlacementMultiplayer == 1) {
            game_settings |= 0x400;
        }
        switch (GameSettings.Resources) {
        case -1:
            break;
        case 1:
            game_settings |= 0x1000;
            break;
        case 2:
            game_settings |= 0x2000;
            break;
        case 3:
            game_settings |= 0x3000;
            break;
        default:
            game_settings |= 0x20000;
            break;
        }
        if (Map.Tileset->Name == "Forest") {
            game_settings |= 0x4000;
        } else if (Map.Tileset->Name == "Winter") {
            game_settings |= 0x8000;
        } else if (Map.Tileset->Name == "Wasteland") {
            game_settings |= 0xC000;
        } else if (Map.Tileset->Name == "Orc Swamp") {
            game_settings |= 0x1C000;
        } else {
            game_settings |= 0x4000; // default to forest
        }
        statstring << std::hex << game_settings << ",";

        statstring << getUsername();
        statstring << "\r";
        if (!Map.Info.Filename.empty()) {
            statstring << Map.Info.Filename;
        } else if (!Map.Info.Description.empty()) {
            statstring << Map.Info.Description;
        } else {
            statstring << "unknown map";
        }
        statstring << "\r";

        msg.serialize(statstring.str().c_str());
        msg.flush(getTCPSocket());
    }

    virtual void stopAdvertising() {
        if (!isConnected()) {
            return;
        }
        BNCSOutputStream msg(0x02);
        msg.flush(getTCPSocket());
    }

    virtual void reportGameResult() {
        BNCSOutputStream msg(0x2c);
        msg.serialize32(8); // number of results
        for (int i = 0; i < 8; i++) {
            if (NetLocalPlayerNumber == i) {
                switch (GameResult) {
                case GameVictory:
                    msg.serialize32(0x01);
                    break;
                case GameDefeat:
                    msg.serialize32(0x02);
                    break;
                case GameDraw:
                    msg.serialize32(0x03);
                    break;
                default:
                    msg.serialize32(0x04);
                    break;
                }
            } else {
                // it's annoying to tease out the other results, we ignore it and let the server merge
                msg.serialize32(0x00);
            }
        }
        for (int i = 0; i < 8; i++) {
            msg.serialize(Hosts[i].PlyName);
        }
        msg.serialize(NetworkMapName.c_str());
        msg.serialize(""); // TODO: transmit player scores
        msg.flush(getTCPSocket());
    }

    void joinChannel(std::string name) {
        if (isConnected()) {
            BNCSOutputStream join(0x0c);
            join.serialize32(0x02); // forced join
            join.serialize(name.c_str());
            join.flush(getTCPSocket());
        }
    }

    void sendUdpConnectionInfo() {
        BNCSOutputStream conntest(0x09, true);
        conntest.serialize32(serverToken);
        conntest.serialize32(udpToken);
        conntest.flush(getUDPSocket(), getHost());
    }

    // UI information
    void setCurrentChannel(std::string name) {
        this->currentChannel = name;
        bool unlisted = true;
        for (const auto c : channelList) {
            if (c == name) {
                unlisted = false;
                break;
            }
        }
        if (unlisted) {
            channelList.push_back(name);
            setChannels(channelList);
        }
        if (SetActiveChannel != NULL) {
          SetActiveChannel->pushPreamble();
          SetActiveChannel->pushString(name);
          SetActiveChannel->run();
        }
    }

    std::string getCurrentChannel() { return currentChannel; }

    void setGamelist(std::vector<Game*> games) {
        for (const auto value : this->games) {
            delete value;
        }
        this->games = games;
        if (SetGames != NULL) {
          SetGames->pushPreamble();
          for (const auto value : games) {
            SetGames->pushTable({{"Creator", value->getCreator()},
                                 {"Host", value->getHost().toString()},
                                 {"IsSavedGame", value->isSavedGame()},
                                 {"Map", value->getMap()},
                                 {"MaxPlayers", value->maxPlayers()},
                                 {"Speed", value->getSpeed()},
                                 {"Approval", value->getApproval()},
                                 {"Settings", value->getGameSettings()},
                                 {"Status", value->getGameStatus()},
                                 {"Type", value->getGameType()}});
          }
          SetGames->run();
        }
    }

    std::vector<Game*> getGames() { return games; }

    void setFriendslist(std::vector<Friend*> friends) {
        for (const auto value : this->friends) {
            delete value;
        }
        this->friends = friends;
        if (SetFriends != NULL) {            
            SetFriends->pushPreamble();
            for (const auto value : friends) {
                SetFriends->pushTable({ { "Name", value->getName() },
                                        { "Status", value->getStatus() },
                                        { "Product", value->getProduct() } });
            }
            SetFriends->run();
        }
    }

    std::vector<Friend*> getFriends() { return friends; }

    void reportUserdata(uint32_t id, std::vector<std::string> values) {
        if (ShowUserInfo != NULL) {
            ShowUserInfo->pushPreamble();
            std::map<std::string, std::variant<std::string, int>> m;
            m["User"] = extendedInfoNames.at(id);
            for (int i = 0; i < values.size(); i++) {
                m[defaultUserKeys.at(i)] = values.at(i);
            }
            ShowUserInfo->pushTable(m);
            ShowUserInfo->run();
        }
    }

    std::queue<std::string> *getInfo() { return &info; }

    void showInfo(std::string arg) {
        if (requestedAddress) {
            // we have requested our external address from the server
            DebugPrint("Requested Address Info: %s\n" _C_ arg.c_str());
            if (arg.find("Server TCP: ") != std::string::npos || arg.find("Client TCP: ") != std::string::npos) {
                // ignore
                return;
            }
            if (arg.find("Client UDP: ") != std::string::npos) {
                unsigned int a, b, c, d, ip, port;
                unsigned char prefix[256]; // longer than any BNet message can be
                int res = sscanf(arg.c_str(), "%s %s %d.%d.%d.%d:%d", prefix, prefix, &a, &b, &c, &d, &port);
                if (res == 7 && a < 255 && b < 255 && c < 255 && d < 255 && port > 1024) {
                    ip = a | b << 8 | c << 16 | d << 24;
                    externalAddress = CHost(ip, port);
                    DebugPrint("My external address is %s\n" _C_ externalAddress.toString().c_str());
                }
                return;
            }
            if (arg.find("Game UDP: ") != std::string::npos) {
                // this is the last line in the /netinfo response
                requestedAddress = false;
                return;
            }
        }
        std::string infoStr = arg;
        info.push(infoStr);
        if (ShowInfo != NULL) {
            ShowInfo->pushPreamble();
            ShowInfo->pushString(infoStr);
            ShowInfo->run();
        }
    }

    void showError(std::string arg) {
        info.push("!!! " + arg + " !!!");
        if (ShowError != NULL) {
            ShowError->pushPreamble();
            ShowError->pushString(arg);
            ShowError->run();
        }
    }

    void showChat(std::string arg) {
        info.push(arg);
        if (ShowChat != NULL) {
            ShowChat->pushPreamble();
            ShowChat->pushString(arg);
            ShowChat->run();
        }
    }

    void addUser(std::string name) {
        userList.insert(name);
        if (AddUser != NULL) {
            AddUser->pushPreamble();
            AddUser->pushString(name);
            AddUser->run();
        }
    }

    void removeUser(std::string name) {
        userList.erase(name);
        if (RemoveUser != NULL) {
            RemoveUser->pushPreamble();
            RemoveUser->pushString(name);
            RemoveUser->run();
        }
    }

    std::set<std::string> getUsers() { return userList; }

    void setChannels(std::vector<std::string> channels) {
        this->channelList = channels;
        if (SetChannels != NULL) {            
            SetChannels->pushPreamble();
            for (const auto value : channels) {
                SetChannels->pushString(value);
            }
            SetChannels->run();
        }
    }

    std::vector<std::string> getChannels() { return channelList; }

    // State
    std::string getUsername() { return username; }

    void setUsername(std::string arg) { username = arg; }

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
        if (pw.empty()) {
            for (int i = 0; i < sizeof(password); i++) {
                this->password[i] = 0;
            }
            hasPassword = false;
        } else {
            pvpgn::sha1_hash(&password, pw.length(), pw.c_str());
            hasPassword = true;
        }
    }

    void setCreateAccount(bool flag) {
        createAccount = flag;
    }

    bool shouldCreateAccount() {
        return createAccount;
    }

    // Protocol
    CHost *getHost() { return host; }

    void setHost(CHost *arg) {
        if (host != NULL) {
            delete host;
        }
        host = arg;
    }

    CUDPSocket *getUDPSocket() {
        if (!NetworkFildes.IsValid()) {
            if (canDoUdp == -1) {
                InitNetwork1();
                if (NetworkFildes.IsValid()) {
                    // I started it, so I'll shut it down
                    canDoUdp = 1;
                } else {
                    // do not try again
                    canDoUdp = 0;
                }
            }
        }
        return &NetworkFildes;
    }

    CTCPSocket *getTCPSocket() { return tcpSocket; }

    BNCSInputStream *getMsgIStream() { return istream; }

    virtual void doOneStep() { if (this->state != NULL) this->state->doOneStep(this); }

    virtual bool handleUDP(const unsigned char *buffer, int len, CHost host) {
        if (host.getIp() != getHost()->getIp() || host.getPort() != getHost()->getPort()) {
            return false;
        }

        uint8_t id = 0;
        if (len >= 4) {
            uint8_t id = buffer[0];
        }
        switch (id) {
        case 0x05:
            // PKT_SERVERPING
            // (UINT32) 0x05 0x00 0x00 0x00
            // (UINT32) UDP Code
            {
                const uint32_t udpCode = reinterpret_cast<const uint32_t*>(buffer)[1];
                BNCSOutputStream udppingresponse(0x14);
                udppingresponse.serialize32(udpCode);
                udppingresponse.flush(getTCPSocket());
            }
            break;
        default:
            // unknown package, ignore
            break;
        }
        return true;
    }

    void setState(NetworkState* newState) {
        assert (newState != this->state);
        if (this->state != NULL) {
            delete this->state;
        }
        this->state = newState;
    }

    uint32_t clientToken;
    uint32_t serverToken;
    uint32_t udpToken;

    LuaCallback *AddUser = NULL;
    LuaCallback *RemoveUser = NULL;
    LuaCallback *SetFriends = NULL;
    LuaCallback *SetGames = NULL;
    LuaCallback *SetChannels = NULL;
    LuaCallback *SetActiveChannel = NULL;
    LuaCallback *ShowError = NULL;
    LuaCallback *ShowInfo = NULL;
    LuaCallback *ShowChat = NULL;
    LuaCallback *ShowUserInfo = NULL;

private:
    std::string gameNameFromUsername(std::string username) {
        return username + "'s game";
    }

    NetworkState *state;
    CHost *host;
    int8_t canDoUdp = -1; // -1,0,1 --- not tried, doesn't work, I created it
    CTCPSocket *tcpSocket;
    BNCSInputStream *istream;

    std::string username;
    uint32_t password[5]; // xsha1 hash of password
    bool hasPassword;
    bool createAccount;

    bool requestedAddress = false;
    CHost externalAddress;

    std::string lastError;

    std::string currentChannel;
    std::set<std::string> userList;
    std::vector<std::string> channelList;
    std::queue<std::string> info;
    std::vector<Game*> games;
    std::vector<Friend*> friends;
    std::vector<std::string> extendedInfoNames;
    std::vector<std::string> defaultUserKeys;
};

int NetworkState::send(Context *ctx, BNCSOutputStream *buf) {
    return buf->flush(ctx->getTCPSocket());
}

class DisconnectedState : public NetworkState {
public:
    DisconnectedState(std::string message) {
        this->message = message;
    };

    virtual void doOneStep(Context *ctx) {
        std::cout << message << std::endl;
        ctx->disconnect();
        ctx->showError(message);
    }

private:
    bool hasPrinted;
    std::string message;
};

class S2C_CHATEVENT : public NetworkState {
public:
    S2C_CHATEVENT() {
        this->ticks = 0;
    }

    virtual void doOneStep(Context *ctx) {
        if ((ticks % 500) == 0) {
            // C>S 0x07 PKT_KEEPALIVE
            // ~500 frames @ ~50fps ~= 10 seconds
            BNCSOutputStream keepalive(0x07, true);
            keepalive.serialize32(ticks);
            keepalive.flush(ctx->getUDPSocket(), ctx->getHost());
        }

        if ((ticks % 2000) == 0) {
            // ~2000 frames @ ~50fps ~= 40 seconds
            ctx->refreshFriends();
            ctx->refreshChannels();
        }

        if ((ticks % 500) == 0) {
            // ~300 frames @ ~50fps ~= 10 seconds
            ctx->refreshGames();
        }

        if (ticks == 50) {
            ctx->requestExternalAddress();
        }

        ticks++;

        if (ctx->getTCPSocket()->HasDataToRead(0)) {
            uint8_t msg = ctx->getMsgIStream()->readMessageId();
            if (msg == 0xff) {
                // try again next time
                return;
            }

            switch (msg) {
            case 0x00: // SID_NULL
                handleNull(ctx);
                break;
            case 0x25: // SID_PING
                handlePing(ctx);
                break;
            case 0x0b: // SID_CHANNELLIST
                ctx->setChannels(ctx->getMsgIStream()->readStringlist());
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
                    ctx->showError("Online game creation failed");
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
                std::cout << "Unhandled message ID: 0x" << std::hex << msg << std::endl;
                std::cout << "Raw contents >>>" << std::endl;
                char* out;
                int len = ctx->getMsgIStream()->readAll(&out);
                std::cout.write(out, len);
                std::cout << "<<<" << std::endl;
                free(out);
            }

            ctx->getMsgIStream()->finishMessage();
        }
    }

private:
    void handleNull(Context *ctx) {
        BNCSOutputStream buffer(0x00);
        send(ctx, &buffer);
    }

    void handlePing(Context *ctx) {
        uint32_t pingValue = ctx->getMsgIStream()->read32();
        BNCSOutputStream buffer(0x25);
        buffer.serialize32(htonl(pingValue));
        send(ctx, &buffer);
    }

    void handleGamelist(Context *ctx) {
        uint32_t cnt = ctx->getMsgIStream()->read32();
        std::vector<Game*> games;
        while (cnt--) {
            uint32_t settings = ctx->getMsgIStream()->read32();
            uint32_t lang = ctx->getMsgIStream()->read32();
            uint16_t addr_fam = ctx->getMsgIStream()->read16();
            // the port is not in network byte order, since it's sent to
            // directly go into a sockaddr_in struct
            uint16_t port = (ctx->getMsgIStream()->read8() << 8) | ctx->getMsgIStream()->read8();
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
            if (!text.empty()) {
                std::string prefix = "/udppunch ";
                unsigned int a, b, c, d, ip, port;
                if (text.size() > prefix.size() && text.rfind(prefix, 0) != std::string::npos) {
                    int res = sscanf(text.substr(prefix.size()).c_str(), "%d.%d.%d.%d:%d", &a, &b, &c, &d, &port);
                    if (res == 5 && a < 255 && b < 255 && c < 255 && d < 255 && port > 1024) {
                         ip = a | b << 8 | c << 16 | d << 24;
                         if (NetConnectType == 1 && !GameRunning) { // the server, waiting for clients
                             const CInitMessage_Header message(MessageInit_FromServer, ICMAYT);
                             NetworkSendICMessage(*(ctx->getUDPSocket()), CHost(ip, port), message);
                         } else {
                             // the client will connect now and send packages, anyway.
                             // any other state shouldn't try to udp hole punch at this stage
                         }
                         return;
                    } else {
                        // incorrect format, fall through and treat as normal whisper;
                    }
                }
            }
            ctx->showChat(username + " whispers " + text);
            break;
        case 0x05: // recv chat
            ctx->showChat(username + ": " + text);
            break;
        case 0x06: // recv broadcast
            ctx->showInfo("[BROADCAST]: " + text);
            break;
        case 0x07: // channel info
            ctx->setCurrentChannel(text);
            ctx->showInfo("Joined channel " + text);
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
            ctx->showInfo("~<" + text + "~>");
            break;
        case 0x13: // error message
            ctx->showError(text);
            break;
        case 0x17: // emote
            break;
        }
    }

    uint64_t ticks;
};

class S2C_ENTERCHAT : public NetworkState {
    virtual void doOneStep(Context *ctx) {
        if (ctx->getTCPSocket()->HasDataToRead(0)) {
            uint8_t msg = ctx->getMsgIStream()->readMessageId();
            if (msg == 0xff) {
                // try again next time
                return;
            }
            if (msg == 0x3a) {
                // pvpgn seems to send a successful logonresponse again
                uint32_t status = ctx->getMsgIStream()->read32();
                assert(status == 0);
                ctx->getMsgIStream()->finishMessage();
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
            ctx->getMsgIStream()->finishMessage();

            ctx->setUsername(uniqueName);
            if (!statString.empty()) {
                ctx->showInfo("Statstring after logon: " + statString);
            }

            ctx->requestExtraUserInfo(ctx->getUsername());

            ctx->setState(new S2C_CHATEVENT());
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

        ctx->refreshChannels();

        BNCSOutputStream join(0x0c);
        join.serialize32(0x01); // first-join
        join.serialize(gameName().c_str());
        join.flush(ctx->getTCPSocket());

        ctx->refreshChannels();

        // TODO: maybe send 0x45 SID_NETGAMEPORT to report our gameport on pvpgn
        // to whatever the user specified on the cmdline?

        ctx->setState(new S2C_ENTERCHAT());
    }
};

class C2S_LOGONRESPONSE2_OR_C2S_CREATEACCOUNT : public NetworkState {
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
            ctx->getMsgIStream()->finishMessage();

            if (!nameSugg.empty()) {
                nameSugg = " (try username: " + nameSugg + ")";
            }

            switch (status) {
            case 0x00:
                // login into created account
                ctx->setState(new C2S_LOGONRESPONSE2_OR_C2S_CREATEACCOUNT());
                return;
            case 0x01:
                ctx->showError("Name too short" + nameSugg);
                ctx->setUsername("");
                ctx->setState(new C2S_LOGONRESPONSE2_OR_C2S_CREATEACCOUNT());
                return;
            case 0x02:
                ctx->showError("Name contains invalid character(s)" + nameSugg);
                ctx->setUsername("");
                ctx->setState(new C2S_LOGONRESPONSE2_OR_C2S_CREATEACCOUNT());
                return;
            case 0x03:
                ctx->showError("Name contains banned word(s)" + nameSugg);
                ctx->setUsername("");
                ctx->setState(new C2S_LOGONRESPONSE2_OR_C2S_CREATEACCOUNT());
                return;
            case 0x04:
                ctx->showError("Account already exists" + nameSugg);
                ctx->setUsername("");
                ctx->setState(new C2S_LOGONRESPONSE2_OR_C2S_CREATEACCOUNT());
                return;
            case 0x05:
                ctx->showError("Account is still being created" + nameSugg);
                ctx->setUsername("");
                ctx->setState(new C2S_LOGONRESPONSE2_OR_C2S_CREATEACCOUNT());
                return;
            case 0x06:
                ctx->showError("Name does not contain enough alphanumeric characters" + nameSugg);
                ctx->setUsername("");
                ctx->setState(new C2S_LOGONRESPONSE2_OR_C2S_CREATEACCOUNT());
                return;
            case 0x07:
                ctx->showError("Name contained adjacent punctuation characters" + nameSugg);
                ctx->setUsername("");
                ctx->setState(new C2S_LOGONRESPONSE2_OR_C2S_CREATEACCOUNT());
                return;
            case 0x08:
                ctx->showError("Name contained too many punctuation characters" + nameSugg);
                ctx->setUsername("");
                ctx->setState(new C2S_LOGONRESPONSE2_OR_C2S_CREATEACCOUNT());
                return;
            default:
                ctx->showError("Unknown error creating account" + nameSugg);
                ctx->setUsername("");
                ctx->setState(new C2S_LOGONRESPONSE2_OR_C2S_CREATEACCOUNT());
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
            ctx->getMsgIStream()->finishMessage();

            switch (status) {
            case 0x00:
                // success. we will send SID_UDPPINGRESPONSE before entering chat
                ctx->setState(new C2S_ENTERCHAT());
                return;
            case 0x01:
            case 0x010000:
                ctx->showError("Account does not exist");
                ctx->setUsername("");
                ctx->setPassword("");
                ctx->setState(new C2S_LOGONRESPONSE2_OR_C2S_CREATEACCOUNT());
                return;
            case 0x02:
            case 0x020000:
                ctx->showError("Incorrect password");
                ctx->setPassword("");
                ctx->setState(new C2S_LOGONRESPONSE2_OR_C2S_CREATEACCOUNT());
                return;
            case 0x06:
            case 0x060000:
                ctx->showError("Account closed: " + ctx->getMsgIStream()->readString());
                ctx->setPassword("");
                ctx->setState(new C2S_LOGONRESPONSE2_OR_C2S_CREATEACCOUNT());
                return;
            default:
                ctx->setState(new DisconnectedState("unknown logon response"));
            }
        }
    }
};

void C2S_LOGONRESPONSE2_OR_C2S_CREATEACCOUNT::doOneStep(Context *ctx) {
  std::string user = ctx->getUsername();
  uint32_t *pw = ctx->getPassword1(); // single-hashed for SID_LOGONRESPONSE2
  if (!user.empty() && pw) {
    if (ctx->shouldCreateAccount()) {
      BNCSOutputStream msg(0x3d);
      uint32_t *pw = ctx->getPassword1();
      for (int i = 0; i < 20; i++) {
        msg.serialize8(reinterpret_cast<uint8_t *>(pw)[i]);
      }
      msg.serialize(ctx->getUsername().c_str());
      msg.flush(ctx->getTCPSocket());
      ctx->setState(new S2C_CREATEACCOUNT2());
      return;
    }

    BNCSOutputStream logon(0x3a);
    logon.serialize32(ctx->clientToken);
    logon.serialize32(ctx->serverToken);

    // Battle.net password hashes are hashed twice using XSHA-1. First, the
    // password is hashed by itself, then the following data is hashed again
    // and sent to Battle.net:
    // (UINT32) Client Token
    // (UINT32) Server Token
    // (UINT32)[5] First password hash
    // The logic below is taken straight from pvpgn
    struct {
      pvpgn::bn_int ticks;
      pvpgn::bn_int sessionkey;
      pvpgn::bn_int passhash1[5];
    } temp;
    uint32_t passhash2[5];

    pvpgn::bn_int_set(&temp.ticks, ntohl(ctx->clientToken));
    pvpgn::bn_int_set(&temp.sessionkey, ntohl(ctx->serverToken));
    pvpgn::hash_to_bnhash((pvpgn::t_hash const *)pw, temp.passhash1);
    pvpgn::bnet_hash(&passhash2, sizeof(temp), &temp); /* do the double hash */

    for (int i = 0; i < 20; i++) {
      logon.serialize8(reinterpret_cast<uint8_t *>(passhash2)[i]);
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
            ctx->getMsgIStream()->finishMessage();

            switch (result) {
            case 0x000:
                // Passed challenge
                ctx->setState(new C2S_LOGONRESPONSE2_OR_C2S_CREATEACCOUNT());
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
            ctx->serverToken = htonl(serverToken); // keep in network order
            uint32_t udpToken = ctx->getMsgIStream()->read32();
            ctx->udpToken = htonl(udpToken);
            uint64_t mpqFiletime = ctx->getMsgIStream()->readFiletime();
            std::string mpqFilename = ctx->getMsgIStream()->readString();
            std::string formula = ctx->getMsgIStream()->readString();
            ctx->getMsgIStream()->finishMessage();

            // immediately respond with pkt_conntest2 udp msg
            ctx->sendUdpConnectionInfo();

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
            exeInfo += gameName();
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
            ctx->getMsgIStream()->finishMessage();

            // immediately respond with C2S_SID_PING
            BNCSOutputStream buffer(0x25);
            buffer.serialize32(htonl(pingValue));
            send(ctx, &buffer);

            ctx->setState(new S2C_SID_AUTH_INFO());
        }
    }
};

class ConnectState : public NetworkState {
    virtual void doOneStep(Context *ctx) {
        // Connect

        std::string localHost = CNetworkParameter::Instance.localHost;
        if (!localHost.compare("127.0.0.1")) {
            localHost = "0.0.0.0";
        }
        if (!ctx->getTCPSocket()->Open(CHost(localHost.c_str(), CNetworkParameter::Instance.localPort))) {
            ctx->setState(new DisconnectedState("TCP open failed"));
            return;
        }
        if (!ctx->getTCPSocket()->IsValid()) {
            ctx->setState(new DisconnectedState("TCP not valid"));
            return;
        }
        if (!ctx->getTCPSocket()->Connect(*ctx->getHost())) {
            ctx->setState(new DisconnectedState("TCP connect failed for server " + ctx->getHost()->toString()));
            return;
        }

        // Send proto byte
        ctx->getTCPSocket()->Send("\x1", 1);

        // C>S SID_AUTH_INFO
        BNCSOutputStream buffer(0x50);
        // (UINT32) Protocol ID
        buffer.serialize32(0x00);
        // (UINT32) Platform code
        buffer.serializeC32("IX86");
        // (UINT32) Product code - we'll just use W2BN and encode what we are in
        // the version
        buffer.serializeC32("W2BN");
        // (UINT32) Version byte - just use the last from W2BN
        buffer.serialize32(0x4f);
        // (UINT32) Language code
        buffer.serialize32(0x00);
        // (UINT32) Local IP
        if (CNetworkParameter::Instance.localHost.compare("127.0.0.1")) {
            // user set a custom local ip, use that
#ifdef USE_WIN32
            uint32_t addr = inet_addr(CNetworkParameter::Instance.localHost.c_str());
            buffer.serialize32(addr);
#else
            struct in_addr addr;
            inet_aton(CNetworkParameter::Instance.localHost.c_str(), &addr);
            buffer.serialize32(addr.s_addr);
#endif
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

static Context _ctx;
OnlineContext *OnlineContextHandler = &_ctx;

static int CclStopAdvertisingOnlineGame(lua_State* l) {
    if (_ctx.isConnected()) {
        _ctx.stopAdvertising();
        lua_pushboolean(l, true);
    } else {
        lua_pushboolean(l, false);
    }
    return 1;
}

static int CclStartAdvertisingOnlineGame(lua_State* l) {
    if (_ctx.isConnected()) {
        _ctx.startAdvertising();
        lua_pushboolean(l, true);
    } else {
        lua_pushboolean(l, false);
    }
    return 1;
}

static int CclSetup(lua_State *l) {
    LuaCheckArgs(l, 1);
    if (!lua_istable(l, 1)) {
        LuaError(l, "incorrect argument");
    }

    for (lua_pushnil(l); lua_next(l, 1); lua_pop(l, 1)) {
        const char *value = LuaToString(l, -2);
        if (!strcmp(value, "AddUser")) {
            if (_ctx.AddUser) {
                delete _ctx.AddUser;
            }
            _ctx.AddUser = new LuaCallback(l, -1);
        } else if (!strcmp(value, "RemoveUser")) {
            if (_ctx.RemoveUser) {
                delete _ctx.RemoveUser;
            }
            _ctx.RemoveUser = new LuaCallback(l, -1);
        } else if (!strcmp(value, "SetFriends")) {
            if (_ctx.SetFriends) {
                delete _ctx.SetFriends;
            }
            _ctx.SetFriends = new LuaCallback(l, -1);
            if (_ctx.isConnected()) {
                _ctx.setFriendslist(_ctx.getFriends());
            }
        } else if (!strcmp(value, "SetGames")) {
          if (_ctx.SetGames) {
            delete _ctx.SetGames;
          }
          _ctx.SetGames = new LuaCallback(l, -1);
        } else if (!strcmp(value, "SetChannels")) {
            if (_ctx.SetChannels) {
                delete _ctx.SetChannels;
            }
            if (_ctx.isConnected()) {
                _ctx.setChannels(_ctx.getChannels());
            }
            _ctx.SetChannels = new LuaCallback(l, -1);
        } else if (!strcmp(value, "SetActiveChannel")) {
            if (_ctx.SetActiveChannel) {
                delete _ctx.SetActiveChannel;
            }
            _ctx.SetActiveChannel = new LuaCallback(l, -1);
            if (_ctx.isConnected()) {
                _ctx.setCurrentChannel(_ctx.getCurrentChannel());
            }
        } else if (!strcmp(value, "ShowChat")) {
            if (_ctx.ShowChat) {
                delete _ctx.ShowChat;
            }
            _ctx.ShowChat = new LuaCallback(l, -1);
        } else if (!strcmp(value, "ShowInfo")) {
            if (_ctx.ShowInfo) {
                delete _ctx.ShowInfo;
            }
            _ctx.ShowInfo = new LuaCallback(l, -1);
        } else if (!strcmp(value, "ShowError")) {
            if (_ctx.ShowError) {
                delete _ctx.ShowError;
            }
            _ctx.ShowError = new LuaCallback(l, -1);
        } else if (!strcmp(value, "ShowUserInfo")) {
            if (_ctx.ShowUserInfo) {
                delete _ctx.ShowUserInfo;
            }
            _ctx.ShowUserInfo = new LuaCallback(l, -1);
        } else {
            LuaError(l, "Unsupported callback: %s" _C_ value);
        }
    }
    return 0;
}

static int CclDisconnect(lua_State *l) {
    LuaCheckArgs(l, 0);
    _ctx.disconnect();
    return 0;
}

static int CclConnect(lua_State *l) {
    _ctx.disconnect();
    LuaCheckArgs(l, 2);
    const char *host = LuaToString(l, 1);
    int port = LuaToNumber(l, 2);

    _ctx.setHost(new CHost(host, port));
    _ctx.setState(new ConnectState());
    return 0;
}

static int CclLogin(lua_State *l) {
    LuaCheckArgs(l, 2);
    _ctx.setCreateAccount(false);
    _ctx.setUsername(LuaToString(l, 1));
    _ctx.setPassword(LuaToString(l, 2));
    return 0;
}

static int CclSignUp(lua_State *l) {
    LuaCheckArgs(l, 2);
    _ctx.setCreateAccount(true);
    _ctx.setUsername(LuaToString(l, 1));
    _ctx.setPassword(LuaToString(l, 2));
    return 0;
}

static int CclStep(lua_State *l) {
    LuaCheckArgs(l, 0);
    if (_ctx.isDisconnected()) {
        LuaError(l, "disconnected service cannot step");
    } else {
        _ctx.doOneStep();
        if (_ctx.isConnecting()) {
            lua_pushstring(l, "connecting");
        } else if (_ctx.isConnected()) {
            lua_pushstring(l, "online");
        } else {
            LuaError(l, "unknown online state");
        }
    }
    return 1;
}

static int CclSendMessage(lua_State *l) {
    LuaCheckArgs(l, 1);
    if (!_ctx.isConnected()) {
        LuaError(l, "connect first");
    }
    _ctx.sendText(LuaToString(l, 1));
    return 0;
}

static int CclJoinChannel(lua_State *l) {
    LuaCheckArgs(l, 1);
    if (!_ctx.isConnected()) {
        LuaError(l, "connect first");
    }
    _ctx.joinChannel(LuaToString(l, 1));
    return 0;
}

static int CclJoinGame(lua_State *l) {
    LuaCheckArgs(l, 2);
    if (!_ctx.isConnected()) {
        LuaError(l, "connect first");
    }
    _ctx.joinGame(LuaToString(l, 1), LuaToString(l, 2));
    return 0;
}

static int CclStatus(lua_State *l) {
    LuaCheckArgs(l, 0);
    if (_ctx.isConnected()) {
        lua_pushstring(l, "connected");
    } else if (_ctx.isConnecting()) {
        lua_pushstring(l, "connecting");
    } else if (_ctx.isDisconnected()) {
        lua_pushstring(l, "disconnected");
    } else {
        if (!_ctx.getInfo()->empty()) {
            lua_pushstring(l, _ctx.getInfo()->back().c_str());
        } else {
            lua_pushstring(l, "unknown error");
        }
    }
    return 1;
}

static int CclRequestUserInfo(lua_State *l) {
    LuaCheckArgs(l, 1);
    if (!_ctx.isConnected()) {
        LuaError(l, "not connected");
    }
    _ctx.requestExtraUserInfo(LuaToString(l, 1));
    return 0;
}

static int CclPunchNAT(lua_State *l) {
    LuaCheckArgs(l, 1);
    _ctx.punchNAT(LuaToString(l, 1));
    return 0;
}

void OnlineServiceCclRegister() {
    lua_createtable(Lua, 0, 3);

    lua_pushcfunction(Lua, CclSetup);
    lua_setfield(Lua, -2, "setup");

    lua_pushcfunction(Lua, CclConnect);
    lua_setfield(Lua, -2, "connect");
    lua_pushcfunction(Lua, CclLogin);
    lua_setfield(Lua, -2, "login");
    lua_pushcfunction(Lua, CclSignUp);
    lua_setfield(Lua, -2, "signup");
    lua_pushcfunction(Lua, CclDisconnect);
    lua_setfield(Lua, -2, "disconnect");

    lua_pushcfunction(Lua, CclStatus);
    lua_setfield(Lua, -2, "status");

    // step is called in the SDL event loop, I don't think we need to expose it
    // lua_pushcfunction(Lua, CclStep);
    // lua_setfield(Lua, -2, "step");
    lua_pushcfunction(Lua, CclSendMessage);
    lua_setfield(Lua, -2, "sendmessage");
    lua_pushcfunction(Lua, CclJoinChannel);
    lua_setfield(Lua, -2, "joinchannel");
    lua_pushcfunction(Lua, CclRequestUserInfo);
    lua_setfield(Lua, -2, "requestuserinfo");
    // join game is called from the netconnect code, I don't think we need to expose it
    // lua_pushcfunction(Lua, CclJoinGame);
    // lua_setfield(Lua, -2, "joingame");
    lua_pushcfunction(Lua, CclStartAdvertisingOnlineGame);
    lua_setfield(Lua, -2, "startadvertising");
    lua_pushcfunction(Lua, CclStopAdvertisingOnlineGame);
    lua_setfield(Lua, -2, "stopadvertising");
    lua_pushcfunction(Lua, CclPunchNAT);
    lua_setfield(Lua, -2, "punchNAT");

    lua_setglobal(Lua, "OnlineService");
}
