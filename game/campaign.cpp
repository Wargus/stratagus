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
/**@name campaign.c - The campaign control. */
//
//      (c) Copyright 2002-2004 by Lutz Sammer and Jimmy Salmon
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

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

char DefaultObjective[] = "-Destroy your enemies";

int GameResult;  /// Outcome of the game
char CurrentMapPath[1024];  /// Path of the current map
char DefaultMap[1024] = "maps/default.pud";  /// Default map path
int RestartScenario;  /// Restart the scenario
int QuitToMenu;  /// Quit to menu
Campaign* Campaigns;  /// Campaigns
int NumCampaigns;  /// Number of campaigns

static Campaign* CurrentCampaign;        /// Playing this campaign
static CampaignChapter* CurrentChapter;  /// Playing this chapter of campaign
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
char* NextChapter(void)
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
void PlayCampaign(const char* name)
{
	char* filename;
	int i;

	//
	// Find the campaign.
	//
	for (i = 0; i < NumCampaigns; ++i) {
		if (!strcmp(Campaigns[i].Ident, name)) {
			CurrentCampaign = Campaigns + i;
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

	GameIntro.Objectives[0] = strdup(DefaultObjective);

	CurrentChapter = CurrentCampaign->Chapters;
	SkipCurrentChapter = 0;
	GameResult = GameVictory;

	filename = NextChapter();
	Assert(filename);

	SkipCurrentChapter = 1;
	GameResult = GameNoResult;

	strcpy(CurrentMapPath, filename);
}

/**
**  Parse campaign show-picture.
**
**  @param l        Lua state.
**  @param chapter  Chapter.
*/
static void ParseShowPicture(lua_State* l, CampaignChapter* chapter)
{
	const char* value;
	int args;
	int j;

	chapter->Type = ChapterShowPicture;

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
			chapter->Data.Picture.Image = strdup(LuaToString(l, -1));
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
			ChapterPictureText** text;
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
			*text = calloc(sizeof(ChapterPictureText), 1);

			subargs = luaL_getn(l, -1);
			for (k = 0; k < subargs; ++k) {
				lua_rawgeti(l, -1, k + 1);
				value = LuaToString(l, -1);
				lua_pop(l, 1);
				++k;

				if (!strcmp(value, "font")) {
					lua_rawgeti(l, -1, k + 1);
					(*text)->Font = FontByIdent(LuaToString(l, -1));
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
					(*text)->Text = strdup(LuaToString(l, -1));
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
static void FreeChapters(CampaignChapter** chapters)
{
	CampaignChapter* ch;
	CampaignChapter* chptr;
	ChapterPictureText* text;
	ChapterPictureText* textptr;

	ch = *chapters;
	while (ch) {
		if (ch->Type == ChapterShowPicture) {
			free(ch->Data.Picture.Image);
			text = ch->Data.Picture.Text;
			while (text) {
				free(text->Text);
				textptr = text;
				text = text->Next;
				free(textptr);
			}
		} else if (ch->Type == ChapterPlayLevel) {
			free(ch->Data.Level.Name);
		} else if (ch->Type == ChapterPlayVideo) {
			free(ch->Data.Movie.PathName);
		}
		chptr = ch;
		ch = ch->Next;
		free(chptr);
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
static int CclDefineCampaign(lua_State* l)
{
	char* ident;
	const char* value;
	Campaign* campaign;
	CampaignChapter* chapter;
	CampaignChapter** tail;
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
	ident = strdup(LuaToString(l, j + 1));
	++j;
	campaign = NULL;

	if (Campaigns) {
		for (i = 0; i < NumCampaigns; ++i) {
			if (!strcmp(Campaigns[i].Ident, ident)) {
				if (!strcmp(ident, "current") && Campaigns[i].Chapters) {
					FreeChapters(&Campaigns[i].Chapters);
				} else if (Campaigns[i].Chapters) {
					// Redefining campaigns causes problems if a campaign is
					// playing.
					return 0;
				}
				campaign = Campaigns + i;
				free(campaign->Ident);
				free(campaign->Name);
				free(campaign->File);
				break;
			}
		}
		if (i == NumCampaigns) {
			Campaigns = realloc(Campaigns, sizeof(Campaign) * (NumCampaigns + 1));
			campaign = Campaigns + NumCampaigns;
			++NumCampaigns;
		}
	} else {
		campaign = Campaigns = malloc(sizeof(Campaign));
		++NumCampaigns;
	}

	memset(campaign, 0, sizeof(Campaign));
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
			campaign->Name = strdup(LuaToString(l, j + 1));
		} else if (!strcmp(value, "file")) {
			campaign->File = strdup(LuaToString(l, j + 1));
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

				chapter = calloc(sizeof(CampaignChapter), 1);
				chapter->Next = *tail;
				*tail = chapter;
				tail = &chapter->Next;

				if (!strcmp(value, "show-picture")) {
					lua_rawgeti(l, j + 1, k + 1);
					ParseShowPicture(l, chapter);
					lua_pop(l, 1);
				} else if (!strcmp(value, "play-movie")) {
					DebugPrint("FIXME: not supported\n");
				} else if (!strcmp(value, "play-level")) {
					chapter->Type = ChapterPlayLevel;
					lua_rawgeti(l, j + 1, k + 1);
					chapter->Data.Level.Name = strdup(LuaToString(l, -1));
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
static int CclSetCurrentChapter(lua_State* l)
{
	int i;

	if (lua_gettop(l) != 1) {
		LuaError(l, "incorrect argument");
	}

	for (i = 0; i < NumCampaigns; ++i) {
		if (!strcmp(Campaigns[i].Ident, "current")) {
			CurrentCampaign = Campaigns + i;
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
static int CclBriefing(lua_State* l)
{
	const char* value;
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
			if (GameIntro.Title) {
				free(GameIntro.Title);
			}
			GameIntro.Title = strdup(LuaToString(l, j + 1));
		} else if (!strcmp(value, "background")) {
			if (GameIntro.Background) {
				free(GameIntro.Background);
			}
			GameIntro.Background = strdup(LuaToString(l, j + 1));
		} else if (!strcmp(value, "text")) {
			if (GameIntro.TextFile) {
				free(GameIntro.TextFile);
			}
			GameIntro.TextFile = strdup(LuaToString(l, j + 1));
		} else if (!strcmp(value, "voice")) {
			if (voice == MAX_BRIEFING_VOICES) {
				LuaError(l, "too many voices");
			}
			if (GameIntro.VoiceFile[voice]) {
				free(GameIntro.VoiceFile[voice]);
			}
			GameIntro.VoiceFile[voice] = strdup(LuaToString(l, j + 1));
			++voice;
		} else if (!strcmp(value, "objective")) {
			if (objective == MAX_OBJECTIVES) {
				LuaError(l, "too many objectives");
			}
			if (GameIntro.Objectives[objective]) {
				free(GameIntro.Objectives[objective]);
			}
			GameIntro.Objectives[objective] = strdup(LuaToString(l, j + 1));
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
void SaveCampaign(CLFile* file)
{
	CampaignChapter* ch;
	ChapterPictureText* text;
	int i;

	CLprintf(file, "\n--- -----------------------------------------\n");
	CLprintf(file, "--- MODULE: campaign $Id$\n\n");
	if (!CurrentCampaign) {
		return;
	}

	CLprintf(file, "DefineCampaign(\"current\", ");
	if (CurrentCampaign->Name) {
		CLprintf(file, " \"name\", \"%s\",", CurrentCampaign->Name);
	}
	CLprintf(file, " \"players\", %d,", CurrentCampaign->Players);
	CLprintf(file, "\n");

	CLprintf(file, "  \"campaign\", {\n");
	for (ch = CurrentCampaign->Chapters; ch; ch = ch->Next) {
		if (ch->Type == ChapterShowPicture) {
			CLprintf(file, "      \"show-picture\", {\n");
			CLprintf(file, "      \"image\", \"%s\",\n", ch->Data.Picture.Image);
			CLprintf(file, "      \"fade-in\", %d,\n", ch->Data.Picture.FadeIn);
			CLprintf(file, "      \"fade-out\", %d,\n", ch->Data.Picture.FadeOut);
			CLprintf(file, "      \"display-time\", %d,\n",
				ch->Data.Picture.DisplayTime);
			for (text = ch->Data.Picture.Text; text; text = text->Next) {
				CLprintf(file, "      \"text\", {\n");
				CLprintf(file, "        \"font\", \"%s\",\n", FontName(text->Font));
				CLprintf(file, "        \"x\", %d,\n", text->X);
				CLprintf(file, "        \"y\", %d,\n", text->Y);
				CLprintf(file, "        \"width\", %d,\n", text->Width);
				CLprintf(file, "        \"height\", %d,\n", text->Height);
				if (text->Align == PictureTextAlignLeft) {
					CLprintf(file,"     \"align\", \",left\",\n");
				} else {
					CLprintf(file,"     \"align\", \"center\",\n");
				}
				CLprintf(file, "        \"text\", \"%s\"", text->Text);
				CLprintf(file, "}");
				if (text->Next) {
					CLprintf(file, ",");
				}
			}
			CLprintf(file,"}, \n");
		} else if (ch->Type == ChapterPlayLevel) {
			CLprintf(file, "    \"play-level\", \"%s\",\n", ch->Data.Level.Name);
		} else if (ch->Type == ChapterPlayVideo) {
			CLprintf(file, "    \"play-movie\", \"%s\", %d\n",
				ch->Data.Movie.PathName, ch->Data.Movie.Flags);
		}
	}
	CLprintf(file, "  }\n");
	CLprintf(file, ")\n");

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
	CLprintf(file, "SetCurrentChapter(%d)\n", i);
}

/**
**  Clean up the campaign module.
*/
void CleanCampaign(void)
{
	int i;

	// FIXME: Can't clean campaign needed for continue.
	DebugPrint("FIXME: Cleaning campaign not written\n");

	if (GameIntro.Title) {
		free(GameIntro.Title);
	}
	if (GameIntro.Background) {
		free(GameIntro.Background);
	}
	if (GameIntro.TextFile) {
		free(GameIntro.TextFile);
	}
	for (i = 0; i < MAX_BRIEFING_VOICES; ++i) {
		free(GameIntro.VoiceFile[i]);
	}
	for (i = 0; i < MAX_OBJECTIVES; ++i) {
		if (GameIntro.Objectives[i]) {
			free(GameIntro.Objectives[i]);
		}
	}
	memset(&GameIntro, 0, sizeof(GameIntro));
}

//@}
