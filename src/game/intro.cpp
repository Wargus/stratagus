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
//	(c) Copyright 2002 by Lutz Sammer and Jimmy Salmon.
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

typedef struct PlayerRanks {
    char **Ranks;
    int *Scores;
} PlayerRanks;

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

global Intro GameIntro;			/// Game intro
global Credits	GameCredits;		/// Game credits
local PlayerRanks Ranks[PlayerMax];	/// Ranks

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

local int IntroNoEvent;			/// Flag got an event
local int IntroButtonPressed;		/// Button pressed
local int UseContinueButton;		/// Handle continue button
local int ContinueButtonX;		/// Continue button position X
local int ContinueButtonY;		/// Continue button position Y
local int ContinueButtonFlags;		/// Flags for continue button

/**
**	Callback for input.
*/
local void IntroCallbackButton1(unsigned button)
{
    if (UseContinueButton) {
	if ((1 << button) == LeftButton && ContinueButtonX <= CursorX
		&& CursorX <= ContinueButtonX + 106
		&& ContinueButtonY <= CursorY
		&& CursorY <= ContinueButtonY + 27) {
	    ContinueButtonFlags |= MenuButtonClicked;
	}
    } else {
	IntroNoEvent = 0;
    }
    IntroButtonPressed = 1;
}

/**
**	Callback for input.
*/
local void IntroCallbackButton2(unsigned button)
{
    if (UseContinueButton) {
	if ((1 << button) == LeftButton && ContinueButtonX <= CursorX
		&& CursorX <= ContinueButtonX + 106
		&& ContinueButtonY <= CursorY
		&& CursorY <= ContinueButtonY + 27
		&& (ContinueButtonFlags & MenuButtonClicked)) {
	    IntroNoEvent = 0;
	}
	ContinueButtonFlags &= ~MenuButtonClicked;
    }
}

/**
**	Callback for input.
*/
local void IntroCallbackKey1(unsigned key, unsigned keychar)
{
    HandleKeyModifiersDown(key,keychar);

    if (UseContinueButton) {
	if (keychar == 'c' || keychar == '\r') {
	    ContinueButtonFlags |= MenuButtonClicked;
	}
    } else {
	IntroNoEvent = 0;
    }
    IntroButtonPressed = 1;
}

/**
**	Callback for input.
*/
local void IntroCallbackKey2(unsigned key, unsigned keychar)
{
    HandleKeyModifiersDown(key,keychar);

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
local void IntroCallbackKey3(unsigned key __attribute__((unused)),
			     unsigned keychar __attribute__((unused)))
{
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
local void DrawContinueButton(void)
{
    DrawMenuButton(MBUTTON_GM_HALF,ContinueButtonFlags,
	106,27,
	ContinueButtonX,ContinueButtonY,
	LargeFont,"~!Continue");
}

/**
**	Init continue button.
**
**	@param x	X screen pixel position of continue button.
**	@param y	Y screen pixel position of continue button.
*/
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
    char* s1;
    TextLines** ptr;

    l=0;
    s=str=strdup(text);
    ptr=lines;

    // Convert \r, \r\n, and \n\r to \n
    s1 = s;
    while( s1 ) {
	char* x1;
	char* x2;
	if( (s1=strpbrk(s1,"\n\r")) ) {
	    if( (s1[0]=='\n' && s1[1]=='\r') ||
		(s1[0]=='\r' && s1[1]=='\n') ) {
		x1=s1+1;
		x2=s1+2;
		while( *x1++ = *x2++ ) {
		}
	    }
	    *s1++='\n';
	}
    }

    for( ;; ) {
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
	    if( space ) {
		*space=' ';
	    }
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
**	@param lines	Lines of the text to display.
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
	if( !ptr ) {
	    break;
	}

	if( y>=miny ) {
	    VideoDrawTextClip(x,y,LargeFont,ptr->text);
	}
	y+=24;

	ptr=ptr->next;
    }

    if( y<miny+24 ) {
	scrolling=0;
    }

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
    int c;
    CLFile* file;
    char buf[1024];
    int stage;
    TextLines* scrolling_text;
    TextLines* objectives_text[MAX_OBJECTIVES];
    int old_video_sync;

    UseContinueButton=1;
    InitContinueButton(VideoWidth-185,VideoHeight-40);
    GameCursor=TheUI.Point.Cursor;
    DestroyCursorBackground();

    VideoLockScreen();
    VideoClearScreen();
    VideoUnlockScreen();

    old_video_sync=VideoSyncSpeed;
    VideoSyncSpeed=100;
    SetVideoSync();

    callbacks.ButtonPressed=IntroCallbackButton1;
    callbacks.ButtonReleased=IntroCallbackButton2;
    callbacks.MouseMoved=IntroCallbackMouse;
    callbacks.MouseExit=IntroCallbackExit;
    callbacks.KeyPressed=IntroCallbackKey1;
    callbacks.KeyReleased=IntroCallbackKey2;
    callbacks.KeyRepeated=IntroCallbackKey3;

    callbacks.NetworkEvent=NetworkEvent;
    callbacks.SoundReady=WriteSound;

    background=LoadGraphic(intro->Background);
    ResizeGraphic(background,VideoWidth,VideoHeight);
#ifdef USE_OPENGL
    MakeTexture(background,background->Width,background->Height);
#endif

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

    SplitTextIntoLines(text,320,&scrolling_text);
    for( i=0; i<MAX_OBJECTIVES; i++) {
	if( intro->Objectives[i] ) {
	    SplitTextIntoLines(intro->Objectives[i],260,&objectives_text[i]);
	} else {
	    objectives_text[i]=NULL;
	}
    }

    x=TheUI.Offset640X*2;
    line=0;
    stage=1;
    IntroNoEvent=1;
    c=0;
    while( 1 ) {
	y=TheUI.Offset480Y;
	if( !PlayingMusic && stage<MAX_BRIEFING_VOICES &&
		intro->VoiceFile[stage] ) {
	    PlayFile(intro->VoiceFile[stage]);
	    stage++;
	}
	VideoLockScreen();
	HideAnyCursor();
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
	VideoDrawTextCentered(VideoWidth-218,y+28,LargeFont,intro->Title);
	//
	//	Draw scrolling text
	//
	ScrollText(70,y+80,320,170,line,scrolling_text);

	//
	//	Draw objectives
	//
	y=VideoHeight-(TheUI.Offset480Y/2)-174;
	VideoDrawText(x+372,y,LargeFont,"Objectives:");
	y+=30;
	for( i=0; i<MAX_OBJECTIVES && objectives_text[i]; ++i ) {
	    TextLines* ptr;

	    ptr=objectives_text[i];
	    while( ptr ) {
		VideoDrawText(x+372,y,LargeFont,ptr->text);
		y+=22;
		ptr=ptr->next;
	    }
	}

	DrawContinueButton();
	DrawAnyCursor();

	VideoUnlockScreen();

	// FIXME: update only the changed area!!!!

	Invalidate();
	RealizeVideoMemory();

	if( !IntroNoEvent ) {
	    break;
	}
	WaitEventsOneFrame(&callbacks);
	if( c==0 ) {
	    c=1;
	}
	else {
	    c=0;
	    ++line;
	}
    }

    FreeTextLines(&scrolling_text);
    for( i=0; i<MAX_OBJECTIVES; i++ ) {
	if( objectives_text[i] ) {
	    FreeTextLines(&objectives_text[i]);
	}
    }

    free(text);
    VideoFree(background);

    VideoLockScreen();
    VideoClearScreen();
    VideoUnlockScreen();

    VideoSyncSpeed=old_video_sync;
    SetVideoSync();

    CallbackMusicOn();
    StopMusic();
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
    TextLines* scrolling_credits;
    int old_video_sync;

    VideoLockScreen();
    VideoClearScreen();
    VideoUnlockScreen();

    old_video_sync=VideoSyncSpeed;
    VideoSyncSpeed=100;
    SetVideoSync();

    callbacks.ButtonPressed=IntroCallbackButton1;
    callbacks.ButtonReleased=IntroCallbackButton2;
    callbacks.MouseMoved=IntroCallbackMouse;
    callbacks.MouseExit=IntroCallbackExit;
    callbacks.KeyPressed=IntroCallbackKey1;
    callbacks.KeyReleased=IntroCallbackKey2;
    callbacks.KeyRepeated=IntroCallbackKey3;

    callbacks.NetworkEvent=NetworkEvent;
    callbacks.SoundReady=WriteSound;

    background=NULL;
    if( credits->Background ) {
	background=LoadGraphic(credits->Background);
	ResizeGraphic(background,VideoWidth,VideoHeight);
#ifdef USE_OPENGL
	MakeTexture(background,background->Width,background->Height);
#endif
    }

    // play different music?

    scrolling_credits=NULL;
    if( credits->Names ) {
	SplitTextIntoLines(credits->Names,320,&scrolling_credits);
    }

    UseContinueButton=1;
    InitContinueButton(TheUI.Offset640X+455,TheUI.Offset480Y+440);
    GameCursor=TheUI.Point.Cursor;
    DestroyCursorBackground();

    x=TheUI.Offset640X;
    y=TheUI.Offset480Y;
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
	if( scrolling_credits ) {
	    scrolling=ScrollText(x+140,y+80,320,275,line,scrolling_credits);
	}

	DrawContinueButton();
	DrawAnyCursor();

	VideoUnlockScreen();

	// FIXME: update only the changed area!!!!

	Invalidate();
	RealizeVideoMemory();

	if( !IntroNoEvent ) {
	    break;
	}

	WaitEventsOneFrame(&callbacks);

	++line;

	// Loop if we're done scrolling
	if( !scrolling ) {
	    line=0;
	}
    }

    if( scrolling_credits ) {
	FreeTextLines(&scrolling_credits);
    }

    if( background ) {
	VideoFree(background);
    }

    VideoLockScreen();
    VideoClearScreen();
    VideoUnlockScreen();
    DestroyCursorBackground();

    VideoSyncSpeed=old_video_sync;
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
    int old_video_sync;
    int do_fade_out;
    int maxi;
    int i;

    UseContinueButton=0;

    old_video_sync=VideoSyncSpeed;
    VideoSyncSpeed=100;
    SetVideoSync();

    callbacks.ButtonPressed=IntroCallbackButton1;
    callbacks.ButtonReleased=IntroCallbackButton2;
    callbacks.MouseMoved=IntroCallbackMouse;
    callbacks.MouseExit=IntroCallbackExit;
    callbacks.KeyPressed=IntroCallbackKey1;
    callbacks.KeyReleased=IntroCallbackKey2;
    callbacks.KeyRepeated=IntroCallbackKey3;

    callbacks.NetworkEvent=NetworkEvent;
    callbacks.SoundReady=WriteSound;

    background=LoadGraphic(picture);
    ResizeGraphic(background,VideoWidth,VideoHeight);
#ifdef USE_OPENGL
    MakeTexture(background,background->Width,background->Height);
#endif
    do_fade_out=1;

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
	do_fade_out=0;
    }

    i=0;
    maxi=1*30;
    while( IntroNoEvent && i<maxi ) {
	VideoLockScreen();
	VideoDrawSubClipFaded(background,0,0,
	    background->Width,background->Height,
	    (VideoWidth-background->Width)/2,
	    (VideoHeight-background->Height)/2,
	    255*i/maxi);
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
    if( do_fade_out ) {
	while( i>=0 ) {
	    VideoLockScreen();
	    VideoDrawSubClipFaded(background,0,0,
		background->Width,background->Height,
		(VideoWidth-background->Width)/2,
		(VideoHeight-background->Height)/2,
		255*i/maxi);
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

    VideoSyncSpeed=old_video_sync;
    SetVideoSync();
}


/**
**	Draw a box with the text inside
*/
local void DrawStatBox(int x,int y,char* text,unsigned color,int percent)
{
    VideoFillRectangleClip(ColorBlack,x,y,80,24);
    VideoDrawRectangleClip(ColorYellow,x+1,y+1,78,22);
    VideoFillRectangleClip(color,x+3,y+3,percent*74/100,18);
    VideoDrawTextCentered(x+40,y+5,LargeFont,text);
}

/**
**	Draw the game stats
*/
local int GameStatsDrawFunc(int frame)
{
    int x;
    int y;
    int dodraw;
    int done;
    const int stats_pause=30;  // Wait one second between each stat
    Player *p;
    int i;
    int c;
    char buf[50];
    int line_spacing;
    int names_font;
    int top_offset;
    int bottom_offset;
    int description_offset;
    int percent;
    int max;
    int draw_all;

#ifdef USE_OPENGL
    draw_all=1;
#else
    draw_all=0;
#endif

#ifndef USE_OPENGL
    // If a button was pressed draw everything
    if( IntroButtonPressed ) {
	draw_all=1;
    }
#endif

    done=0;

    if( !draw_all && (frame%stats_pause)!=0 ) {
	return done;
    }

    percent=100;
    x=TheUI.Offset640X;
    y=TheUI.Offset480Y;
    dodraw=99;
    if( !IntroButtonPressed ) {
	dodraw=frame/stats_pause;
    }

    for( i=0,c=0; i<PlayerMax; i++) {
	if(Players[i].Type==PlayerPerson || Players[i].Type==PlayerComputer) {
	    c++;
	}
    }
    if( c<=4 ) {
	names_font=SmallTitleFont;
	top_offset=57;
	bottom_offset=178;
	description_offset=30;
    }
    else {
	names_font=LargeFont;
	top_offset=6;
	bottom_offset=90;
	description_offset=20;
    }
    line_spacing=(432-bottom_offset-description_offset)/c;

    if( !draw_all || (dodraw<=10 && (frame%stats_pause)==0) ) {
	PlayGameSound(SoundIdForName("fist"),MaxSampleVolume);
    }


    if( dodraw==1 || (draw_all && dodraw>=1) ) {
	char* Outcome;

	VideoDrawTextCentered(x+106,y+top_offset,LargeFont,"Outcome");
	if( GameResult==GameVictory ) {
	    Outcome="Victory!";
	} else {
	    Outcome="Defeat!";
	}
	VideoDrawTextCentered(x+106,y+top_offset+21,LargeTitleFont,Outcome);
    }

    if( dodraw==2 || (draw_all && dodraw>=2) ) {
	char* rank;
	char** ranks;
	int* scores;

	ranks = NULL;	/* .. -Wuninitialized .. */
	scores = NULL;
	for( i=0; RaceWcNames[i]; ++i ) {
	    if( !strcmp(RaceWcNames[i],ThisPlayer->RaceName) ) {
		ranks=Ranks[i].Ranks;
		scores=Ranks[i].Scores;
		break;
	    }
	}
	DebugCheck( !RaceWcNames[i] );

	rank=ranks[0];
	i=0;
	while( 1 ) {
	    if( ThisPlayer->Score<scores[i] || !ranks[i] ) {
		break;
	    }
	    rank=ranks[i];
	    ++i;
	}

	VideoDrawTextCentered(x+324,y+top_offset,LargeFont,"Rank");
	VideoDrawTextCentered(x+324,y+top_offset+21,SmallTitleFont,rank);
    }

    if( dodraw==3 || (draw_all && dodraw>=3) ) {
	VideoDrawTextCentered(x+540,y+top_offset,LargeFont,"Score");
	sprintf(buf,"%u",ThisPlayer->Score);
	VideoDrawTextCentered(x+540,y+top_offset+21,SmallTitleFont,buf);
    }

    if( dodraw==4 || (draw_all && dodraw>=4) ) {
	max=Players[0].TotalUnits;
	for( i=1; i<PlayerMax; i++ ) {
	    p=&Players[i];
	    if( p->Type!=PlayerPerson && p->Type!=PlayerComputer ) {
		continue;
	    }
	    if( p->TotalUnits>max ) {
		max=p->TotalUnits;
	    }
	}
	if( max==0 ) {
	    max=1;
	}

	sprintf(buf,"%s - You",ThisPlayer->Name);
	VideoDrawTextCentered(x+320,y+bottom_offset+description_offset+26,
	                      names_font,buf);
	VideoDrawTextCentered(x+50,y+bottom_offset,LargeFont,"Units");
	sprintf(buf,"%u",ThisPlayer->TotalUnits);
	percent=ThisPlayer->TotalUnits*100/max;
	DrawStatBox(x+10,y+bottom_offset+description_offset,buf,
	            ThisPlayer->Color,percent);
	for( i=0,c=1; i<PlayerMax; i++ ) {
	    p=&Players[i];
	    if( p==ThisPlayer ||
		    (p->Type!=PlayerPerson && p->Type!=PlayerComputer) ) {
		continue;
	    }
	    if( ThisPlayer->Enemy&(1<<i) ) {
		sprintf(buf,"%s - Enemy",p->Name);
	    } else {
		sprintf(buf,"%s - Ally",p->Name);
	    }
	    VideoDrawTextCentered(x+320,y+bottom_offset+description_offset+26+line_spacing*c,
	                          names_font,buf);
	    sprintf(buf,"%u",p->TotalUnits);
	    percent=p->TotalUnits*100/max;
	    DrawStatBox(x+10,y+bottom_offset+description_offset+line_spacing*c,
	                buf,p->Color,percent);
	    c++;
	}
    }

    if( dodraw==5 || (draw_all && dodraw>=5) ) {
	max=Players[0].TotalBuildings;
	for( i=1; i<PlayerMax; i++ ) {
	    p=&Players[i];
	    if( p->Type!=PlayerPerson && p->Type!=PlayerComputer ) {
		continue;
	    }
	    if( p->TotalBuildings>max ) {
		max=p->TotalBuildings;
	    }
	}
	if( max==0 ) {
	    max=1;
	}

	VideoDrawTextCentered(x+140,y+bottom_offset,LargeFont,"Buildings");
	sprintf(buf,"%u",ThisPlayer->TotalBuildings);
	percent=ThisPlayer->TotalBuildings*100/max;
	DrawStatBox(x+100,y+bottom_offset+description_offset,buf,
	            ThisPlayer->Color,percent);
	for( i=0,c=1; i<PlayerMax; i++ ) {
	    p=&Players[i];
	    if( p==ThisPlayer ||
		(p->Type!=PlayerPerson && p->Type!=PlayerComputer) ) {
		continue;
	    }
	    sprintf(buf,"%u",p->TotalBuildings);
	    percent=p->TotalBuildings*100/max;
	    DrawStatBox(x+100,y+bottom_offset+description_offset+line_spacing*c,
	                buf,p->Color,percent);
	    c++;
	}
    }

    if( dodraw==6 || (draw_all && dodraw>=6) ) {
	max=Players[0].TotalResources[GoldCost];
	for( i=1; i<PlayerMax; i++ ) {
	    p=&Players[i];
	    if( p->Type!=PlayerPerson && p->Type!=PlayerComputer ) {
		continue;
	    }
	    if( p->TotalResources[GoldCost]>max ) {
		max=p->TotalResources[GoldCost];
	    }
	}
	if( max==0 ) {
	    max=1;
	}

	VideoDrawTextCentered(x+230,y+bottom_offset,LargeFont,"Gold");
	sprintf(buf,"%u",ThisPlayer->TotalResources[GoldCost]);
	percent=ThisPlayer->TotalResources[GoldCost]*100/max;
	DrawStatBox(x+190,y+bottom_offset+description_offset,buf,
	            ThisPlayer->Color,percent);
	for( i=0,c=1; i<PlayerMax; i++ ) {
	    p=&Players[i];
	    if( p==ThisPlayer ||
		(p->Type!=PlayerPerson && p->Type!=PlayerComputer) ) {
		continue;
	    }
            sprintf(buf,"%u",p->TotalResources[GoldCost]);
	    percent=p->TotalResources[GoldCost]*100/max;
	    DrawStatBox(x+190,y+bottom_offset+description_offset+line_spacing*c,
	                buf,p->Color,percent);
	    c++;
	}
    }

    if( dodraw==7 || (draw_all && dodraw>=7) ) {
	max=Players[0].TotalResources[WoodCost];
	for( i=1; i<PlayerMax; i++ ) {
	    p=&Players[i];
	    if( p->Type!=PlayerPerson && p->Type!=PlayerComputer ) {
		continue;
	    }
	    if( p->TotalResources[WoodCost]>max ) {
		max=p->TotalResources[WoodCost];
	    }
	}
	if( max==0 ) {
	    max=1;
	}

	VideoDrawTextCentered(x+320,y+bottom_offset,LargeFont,"Lumber");
	sprintf(buf,"%u",ThisPlayer->TotalResources[WoodCost]);
	percent=ThisPlayer->TotalResources[WoodCost]*100/max;
	DrawStatBox(x+280,y+bottom_offset+description_offset,buf,
	            ThisPlayer->Color,percent);
	for( i=0,c=1; i<PlayerMax; i++ ) {
	    p=&Players[i];
	    if( p==ThisPlayer ||
		(p->Type!=PlayerPerson && p->Type!=PlayerComputer) ) {
		continue;
	    }
            sprintf(buf,"%u",p->TotalResources[WoodCost]);
	    percent=p->TotalResources[WoodCost]*100/max;
	    DrawStatBox(x+280,y+bottom_offset+description_offset+line_spacing*c,
	                buf,p->Color,percent);
	    c++;
	}
    }

    if( dodraw==8 || (draw_all && dodraw>=8) ) {
	max=Players[0].TotalResources[OilCost];
	for( i=1; i<PlayerMax; i++ ) {
	    p=&Players[i];
	    if( p->Type!=PlayerPerson && p->Type!=PlayerComputer ) {
		continue;
	    }
	    if( p->TotalResources[OilCost]>max ) {
		max=p->TotalResources[OilCost];
	    }
	}
	if( max==0 ) {
	    max=1;
	}

	VideoDrawTextCentered(x+410,y+bottom_offset,LargeFont,"Oil");
	sprintf(buf,"%u",ThisPlayer->TotalResources[OilCost]);
	percent=ThisPlayer->TotalResources[OilCost]*100/max;
	DrawStatBox(x+370,y+bottom_offset+description_offset,buf,
	            ThisPlayer->Color,percent);
	for( i=0,c=1; i<PlayerMax; i++ ) {
	    p=&Players[i];
	    if( p==ThisPlayer ||
		(p->Type!=PlayerPerson && p->Type!=PlayerComputer) ) {
		continue;
	    }
	    sprintf(buf,"%u",p->TotalResources[OilCost]);
	    percent=p->TotalResources[OilCost]*100/max;
	    DrawStatBox(x+370,y+bottom_offset+description_offset+line_spacing*c,
	                buf,p->Color,percent);
	    c++;
	}
    }

    if( dodraw==9 || (draw_all && dodraw>=9) ) {
	max=Players[0].TotalKills;
	for( i=1; i<PlayerMax; i++ ) {
	    p=&Players[i];
	    if( p->Type!=PlayerPerson && p->Type!=PlayerComputer ) {
		continue;
	    }
	    if( p->TotalKills>max ) {
		max=p->TotalKills;
	    }
	}
	if( max==0 ) {
	    max=1;
	}

	VideoDrawTextCentered(x+500,y+bottom_offset,LargeFont,"Kills");
	percent=ThisPlayer->TotalKills*100/max;
	sprintf(buf,"%u",ThisPlayer->TotalKills);
	DrawStatBox(x+460,y+bottom_offset+description_offset,buf,
	            ThisPlayer->Color,percent);
	for( i=0,c=1; i<PlayerMax; i++ ) {
	    p=&Players[i];
	    if( p==ThisPlayer ||
		(p->Type!=PlayerPerson && p->Type!=PlayerComputer) ) {
		continue;
	    }
	    sprintf(buf,"%u",p->TotalKills);
	    percent=p->TotalKills*100/max;
	    DrawStatBox(x+460,y+bottom_offset+description_offset+line_spacing*c,
	                buf,p->Color,percent);
	    c++;
	}
    }

    if( dodraw==10 || (draw_all && dodraw>=10) ) {
	max=Players[0].TotalRazings;
	for( i=1; i<PlayerMax; i++ ) {
	    p=&Players[i];
	    if( p->Type!=PlayerPerson && p->Type!=PlayerComputer ) {
		continue;
	    }
	    if( p->TotalRazings>max ) {
		max=p->TotalRazings;
	    }
	}
	if( max==0 ) {
	    max=1;
	}

	VideoDrawTextCentered(x+590,y+bottom_offset,LargeFont,"Razings");
	sprintf(buf,"%u",ThisPlayer->TotalRazings);
	percent=ThisPlayer->TotalRazings*100/max;
	DrawStatBox(x+550,y+bottom_offset+description_offset,buf,
	            ThisPlayer->Color,percent);
	for( i=0,c=1; i<PlayerMax; i++ ) {
	    p=&Players[i];
	    if( p==ThisPlayer ||
		(p->Type!=PlayerPerson && p->Type!=PlayerComputer) ) {
		continue;
	    }
	    sprintf(buf,"%u",p->TotalRazings);
	    percent=p->TotalRazings*100/max;
	    DrawStatBox(x+550,y+bottom_offset+description_offset+line_spacing*c,
	                buf,p->Color,percent);
	    c++;
	}
	done=1;
    }

    return done;
}

/**
**	Show the game stats
*/
global void ShowStats(void)
{
    EventCallback callbacks;
    int done;
    int old_video_sync;
    Graphic* background;
    int frame;

    old_video_sync=VideoSyncSpeed;
    VideoSyncSpeed=100;
    SetVideoSync();

    callbacks.ButtonPressed=IntroCallbackButton1;
    callbacks.ButtonReleased=IntroCallbackButton2;
    callbacks.MouseMoved=IntroCallbackMouse;
    callbacks.MouseExit=IntroCallbackExit;
    callbacks.KeyPressed=IntroCallbackKey1;
    callbacks.KeyReleased=IntroCallbackKey2;
    callbacks.KeyRepeated=IntroCallbackKey3;
    callbacks.NetworkEvent=NetworkEvent;
    callbacks.SoundReady=WriteSound;

    background=LoadGraphic(MenuBackground);
    ResizeGraphic(background,VideoWidth,VideoHeight);
#ifdef USE_OPENGL
    MakeTexture(background,background->Width,background->Height);
#endif

    VideoLockScreen();
    VideoClearScreen();
    VideoDrawSubClip(background,0,0,background->Width,background->Height,
	(VideoWidth-background->Width)/2,(VideoHeight-background->Height)/2);
    VideoUnlockScreen();

    UseContinueButton=1;
    InitContinueButton(TheUI.Offset640X+455,TheUI.Offset480Y+440);
    GameCursor=TheUI.Point.Cursor;
    DestroyCursorBackground();

    frame=1;
    done=0;
    IntroNoEvent=1;
    IntroButtonPressed=0;
    while( 1 ) {
	VideoLockScreen();
	HideAnyCursor();
#ifdef USE_OPENGL
	VideoDrawSubClip(background,0,0,background->Width,background->Height,
	    (VideoWidth-background->Width)/2,(VideoHeight-background->Height)/2);
	GameStatsDrawFunc(frame);
#else
	if( !done ) {
	    done=GameStatsDrawFunc(frame);
	}
#endif
	DrawContinueButton();
	DrawAnyCursor();
	VideoUnlockScreen();

	Invalidate();
	RealizeVideoMemory();

	if( !IntroNoEvent ) {
	    break;
	}

	WaitEventsOneFrame(&callbacks);
	frame++;
    }

    VideoFree(background);
    VideoSyncSpeed=old_video_sync;
    SetVideoSync();
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

/**
**	Parse the add objective ccl function
**
**	The list contains the objective text followed by an optional number
**	specifying where in the list it should be added.  If no number is
**	given it is added at the end.
*/
local SCM CclAddObjective(SCM list)
{
    int i;
    const char *obj;

    obj=get_c_string(gh_car(list));

    list=gh_cdr(list);
    if( !gh_null_p(list) ) {
	// Optional location number given
	int num;

	num=gh_scm2int(gh_car(list));
	if( num<0 ) {
	    num=0;
	}

	i=0;
	while( i!=MAX_OBJECTIVES && GameIntro.Objectives[i] ) {
	    ++i;
	}
	if( i==MAX_OBJECTIVES ) {
	    fprintf(stderr,"Too many objectives: %s\n",obj);
	    ExitFatal(-1);
	}
	if( num>i ) {
	    num=i;
	}
	for( ; i>num; --i ) {
	    GameIntro.Objectives[i]=GameIntro.Objectives[i-1];
	}
	GameIntro.Objectives[num]=strdup(obj);
    } else {
	// Add objective to the end of the list
	i=0;
	while( i!=MAX_OBJECTIVES && GameIntro.Objectives[i] ) {
	    ++i;
	}
	if( i==MAX_OBJECTIVES ) {
	    fprintf(stderr,"Too many objectives: %s\n",obj);
	    ExitFatal(-1);
	}
	GameIntro.Objectives[i]=strdup(obj);
    }

    return SCM_UNSPECIFIED;
}

/**
**	Parse the remove objective ccl function
*/
local SCM CclRemoveObjective(SCM objective)
{
    int num;

    num=gh_scm2int(objective);
    if( num<0 || num>=MAX_OBJECTIVES ) {
	fprintf(stderr,"remove-objective: Invalid number: %d\n",num);
	ExitFatal(-1);
    }
    if( !GameIntro.Objectives[num] ) {
	fprintf(stderr,"remove-objective: No objective at location: %d\n",num);
	ExitFatal(-1);
    }

    free(GameIntro.Objectives[num]);

    if( num==MAX_OBJECTIVES-1 ) {
	GameIntro.Objectives[num]=NULL;
    }
    for( ; num<MAX_OBJECTIVES-1 && GameIntro.Objectives[num]; ++num ) {
	GameIntro.Objectives[num]=GameIntro.Objectives[num+1];
    }

    return SCM_UNSPECIFIED;
}

/**
**	Parse the define-ranks ccl function
*/
local SCM CclDefineRanks(SCM list)
{
    PlayerRanks *rank;
    const char *race;
    int i;
    int len;

    rank=NULL;
    race=get_c_string(gh_car(list));
    for( i=0; RaceWcNames[i]; ++i ) {
	if( !strcmp(RaceWcNames[i], race) ) {
	    rank=&Ranks[i];
	    break;
	}
    }
    if( !RaceWcNames[i] ) {
	fprintf(stderr,"define-ranks: Invalid race name: %s\n",race);
	ExitFatal(-1);
    }

    if( rank->Ranks ) {
	for( i=0; rank->Ranks[i]; ++i ) {
	    free(rank->Ranks[i]);
	}
	free(rank->Ranks);
	free(rank->Scores);
    }

    list=gh_car(gh_cdr(list));
    len=gh_length(list)/2;

    rank->Ranks=(char**)malloc((len+1)*sizeof(char*));
    rank->Ranks[len]=NULL;
    rank->Scores=(int*)malloc(len*sizeof(int));

    i=0;
    while( !gh_null_p(list) ) {
	rank->Scores[i]=gh_scm2int(gh_car(list));
	list=gh_cdr(list);
	rank->Ranks[i]=gh_scm2newstr(gh_car(list),NULL);
	list=gh_cdr(list);
	++i;
    }

    return SCM_UNSPECIFIED;
}

/**
**	Register CCL functions for objectives
*/
global void ObjectivesCclRegister(void)
{
    gh_new_procedureN("add-objective",CclAddObjective);
    gh_new_procedure1_0("remove-objective",CclRemoveObjective);
    gh_new_procedureN("define-ranks",CclDefineRanks);
}

/**
**	Save the objectives.
*/
global void SaveObjectives(FILE *file)
{
    int i;
    for( i=0; i<MAX_OBJECTIVES && GameIntro.Objectives[i]; ++i ) {
	fprintf(file,"(add-objective \"%s\")\n",GameIntro.Objectives[i]);
    }
}

//@}
