//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __|
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name campaign.c	-	The campaign control. */
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

//@{

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freecraft.h"
#include "ccl.h"
#include "unittype.h"
#include "map.h"
#include "campaign.h"
#include "settings.h"
#include "iolib.h"
#include "font.h"

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

global char DefaultObjective[] = "-Destroy your enemies";

global int GameResult;			/// Outcome of the game
global char CurrentMapPath[1024];	/// Path of the current map
global int RestartScenario;		/// Restart the scenario
global int QuitToMenu;			/// Quit to menu
global Campaign* Campaigns;		/// Campaigns
global int NumCampaigns;		/// Number of campaigns

global Campaign* CurrentCampaign;	/// Playing this campaign
global CampaignChapter* CurrentChapter;	/// Playing this chapter of campaign
local int SkipCurrentChapter=1;		/// Skip the current chapter when
                                        /// looking for the next one

/**
**	Unit-type type definition
*/
global const char CampaignType[] = "campaign-type";

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

/**
**	Return filename of next chapter.
**
**	@return		The filename of the next level
*/
global char* NextChapter(void)
{
    if( RestartScenario ) {
	RestartScenario=0;
	return CurrentMapPath;
    }
    if( QuitToMenu ) {
	QuitToMenu=0;
	CurrentCampaign=NULL;
	return NULL;
    }
    if( !CurrentCampaign ) {
	return NULL;
    }
    if( !CurrentChapter ) {
	return NULL;
    }

    CurrentChapter->Result = GameResult;

    if( GameResult == GameVictory ) {
	//
	//  FIXME: do other chapter types.
	//
	if( SkipCurrentChapter ) {
	    CurrentChapter = CurrentChapter->Next;
	}
	while( CurrentChapter) {
	    if( CurrentChapter->Type==ChapterShowPicture ) {
		ShowPicture(CurrentChapter);
	    }
	    else if( CurrentChapter->Type==ChapterPlayLevel ) {
		break;
	    }

	    CurrentChapter = CurrentChapter->Next;
	}
    }
    else {
	// FIXME: handle defeat
    }

    if( !CurrentChapter ) {
	return NULL;
    }

    return CurrentChapter->Data.Level.Name;
}

/**
**	Play the campaign.
**
**	@param name	Name of the campaign.
**	@note		::CurrentMapPath contains the filename of first level.
*/
global void PlayCampaign(const char* name)
{
    char* filename;
    int i;

    //
    //  Find the campaign.
    //
    for( i=0; i<NumCampaigns; ++i ) {
	if( !strcmp(Campaigns[i].Ident, name) ) {
	    CurrentCampaign = Campaigns + i;
	}
    }
    if( !CurrentCampaign ) {
	return;
    }

    if( !CurrentCampaign->Chapters ) {
	char buf[1024];
	filename=LibraryFileName(CurrentCampaign->File,buf);
	vload(filename, 0, 1);
    }

    GameIntro.Objectives[0]=strdup(DefaultObjective);

    CurrentChapter=CurrentCampaign->Chapters;
    SkipCurrentChapter=0;
    GameResult=GameVictory;

    filename=NextChapter();
    DebugCheck(!filename);

    SkipCurrentChapter=1;
    GameResult=GameNoResult;

    strcpy(CurrentMapPath, filename);
}

/**
**	Parse campaign show-picture.
**
**	@param chapter	    Chapter.
**	@param list	    List describing show-picture.
*/
local void ParseShowPicture(CampaignChapter *chapter,SCM list)
{
    SCM value;
    SCM sublist;

    chapter->Type=ChapterShowPicture;

    while( !gh_null_p(list) ) {
	value=gh_car(list);
	list=gh_cdr(list);

	if( gh_eq_p(value,gh_symbol2scm("image")) ) {
	    chapter->Data.Picture.Image=gh_scm2newstr(gh_car(list),NULL);
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("fade-in")) ) {
	    chapter->Data.Picture.FadeIn=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("fade-out")) ) {
	    chapter->Data.Picture.FadeOut=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("display-time")) ) {
	    chapter->Data.Picture.DisplayTime=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("text")) ) {
	    ChapterPictureText **text;

	    sublist=gh_car(list);
	    list=gh_cdr(list);

	    text = &chapter->Data.Picture.Text;
	    while( *text ) {
		text = &((*text)->Next);
	    }
	    *text = calloc(sizeof(ChapterPictureText),1);

	    while( !gh_null_p(sublist) ) {
		value=gh_car(sublist);
		sublist=gh_cdr(sublist);

		if( gh_eq_p(value,gh_symbol2scm("font")) ) {
		    (*text)->Font=CclFontByIdentifier(gh_car(sublist));
		    sublist=gh_cdr(sublist);
		} else if( gh_eq_p(value,gh_symbol2scm("x")) ) {
		    (*text)->X=gh_scm2int(gh_car(sublist));
		    sublist=gh_cdr(sublist);
		} else if( gh_eq_p(value,gh_symbol2scm("y")) ) {
		    (*text)->Y=gh_scm2int(gh_car(sublist));
		    sublist=gh_cdr(sublist);
		} else if( gh_eq_p(value,gh_symbol2scm("width")) ) {
		    (*text)->Width=gh_scm2int(gh_car(sublist));
		    sublist=gh_cdr(sublist);
		} else if( gh_eq_p(value,gh_symbol2scm("height")) ) {
		    (*text)->Height=gh_scm2int(gh_car(sublist));
		    sublist=gh_cdr(sublist);
		} else if( gh_eq_p(value,gh_symbol2scm("align")) ) {
		    char* str;
		    str=gh_scm2newstr(gh_car(sublist),NIL);
		    if( !strcmp(str,"left") ) {
			(*text)->Align=PictureTextAlignLeft;
		    } else if( !strcmp(str,"center") ) {
			(*text)->Align=PictureTextAlignCenter;
		    } else {
			errl("Invalid chapter picture text align value",gh_car(sublist));
		    }
		    free(str);
		    sublist=gh_cdr(sublist);
		} else if( gh_eq_p(value,gh_symbol2scm("text")) ) {
		    (*text)->Text=gh_scm2newstr(gh_car(sublist),NIL);
		    sublist=gh_cdr(sublist);
		}
	    }
	}
    }
}

/**
**	Free campaign chapters.
**
**	@param chapters	    Chapters to be freed.
*/
local void FreeChapters(CampaignChapter** chapters)
{
    CampaignChapter* ch;
    CampaignChapter* chptr;
    ChapterPictureText* text;
    ChapterPictureText* textptr;

    ch=*chapters;
    while( ch ) {
	if( ch->Type==ChapterShowPicture ) {
	    free(ch->Data.Picture.Image);
	    text=ch->Data.Picture.Text;
	    while( text ) {
		free(text->Text);
		textptr=text;
		text=text->Next;
		free(textptr);
	    }
	} else if( ch->Type==ChapterPlayLevel ) {
	    free(ch->Data.Level.Name);
	} else if( ch->Type==ChapterPlayVideo ) {
	    free(ch->Data.Movie.PathName);
	}
	chptr=ch;
	ch=ch->Next;
	free(chptr);
    }
    *chapters=NULL;
}

/**
**	Define a campaign.
**
**	@param list	List describing the campaign.
**
**	@note FIXME: play-video, defeat, draw are missing.
*/
local SCM CclDefineCampaign(SCM list)
{
    char* ident;
    SCM value;
    SCM sublist;
    Campaign* campaign;
    CampaignChapter* chapter;
    CampaignChapter** tail;
    int i;

    //
    //	Campaign name
    //
    ident=gh_scm2newstr(gh_car(list),NULL);
    list=gh_cdr(list);
    campaign=NULL;

    if( Campaigns ) {
	for( i=0; i<NumCampaigns; ++i ) {
	    if( !strcmp(Campaigns[i].Ident, ident) ) {
		if( !strcmp(ident, "current") && Campaigns[i].Chapters ) {
		    FreeChapters(&Campaigns[i].Chapters);
		} else if( Campaigns[i].Chapters ) {
		    // Redefining campaigns causes problems if a campaign is
		    // playing.
		    return SCM_UNSPECIFIED;
		}
		campaign=Campaigns+i;
		free(campaign->Ident);
		free(campaign->Name);
		free(campaign->File);
		break;
	    }
	}
	if( i==NumCampaigns ) {
	    Campaigns=realloc(Campaigns,sizeof(Campaign)*(NumCampaigns+1));
	    campaign=Campaigns+NumCampaigns;
	    ++NumCampaigns;
	}
    } else {
	campaign=Campaigns=malloc(sizeof(Campaign));
	NumCampaigns++;
    }

    memset(campaign,0,sizeof(Campaign));
    campaign->Ident=ident;
    campaign->Players=1;
    tail=&campaign->Chapters;

    //
    //	Parse the list:	(still everything could be changed!)
    //
    while( !gh_null_p(list) ) {

	value=gh_car(list);
	list=gh_cdr(list);

	if( gh_eq_p(value,gh_symbol2scm("name")) ) {
	    campaign->Name=gh_scm2newstr(gh_car(list),NULL);
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("file")) ) {
	    campaign->File=gh_scm2newstr(gh_car(list),NULL);
	    list=gh_cdr(list);
	} else if ( gh_eq_p(value,gh_symbol2scm("players")) ) {
	    campaign->Players=gh_scm2int(gh_car(list));
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("campaign")) ) {
	    sublist=gh_car(list);
	    list=gh_cdr(list);
	    //
	    //	Parse the list
	    //
	    while( !gh_null_p(sublist) ) {

		value=gh_car(sublist);
		sublist=gh_cdr(sublist);

		chapter=calloc(sizeof(CampaignChapter),1);
		chapter->Next=*tail;
		*tail=chapter;
		tail=&chapter->Next;

		if( gh_eq_p(value,gh_symbol2scm("show-picture")) ) {
		    ParseShowPicture(chapter,gh_car(sublist));
		    sublist=gh_cdr(sublist);
		} else if( gh_eq_p(value,gh_symbol2scm("play-movie")) ) {
		    DebugLevel0Fn("FIXME: not supported\n");
		} else if( gh_eq_p(value,gh_symbol2scm("play-level")) ) {
		    value=gh_car(sublist);
		    sublist=gh_cdr(sublist);

		    chapter->Type=ChapterPlayLevel;
		    chapter->Data.Level.Name=gh_scm2newstr(value,NULL);
		} else {
		   // FIXME: this leaves a half initialized campaign
		   errl("Unsupported tag",value);
		}
	    }
	} else {
	   // FIXME: this leaves a half initialized campaign
	   errl("Unsupported tag",value);
	}
    }

    return SCM_UNSPECIFIED;
}

/**
**	Set the current campaign chapter
**
**	@param num	Number of current chapter in current campaign.
*/
local SCM CclSetCurrentChapter(SCM num)
{
    int i;

    for( i=0; i<NumCampaigns; ++i ) {
	if( !strcmp(Campaigns[i].Ident, "current") ) {
	    CurrentCampaign=Campaigns+i;
	    break;
	}
    }
    if( !CurrentCampaign ) {
	return SCM_UNSPECIFIED;
    }

    i=gh_scm2int(num);
    CurrentChapter=CurrentCampaign->Chapters;
    while( i && CurrentChapter ) {
	--i;
	CurrentChapter=CurrentChapter->Next;
    }

    return SCM_UNSPECIFIED;
}

/**
**	Set the briefing.
**
**	@param list	List describing the briefing.
*/
local SCM CclBriefing(SCM list)
{
    SCM value;
    int voice;
    int objective;

    voice=objective=0;
    //
    //	Parse the list:	(still everything could be changed!)
    //
    while( !gh_null_p(list) ) {

	value=gh_car(list);
	list=gh_cdr(list);

	if( gh_eq_p(value,gh_symbol2scm("type")) ) {
	    if( !gh_eq_p(gh_car(list),gh_symbol2scm("wc2")) &&
		!gh_eq_p(gh_car(list),gh_symbol2scm("sc")) ) {
	       // FIXME: this leaves a half initialized briefing
	       errl("Unsupported briefing type",value);
	    }
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("title")) ) {
	    if( GameIntro.Title ) {
		free(GameIntro.Title);
	    }
	    GameIntro.Title=gh_scm2newstr(gh_car(list),NULL);
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("background")) ) {
	    if( GameIntro.Background ) {
		free(GameIntro.Background);
	    }
	    GameIntro.Background=gh_scm2newstr(gh_car(list),NULL);
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("text")) ) {
	    if( GameIntro.TextFile ) {
		free(GameIntro.TextFile);
	    }
	    GameIntro.TextFile=gh_scm2newstr(gh_car(list),NULL);
	    list=gh_cdr(list);
	} else if( gh_eq_p(value,gh_symbol2scm("voice")) ) {
	    if( voice==MAX_BRIEFING_VOICES ) {
		   errl("too much voices",value);
	    }
	    if( GameIntro.VoiceFile[voice] ) {
		free(GameIntro.VoiceFile[voice]);
	    }
	    GameIntro.VoiceFile[voice]=gh_scm2newstr(gh_car(list),NULL);
	    list=gh_cdr(list);
	    ++voice;
	} else if( gh_eq_p(value,gh_symbol2scm("objective")) ) {
	    if( objective==MAX_OBJECTIVES ) {
		   errl("too much objectives",value);
	    }
	    if( GameIntro.Objectives[objective] ) {
		free(GameIntro.Objectives[objective]);
	    }
	    GameIntro.Objectives[objective]=gh_scm2newstr(gh_car(list),NULL);
	    list=gh_cdr(list);
	    ++objective;
	} else {
	   // FIXME: this leaves a half initialized briefing
	   errl("Unsupported tag",value);
	}
    }

    return SCM_UNSPECIFIED;
}

/**
**	Register CCL features for campaigns.
*/
global void CampaignCclRegister(void)
{
    gh_new_procedureN("define-campaign",CclDefineCampaign);
    gh_new_procedure1_0("set-current-chapter!",CclSetCurrentChapter);
    gh_new_procedureN("briefing",CclBriefing);
}

/**
**	FIXME: should use the names of the real fonts.
*/
local char *FontNames[] = {
    "small",
    "game",
    "large",
    "small-title",
    "large-title",
    "user1",
    "user2",
    "user3",
    "user4",
    "user5",
};

/**
**	Save the campaign module.
*/
global void SaveCampaign(FILE* file)
{
    CampaignChapter *ch;
    ChapterPictureText *text;
    int i;

    fprintf(file,"\n;;; -----------------------------------------\n");
    fprintf(file,";;; MODULE: campaign $Id$\n\n");
    if( !CurrentCampaign ) {
	return;
    }

    fprintf(file,"(define-campaign 'current");
    if( CurrentCampaign->Name ) {
	fprintf(file," 'name \"%s\"",CurrentCampaign->Name);
    }
    fprintf(file," 'players %d",CurrentCampaign->Players);
    fprintf(file,"\n");

    fprintf(file,"  'campaign (list\n");
    for( ch=CurrentCampaign->Chapters; ch; ch=ch->Next ) {
	if( ch->Type == ChapterShowPicture ) {
	    fprintf(file,"    'show-picture (list\n");
	    fprintf(file,"      'image \"%s\"\n",ch->Data.Picture.Image);
	    fprintf(file,"      'fade-in %d\n",ch->Data.Picture.FadeIn);
	    fprintf(file,"      'fade-out %d\n",ch->Data.Picture.FadeOut);
	    fprintf(file,"      'display-time %d\n",
		ch->Data.Picture.DisplayTime);
	    for( text=ch->Data.Picture.Text; text; text=text->Next ) {
		fprintf(file,"      'text (list\n");
		fprintf(file,"        'font '%s\n",FontNames[text->Font]);
		fprintf(file,"        'x %d\n",text->X);
		fprintf(file,"        'y %d\n",text->Y);
		fprintf(file,"        'width %d\n",text->Width);
		fprintf(file,"        'height %d\n",text->Height);
		if (text->Align == PictureTextAlignLeft) {
		    fprintf(file,"        'align 'left\n");
		} else {
		    fprintf(file,"        'align 'center\n");
		}
		fprintf(file,"        'text \"%s\"\n",text->Text);
		fprintf(file,"      )\n");
	    }
	    fprintf(file,"    )\n");
	} else if( ch->Type == ChapterPlayLevel ) {
	    fprintf(file,"    'play-level \"%s\"\n",ch->Data.Level.Name);
	} else if( ch->Type == ChapterPlayVideo ) {
	    fprintf(file,"    'play-movie \"%s\" %d\n",
		ch->Data.Movie.PathName,ch->Data.Movie.Flags);
	}
    }
    fprintf(file,"  )\n");
    fprintf(file,")\n");

    ch=CurrentCampaign->Chapters;
    i=0;
    while( ch ) {
	if( ch==CurrentChapter ) {
	    break;
	}
	ch=ch->Next;
	++i;
    }
    if( !ch ) {
	i=0;
    }
    fprintf(file,"(set-current-chapter! %d)\n", i);
}

/**
**	Clean up the campaign module.
*/
global void CleanCampaign(void)
{
    int i;

    // FIXME: Can't clean campaign needed for continue.
    DebugLevel0Fn("FIXME: Cleaning campaign not written\n");

    if( GameIntro.Title ) {
	free(GameIntro.Title);
    }
    if( GameIntro.Background ) {
	free(GameIntro.Background);
    }
    if( GameIntro.TextFile ) {
	free(GameIntro.TextFile);
    }
    for( i=0; i<MAX_BRIEFING_VOICES; ++i ) {
	free(GameIntro.VoiceFile[i]);
    }
    for( i=0; i<MAX_OBJECTIVES; ++i ) {
	if( GameIntro.Objectives[i] ) {
	    free(GameIntro.Objectives[i]);
	}
    }
    memset(&GameIntro,0,sizeof(GameIntro));
}

//@}
