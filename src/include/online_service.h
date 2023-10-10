#ifndef __ONLINE_SERVICE_H__
#define __ONLINE_SERVICE_H__

#include "network/netsockets.h"

#include <string>

class OnlineContext
{
public:
	virtual ~OnlineContext() = default;

	// called in the sdl event loop
	virtual bool handleUDP(const unsigned char *buffer, int len, CHost host) = 0;

	// called in the sdl event loop
	virtual void doOneStep() = 0;

	// called when joining a network game
	virtual void joinGame(std::string hostPlayerName, std::string pw) = 0;

	// called when leaving a network game
	virtual void leaveGame() = 0;

	// called when advertised game is starting (just reports the game as in-progress)
	virtual void startAdvertising(bool isStarted = false) = 0;

	// called when advertised game is left by the server
	virtual void stopAdvertising() = 0;

	// called when network game ends
	virtual void reportGameResult() = 0;
};

extern OnlineContext *OnlineContextHandler;

extern void OnlineServiceCclRegister();

#endif
