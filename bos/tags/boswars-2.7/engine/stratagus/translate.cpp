//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name translate.cpp - Translate languages. */
//
//      (c) Copyright 2005-2009 by Jimmy Salmon
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

#include "stratagus.h"

#include <map>
#include <string>
#include <cstdio>

#include "translate.h"
#include "iolib.h"

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

typedef std::map<std::string, std::string> EntriesType;
static EntriesType Entries;
std::string StratagusTranslation;
std::string GameTranslation;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Translate a string
*/
const char *Translate(const char *str)
{
	EntriesType::iterator i = Entries.find(str);
	if (i != Entries.end() && !i->second.empty()) {
		return i->second.c_str();
	} else {
		return str;
	}
}

/**
**  Add a translation
*/
void AddTranslation(const std::string &str1, const std::string &str2)
{
	if (str2.empty()) {
		Entries[str1] = str1;
	} else {
		Entries[str1] = str2;
	}
}

/**
**  Load a .po file
*/
void LoadPO(const std::string &file)
{
	FILE *fd;
	char buf[4096];
	enum { MSGNONE, MSGID, MSGSTR } state;
	char msgid[16 * 1024];
	char msgstr[16 * 1024];
	char *s;
	char *currmsg = NULL;
	char fullfile[1024];

	if (file.empty()) {
		return;
	}

	LibraryFileName(file.c_str(), fullfile, sizeof(fullfile));
	fd = fopen(fullfile, "rb");
	if (!fd) {
		fprintf(stderr, "Could not open file: %s\n", file.c_str());
		return;
	}

	state = MSGNONE;
	msgid[0] = msgstr[0] = '\0';

	// skip 0xEF utf8 intro if found
	char c = fgetc(fd);
	if (c == (char)0xEF) {
		fgetc(fd);
		fgetc(fd);
	} else {
		rewind(fd);
	}

	while (fgets(buf, sizeof(buf), fd)) {
		// Comment
		if (buf[0] == '#') {
			continue;
		}

		s = buf;

		// msgid or msgstr
		if (!strncmp(s, "msgid ", 6)) {
			if (state == MSGSTR) {
				*currmsg = '\0';
				if (*msgid != '\0') {
					AddTranslation(msgid, msgstr);
				}
			}
			state = MSGID;
			currmsg = msgid;
			*currmsg = '\0';
			s += 6;
			while (*s == ' ') ++s;
		} else if (!strncmp(s, "msgstr ", 7)) {
			if (state == MSGID) {
				*currmsg = '\0';
			}
			state = MSGSTR;
			currmsg = msgstr;
			*currmsg = '\0';
			s += 7;
			while (*s == ' ') ++s;
		}

		// String
		if (*s == '"') {
			++s;
			while (*s && *s != '"') {
				if (*s == '\\') {
					++s;
					if (*s) {
						if (*s == 'n') {
							*currmsg++ = '\n';
						} else if (*s == 't') {
							*currmsg++ = '\t';
						} else if (*s == 'r') {
							*currmsg++ = '\r';
						} else if (*s == '"') {
							*currmsg++ = '"';
						} else if (*s == '\\') {
							*currmsg++ = '\\';
						} else {
							fprintf(stderr, "Invalid escape character: %c\n", *s);
						}
						++s;
					} else {
						fprintf(stderr, "Unterminated string\n");
					}
				} else {
					*currmsg++ = *s++;
				}
			}
			continue;
		}
	}
	if (state == MSGSTR) {
		*currmsg = '\0';
		AddTranslation(msgid, msgstr);
	}

	fclose(fd);
}

/**
**  Set the stratagus and game translations
**
**  Those filenames will be saved in the preferences when SavePreferences will be called.
*/
void SetTranslationsFiles(const std::string &stratagusfile, const std::string &gamefile)
{
	LoadPO(stratagusfile);
	LoadPO(gamefile);
	StratagusTranslation = stratagusfile;
	GameTranslation = gamefile;
}
//@}

