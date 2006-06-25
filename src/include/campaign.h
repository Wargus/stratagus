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

#ifndef __CAMPAIGN_H__
#define __CAMPAIGN_H__

//@{

#include <vector>

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CFile;
class CFont;

/**
**  Possible outcomes of the game.
*/
typedef enum GameResults {
	GameNoResult,  /// Game has no result
	GameVictory,   /// Game was won
	GameDefeat,    /// Game was lost
	GameDraw,      /// Game was draw
};                 /// Game results

/**
**  Type of the chapters.
*/
enum ChapterTypes {
	ChapterPlayMovie,    /// Play a movie
	ChapterShowPicture,  /// Show a picture
	ChapterPlayLevel,    /// Play a level
	ChapterDefeat,       /// Levels played on defeat
	ChapterDraw,         /// Levels played on draw
	ChapterEnd,          /// End chapter (NOT SUPPORTED)
};

/**
**  Picture text alignment
*/
enum PictureTextAlignment {
	PictureTextAlignLeft,    /// Left align
	PictureTextAlignCenter,  /// Center align
};

/**
**  Campaign picture text
*/
class ChapterPictureText {
public:
	ChapterPictureText() : Font(0), X(0), Y(0), Width(0), Height(0),
		Align(PictureTextAlignLeft), Text(NULL), Next(NULL) {}

	char *FontIdent;                 /// Font ident
	CFont *Font;                     /// Font
	int X;                           /// X position
	int Y;                           /// Y position
	int Width;                       /// Width
	int Height;                      /// Height
	PictureTextAlignment Align;      /// Alignment
	char *Text;                      /// Text
	ChapterPictureText *Next;        /// Next
};

/**
**  Campaign chapter structure.
*/
class CampaignChapter {
public:
	CampaignChapter() :
		Next(NULL), Type(ChapterPlayLevel), Result(GameNoResult)
	{
		Data.Level.Name = NULL;
	}

	CampaignChapter *Next;   /// Next campaign chapter
	ChapterTypes     Type;   /// Type of the chapter (level,...)
	union {
		struct {
			char *Name;      /// Chapter name
		} Level;             /// Data for a level
		struct {
			char *Image;               /// File name of image
			int   FadeIn;              /// Number of cycles to fade in
			int   FadeOut;             /// Number of cycles to fade out
			int   DisplayTime;         /// Number of cycles to display image
			ChapterPictureText *Text;  /// Linked list of text data
		} Picture;                     /// Data for a picture
		struct {
			char *File;      /// File name of video
			int Flags;       /// Playback flags
		} Movie;             /// Data for a movie
	} Data;                  /// Data of the different chapter types
	GameResults Result;      /// Result of this chapter
};

/**
**  Campaign structure.
*/
class Campaign {
public:
	Campaign() : Ident(NULL), Name(NULL), Players(0), File(NULL), Chapters(NULL) {}

	char *Ident;    /// Unique identifier
	char *Name;     /// Campaign name
	int   Players;  /// Campaign for X players

	char *File;     /// File containing the campaign

	CampaignChapter *Chapters;  /// Campaign chapters
};

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern GameResults GameResult;   /// Outcome of the game
extern int RestartScenario;      /// Restart the scenario
extern char DefaultMap[1024];    /// Default map path
extern char DefaultObjective[];  /// The default scenario objective

extern std::vector<Campaign *> Campaigns;/// Campaigns

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

	/// Play a campaign
extern void PlayCampaign(const char *name);
	/// Next chapter of a campaign
extern char *NextChapter(void);

extern void CampaignCclRegister(void);   /// Register ccl features
extern void SaveCampaign(CFile *file);  /// Save the campaign module
extern void CleanCampaign(void);         /// Cleanup the campaign module

//@}

#endif // !__CAMPAIGN_H__
