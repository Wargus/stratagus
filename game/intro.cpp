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
/**@name intro.c	-	The game intros. */
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
#include "iolib.h"
#include "video.h"
#include "font.h"
#include "network.h"
#include "sound_server.h"
#include "sound.h"
#include "settings.h"
#include "ccl.h"
#include "campaign.h"
#include "cursor.h"
#include "menus.h"
#include "interface.h"
#include "ui.h"

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

/**
**	Linked list struct used to split text up into lines
*/
typedef struct TextLines {
    char* text;			/// line of text
    struct TextLines* next;	/// pointer to next line
} TextLines;


/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

global Intro	GameIntro;		/// Game intro
global Credits	GameCredits;		/// Game credits

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

local int IntroNoEvent;			/// Flag got an event.
local int UseContinueButton;
local int ContinueButtonX;
local int ContinueButtonY;
local int ContinueButtonFlags;

/**
**	Callback for input.
*/
local void IntroCallbackButton1(unsigned button)
{
    if( UseContinueButton ) {
	if( (1<<button)==LeftButton &&
	    ContinueButtonX<=CursorX && CursorX<=ContinueButtonX+106 &&
	    ContinueButtonY<=CursorY && CursorY<=ContinueButtonY+27 ) {
	    ContinueButtonFlags|=MenuButtonClicked;
	}
    }
    else {
	IntroNoEvent=0;
    }
}

/**
**	Callback for input.
*/
local void IntroCallbackButton2(unsigned button)
{
    if( UseContinueButton ) {
	if( (1<<button)==LeftButton &&
	    ContinueButtonX<=CursorX && CursorX<=ContinueButtonX+106 &&
	    ContinueButtonY<=CursorY && CursorY<=ContinueButtonY+27 &&
	    (ContinueButtonFlags&MenuButtonClicked) ) {
	    IntroNoEvent=0;
	}
	ContinueButtonFlags&=~MenuButtonClicked;
    }
}

/**
**	Callback for input.
*/
local void IntroCallbackKey1(unsigned key, unsigned keychar)
{
    if( UseContinueButton ) {
	if( keychar=='c' || keychar=='\r' ) {
	    ContinueButtonFlags|=MenuButtonClicked;
	}
    }
    else {
	IntroNoEvent=0;
    }
}

/**
**	Callback for input.
*/
local void IntroCallbackKey2(unsigned key, unsigned keychar)
{
    if( UseContinueButton ) {
	if( (key =='c' || key =='\r') &&
	    (ContinueButtonFlags&MenuButtonClicked) ) {
	    IntroNoEvent=0;
	    ContinueButtonFlags&=~MenuButtonClicked;
	}
    }
}

/**
**	Callback for input.
*/
local void IntroCallbackMouse(int x, int y)
{
    CursorX=x;
    CursorY=y;

    if( UseContinueButton ) {
	if( ContinueButtonX<=CursorX && CursorX<=ContinueButtonX+106 &&
	    ContinueButtonY<=CursorY && CursorY<=ContinueButtonY+27 ) {
	    ContinueButtonFlags|=MenuButtonActive;
	}
	else {
	    ContinueButtonFlags&=~MenuButtonActive;
	}
    }
}

/**
**	Callback for exit.
*/
local void IntroCallbackExit(void)
{
    DebugLevel3Fn("Exit\n");
}

/**
**	Draws a continue button at x,y
*/
local void DrawContinueButton()
{
    DrawMenuButton(MBUTTON_GM_HALF,ContinueButtonFlags,
	106,27,
	ContinueButtonX,ContinueButtonY,
	LargeFont,"~!Continue");
}

local void InitContinueButton(int x,int y)
{
    ContinueButtonX=x;
    ContinueButtonY=y;
    ContinueButtonFlags=MenuButtonSelected;
}

/**
**	Splits text up into a linked list of lines less than a given width.
**
**	@param text	The text to be split up.
**	@param w	Maximum width of a line.
**	@param lines	Pointer to linked list structure.
*/
local void SplitTextIntoLines(const char* text,int w,TextLines** lines)
{
    int l;
    char* s;
    char* str;
    TextLines** ptr;

    l=0;
    s=str=strdup(text);
    ptr=lines;

    for( ;; ) {
	char* s1;
	char* space;

	if( (s1=strchr(s,'\n')) ) {
	    *s1='\0';
	}
	space=NULL;
	for( ;; ) {
	    if( VideoTextLength(LargeFont,s)<w ) {
		break;
	    }
	    s1=strrchr(s,' ');
	    if( !s1 ) {
		fprintf(stderr, "line too long: \"%s\"\n", s);
		break;
	    }
	    if( space )
		*space=' ';
	    space=s1;
	    *space='\0';
	}

	*ptr=(TextLines*)malloc(sizeof(TextLines));
	(*ptr)->text=strdup(s);
	(*ptr)->next=NULL;
	ptr=&((*ptr)->next);

	l+=strlen(s);
	if( !text[l] ) {
	    break;
	}
	++l;
	s=str+l;
    }

    free(str);
}

/**
**	Frees memory in a TextLines struct
**
**	@param lines	Address of the pointer to free
*/
local void FreeTextLines(TextLines** lines)
{
    TextLines* ptr;
    while( *lines ) {
	ptr=(*lines)->next;
	free((*lines)->text);
	free(*lines);
	*lines=ptr;
    }
}

/**
**	Scroll text.
**
**	@param x	x start pixel screen position.
**	@param y	y start pixel screen position.
**	@param w	width of text area
**	@param h	height of text area
**	@param i	scroll index.
**	@param text	Text to display.
**
**	@return		1 if there is more to scroll, 0 if it is done
*/
local int ScrollText(int x,int y,int w,int h,int i,TextLines *lines)
{
    int miny,endy;
    TextLines* ptr;
    int scrolling;

    scrolling=1;

    PushClipping();
    SetClipping(x,y,x+w,y+h);

    miny=y-24;
    endy=y+h;
    y=endy-i;
    ptr=lines;

    for( ; y<endy; ) {
	if( !ptr )
	    break;

	if( y>=miny ) {
	    VideoDrawTextClip(x,y,LargeFont,ptr->text);
	}
	y+=24;

	ptr=ptr->next;
    }

    if( y<miny+24 )
	scrolling=0;

    PopClipping();

    return scrolling;
}

/**
**	Show level intro.
**
**	@param intro	Intro struct
*/
global void ShowIntro(const Intro *intro)
{
    EventCallback callbacks;
    Graphic* background;
    char* text;
    int line;
    int i;
    int l;
    int x;
    int y;
    CLFile* file;
    char buf[1024];
    int stage;
    TextLines* ScrollingText;
    TextLines* ObjectivesText[MAX_OBJECTIVES];
    int OldVideoSyncSpeed;

    UseContinueButton=0;

    VideoLockScreen();
    VideoClearScreen();
    VideoUnlockScreen();
    VideoCreatePalette(GlobalPalette);

    OldVideoSyncSpeed=VideoSyncSpeed;
    VideoSyncSpeed=100;
    SetVideoSync();

    callbacks.ButtonPressed=IntroCallbackButton1;
    callbacks.ButtonReleased=IntroCallbackButton2;
    callbacks.MouseMoved=IntroCallbackMouse;
    callbacks.MouseExit=IntroCallbackExit;
    callbacks.KeyPressed=IntroCallbackKey1;
    callbacks.KeyReleased=IntroCallbackKey2;

    callbacks.NetworkEvent=NetworkEvent;
    callbacks.SoundReady=WriteSound;

    background=LoadGraphic(intro->Background);

    LibraryFileName(intro->TextFile,buf);
    if( !(file=CLopen(buf)) ) {
	fprintf(stderr,"Can't open file `%s'\n",intro->TextFile);
	ExitFatal(-1);
    }
    l=0;
    text=malloc(8192);
    while( (i=CLread(file,text+l,8192))==8192 ) {
	l+=8192;
	text=realloc(text,l+8192);
    }
    text[l+i]='\0';
    l+=i+1;
    text=realloc(text,l);
    CLclose(file);

    CallbackMusicOff();
    StopMusic();
    if( intro->VoiceFile[0] ) {
	PlayFile(intro->VoiceFile[0]);
    }

    SplitTextIntoLines(text,320,&ScrollingText);
    for( i=0; i<MAX_OBJECTIVES; i++) {
	if( intro->Objectives[i] ) {
	    SplitTextIntoLines(intro->Objectives[i],260,&ObjectivesText[i]);
	}
	else {
	    ObjectivesText[i]=NULL;
	}
    }

    x=(VideoWidth-640)/2;
    line=0;
    stage=1;
    IntroNoEvent=1;
    while( IntroNoEvent ) {
	y=(VideoHeight-480)/2;
	if( !PlayingMusic && stage<MAX_BRIEFING_VOICES &&
		intro->VoiceFile[stage] ) {
	    PlayFile(intro->VoiceFile[stage]);
	    stage++;
	}
	VideoLockScreen();
	//
	//	Draw background
	//
	VideoDrawSubClip(background,0,0,
		background->Width,background->Height,
		(VideoWidth-background->Width)/2,
		(VideoHeight-background->Height)/2);
	//
	//	Draw title
	//
	VideoDrawTextCentered(x+422,y+28,LargeFont,intro->Title);
	//
	//	Draw scrolling text
	//
	ScrollText(x+70,y+80,320,170,line,ScrollingText);

	//
	//	Draw objectives
	//
	VideoDrawText(x+372,y+306,LargeFont,"Objectives:");
	y+=330;
	for( i=0; i<MAX_OBJECTIVES && ObjectivesText[i]; ++i ) {
	    TextLines* ptr;

	    ptr=ObjectivesText[i];
	    while( ptr ) {
		VideoDrawText(x+372,y,LargeFont,ptr->text);
		y+=22;
		ptr=ptr->next;
	    }
	}

	VideoUnlockScreen();

	// FIXME: update only the changed area!!!!

	Invalidate();
	RealizeVideoMemory();

	WaitEventsOneFrame(&callbacks);
	WaitEventsOneFrame(&callbacks);
	++line;
    }

    FreeTextLines(&ScrollingText);
    for( i=0; i<MAX_OBJECTIVES; i++ ) {
	if( ObjectivesText[i] )
	    FreeTextLines(&ObjectivesText[i]);
    }

    free(text);
    VideoFree(background);

    VideoLockScreen();
    VideoClearScreen();
    VideoUnlockScreen();

    VideoSyncSpeed=OldVideoSyncSpeed;
    SetVideoSync();

    CallbackMusicOn();
    // FIXME: should this be GameMusic?
    PlayMusic(MenuMusic);
}

/**
**	Show the credits
**
**	@param credits	Credits structure
*/
global void ShowCredits(Credits *credits)
{
    EventCallback callbacks;
    Graphic* background;
    int line;
    int x;
    int y;
    int scrolling;
    TextLines* ScrollingCredits;
    int OldVideoSyncSpeed;

    VideoLockScreen();
    VideoClearScreen();
    VideoUnlockScreen();

    OldVideoSyncSpeed=VideoSyncSpeed;
    VideoSyncSpeed=100;
    SetVideoSync();

    callbacks.ButtonPressed=IntroCallbackButton1;
    callbacks.ButtonReleased=IntroCallbackButton2;
    callbacks.MouseMoved=IntroCallbackMouse;
    callbacks.MouseExit=IntroCallbackExit;
    callbacks.KeyPressed=IntroCallbackKey1;
    callbacks.KeyReleased=IntroCallbackKey2;

    callbacks.NetworkEvent=NetworkEvent;
    callbacks.SoundReady=WriteSound;

    background=NULL;
    if( credits->Background ) {
	background=LoadGraphic(credits->Background);
    }

    // play different music?

    ScrollingCredits=NULL;
    if( credits->Names ) {
	SplitTextIntoLines(credits->Names,320,&ScrollingCredits);
    }

    UseContinueButton=1;
    InitContinueButton(455,480-40);
    GameCursor=TheUI.Point.Cursor;
    DestroyCursorBackground();

    x=(VideoWidth-640)/2;
    y=(VideoHeight-480)/2;
    IntroNoEvent=1;
    line=0;
    scrolling=1;
    while( 1 ) {

	VideoLockScreen();
	HideAnyCursor();

	//
	//	Draw background
	//
	if( background ) {
	    VideoDrawSubClip(background,0,0,
		background->Width,background->Height,
		(VideoWidth-background->Width)/2,
		(VideoHeight-background->Height)/2);
	}

	//
	//	Draw scrolling text
	//
	if( ScrollingCredits ) {
	    scrolling=ScrollText(x+140,y+80,320,275,line,ScrollingCredits);
	}

	DrawContinueButton();
	DrawAnyCursor();

	VideoUnlockScreen();

	// FIXME: update only the changed area!!!!

	Invalidate();
	RealizeVideoMemory();

	if( !IntroNoEvent )
	    break;

	WaitEventsOneFrame(&callbacks);

	++line;

	// Loop if we're done scrolling
	if( !scrolling )
	    line=0;
    }

    if( ScrollingCredits ) {
	FreeTextLines(&ScrollingCredits);
    }

    if( background ) {
	VideoFree(background);
    }

    VideoLockScreen();
    VideoClearScreen();
    VideoUnlockScreen();

    VideoSyncSpeed=OldVideoSyncSpeed;
    SetVideoSync();

    // CallbackMusicOn();
    // FIXME: should this be GameMusic?
    // PlayMusic(MenuMusic);
}

local void DrawTitle(const char* act,const char* title)
{
    VideoDrawTextCentered(
	VideoWidth/2,
	VideoHeight/2 - VideoTextHeight(LargeTitleFont) - 
	    VideoTextHeight(SmallTitleFont)/2,
	SmallTitleFont,
	act);
    VideoDrawTextCentered(
	VideoWidth/2,
	VideoHeight/2 - VideoTextHeight(LargeTitleFont)/2,
	LargeTitleFont,
	title);
}

/**
**	Show picture.
**
**	@param act	Text showing the act number
**	@param title	Title text
**	@param picture	Name of the picture file to show.
*/
global void ShowPicture(const char* act,const char* title,const char* picture)
{
    EventCallback callbacks;
    Graphic* background;
    int OldVideoSyncSpeed;
    int DoFadeOut;
    int maxi;
    int i;

    UseContinueButton=0;

    OldVideoSyncSpeed=VideoSyncSpeed;
    VideoSyncSpeed=100;
    SetVideoSync();
    VideoCreatePalette(GlobalPalette);

    callbacks.ButtonPressed=IntroCallbackButton1;
    callbacks.ButtonReleased=IntroCallbackButton2;
    callbacks.MouseMoved=IntroCallbackMouse;
    callbacks.MouseExit=IntroCallbackExit;
    callbacks.KeyPressed=IntroCallbackKey1;
    callbacks.KeyReleased=IntroCallbackKey2;

    callbacks.NetworkEvent=NetworkEvent;
    callbacks.SoundReady=WriteSound;

    background=LoadGraphic(picture);
    DoFadeOut=1;

    //
    // Show title then fade in background
    //
    VideoLockScreen();
    VideoClearScreen();
    DrawTitle(act,title);
    VideoUnlockScreen();

    Invalidate();
    RealizeVideoMemory();
    IntroNoEvent=1;
    i=0;
    maxi=2*30;
    while( IntroNoEvent && i<maxi ) {
	WaitEventsOneFrame(&callbacks);
	i++;
    }
    if( !IntroNoEvent ) {
	DoFadeOut=0;
    }

    i=0;
    maxi=1*30;
    while( IntroNoEvent && i<maxi ) {
	VideoLockScreen();
	VideoDrawSubClipFaded(background,0,0,
	    background->Width,background->Height,
	    (VideoWidth-background->Width)/2,
	    (VideoHeight-background->Height)/2,
	    256*i/maxi);
	DrawTitle(act,title);
	VideoUnlockScreen();

	Invalidate();
	RealizeVideoMemory();

	WaitEventsOneFrame(&callbacks);
	i++;
    }

    //
    //	Draw background and title
    //
    if( IntroNoEvent ) {
	VideoLockScreen();
	VideoDrawSubClip(background,0,0,
	    background->Width,background->Height,
	    (VideoWidth-background->Width)/2,
	    (VideoHeight-background->Height)/2);
	DrawTitle(act,title);
	VideoUnlockScreen();

	Invalidate();
	RealizeVideoMemory();

	while( IntroNoEvent ) {
	    WaitEventsOneFrame(&callbacks);
	}
    }

    // Fade out background and title
    if( DoFadeOut ) {
	while( i>=0 ) {
	    VideoLockScreen();
	    VideoDrawSubClipFaded(background,0,0,
		background->Width,background->Height,
		(VideoWidth-background->Width)/2,
		(VideoHeight-background->Height)/2,
		256*i/maxi);
	    DrawTitle(act,title);
	    VideoUnlockScreen();

	    Invalidate();
	    RealizeVideoMemory();

	    WaitEventsOneFrame(&callbacks);
	    i--;
	}
    }


    VideoFree(background);

    VideoLockScreen();
    VideoClearScreen();
    VideoUnlockScreen();

    VideoSyncSpeed=OldVideoSyncSpeed;
    SetVideoSync();
}

/**
**	FIXME: docu.
*/
enum {
    STATS_OUTCOME,			/// FIXME: docu
    STATS_RANK,				/// FIXME: docu
    STATS_SCORE,			/// FIXME: docu
    STATS_UNITS,			/// FIXME: docu
    STATS_BUILDINGS,			/// FIXME: docu
    STATS_GOLD,				/// FIXME: docu
    STATS_WOOD,				/// FIXME: docu
    STATS_OIL,				/// FIXME: docu
    STATS_KILLS,			/// FIXME: docu
    STATS_RAZINGS,			/// FIXME: docu
    MAX_STATS_TEXT
};

local char* GameStatsText[MAX_STATS_TEXT];	/// FIXME: docu
local int OldVideoSyncSpeed;			/// FIXME: docu
local int GameStatsFrameCounter;		/// FIXME: docu

/**
**	FIXME: docu.
*/
local void GameStatsInit(void)
{
    Graphic* background;
    char buf[20];

    OldVideoSyncSpeed=VideoSyncSpeed;
    VideoSyncSpeed=100;
    SetVideoSync();
    GameStatsFrameCounter=0;

    background=LoadGraphic(MenuBackground);
    VideoCreatePalette(GlobalPalette);

    VideoLockScreen();
    VideoClearScreen();
    VideoDrawSubClip(background,0,0,background->Width,background->Height,
	(VideoWidth-background->Width)/2,(VideoHeight-background->Height)/2);
    VideoUnlockScreen();

    if( GameResult==GameVictory )
	GameStatsText[STATS_OUTCOME]="Victory!";
    else
	GameStatsText[STATS_OUTCOME]="Defeat!";

    GameStatsText[STATS_RANK]="Overlord";

    sprintf(buf,"%u",ThisPlayer->Score);
    GameStatsText[STATS_SCORE]=strdup(buf);

    sprintf(buf,"%u",ThisPlayer->TotalUnits);
    GameStatsText[STATS_UNITS]=strdup(buf);

    sprintf(buf,"%u",ThisPlayer->TotalBuildings);
    GameStatsText[STATS_BUILDINGS]=strdup(buf);

    sprintf(buf,"%u",ThisPlayer->TotalResources[GoldCost]);
    GameStatsText[STATS_GOLD]=strdup(buf);

    sprintf(buf,"%u",ThisPlayer->TotalResources[WoodCost]);
    GameStatsText[STATS_WOOD]=strdup(buf);

    sprintf(buf,"%u",ThisPlayer->TotalResources[OilCost]);
    GameStatsText[STATS_OIL]=strdup(buf);

    sprintf(buf,"%u",ThisPlayer->TotalKills);
    GameStatsText[STATS_KILLS]=strdup(buf);

    sprintf(buf,"%u",ThisPlayer->TotalRazings);
    GameStatsText[STATS_RAZINGS]=strdup(buf);
}

/**
**	FIXME: docu.
*/
local int GameStatsDrawFunc(void)
{
    int x;
    int y;
    int dodraw;
    int done;
    const int StatsPause=30;  // Wait one second between each stat

    done=0;
    GameStatsFrameCounter++;

    if( (GameStatsFrameCounter%StatsPause)!=0 )
	return done;

    x=(VideoWidth-640)/2;
    y=(VideoHeight-480)/2;
    dodraw=GameStatsFrameCounter/StatsPause;


    if( dodraw==1 ) {
	VideoDrawTextCentered(x+106,y+57,LargeFont,"Outcome");
	VideoDrawTextCentered(x+106,y+78,LargeTitleFont,
	    GameStatsText[STATS_OUTCOME]);
    }

    if( dodraw==2 ) {
	VideoDrawTextCentered(x+324,y+57,LargeFont,"Rank");
	VideoDrawTextCentered(x+324,y+78,SmallTitleFont,
	    GameStatsText[STATS_RANK]);
    }

    if( dodraw==3 ) {
	VideoDrawTextCentered(x+540,y+57,LargeFont,"Score");
	VideoDrawTextCentered(x+540,y+78,SmallTitleFont,
	    GameStatsText[STATS_SCORE]);
    }

    if( dodraw==4 ) {
	VideoDrawTextCentered(x+50,y+178,LargeFont,"Units");
	VideoDrawRectangleClip(ColorBlack,x+10,y+208,80,24);
	VideoDrawRectangleClip(ColorYellow,x+11,y+209,78,22);
	VideoFillRectangleClip(ColorBlack,x+12,y+210,76,20);
	VideoDrawTextCentered(x+50,y+213,LargeFont,
	    GameStatsText[STATS_UNITS]);
    }

    if( dodraw==5 ) {
	VideoDrawTextCentered(x+140,y+178,LargeFont,"Buildings");
	VideoDrawRectangleClip(ColorBlack,x+100,y+208,80,24);
	VideoDrawRectangleClip(ColorYellow,x+101,y+209,78,22);
	VideoFillRectangleClip(ColorBlack,x+102,y+210,76,20);
	VideoDrawTextCentered(x+140,y+213,LargeFont,
	    GameStatsText[STATS_BUILDINGS]);
    }

    if( dodraw==6 ) {
	VideoDrawTextCentered(x+230,y+178,LargeFont,"Gold");
	VideoDrawRectangleClip(ColorBlack,x+190,y+208,80,24);
	VideoDrawRectangleClip(ColorYellow,x+191,y+209,78,22);
	VideoFillRectangleClip(ColorBlack,x+192,y+210,76,20);
	VideoDrawTextCentered(x+230,y+213,LargeFont,
	    GameStatsText[STATS_GOLD]);
    }

    if( dodraw==7 ) {
	VideoDrawTextCentered(x+320,y+178,LargeFont,"Lumber");
	VideoDrawRectangleClip(ColorBlack,x+280,y+208,80,24);
	VideoDrawRectangleClip(ColorYellow,x+281,y+209,78,22);
	VideoFillRectangleClip(ColorBlack,x+282,y+210,76,20);
	VideoDrawTextCentered(x+320,y+213,LargeFont,
	    GameStatsText[STATS_WOOD]);
    }

    if( dodraw==8 ) {
	VideoDrawTextCentered(x+410,y+178,LargeFont,"Oil");
	VideoDrawRectangleClip(ColorBlack,x+370,y+208,80,24);
	VideoDrawRectangleClip(ColorYellow,x+371,y+209,78,22);
	VideoFillRectangleClip(ColorBlack,x+372,y+210,76,20);
	VideoDrawTextCentered(x+410,y+213,LargeFont,
	    GameStatsText[STATS_OIL]);
    }

    if( dodraw==9 ) {
	VideoDrawTextCentered(x+500,y+178,LargeFont,"Kills");
	VideoDrawRectangleClip(ColorBlack,x+460,y+208,80,24);
	VideoDrawRectangleClip(ColorYellow,x+461,y+209,78,22);
	VideoFillRectangleClip(ColorBlack,x+462,y+210,76,20);
	VideoDrawTextCentered(x+500,y+213,LargeFont,
	    GameStatsText[STATS_KILLS]);
    }

    if( dodraw==10 ) {
	VideoDrawTextCentered(x+590,y+178,LargeFont,"Razings");
	VideoDrawRectangleClip(ColorBlack,x+550,y+208,80,24);
	VideoDrawRectangleClip(ColorYellow,x+551,y+209,78,22);
	VideoFillRectangleClip(ColorBlack,x+552,y+210,76,20);
	VideoDrawTextCentered(x+590,y+213,LargeFont,
	    GameStatsText[STATS_RAZINGS]);
	done=1;
    }

    return done;
}

/**
**	FIXME: docu.
*/
local void GameStatsEnd(void)
{
    int i;

    for( i=2; i<MAX_STATS_TEXT; i++ ) {
	free(GameStatsText[i]);
    }

    VideoSyncSpeed=OldVideoSyncSpeed;
    SetVideoSync();
}

/**
**	FIXME: docu.
*/
global void ShowStats(void)
{
    EventCallback callbacks;
    int done;

    callbacks.ButtonPressed=IntroCallbackButton1;
    callbacks.ButtonReleased=IntroCallbackButton2;
    callbacks.MouseMoved=IntroCallbackMouse;
    callbacks.MouseExit=IntroCallbackExit;
    callbacks.KeyPressed=IntroCallbackKey1;
    callbacks.KeyReleased=IntroCallbackKey2;
    callbacks.NetworkEvent=NetworkEvent;
    callbacks.SoundReady=WriteSound;


    GameStatsInit();

    UseContinueButton=1;
    InitContinueButton(455,480-40);
    GameCursor=TheUI.Point.Cursor;
    DestroyCursorBackground();

    done=0;
    IntroNoEvent=1;
    // FIXME: Need continue button at 455,480-40
    while( 1 ) {
	VideoLockScreen();
	HideAnyCursor();
	if( !done ) {
	    done=GameStatsDrawFunc();
	}
	DrawContinueButton();
	DrawAnyCursor();
	VideoUnlockScreen();

	Invalidate();
	RealizeVideoMemory();

	if( !IntroNoEvent )
	    break;

	WaitEventsOneFrame(&callbacks);
    }

    GameStatsEnd();
}


/**
**	Parse the credits configuration.
**
**	@param list	Scheme list containing the credits.
**
**	@todo	'comment and 'title are only parsed, but not used.
*/
local SCM CclCredits(SCM list)
{
    SCM value;
    const char* n;
    int nlen;
    int len;

    if( GameCredits.Background ) {
	free(GameCredits.Background);
    }
    GameCredits.Background=NULL;
    if( GameCredits.Names ) {
	free(GameCredits.Names);
	GameCredits.Names=(char*)malloc(1);
	GameCredits.Names[0]='\0';
    }
    len=0;

    while( !gh_null_p(list) ) {
	value=gh_car(list);
	list=gh_cdr(list);

	if( gh_eq_p(value,gh_symbol2scm("background")) ) {
	    GameCredits.Background=gh_scm2newstr(gh_car(list),NULL);
	    list=gh_cdr(list);
	}
	if( gh_eq_p(value,gh_symbol2scm("name")) 
		|| gh_eq_p(value,gh_symbol2scm("title")) 
		|| gh_eq_p(value,gh_symbol2scm("comment")) ) {
	    n=get_c_string(gh_car(list));
	    nlen=strlen(n);
	    GameCredits.Names=(char*)realloc(GameCredits.Names,len+nlen+2);
	    if( len!=0 ) {
		GameCredits.Names[len++]='\n';
	    }
	    strcpy(GameCredits.Names+len,n);
	    len+=nlen;
	    list=gh_cdr(list);
	}
    }

    return SCM_UNSPECIFIED;
}

/**
**	Register CCL features for credits.
*/
global void CreditsCclRegister(void)
{
    GameCredits.Background=NULL;
    GameCredits.Names=NULL;
    gh_new_procedureN("credits",CclCredits);
}

//@}
