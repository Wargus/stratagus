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

	// FIXME: draw Continue button

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

    x=(VideoWidth-640)/2;
    y=(VideoHeight-480)/2;
    IntroNoEvent=1;
    line=0;
    scrolling=1;
    while( IntroNoEvent ) {

	VideoLockScreen();

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

	// FIXME: draw Continue button

	VideoUnlockScreen();

	// FIXME: update only the changed area!!!!

	Invalidate();
	RealizeVideoMemory();

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

/**
**	Show picture.
**
**	@param name	Path file name of the picture.
*/
global void ShowPicture(const char* name)
{
    EventCallback callbacks;
    Graphic* background;
    int x;
    int y;

    VideoLockScreen();
    VideoClearScreen();
    VideoUnlockScreen();

    callbacks.ButtonPressed=IntroCallbackButton1;
    callbacks.ButtonReleased=IntroCallbackButton2;
    callbacks.MouseMoved=IntroCallbackMouse;
    callbacks.MouseExit=IntroCallbackExit;
    callbacks.KeyPressed=IntroCallbackKey1;
    callbacks.KeyReleased=IntroCallbackKey2;

    callbacks.NetworkEvent=NetworkEvent;
    callbacks.SoundReady=WriteSound;

    background=LoadGraphic(name);

    x=(VideoWidth-640)/2;
    IntroNoEvent=1;
    while( IntroNoEvent ) {
	y=(VideoHeight-480)/2;

	VideoLockScreen();
	//
	//	Draw background
	//
	VideoDrawSubClip(background,0,0,
		background->Width,background->Height,
		(VideoWidth-background->Width)/2,
		(VideoHeight-background->Height)/2);

	VideoUnlockScreen();

	Invalidate();
	RealizeVideoMemory();

	WaitEventsOneFrame(&callbacks);
    }

    VideoFree(background);

    VideoLockScreen();
    VideoClearScreen();
    VideoUnlockScreen();
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
