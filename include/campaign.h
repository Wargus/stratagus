//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __\ 
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name campaign.h	-	The campaign headerfile. */
//
//	(c) Copyright 2002 by Lutz Sammer
//
//	FreeCraft is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published
//	by the Free Software Foundation; only version 2 of the License.
//
//	FreeCraft is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	$Id$

#ifndef __CAMPAIGN_H__
#define __CAMPAIGN_H__

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Declaration
----------------------------------------------------------------------------*/

/**
**	Possible outcomes of the game.
*/
typedef enum _game_results_ {
    GameNoResult,			/// Game has no result
    GameVictory,			/// Game was won
    GameDefeat,				/// Game was lost
    GameDraw,				/// Game was draw
} GameResults;				/// Game results

/**
**	Type of the chapters.
*/
typedef enum _chapter_types_ {
    ChapterPlayVideo,			/// Play a video
    ChapterShowPicture,			/// Show a picture
    ChapterPlayLevel,			/// Play a level
    ChapterDefeat,			/// Levels played on defeat
    ChapterDraw,			/// Levels played on draw
    ChapterEnd,				/// End chapter (NOT SUPPORTED)
} ChapterTypes;

/**
**	Campaign chapter.
*/
typedef struct _campaign_chapter_ CampaignChapter;

/**
**	Campaign chapter structure.
*/
struct _campaign_chapter_ {
    CampaignChapter*	Next;		/// Next campaign chapter
    ChapterTypes	Type;		/// Type of the chapter (level,...)
    union {
	struct {
	    char* Name;			/// Chapter name
	} Level;			/// Data for a level
	struct {
	    char* Act;			/// Act number
	    char* Title;		/// Title text
	    char* Background;		/// Background image
	} Picture;			/// Data for a picture
	struct {
	    char* PathName;		/// File name of video
	    int Flags;			/// Playback flags
	} Movie;			/// Data for a movie
    } Data;				/// Data of the different chapter types
    GameResults		Result;		/// Result of this chapter
};

/**
**	Campaign structure.
*/
typedef struct _campaign_ {
    const void*		OType;		/// Object type (future extensions)

    char*		Ident;		/// Unique identifier
    char*		Name;		/// Campaign name
    int			Players;	/// Campaign for X players

    char*		File;		/// File containing the campaign

    CampaignChapter*	Chapters;	/// Campaign chapters
} Campaign;

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern int GameResult;			/// Outcome of the game
extern int RestartScenario;		/// Restart the scenario
extern int QuitToMenu;			/// Quit to menu
extern char CurrentMapPath[];		/// Path of the current map
extern char DefaultObjective[];		/// The default scenario objective

extern const char CampaignType[];	/// Campaign type
extern Campaign* Campaigns;		/// Campaigns
extern int NumCampaigns;		/// Number of campaigns
extern Campaign* CurrentCampaign;	/// Playing this campaign
extern CampaignChapter* CurrentChapter;	/// Playing this chapter of campaign

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

    /// Play a campaign
extern void PlayCampaign(const char* name);
    /// Next chapter of a campaign
extern char* NextChapter(void);

extern void CampaignCclRegister(void);	/// Register ccl features
extern void SaveCampaign(FILE*);	/// Save the campaign module
extern void CleanCampaign(void);	/// Cleanup the campaign module

//@}

#endif	// !__CAMPAIGN_H__
