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

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

local int IntroNoEvent;			/// Flag got an event.

/**
**	Callback for input.
*/
local void IntroCallbackButton1(unsigned dummy __attribute__((unused)))
{
    IntroNoEvent=0;
}

/**
**	Callback for input.
*/
local void IntroCallbackButton2(unsigned dummy __attribute__((unused)))
{
}

/**
**	Callback for input.
*/
local void IntroCallbackKey1(unsigned dummy1 __attribute__((unused)),
	unsigned dummy2 __attribute__((unused)))
{
    IntroNoEvent=0;
}

/**
**	Callback for input.
*/
local void IntroCallbackKey2(unsigned dummy1 __attribute__((unused)),
	unsigned dummy2 __attribute__((unused)))
{
}

/**
**	Callback for input.
*/
local void IntroCallbackMouse(int dummy_x __attribute__((unused)),
	int dummy_y __attribute__((unused)))
{
}

/**
**	Callback for exit.
*/
local void IntroCallbackExit(void)
{
    DebugLevel3Fn("Exit\n");
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
*/
local void ScrollText(int x,int y,int w,int h,int i,TextLines *lines)
{
    int miny,endy;
    TextLines* ptr;

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

    PopClipping();
}

/**
**	Show level intro.
**
**	First version - only testing.
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

    VideoLockScreen();
    VideoClearScreen();
    VideoUnlockScreen();
    Invalidate();
    RealizeVideoMemory();

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
	exit(-1);
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

	// FIXME: draw Continue button

	VideoUnlockScreen();

	// FIXME: update only the changed area!!!!

	Invalidate();
	RealizeVideoMemory();

	WaitEventsOneFrame(&callbacks);
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
    VideoFillRectangle(ColorBlack,0,0,VideoWidth,VideoHeight);
    VideoUnlockScreen();
    Invalidate();
    RealizeVideoMemory();


    CallbackMusicOn();
    // FIXME: should this be GameMusic?
    PlayMusic(MenuMusic);
}

//@}
