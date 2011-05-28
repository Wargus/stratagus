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
/**@name db.cpp - Database routines. */
//
//      (c) Copyright 2005 by Jimmy Salmon
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "stratagus.h"
#include "sqlite3.h"
#include "games.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

static const char *dbfile = "metaserver.db";
static sqlite3 *DB;

#define SQLCreatePlayersTable \
	"CREATE TABLE players (" \
	"username TEXT PRIMARY KEY," \
	"password TEXT," \
	"register_date INTEGER," \
	"last_login_date INTEGER" \
	");"

#define SQLCreateGamesTable \
	"CREATE TABLE games (" \
	"date INTEGER," \
	"id INTEGER PRIMARY KEY," \
	"gamename TEXT," \
	"map_id INTEGER" \
	");"

#define SQLCreateGameDataTable \
	"CREATE TABLE game_data (" \
	"id INTEGER," \
	"username TEXT," \
	"result INTEGER," \
	"score INTEGER," \
	"units INTEGER," \
	"buildings INTEGER," \
	"res1 INTEGER, res2 INTEGER, res3 INTEGER, res4 INTEGER, res5 INTEGER, res6 INTEGER," \
	"res1_used INTEGER, res2_used INTEGER, res3_used INTEGER, res4_used INTEGER, res5_used INTEGER, res6_used INTEGER," \
	"kills INTEGER," \
	"razings INTEGER," \
	"gamecycle INTEGER," \
	"PRIMARY KEY (id, username));"

#define SQLCreateRankingsTable \
	"CREATE TABLE rankings (" \
	"username TEXT," \
	"gamename TEXT," \
	"ranking INTEGER," \
	"PRIMARY KEY (username, gamename)" \
	");"

#define SQLCreateMapsTable \
	"CREATE TABLE maps (" \
	"map_id INTEGER PRIMARY KEY," \
	"map_name TEXT," \
	"map_filename TEXT," \
	"map_uuid TEXT," \
	"map_size_x INTEGER," \
	"map_size_y INTEGER" \
	");"

#define SQLCreateTables \
	SQLCreatePlayersTable SQLCreateGamesTable SQLCreateGameDataTable \
	SQLCreateRankingsTable SQLCreateMapsTable

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Max id callback
*/
static int DBMaxIDCallback(void *password, int argc, char **argv, char **colname)
{
	Assert(argc == 1);
	if (argv[0])
		GameID = atoi(argv[0]);
	return 0;
}

/**
**  Initialize the database
**
**  @return  0 for success, non-zero for failure
*/
int DBInit(void)
{
	FILE *fd;
	int doinit;
	char *errmsg;

	// Check if this is the first time running
	doinit = 0;
	if ((fd = fopen(dbfile, "rb"))) {
		fclose(fd);
	} else {
		doinit = 1;
	}

	if (sqlite3_open(dbfile, &DB) != SQLITE_OK) {
		fprintf(stderr, "ERROR: sqlite3_open failed: %s\n", sqlite3_errmsg(DB));
		return -1;
	}

	if (!doinit) {
		return 0;
	}

	errmsg = NULL;
	if (sqlite3_exec(DB, SQLCreateTables, NULL, NULL, &errmsg) != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", errmsg);
		sqlite3_free(errmsg);
		return -1;
	}

	errmsg = NULL;
	if (sqlite3_exec(DB, "SELECT MAX(id) FROM games;", DBMaxIDCallback, NULL, &errmsg) != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", errmsg);
		sqlite3_free(errmsg);
		return -1;
	}

	return 0;
}

/**
**  Close the database
*/
void DBQuit(void)
{
	sqlite3_close(DB);
}

/**
**  Find user callback
*/
static int DBFindUserCallback(void *password, int argc, char **argv, char **colname)
{
	Assert(argc == 1);
	strcpy((char *)password, argv[0]);
	return 0;
}

/**
**  Find a user and return the password
**
**  @param username  User name to find
**  @param password  If the user is found the password will be returned here
**
**  @return          1 if user is found, 0 otherwise
*/
int DBFindUser(char *username, char *password)
{
	char buf[1024];
	char *errmsg;

	password[0] = '\0';
	if (strchr(username, '\'')) {
		return 0;
	}

	sprintf(buf, "SELECT password FROM players WHERE username = '%s';", username);
	if (sqlite3_exec(DB, buf, DBFindUserCallback, password, &errmsg) != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", errmsg);
		sqlite3_free(errmsg);
		return 0;
	}

	if (password[0]) {
		return 1;
	}
	return 0;
}

/**
**  Add a user
**
**  @param username  User name
**  @param password  Password
**
**  @return          0 for success, non-zero otherwise
*/
int DBAddUser(char *username, char *password)
{
	char buf[1024];
	int t;
	char *errmsg;

	t = (int)time(0);
	sprintf(buf, "INSERT INTO players VALUES('%s', '%s', %d, %d);",
		username, password, t, t);
	if (sqlite3_exec(DB, buf, NULL, NULL, &errmsg) != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", errmsg);
		sqlite3_free(errmsg);
		return -1;
	}
	return 0;
}

/**
**  Log in a user
**
**  @param username  User name
**
**  @return          0 for success, non-zero otherwise
*/
int DBUpdateLoginDate(char *username)
{
	char buf[1024];
	int t;
	char *errmsg;

	t = (int)time(0);
	sprintf(buf, "UPDATE players SET last_login_date = %d WHERE username = '%s'",
		t, username);
	if (sqlite3_exec(DB, buf, NULL, NULL, &errmsg) != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", errmsg);
		sqlite3_free(errmsg);
		return -1;
	}
	return 0;
}
