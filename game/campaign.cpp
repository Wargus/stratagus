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
/**@name campaign.cpp - The campaign control. */
//
//      (c) Copyright 2002-2005 by Lutz Sammer and Jimmy Salmon
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
//      $Id$

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "script.h"
#include "unittype.h"
#include "map.h"
#include "campaign.h"
#include "settings.h"
#include "iolib.h"
#include "font.h"
#include "video.h"
#include "movie.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

char DefaultObjective[] = "-Destroy your enemies";

GameResults GameResult;                      /// Outcome of the game
char CurrentMapPath[1024];                   /// Path of the current map
char DefaultMap[1024] = "maps/default.smp";  /// Default map path
int RestartScenario;                         /// Restart the scenario
int QuitToMenu;                              /// Quit to menu
std::vector<Campaign *> Campaigns;           /// Campaigns

static Campaign *CurrentCampaign;        /// Playing this campaign
static CampaignChapter *CurrentChapter;  /// Playing this chapter of campaign
static int SkipCurrentChapter = 1;       /// Skip the current chapter when
                                         /// looking for the next one

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Return filename of next chapter.
**
**  @return  The filename of the next level
*/
char *NextChapter(void)
{
	if (RestartScenario) {
		RestartScenario = 0;
		return CurrentMapPath;
	}
	if (QuitToMenu) {
		QuitToMenu = 0;
		CurrentCampaign = NULL;
		return NULL;
	}
	if (!CurrentCampaign) {
		return NULL;
	}
	if (!CurrentChapter) {
		return NULL;
	}

	CurrentChapter->Result = GameResult;

	if (GameResult == GameVictory) {
		//
		// FIXME: do other chapter types.
		//
		if (SkipCurrentChapter) {
			CurrentChapter = CurrentChapter->Next;
		}
		while (CurrentChapter) {
			if (CurrentChapter->Type == ChapterShowPicture) {
				ShowPicture(CurrentChapter);
			} else if (CurrentChapter->Type == ChapterPlayMovie) {
				Video.ClearScreen();
				PlayMovie(CurrentChapter->Data.Movie.File);
			} else if (CurrentChapter->Type == ChapterPlayLevel) {
				break;
			}

			CurrentChapter = CurrentChapter->Next;
		}
	} else {
		// FIXME: handle defeat
	}

	if (!CurrentChapter) {
		return NULL;
	}

	return CurrentChapter->Data.Level.Name;
}

/**
**  Play the campaign.
**
**  @param name  Name of the campaign.
**
**  @note  ::CurrentMapPath contains the filename of first level.
*/
void PlayCampaign(const char *name)
{
	char *filename;
	int i;

	//
	// Find the campaign.
	//
	for (i = 0; i < (int)Campaigns.size(); ++i) {
		if (!strcmp(Campaigns[i]->Ident, name)) {
			CurrentCampaign = Campaigns[i];
		}
	}
	if (!CurrentCampaign) {
		return;
	}

	if (!CurrentCampaign->Chapters) {
		char buf[1024];
		filename = LibraryFileName(CurrentCampaign->File, buf);
		LuaLoadFile(filename);
	}

	GameIntro.Objectives[0] = new_strdup(DefaultObjective);

	CurrentChapter = CurrentCampaign->Chapters;
	SkipCurrentChapter = 0;
	GameResult = GameVictory;

	filename = NextChapter();
	Assert(filename);

	SkipCurrentChapter = 1;
	GameResult = GameNoResult;

	FreeMapInfo(&Map.Info);
	strcpy(CurrentMapPath, filename);
}

/**
**  Parse campaign show-picture.
**
**  @param l        Lua state.
**  @param chapter  Chapter.
*/
static void ParseShowPicture(lua_State *l, CampaignChapter *chapter)
{
	const char *value;
	int args;
	int j;

	chapter->Type = ChapterShowPicture;
	chapter->Data.Picture.Image = NULL;
	chapter->Data.Picture.FadeIn = 0;
	chapter->Data.Picture.FadeOut = 0;
	chapter->Data.Picture.DisplayTime = 30;
	chapter->Data.Picture.Text = NULL;

	if (!lua_istable(l, -1)) {
		LuaError(l, "incorrect argument");
	}
	args = luaL_getn(l, -1);
	for (j = 0; j < args; ++j) {
		lua_rawgeti(l, -1, j + 1);
		value = LuaToString(l, -1);
		lua_pop(l, 1);
		++j;

		if (!strcmp(value, "image")) {
			lua_rawgeti(l, -1, j + 1);
			chapter->Data.Picture.Image = new_strdup(LuaToString(l, -1));
			lua_pop(l, 1);
		} else if (!strcmp(value, "fade-in")) {
			lua_rawgeti(l, -1, j + 1);
			chapter->Data.Picture.FadeIn = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "fade-out")) {
			lua_rawgeti(l, -1, j + 1);
			chapter->Data.Picture.FadeOut = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "display-time")) {
			lua_rawgeti(l, -1, j + 1);
			chapter->Data.Picture.DisplayTime = LuaToNumber(l, -1);
			lua_pop(l, 1);
		} else if (!strcmp(value, "text")) {
			ChapterPictureText **text;
			int subargs;
			int k;

			lua_rawgeti(l, -1, j + 1);
			if (!lua_istable(l, -1)) {
				LuaError(l, "incorrect argument");
			}

			text = &chapter->Data.Picture.Text;
			while (*text) {
				text = &((*text)->Next);
			}
			*text = new ChapterPictureText;

			subargs = luaL_getn(l, -1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;

				if (!strcmp(value, "font")) {
					lua_rawgeti(l, -1, k + 1);
					(*text)->FontIdent = new_strdup(LuaToString(l, -1));
					lua_pop(l, 1);
				} else if (!strcmp(value, "x")) {
					lua_rawgeti(l, -1, k + 1);
					(*text)->X = LuaToNumber(l, -1);
					lua_pop(l, 1);
				} else if (!strcmp(value, "y")) {
					lua_rawgeti(l, -1, k + 1);
					(*text)->Y = LuaToNumber(l, -1);
					lua_pop(l, 1);
				} else if (!strcmp(value, "width")) {
					lua_rawgeti(l, -1, k + 1);
					(*text)->Width = LuaToNumber(l, -1);
					lua_pop(l, 1);
				} else if (!strcmp(value, "height")) {
					lua_rawgeti(l, -1, k + 1);
					(*text)->Height = LuaToNumber(l, -1);
					lua_pop(l, 1);
				} else if (!strcmp(value, "align")) {
					lua_rawgeti(l, -1, k + 1);
					value = LuaToString(l, -1);
					if (!strcmp(value, "left")) {
						(*text)->Align = PictureTextAlignLeft;
					} else if (!strcmp(value, "center")) {
						(*text)->Align = PictureTextAlignCenter;
					} else {
						LuaError(l, "Invalid chapter picture text align value: %s" _C_
							value);
					}
					lua_pop(l, 1);
				} else if (!strcmp(value, "text")) {
					lua_rawgeti(l, -1, k + 1);
					(*text)->Text = new_strdup(LuaToString(l, -1));
					lua_pop(l, 1);
				}
			}
			lua_pop(l, 1);
		}
	}
}

/**
**  Free campaign chapters.
**
**  @param chapters  Chapters to be freed.
*/
static void FreeChapters(CampaignChapter **chapters)
{
	CampaignChapter *ch;
	CampaignChapter *chptr;
	ChapterPictureText *text;
	ChapterPictureText *textptr;

	ch = *chapters;
	while (ch) {
		if (ch->Type == ChapterShowPicture) {
			delete[] ch->Data.Picture.Image;
			text = ch->Data.Picture.Text;
			while (text) {
				delete[] text->FontIdent;
				delete[] text->Text;
				textptr = text;
				text = text->Next;
				delete textptr;
			}
		} else if (ch->Type == ChapterPlayLevel) {
			delete[] ch->Data.Level.Name;
		} else if (ch->Type == ChapterPlayMovie) {
			delete[] ch->Data.Movie.File;
		}
		chptr = ch;
		ch = ch->Next;
		delete chptr;
	}
	*chapters = NULL;
}

/**
**  Define a campaign.
**
**  @param l  Lua state.
**
**  @note FIXME: play-video, defeat, draw are missing.
*/
static int CclDefineCampaign(lua_State *l)
{
	char *ident;
	const char *value;
	Campaign *campaign;
	CampaignChapter *chapter;
	CampaignChapter **tail;
	int i;
	int args;
	int j;
	int subargs;
	int k;

	args = lua_gettop(l);
	j = 0;

	//
	// Campaign name
	//
	ident = new_strdup(LuaToString(l, j + 1));
	++j;
	campaign = NULL;

	if (!Campaigns.empty()) {
		for (i = 0; i < (int)Campaigns.size(); ++i) {
			if (!strcmp(Campaigns[i]->Ident, ident)) {
				if (!strcmp(ident, "current") && Campaigns[i]->Chapters) {
					FreeChapters(&Campaigns[i]->Chapters);
				} else if (Campaigns[i]->Chapters) {
					// Redefining campaigns causes problems if a campaign is
					// playing.
					return 0;
				}
				campaign = Campaigns[i];
				delete[] campaign->Ident;
				campaign->Ident = NULL;
				delete[] campaign->Name;
				campaign->Name = NULL;
				campaign->Players = 0;
				delete[] campaign->File;
				campaign->File = NULL;
				break;
			}
		}
		if (i == (int)Campaigns.size()) {
			campaign = new Campaign;
			Campaigns.push_back(campaign);
		}
	} else {
		campaign = new Campaign;
		Campaigns.push_back(campaign);
	}

	campaign->Ident = ident;
	campaign->Players = 1;
	tail = &campaign->Chapters;

	//
	// Parse the list: (still everything could be changed!)
	//
	for (; j < args; ++j) {
		value = LuaToString(l, j + 1);
		++j;

		if (!strcmp(value, "name")) {
			campaign->Name = new_strdup(LuaToString(l, j + 1));
		} else if (!strcmp(value, "file")) {
			campaign->File = new_strdup(LuaToString(l, j + 1));
		} else if (!strcmp(value, "players")) {
			campaign->Players = LuaToNumber(l, j + 1);
		} else if (!strcmp(value, "campaign")) {
			if (!lua_istable(l, j + 1)) {
				LuaError(l, "incorrect argument");
			}
			//
			// Parse the list
			//
			subargs = luaL_getn(l, j + 1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, j + 1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;

				chapter = new CampaignChapter;
				chapter->Next = *tail;
				*tail = chapter;
				tail = &chapter->Next;

				if (!strcmp(value, "show-picture")) {
					lua_rawgeti(l, j + 1, k + 1);
					ParseShowPicture(l, chapter);
					lua_pop(l, 1);
				} else if (!strcmp(value, "play-movie")) {
					chapter->Type = ChapterPlayMovie;
					chapter->Data.Movie.Flags = 0;
					lua_rawgeti(l, j + 1, k + 1);
					chapter->Data.Movie.File = new_strdup(LuaToString(l, -1));
					lua_pop(l, 1);
				} else if (!strcmp(value, "play-level")) {
					chapter->Type = ChapterPlayLevel;
					lua_rawgeti(l, j + 1, k + 1);
					chapter->Data.Level.Name = new_strdup(LuaToString(l, -1));
					lua_pop(l, 1);
				} else {
					LuaError(l, "Unsupported tag: %s" _C_ value);
				}
			}
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
	}

	return 0;
}

/**
**  Set the current campaign chapter
**
**  @param l  Lua state.
*/
static int CclSetCurrentChapter(lua_State *l)
{
	int i;

	LuaCheckArgs(l, 1);
	for (i = 0; i < (int)Campaigns.size(); ++i) {
		if (!strcmp(Campaigns[i]->Ident, "current")) {
			CurrentCampaign = Campaigns[i];
			break;
		}
	}
	if (!CurrentCampaign) {
		return 0;
	}

	i = LuaToNumber(l, 1);
	CurrentChapter = CurrentCampaign->Chapters;
	while (i && CurrentChapter) {
		--i;
		CurrentChapter = CurrentChapter->Next;
	}

	return 0;
}

/**
**  Set the briefing.
**
**  @param l  Lua state.
*/
static int CclBriefing(lua_State *l)
{
	const char *value;
	int voice;
	int objective;
	int args;
	int j;

	voice = objective = 0;
	//
	// Parse the list: (still everything could be changed!)
	//
	args = lua_gettop(l);
	for (j = 0; j < args; ++j) {
		value = LuaToString(l, j + 1);
		++j;

		if (!strcmp(value, "type")) {
			value = LuaToString(l, j + 1);
			if (strcmp(value, "wc2") && strcmp(value, "sc")) {
				LuaError(l, "Unsupported briefing type: %s" _C_ value);
			}
		} else if (!strcmp(value, "title")) {
			delete[] GameIntro.Title;
			GameIntro.Title = new_strdup(LuaToString(l, j + 1));
		} else if (!strcmp(value, "background")) {
			delete[] GameIntro.Background;
			GameIntro.Background = new_strdup(LuaToString(l, j + 1));
		} else if (!strcmp(value, "text")) {
			delete[] GameIntro.TextFile;
			GameIntro.TextFile = new_strdup(LuaToString(l, j + 1));
		} else if (!strcmp(value, "voice")) {
			if (voice == MAX_BRIEFING_VOICES) {
				LuaError(l, "too many voices");
			}
			delete[] GameIntro.VoiceFile[voice];
			GameIntro.VoiceFile[voice] = new_strdup(LuaToString(l, j + 1));
			++voice;
		} else if (!strcmp(value, "objective")) {
			if (objective == MAX_OBJECTIVES) {
				LuaError(l, "too many objectives");
			}
			delete[] GameIntro.Objectives[objective];
			GameIntro.Objectives[objective] = new_strdup(LuaToString(l, j + 1));
			++objective;
		} else {
			LuaError(l, "Unsupported tag: %s" _C_ value);
		}
	}

	return 0;
}

/**
**  Register CCL features for campaigns.
*/
void CampaignCclRegister(void)
{
	lua_register(Lua, "DefineCampaign", CclDefineCampaign);
	lua_register(Lua, "SetCurrentChapter", CclSetCurrentChapter);
	lua_register(Lua, "Briefing", CclBriefing);
}

/**
**  Save the campaign module.
*/
void SaveCampaign(CFile *file)
{
	CampaignChapter *ch;
	ChapterPictureText *text;
	int i;

	file->printf("\n--- -----------------------------------------\n");
	file->printf("--- MODULE: campaign $Id$\n\n");
	if (!CurrentCampaign) {
		return;
	}

	file->printf("DefineCampaign(\"current\", ");
	if (CurrentCampaign->Name) {
		file->printf(" \"name\", \"%s\",", CurrentCampaign->Name);
	}
	file->printf(" \"players\", %d,", CurrentCampaign->Players);
	file->printf("\n");

	file->printf("  \"campaign\", {\n");
	for (ch = CurrentCampaign->Chapters; ch; ch = ch->Next) {
		if (ch->Type == ChapterShowPicture) {
			file->printf("      \"show-picture\", {\n");
			file->printf("      \"image\", \"%s\",\n", ch->Data.Picture.Image);
			file->printf("      \"fade-in\", %d,\n", ch->Data.Picture.FadeIn);
			file->printf("      \"fade-out\", %d,\n", ch->Data.Picture.FadeOut);
			file->printf("      \"display-time\", %d,\n",
				ch->Data.Picture.DisplayTime);
			for (text = ch->Data.Picture.Text; text; text = text->Next) {
				file->printf("      \"text\", {\n");
				file->printf("        \"font\", \"%s\",\n", text->FontIdent);
				file->printf("        \"x\", %d,\n", text->X);
				file->printf("        \"y\", %d,\n", text->Y);
				file->printf("        \"width\", %d,\n", text->Width);
				file->printf("        \"height\", %d,\n", text->Height);
				if (text->Align == PictureTextAlignLeft) {
					file->printf("     \"align\", \",left\",\n");
				} else {
					file->printf("     \"align\", \"center\",\n");
				}
				file->printf("        \"text\", \"%s\"", text->Text);
				file->printf("}");
				if (text->Next) {
					file->printf(",");
				}
			}
			file->printf("}, \n");
		} else if (ch->Type == ChapterPlayLevel) {
			file->printf("    \"play-level\", \"%s\",\n", ch->Data.Level.Name);
		} else if (ch->Type == ChapterPlayMovie) {
			file->printf("    \"play-movie\", \"%s\",\n", ch->Data.Movie.File);
		}
	}
	file->printf("  }\n");
	file->printf(")\n");

	ch = CurrentCampaign->Chapters;
	i = 0;
	while (ch) {
		if (ch == CurrentChapter) {
			break;
		}
		ch = ch->Next;
		++i;
	}
	if (!ch) {
		i = 0;
	}
	file->printf("SetCurrentChapter(%d)\n", i);
}

/**
**  Clean up the campaign module.
*/
void CleanCampaign(void)
{
	int i;

	// FIXME: Can't clean campaign needed for continue.
	DebugPrint("FIXME: Cleaning campaign not written\n");

	delete[] GameIntro.Title;
	delete[] GameIntro.Background;
	delete[] GameIntro.TextFile;
	for (i = 0; i < MAX_BRIEFING_VOICES; ++i) {
		delete[] GameIntro.VoiceFile[i];
	}
	for (i = 0; i < MAX_OBJECTIVES; ++i) {
		delete[] GameIntro.Objectives[i];
	}
	memset(&GameIntro, 0, sizeof(GameIntro));
}

//@}
