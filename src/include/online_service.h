#ifndef __ONLINE_SERVICE_H__
#define __ONLINE_SERVICE_H__

class OnlineContext {
public:
    virtual void doOneStep();

    virtual void goOnline();

    virtual void joinGame(std:string name, std::string pw);

    // TODO: allow passing all the other options, like 1 peon only, resource amount, game type, ...
    virtual void advertiseGame(std::string name, std::string pw, std::string creatorName, std::string mapName,
                               int mapX, int mapY, int maxPlayers, int playersInGame);

    virtual void stopAdvertisingGame();

    virtual void reportGameResult();
};

extern OnlineContext *OnlineContextHandler;

#endif
