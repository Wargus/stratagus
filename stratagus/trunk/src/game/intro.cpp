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


/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

global Intro GameIntro;			/// Game intro
global Credits	GameCredits;		/// Game credits

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

local int IntroNoEvent;			/// Flag got an event
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
    int c;
    CLFile* file;
    char buf[1024];
    int stage;
    TextLines* ScrollingText;
    TextLines* ObjectivesText[MAX_OBJECTIVES];
    int OldVideoSyncSpeed;

    UseContinueButton=1;
    InitContinueButton(TheUI.Offset640X+455,TheUI.Offset480Y+440);
    GameCursor=TheUI.Point.Cursor;
    DestroyCursorBackground();

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
    callbacks.KeyRepeated=IntroCallbackKey3;

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

    x=TheUI.Offset640X;
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

	DrawContinueButton();
	DrawAnyCursor();

	VideoUnlockScreen();

	// FIXME: update only the changed area!!!!

	Invalidate();
	RealizeVideoMemory();

	if( !IntroNoEvent )
	    break;
	WaitEventsOneFrame(&callbacks);
	if( c==0 ) {
	    c=1;
	}
	else {
	    c=0;
	    ++line;
	}
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
    callbacks.KeyRepeated=IntroCallbackKey3;

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
    DestroyCursorBackground();

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
    callbacks.KeyRepeated=IntroCallbackKey3;

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
**	Draw a box with the text inside
*/
local void DrawStatBox(int x,int y,char* text,unsigned color,float percent)
{
    VideoFillRectangleClip(ColorBlack,x,y,80,24);
    VideoDrawRectangleClip(ColorYellow,x+1,y+1,78,22);
    VideoFillRectangleClip(color,x+3,y+3,(int)(percent*74),18);
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
    const int StatsPause=30;  // Wait one second between each stat
    Player *p;
    int i;
    int c;
    char buf[50];
    int LineSpacing;
    int NamesFont;
    int TopOffset;
    int BottomOffset;
    int DescriptionOffset;
    float percent;
    float max;

    percent=1.0;
    done=0;

    if( (frame%StatsPause)!=0 )
	return done;

    x=TheUI.Offset640X;
    y=TheUI.Offset480Y;
    dodraw=frame/StatsPause;

    for( i=0,c=0; i<PlayerMax; i++) {
	if(Players[i].Type==PlayerPerson || Players[i].Type==PlayerComputer)
	    c++;
    }
    if( c<=4 ) {
	NamesFont=SmallTitleFont;
	TopOffset=57;
	BottomOffset=178;
	DescriptionOffset=30;
    }
    else {
	NamesFont=LargeFont;
	TopOffset=6;
	BottomOffset=90;
	DescriptionOffset=20;
    }
    LineSpacing=(432-BottomOffset-DescriptionOffset)/c;

    PlayGameSound(SoundIdForName("fist"),MaxSampleVolume);


    if( dodraw==1 ) {
	char* Outcome;

	VideoDrawTextCentered(x+106,y+TopOffset,LargeFont,"Outcome");
	if( GameResult==GameVictory )
	    Outcome="Victory!";
	else
	    Outcome="Defeat!";
	VideoDrawTextCentered(x+106,y+TopOffset+21,LargeTitleFont,Outcome);
    }

    else if( dodraw==2 ) {
	// FIXME: Use ccl
	char* Rank;
	char** Ranks;
	char* HumanRanks[] = {
	    "Servant", "Peasant", "Squire", "Footman", "Corporal",
	    "Sergeant", "Lieutenant", "Captain", "Major", "Knight",
	    "General", "Admiral", "Marshall", "Lord", "Grand Admiral",
	    "Highlord", "Thundergod", "God", "Designer"
	};
	char* OrcRanks[] = {
	    "Slave", "Peon", "Rogue", "Grunt", "Slasher",
	    "Marauder", "Commander", "Captain", "Major", "Knight",
	    "General", "Master", "Marshall", "Chieftain", "Overlord",
	    "War Chief", "Demigod", "God", "Designer"
	};
	unsigned RankScores[] = {
	    2000, 5000, 8000, 18000, 28000,
	    40000, 55000, 70000, 85000, 105000,
	    125000, 145000, 165000, 185000, 205000,
	    230000, 255000, 280000, -1
	};

	if( ThisPlayer->Race==PlayerRaceHuman )
	    Ranks=HumanRanks;
	else
	    Ranks=OrcRanks;

	Rank = NULL;
	for( i=0; i<sizeof(RankScores); i++ ) {
	    if( ThisPlayer->Score<RankScores[i] ) {
		Rank=Ranks[i];
		break;
	    }
	}
	if( !Rank )
	    Rank="No Rank";

	VideoDrawTextCentered(x+324,y+TopOffset,LargeFont,"Rank");
	VideoDrawTextCentered(x+324,y+TopOffset+21,SmallTitleFont,Rank);
    }

    else if( dodraw==3 ) {
	VideoDrawTextCentered(x+540,y+TopOffset,LargeFont,"Score");
	sprintf(buf,"%u",ThisPlayer->Score);
	VideoDrawTextCentered(x+540,y+TopOffset+21,SmallTitleFont,buf);
    }

    else if( dodraw==4 ) {
	max=Players[0].TotalUnits;
	for( i=1; i<PlayerMax; i++ ) {
	    p=&Players[i];
	    if( p->Type!=PlayerPerson && p->Type!=PlayerComputer )
		continue;
	    if( p->TotalUnits>max )
		max=p->TotalUnits;
	}
	if( (int)max==0 )
	    max=1.0;

	sprintf(buf,"%s - You",ThisPlayer->Name);
	VideoDrawTextCentered(x+320,y+BottomOffset+DescriptionOffset+26,
	                      NamesFont,buf);
	VideoDrawTextCentered(x+50,y+BottomOffset,LargeFont,"Units");
	sprintf(buf,"%u",ThisPlayer->TotalUnits);
	percent=ThisPlayer->TotalUnits/max;
	DrawStatBox(x+10,y+BottomOffset+DescriptionOffset,buf,
	            ThisPlayer->Color,percent);
	for( i=0,c=1; i<PlayerMax; i++ ) {
	    p=&Players[i];
	    if( p==ThisPlayer || 
		(p->Type!=PlayerPerson && p->Type!=PlayerComputer) ) {
		continue;
	    }
	    sprintf(buf,"%s - Enemy",p->Name);
	    VideoDrawTextCentered(x+320,y+BottomOffset+DescriptionOffset+26+LineSpacing*c,
	                          NamesFont,buf);
	    sprintf(buf,"%u",p->TotalUnits);
	    percent=p->TotalUnits/max;
	    DrawStatBox(x+10,y+BottomOffset+DescriptionOffset+LineSpacing*c,
	                buf,p->Color,percent);
	    c++;
	}
    }

    else if( dodraw==5 ) {
	max=Players[0].TotalBuildings;
	for( i=1; i<PlayerMax; i++ ) {
	    p=&Players[i];
	    if( p->Type!=PlayerPerson && p->Type!=PlayerComputer )
		continue;
	    if( p->TotalBuildings>max )
		max=p->TotalBuildings;
	}
	if( (int)max==0 )
	    max=1.0;

	VideoDrawTextCentered(x+140,y+BottomOffset,LargeFont,"Buildings");
	sprintf(buf,"%u",ThisPlayer->TotalBuildings);
	percent=ThisPlayer->TotalBuildings/max;
	DrawStatBox(x+100,y+BottomOffset+DescriptionOffset,buf,
	            ThisPlayer->Color,percent);
	for( i=0,c=1; i<PlayerMax; i++ ) {
	    p=&Players[i];
	    if( p==ThisPlayer ||
		(p->Type!=PlayerPerson && p->Type!=PlayerComputer) ) {
		continue;
	    }
	    sprintf(buf,"%u",p->TotalBuildings);
	    percent=p->TotalBuildings/max;
	    DrawStatBox(x+100,y+BottomOffset+DescriptionOffset+LineSpacing*c,
	                buf,p->Color,percent);
	    c++;
	}
    }

    else if( dodraw==6 ) {
	max=Players[0].TotalResources[GoldCost];
	for( i=1; i<PlayerMax; i++ ) {
	    p=&Players[i];
	    if( p->Type!=PlayerPerson && p->Type!=PlayerComputer )
		continue;
	    if( p->TotalResources[GoldCost]>max )
		max=p->TotalResources[GoldCost];
	}
	if( (int)max==0 )
	    max=1.0;

	VideoDrawTextCentered(x+230,y+BottomOffset,LargeFont,"Gold");
	sprintf(buf,"%u",ThisPlayer->TotalResources[GoldCost]);
	percent=ThisPlayer->TotalResources[GoldCost]/max;
	DrawStatBox(x+190,y+BottomOffset+DescriptionOffset,buf,
	            ThisPlayer->Color,percent);
	for( i=0,c=1; i<PlayerMax; i++ ) {
	    p=&Players[i];
	    if( p==ThisPlayer ||
		(p->Type!=PlayerPerson && p->Type!=PlayerComputer) ) {
		continue;
	    }
            sprintf(buf,"%u",p->TotalResources[GoldCost]);
	    percent=p->TotalResources[GoldCost]/max;
	    DrawStatBox(x+190,y+BottomOffset+DescriptionOffset+LineSpacing*c,
	                buf,p->Color,percent);
	    c++;
	}
    }

    else if( dodraw==7 ) {
	max=Players[0].TotalResources[WoodCost];
	for( i=1; i<PlayerMax; i++ ) {
	    p=&Players[i];
	    if( p->Type!=PlayerPerson && p->Type!=PlayerComputer )
		continue;
	    if( p->TotalResources[WoodCost]>max )
		max=p->TotalResources[WoodCost];
	}
	if( (int)max==0 )
	    max=1.0;

	VideoDrawTextCentered(x+320,y+BottomOffset,LargeFont,"Lumber");
	sprintf(buf,"%u",ThisPlayer->TotalResources[WoodCost]);
	percent=ThisPlayer->TotalResources[WoodCost]/max;
	DrawStatBox(x+280,y+BottomOffset+DescriptionOffset,buf,
	            ThisPlayer->Color,percent);
	for( i=0,c=1; i<PlayerMax; i++ ) {
	    p=&Players[i];
	    if( p==ThisPlayer ||
		(p->Type!=PlayerPerson && p->Type!=PlayerComputer) ) {
		continue;
	    }
            sprintf(buf,"%u",p->TotalResources[WoodCost]);
	    percent=p->TotalResources[WoodCost]/max;
	    DrawStatBox(x+280,y+BottomOffset+DescriptionOffset+LineSpacing*c,
	                buf,p->Color,percent);
	    c++;
	}
    }

    else if( dodraw==8 ) {
	max=Players[0].TotalResources[OilCost];
	for( i=1; i<PlayerMax; i++ ) {
	    p=&Players[i];
	    if( p->Type!=PlayerPerson && p->Type!=PlayerComputer )
		continue;
	    if( p->TotalResources[OilCost]>max )
		max=p->TotalResources[OilCost];
	}
	if( (int)max==0 )
	    max=1.0;

	VideoDrawTextCentered(x+410,y+BottomOffset,LargeFont,"Oil");
	sprintf(buf,"%u",ThisPlayer->TotalResources[OilCost]);
	percent=ThisPlayer->TotalResources[OilCost]/max;
	DrawStatBox(x+370,y+BottomOffset+DescriptionOffset,buf,
	            ThisPlayer->Color,percent);
	for( i=0,c=1; i<PlayerMax; i++ ) {
	    p=&Players[i];
	    if( p==ThisPlayer ||
		(p->Type!=PlayerPerson && p->Type!=PlayerComputer) ) {
		continue;
	    }
	    sprintf(buf,"%u",p->TotalResources[OilCost]);
	    percent=p->TotalResources[OilCost]/max;
	    DrawStatBox(x+370,y+BottomOffset+DescriptionOffset+LineSpacing*c,
	                buf,p->Color,percent);
	    c++;
	}
    }

    else if( dodraw==9 ) {
	max=Players[0].TotalKills;
	for( i=1; i<PlayerMax; i++ ) {
	    p=&Players[i];
	    if( p->Type!=PlayerPerson && p->Type!=PlayerComputer )
		continue;
	    if( p->TotalKills>max )
		max=p->TotalKills;
	}
	if( (int)max==0 )
	    max=1.0;

	VideoDrawTextCentered(x+500,y+BottomOffset,LargeFont,"Kills");
	percent=ThisPlayer->TotalKills/max;
	sprintf(buf,"%u",ThisPlayer->TotalKills);
	DrawStatBox(x+460,y+BottomOffset+DescriptionOffset,buf,
	            ThisPlayer->Color,percent);
	for( i=0,c=1; i<PlayerMax; i++ ) {
	    p=&Players[i];
	    if( p==ThisPlayer ||
		(p->Type!=PlayerPerson && p->Type!=PlayerComputer) ) {
		continue;
	    }
	    sprintf(buf,"%u",p->TotalKills);
	    percent=p->TotalKills/max;
	    DrawStatBox(x+460,y+BottomOffset+DescriptionOffset+LineSpacing*c,
	                buf,p->Color,percent);
	    c++;
	}
    }

    else if( dodraw==10 ) {
	max=Players[0].TotalRazings;
	for( i=1; i<PlayerMax; i++ ) {
	    p=&Players[i];
	    if( p->Type!=PlayerPerson && p->Type!=PlayerComputer )
		continue;
	    if( p->TotalRazings>max )
		max=p->TotalRazings;
	}
	if( (int)max==0 )
	    max=1.0;

	VideoDrawTextCentered(x+590,y+BottomOffset,LargeFont,"Razings");
	sprintf(buf,"%u",ThisPlayer->TotalRazings);
	percent=ThisPlayer->TotalRazings/max;
	DrawStatBox(x+550,y+BottomOffset+DescriptionOffset,buf,
	            ThisPlayer->Color,percent);
	for( i=0,c=1; i<PlayerMax; i++ ) {
	    p=&Players[i];
	    if( p==ThisPlayer ||
		(p->Type!=PlayerPerson && p->Type!=PlayerComputer) ) {
		continue;
	    }
	    sprintf(buf,"%u",p->TotalRazings);
	    percent=p->TotalRazings/max;
	    DrawStatBox(x+550,y+BottomOffset+DescriptionOffset+LineSpacing*c,
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
    int OldVideoSyncSpeed;
    Graphic* background;
    int frame;


    OldVideoSyncSpeed=VideoSyncSpeed;
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
    VideoCreatePalette(GlobalPalette);

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
    while( 1 ) {
	VideoLockScreen();
	HideAnyCursor();
	if( !done ) {
	    done=GameStatsDrawFunc(frame);
	}
	DrawContinueButton();
	DrawAnyCursor();
	VideoUnlockScreen();

	Invalidate();
	RealizeVideoMemory();

	if( !IntroNoEvent )
	    break;

	WaitEventsOneFrame(&callbacks);
	frame++;
    }

    VideoSyncSpeed=OldVideoSyncSpeed;
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

//@}
