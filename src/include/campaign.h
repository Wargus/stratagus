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
/**@name campaign.h - The campaign headerfile. */
//
//      (c) Copyright 2002 by Lutz Sammer
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

#ifndef __CAMPAIGN_H__
#define __CAMPAIGN_H__

//@{

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

struct _CL_File_;

/**
**  Possible outcomes of the game.
*/
typedef enum _game_results_ {
	GameNoResult,  ///< Game has no result
	GameVictory,   ///< Game was won
	GameDefeat,    ///< Game was lost
	GameDraw,      ///< Game was draw
} GameResults;     ///< Game results

/**
**  Type of the chapters.
*/
typedef enum _chapter_types_ {
	ChapterPlayVideo,    ///< Play a video
	ChapterShowPicture,  ///< Show a picture
	ChapterPlayLevel,    ///< Play a level
	ChapterDefeat,       ///< Levels played on defeat
	ChapterDraw,         ///< Levels played on draw
	ChapterEnd,          ///< End chapter (NOT SUPPORTED)
} ChapterTypes;

/**
**  Campaign chapter.
*/
typedef struct _campaign_chapter_ CampaignChapter;

/**
**  Picture text alignment
*/
typedef enum _picture_text_alignment_ {
	PictureTextAlignLeft,    ///< Left align
	PictureTextAlignCenter,  ///< Center align
} PictureTextAlignment;

/**
**  Campaign picture text
*/
typedef struct _chapter_picture_text_ {
	int                  Font;            ///< Font
	int                  X;               ///< X position
	int                  Y;               ///< Y position
	int                  Width;           ///< Width
	int                  Height;          ///< Height
	PictureTextAlignment Align;           ///< Alignment
	char*                Text;            ///< Text
	struct _chapter_picture_text_* Next;  ///< Next
} ChapterPictureText;

/**
**  Campaign chapter structure.
*/
struct _campaign_chapter_ {
	CampaignChapter* Next;   ///< Next campaign chapter
	ChapterTypes     Type;   ///< Type of the chapter (level,...)
	union {
		struct {
			char* Name;      ///< Chapter name
		} Level;             ///< Data for a level
		struct {
			char* Image;               ///< File name of image
			int   FadeIn;              ///< Number of cycles to fade in
			int   FadeOut;             ///< Number of cycles to fade out
			int   DisplayTime;         ///< Number of cycles to display image
			ChapterPictureText* Text;  ///< Linked list of text data
		} Picture;                     ///< Data for a picture
		struct {
			char* PathName;  ///< File name of video
			int Flags;       ///< Playback flags
		} Movie;             ///< Data for a movie
	} Data;                  ///< Data of the different chapter types
	GameResults Result;      ///< Result of this chapter
};

/**
**  Campaign structure.
*/
typedef struct _campaign_ {
	const void* OType;  ///< Object type (future extensions)

	char* Ident;    ///< Unique identifier
	char* Name;     ///< Campaign name
	int   Players;  ///< Campaign for X players

	char* File;     ///< File containing the campaign

	CampaignChapter* Chapters;  ///< Campaign chapters
} Campaign;

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern int GameResult;           ///< Outcome of the game
extern int RestartScenario;      ///< Restart the scenario
extern int QuitToMenu;           ///< Quit to menu
extern char CurrentMapPath[];    ///< Path of the current map
extern char DefaultMap[1024];    ///< Default map path
extern char DefaultObjective[];  ///< The default scenario objective

extern Campaign* Campaigns;      ///< Campaigns
extern int NumCampaigns;         ///< Number of campaigns

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

	/// Play a campaign
extern void PlayCampaign(const char* name);
	/// Next chapter of a campaign
extern char* NextChapter(void);

extern void CampaignCclRegister(void);   /// Register ccl features
extern void SaveCampaign(struct _CL_File_* file); /// Save the campaign module
extern void CleanCampaign(void);         /// Cleanup the campaign module

//@}

#endif // !__CAMPAIGN_H__
