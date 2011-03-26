//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
/**@name cmd.cpp - Client/Server Command Interpreter. */
//
//      (c) Copyright 2005 by Edward Haase and Jimmy Salmon
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
//

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "stratagus.h"
#include "cmd.h"
#include "netdriver.h"
#include "db.h"
#include "games.h"

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

#define GetNextArg(buf, arg) \
	do { \
		if (*buf == '\"') { \
			*arg = ++buf; \
			while (*buf != '\"' && *buf) ++buf; \
			if (*buf != '\"') return 1; \
			*buf++ = '\0'; \
			if (**arg == '\0') return 1; \
			if (*buf != ' ') return 1; \
		} else { \
			*arg = buf; \
			while (*buf != ' ' && *buf) ++buf; \
			if (!*buf) return 1; \
		} \
		*buf++ = '\0'; \
	} while (0)

#define GetLastArg(buf, arg) \
	do { \
		if (*buf == '\"') { \
			*arg = ++buf; \
			while (*buf != '\"' && *buf) ++buf; \
			if (*buf != '\"') return 1; \
			*buf++ = '\0'; \
			if (**arg == '\0') return 1; \
			if (*buf != ' ' && *buf) return 1; \
		} else { \
			*arg = buf; \
			while (*buf != ' ' && *buf) ++buf; \
		} \
	} while (0)

#define GetNextAndOptionalArg(buf, arg1, arg2) \
	do { \
		if (*buf == '\"') { \
			*arg1 = ++buf; \
			while (*buf != '\"' && *buf) ++buf; \
			if (*buf != '\"') return 1; \
			*buf++ = '\0'; \
			if (**arg1 == '\0') return 1; \
			if (*buf != ' ' && *buf) return 1; \
		} else { \
			*arg1 = buf; \
			while (*buf != ' ' && *buf) ++buf; \
		} \
		if (*buf == ' ') *buf++ = '\0'; \
 \
		*arg2 = NULL; \
		while (*buf == ' ') ++buf; \
		if (!*buf) return 0; \
 \
		GetLastArg(buf, arg2); \
	} while (0)

#define SkipSpaces(buf) \
	do { \
		while (*buf == ' ') ++buf; \
		if (!*buf) return 1; \
	} while (0)

#define CheckExtraParameter(buf) \
	do { \
		if (*buf) { \
			while (*buf == ' ') ++buf; \
			if (*buf) return 1; \
		} \
	} while (0)

static int Parse1Arg(char *buf, char **arg1)
{
	SkipSpaces(buf);

	GetLastArg(buf, arg1);
	CheckExtraParameter(buf);

	return 0;
}

static int Parse1or2Args(char *buf, char **arg1, char **arg2)
{
	SkipSpaces(buf);

	GetNextAndOptionalArg(buf, arg1, arg2);
	CheckExtraParameter(buf);

	return 0;
}

static int Parse4Args(char *buf, char **arg1, char **arg2, char **arg3,
	char **arg4)
{
	SkipSpaces(buf);

	GetNextArg(buf, arg1);
	SkipSpaces(buf);

	GetNextArg(buf, arg2);
	SkipSpaces(buf);

	GetNextArg(buf, arg3);
	SkipSpaces(buf);

	GetLastArg(buf, arg4);
	CheckExtraParameter(buf);

	return 0;
}

#if 0 // not used
static int Parse5Args(char *buf, char **arg1, char **arg2, char **arg3,
	char **arg4, char **arg5)
{
	SkipSpaces(buf);

	GetNextArg(buf, arg1);
	SkipSpaces(buf);

	GetNextArg(buf, arg2);
	SkipSpaces(buf);

	GetNextArg(buf, arg3);
	SkipSpaces(buf);

	GetNextArg(buf, arg4);
	SkipSpaces(buf);

	GetLastArg(buf, arg5);
	CheckExtraParameter(buf);

	return 0;
}
#endif

static int Parse5or6Args(char *buf, char **arg1, char **arg2, char **arg3,
	char **arg4, char **arg5, char **arg6)
{
	SkipSpaces(buf);

	GetNextArg(buf, arg1);
	SkipSpaces(buf);

	GetNextArg(buf, arg2);
	SkipSpaces(buf);

	GetNextArg(buf, arg3);
	SkipSpaces(buf);

	GetNextArg(buf, arg4);
	SkipSpaces(buf);

	GetNextAndOptionalArg(buf, arg5, arg6);
	CheckExtraParameter(buf);

	return 0;
}

/**
**  Parse USER
*/
static void ParseUser(Session *session, char *buf)
{
	char *username;
	char *password;
	char *gamename;
	char *gamever;
	char pw[MAX_PASSWORD_LENGTH + 1];

	if (Parse4Args(buf, &username, &password, &gamename, &gamever)) {
		Send(session, "ERR_BADPARAMETER\n");
		return;
	}

	if (strlen(username) > MAX_USERNAME_LENGTH ||
			strlen(password) > MAX_PASSWORD_LENGTH ||
			strlen(gamename) > MAX_GAMENAME_LENGTH ||
			strlen(gamever) > MAX_VERSION_LENGTH) {
		Send(session, "ERR_BADPARAMETER\n");
		return;
	}

	if (!DBFindUser(username, pw)) {
		DebugPrint("Username doesn't exist: %s\n" _C_ username);
		Send(session, "ERR_NOUSER\n");
		return;
	}
	if (strcmp(pw, password)) {
		DebugPrint("Bad password for user %s\n" _C_ username);
		Send(session, "ERR_BADPASSWORD\n");
		return;
	}

	DebugPrint("User logged in: %s\n" _C_ username);
	strcpy(session->UserData.Name, username);
	strcpy(session->UserData.GameName, gamename);
	strcpy(session->UserData.Version, gamever);
	session->UserData.LoggedIn = 1;
	DBUpdateLoginDate(username);
	Send(session, "USER_OK\n");
}

/**
**  Parse REGISTER
*/
static void ParseRegister(Session *session, char *buf)
{
	char *username;
	char *password;
	char *gamename;
	char *gamever;
	char pw[MAX_PASSWORD_LENGTH + 1];

	if (Parse4Args(buf, &username, &password, &gamename, &gamever)) {
		Send(session, "ERR_BADPARAMETER\n");
		return;
	}

	if (strlen(username) > MAX_USERNAME_LENGTH ||
			strlen(password) > MAX_PASSWORD_LENGTH ||
			strlen(gamename) > MAX_GAMENAME_LENGTH ||
			strlen(gamever) > MAX_VERSION_LENGTH) {
		Send(session, "ERR_BADPARAMETER\n");
		return;
	}

	if (DBFindUser(username, pw)) {
		DebugPrint("Tried to register existing user: %s\n" _C_ username);
		Send(session, "ERR_USEREXISTS\n");
		return;
	}

	DebugPrint("New user registered: %s\n" _C_ username);
	session->UserData.LoggedIn = 1;
	strcpy(session->UserData.Name, username);
	strcpy(session->UserData.GameName, gamename);
	strcpy(session->UserData.Version, gamever);
	DBAddUser(username, password); // FIXME: if this fails?
	Send(session, "REGISTER_OK\n");
}

/**
**  Parse CREATEGAME
*/
static void ParseCreateGame(Session *session, char *buf)
{
	char *description;
	char *map;
	char *players;
	char *ip;
	char *port;
	char *password;
	int players_int;
	int port_int;


	if (Parse5or6Args(buf, &description, &map, &players, &ip, &port, &password)) {
		Send(session, "ERR_BADPARAMETER\n");
		return;
	}

	players_int = atoi(players);
	port_int = atoi(port);
	// FIXME: check ip
	if (strlen(description) > MAX_DESCRIPTION_LENGTH ||
			strlen(map) > MAX_MAP_LENGTH ||
			players_int < 1 || players_int > 16 ||
			port_int < 1 || port_int > 66535 ||
			(password && strlen(password) > MAX_GAME_PASSWORD_LENGTH)) {
		Send(session, "ERR_BADPARAMETER\n");
		return;
	}

	CreateGame(session, description, map, players, ip, port, password);

	DebugPrint("%s created a game\n" _C_ session->UserData.Name);
	Send(session, "CREATEGAME_OK\n");
}

/**
**  Parse CANCELGAME
*/
static void ParseCancelGame(Session *session, char *buf)
{
	// No args
	while (*buf == ' ') ++buf;
	if (*buf) {
		Send(session, "ERR_BADPARAMETER\n");
		return;
	}

	if (CancelGame(session)) {
		Send(session, "ERR_NOGAMECREATED\n");
		return;
	}

	DebugPrint("%s canceled a game\n" _C_ session->UserData.Name);
	Send(session, "CANCELGAME_OK\n");
}

/**
**  Parse STARTGAME
*/
static void ParseStartGame(Session *session, char *buf)
{
	// No args
	while (*buf == ' ') ++buf;
	if (*buf) {
		Send(session, "ERR_BADPARAMETER\n");
		return;
	}

	if (StartGame(session)) {
		Send(session, "ERR_NOGAMECREATED\n");
		return;
	}

	DebugPrint("%s started a game\n" _C_ session->UserData.Name);
	Send(session, "STARTGAME_OK\n");
}

/**
**  Parse LISTGAMES
*/
static void ParseListGames(Session *session, char *buf)
{
	// No args
	while (*buf == ' ') ++buf;
	if (*buf) {
		Send(session, "ERR_BADPARAMETER\n");
		return;
	}

	ListGames(session);

	Send(session, "LISTGAMES_OK\n");
}

/**
**  Parse JOINGAME
*/
static void ParseJoinGame(Session *session, char *buf)
{
	char *id;
	char *password;
	int ret;

	if (Parse1or2Args(buf, &id, &password)) {
		Send(session, "ERR_BADPARAMETER\n");
		return;
	}

	ret = JoinGame(session, atoi(id), password);
	if (ret == -1) {
		Send(session, "ERR_ALREADYINGAME\n");
		return;
	} else if (ret == -2) { // ID not found
		Send(session, "ERR_BADPARAMETER\n");
		return;
	} else if (ret == -3) {
		if (!password) {
			Send(session, "ERR_NEEDPASSWORD\n");
		} else {
			Send(session, "ERR_BADPASSWORD\n");
		}
		return;
	} else if (ret == -4) {
		Send(session, "ERR_GAMEFULL\n");
		return;
	}

	DebugPrint("%s joined game %d\n" _C_ session->UserData.Name _C_ atoi(id));
	Send(session, "JOINGAME_OK\n");
}

/**
**  Parse PARTGAME
*/
static void ParsePartGame(Session *session, char *buf)
{
	int ret;

	// No args
	while (*buf == ' ') ++buf;
	if (*buf) {
		Send(session, "ERR_BADPARAMETER\n");
		return;
	}

	ret = PartGame(session);
	if (ret == -1) {
		Send(session, "ERR_NOTINGAME\n");
		return;
	} else if (ret == -2) {
		Send(session, "ERR_GAMESTARTED\n");
		return;
	}

	DebugPrint("%s left a game\n" _C_ session->UserData.Name);
	Send(session, "PARTGAME_OK\n");
}

/**
**  Parse ENDGAME
*/
static void ParseEndGame(Session *session, char *buf)
{
	char *result;

	Parse1Arg(buf, &result);

	Send(session, "ENDGAME_OK\n");
}

/**
**  Parse MSG
*/
static void ParseMsg(Session *session, char *buf)
{
}

/**
**  ParseBuffer: Handler client/server interaction.
**
**  @param session  Current session.
*/
static void ParseBuffer(Session *session)
{
	char *buf;

	if (!session || session->Buffer[0] == '\0') {
		return;
	}

	buf = session->Buffer;

	if (!session->UserData.LoggedIn) {
		if (!strncmp(buf, "USER ", 5)) {
			ParseUser(session, buf + 5);
		} else if (!strncmp(buf, "REGISTER ", 9)) {
			ParseRegister(session, buf + 9);
		} else {
			fprintf(stderr, "Unknown command: %s\n", session->Buffer);
			Send(session, "ERR_BADCOMMAND\n");
		}
	} else {
		if (!strncmp(buf, "USER ", 5) || !strncmp(buf, "REGISTER ", 9)) {
			Send(session, "ERR_ALREADYLOGGEDIN\n");
		} else if (!strncmp(buf, "CREATEGAME ", 11)) {
			ParseCreateGame(session, buf + 11);
		} else if (!strcmp(buf, "CANCELGAME") || !strncmp(buf, "CANCELGAME ", 11)) {
			ParseCancelGame(session, buf + 10);
		} else if (!strcmp(buf, "STARTGAME") || !strncmp(buf, "STARTGAME ", 10)) {
			ParseStartGame(session, buf + 9);
		} else if (!strcmp(buf, "LISTGAMES") || !strncmp(buf, "LISTGAMES ", 10)) {
			ParseListGames(session, buf + 9);
		} else if (!strncmp(buf, "JOINGAME ", 9)) {
			ParseJoinGame(session, buf + 9);
		} else if (!strcmp(buf, "PARTGAME") || !strncmp(buf, "PARTGAME ", 9)) {
			ParsePartGame(session, buf + 8);
		} else if (!strncmp(buf, "ENDGAME ", 8)) {
			ParseEndGame(session, buf + 8);
		} else if (!strncmp(buf, "MSG ", 4)) {
			ParseMsg(session, buf + 4);
		} else {
			fprintf(stderr, "Unknown command: %s\n", session->Buffer);
			Send(session, "ERR_BADCOMMAND\n");
		}
	}
}

/**
**  Parse all session buffers
*/
int UpdateParser(void)
{
	Session *session;
	int len;
	char *next;

	if (!Pool || !Pool->First) {
		// No connections
		return 0;
	}

	for (session = Pool->First; session; session = session->Next) {
		// Confirm full message.
		while ((next = strpbrk(session->Buffer, "\r\n"))) {
			*next++ = '\0';
			if (*next == '\r' || *next == '\n') {
				++next;
			}

			ParseBuffer(session);

			// Remove parsed message
			len = next - session->Buffer;
			memmove(session->Buffer, next, sizeof(session->Buffer) - len);
			session->Buffer[sizeof(session->Buffer) - len] = '\0';
		}

	}
	return 0;
}

//@}
