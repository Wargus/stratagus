#ifndef __ONLINE_SERVICE_H__
#define __ONLINE_SERVICE_H__

#include <string>

class OnlineContext {
public:
    // called in the sdl event loop
    virtual void doOneStep();

    // called when joining a network game
    virtual void joinGame(std::string hostPlayerName, std::string pw);

    // called when leaving a network game
    virtual void leaveGame();

    // called when advertised game is starting (just reports the game as in-progress)
    virtual void startAdvertising(bool isStarted = false);

    // called when advertised game is left by the server
    virtual void stopAdvertising();

    // called when network game ends
    virtual void reportGameResult();
};

extern OnlineContext *OnlineContextHandler;

extern void OnlineServiceCclRegister();

#endif
