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

/*----------------------------------------------------------------------------
--	Declarations
----------------------------------------------------------------------------*/

#define MAX_OBJECTIVES 9

typedef struct _intro_ {
    char*	Title;				/// Intro title
    char*	Background;			/// Background picture
    char*	TextFile;			/// Intro text file
    char*	VoiceFile1;			/// Intro voice file
    char*	VoiceFile2;			/// Intro voice file
    char*	Objectives[MAX_OBJECTIVES];	/// Objectives
} Intro;					/// Intro definition

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

local Intro MyIntro[1] = {
{
    "I. Welcome to FreeCraft",
    "campaigns/human/interface/introscreen1.png",
    "campaigns/human/level01h.txt.gz",
    "campaigns/human/level01h-intro1.wav.gz",
    "campaigns/human/level01h-intro2.wav.gz",
    {
	"- This is a dummy intro",
	"- Kill all enemies",
	"- Be the lonly surivor",
    },
}
};

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
**	Scroll text.
**
**	@param x	x start pixel screen position.
**	@param y	y start pixel screen position.
**	@param i	scroll index.
**	@param text	Text to display.
**
**	@note this is very slow, a faster way can be written.
*/
local void ScrollText(int x,int y,int i,const char* text)
{
    char* s;
    int l;

    l=0;
    //
    //	Split the text into lines.
    //
    for( ; y<250; ) {
	char* s1;

	s=strdup(text+l);
	if( (s1=strchr(s,'\n')) ) {
	    *s1='\0';
	}
	for( ;; ) {
	    if( VideoTextLength(LargeFont,s)<370 ) {
		break;
	    }
	    *strrchr(s,' ')='\0';
	}

	if( i<24 ) {
	    // FIXME: must clip the text
	    VideoDrawText(x,y-i,LargeFont,s);
	    y+=24;
	} else {
	    i-=24;
	}
	l+=strlen(s);
	free(s);
	if( !text[l] ) {
	    break;
	}
	++l;
    }
}

/**
**	Show level intro.
**
**	First version - only testing.
*/
global void ShowIntro(void)
{
    EventCallback callbacks;
    Graphic* background;
    char* text;
    int line;
    int i;
    int l;
    int y;
    const Intro* intro;
    CLFile* file;
    char buf[1024];
    int stage;

    intro=MyIntro;

    VideoLockScreen();
    VideoFillRectangle(ColorBlack,0,0,VideoWidth,VideoHeight);
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
    //VideoSetPalette(background->Pixels);

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
    PlayFile(intro->VoiceFile1);

    stage=line=0;
    IntroNoEvent=1;
    while( IntroNoEvent ) {
	if( !PlayingMusic && !stage && intro->VoiceFile2 ) {
	    PlayFile(intro->VoiceFile2);
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
	VideoDrawTextCentered(424,28,LargeFont,intro->Title);
	//
	//	Draw scrolling text
	//
	PushClipping();
	SetClipping(268,80,640-1,480-1);
	ScrollText(268,80,line,text);
	PopClipping();
	//
	//	Draw objectives
	//
	VideoDrawText(372,306,LargeFont,"Objectives:");
	y=330;
	for( i=0; i<MAX_OBJECTIVES && intro->Objectives[i]; ++i ) {
	    char buf[1024];

	    l=0;
	    //
	    //	Split the text into lines.
	    //
	    for( ;; ) {
		strncpy(buf,intro->Objectives[i]+l,sizeof(buf));
		buf[sizeof(buf)-1]='\0';
		for( ;; ) {
		    if( VideoTextLength(LargeFont,buf)<260 ) {
			break;
		    }
		    *strrchr(buf,' ')='\0';
		}

		VideoDrawText(372,y,LargeFont,buf);
		y+=22;
		l+=strlen(buf);
		if( !intro->Objectives[i][l] ) {
		    break;
		}
		++l;
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

    free(text);

    VideoLockScreen();
    VideoFillRectangle(ColorBlack,0,0,VideoWidth,VideoHeight);
    VideoUnlockScreen();
    Invalidate();
    RealizeVideoMemory();

    CallbackMusicOn();
    // FIXME: should make it configurable
    PlayMusic("music/default.mod");
}

//@}
