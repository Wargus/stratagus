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
/**@name intro.c - The game intros. */
//
//      (c) Copyright 2002-2005 by Lutz Sammer and Jimmy Salmon.
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

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"
#include "iolib.h"
#include "video.h"
#include "font.h"
#include "network.h"
#include "sound_server.h"
#include "sound.h"
#include "settings.h"
#include "script.h"
#include "campaign.h"
#include "cursor.h"
#include "menus.h"
#include "interface.h"
#include "ui.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

/**
**  Linked list struct used to split text up into lines
*/
typedef struct TextLines {
	char* Text;              /// Line of text
	struct TextLines* Next;  /// Pointer to next line
} TextLines;

/**
**  Player ranks
*/
typedef struct PlayerRanks {
	char** Ranks;  /// Array of ranks
	int* Scores;   /// Array of scores
} PlayerRanks;

/**
**  Linked list of TextLines
*/
typedef struct ChapterTextLines {
	struct TextLines* Text;         /// TextLines struct
	struct ChapterTextLines* Next;  /// Pointer to next TextLines
} ChapterTextLines;

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

Intro GameIntro;              /// Game intro
Credits GameCredits;          /// Game credits
static PlayerRanks Ranks[PlayerMax];  /// Ranks

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

static int IntroNoEvent;         /// Flag got an event
static int IntroButtonPressed;   /// Button pressed
static int UseContinueButton;    /// Handle continue button
static int ContinueButtonX;      /// Continue button position X
static int ContinueButtonY;      /// Continue button position Y
static int ContinueButtonFlags;  /// Flags for continue button

/**
**  Callback for input.
*/
static void IntroCallbackButton1(unsigned button)
{
	if (UseContinueButton) {
		if ((1 << button) == LeftButton && ContinueButtonX <= CursorX &&
				CursorX <= ContinueButtonX + 106 &&
				ContinueButtonY <= CursorY &&
				CursorY <= ContinueButtonY + 27) {
			ContinueButtonFlags |= MI_FLAGS_CLICKED;
		}
	} else {
		IntroNoEvent = 0;
	}
	IntroButtonPressed = 1;
}

/**
**  Callback for input.
*/
static void IntroCallbackButton2(unsigned button)
{
	if (UseContinueButton) {
		if ((1 << button) == LeftButton && ContinueButtonX <= CursorX &&
				CursorX <= ContinueButtonX + 106 &&
				ContinueButtonY <= CursorY &&
				CursorY <= ContinueButtonY + 27 &&
				(ContinueButtonFlags & MI_FLAGS_CLICKED)) {
			IntroNoEvent = 0;
		}
		ContinueButtonFlags &= ~MI_FLAGS_CLICKED;
	}
}

/**
**  Callback for input.
*/
static void IntroCallbackKey1(unsigned key, unsigned keychar)
{
	HandleKeyModifiersDown(key,keychar);

	if (UseContinueButton) {
		if (keychar == 'c' || keychar == '\r') {
			ContinueButtonFlags |= MI_FLAGS_CLICKED;
		}
	} else {
		IntroNoEvent = 0;
	}
	IntroButtonPressed = 1;
}

/**
**  Callback for input.
*/
static void IntroCallbackKey2(unsigned key, unsigned keychar)
{
	HandleKeyModifiersDown(key, keychar);

	if (UseContinueButton) {
		if ((key == 'c' || key == '\r') &&
				(ContinueButtonFlags & MI_FLAGS_CLICKED)) {
			IntroNoEvent = 0;
			ContinueButtonFlags &= ~MI_FLAGS_CLICKED;
		}
	}
}

/**
**  Callback for input.
*/
static void IntroCallbackKey3(unsigned key __attribute__((unused)),
	unsigned keychar __attribute__((unused)))
{
}

/**
**  Callback for input.
*/
static void IntroCallbackMouse(int x, int y)
{
	CursorX = x;
	CursorY = y;

	if (UseContinueButton) {
		if (ContinueButtonX <= CursorX && CursorX <= ContinueButtonX + 106 &&
				ContinueButtonY <= CursorY && CursorY <= ContinueButtonY + 27) {
			ContinueButtonFlags |= MI_FLAGS_ACTIVE;
		}
		else {
			ContinueButtonFlags &= ~MI_FLAGS_ACTIVE;
		}
	}
}

/**
**  Callback for exit.
*/
static void IntroCallbackExit(void)
{
}

/**
**  Draws a continue button at x,y
*/
static void DrawContinueButton(void)
{
	DrawMenuButton(FindButtonStyle("gm-half"), ContinueButtonFlags,
		ContinueButtonX, ContinueButtonY, "~!Continue");
}

/**
**  Init continue button.
**
**  @param x  X screen pixel position of continue button.
**  @param y  Y screen pixel position of continue button.
*/
static void InitContinueButton(int x, int y)
{
	ContinueButtonX = x;
	ContinueButtonY = y;
	ContinueButtonFlags = MI_FLAGS_SELECTED;
}

/**
**  Splits text up into a linked list of lines less than a given width.
**
**  @param text   The text to be split up.
**  @param w      Maximum width of a line.
**  @param lines  Pointer to linked list structure.
*/
static void SplitTextIntoLines(const char* text, int w, TextLines** lines)
{
	int l;
	char* s;
	char* str;
	char* s1;
	TextLines** ptr;

	l = 0;
	s = str = strdup(text);
	ptr = lines;

	// Convert \r, \r\n, and \n\r to \n
	s1 = s;
	while (s1) {
		char* x1;
		char* x2;

		if ((s1 = strpbrk(s1, "\n\r"))) {
			if ((s1[0] == '\n' && s1[1] == '\r') ||
					(s1[0] == '\r' && s1[1] == '\n')) {
				x1 = s1 + 1;
				x2 = s1 + 2;
				while ((*x1++ = *x2++)) {
				}
			}
			*s1++ = '\n';
		}
	}

	for (;;) {
		char* space;

		if ((s1 = strchr(s, '\n'))) {
			*s1 = '\0';
		}
		space = NULL;
		for (;;) {
			if (VideoTextLength(LargeFont, s) < w) {
				break;
			}
			s1 = strrchr(s, ' ');
			if (!s1) {
				fprintf(stderr, "line too long: \"%s\"\n", s);
				break;
			}
			if (space) {
				*space = ' ';
			}
			space = s1;
			*space = '\0';
		}

		*ptr = malloc(sizeof(TextLines));
		(*ptr)->Text = strdup(s);
		(*ptr)->Next = NULL;
		ptr = &((*ptr)->Next);

		l += strlen(s);
		if (!text[l]) {
			break;
		}
		++l;
		s = str + l;
	}

	free(str);
}

/**
**  Frees memory in a TextLines struct
**
**  @param lines  Address of the pointer to free
*/
static void FreeTextLines(TextLines** lines)
{
	TextLines* ptr;

	while (*lines) {
		ptr = (*lines)->Next;
		free((*lines)->Text);
		free(*lines);
		*lines = ptr;
	}
}

/**
**  Scroll text.
**
**  @param x      x start pixel screen position.
**  @param y      y start pixel screen position.
**  @param w      width of text area
**  @param h      height of text area
**  @param i      scroll index.
**  @param lines  Lines of the text to display.
**
**  @return       1 if there is more to scroll, 0 if it is done
*/
static int ScrollText(int x, int y, int w, int h, int i, TextLines* lines)
{
	int miny;
	int endy;
	TextLines* ptr;
	int scrolling;

	scrolling = 1;

	PushClipping();
	SetClipping(x, y, x + w, y + h);

	miny = y - 24;
	endy = y + h;
	y = endy - i;
	ptr = lines;

	while (y < endy) {
		if (!ptr) {
			break;
		}

		if (y >= miny) {
			VideoDrawTextClip(x, y, LargeFont, ptr->Text);
		}
		y += 24;

		ptr = ptr->Next;
	}

	if (y < miny + 24) {
		scrolling = 0;
	}

	PopClipping();

	return scrolling;
}

/**
**  Show level intro.
**
**  @param intro  Intro struct
*/
void ShowIntro(const Intro* intro)
{
	EventCallback callbacks;
	Graphic* background;
	char* text;
	int line;
	int i;
	int l;
	int y;
	int c;
	CLFile* file;
	char buf[1024];
	int stage;
	TextLines* scrolling_text;
	TextLines* objectives_text[MAX_OBJECTIVES];
	int old_video_sync;
	int soundfree;
	int soundout;
	int soundcount;

	UseContinueButton = 1;
	InitContinueButton(455 * VideoWidth / 640, 440 * VideoHeight / 480);
	GameCursor = TheUI.Point.Cursor;

	VideoClearScreen();

	old_video_sync = VideoSyncSpeed;
	VideoSyncSpeed = 100;
	SetVideoSync();

	callbacks.ButtonPressed = IntroCallbackButton1;
	callbacks.ButtonReleased = IntroCallbackButton2;
	callbacks.MouseMoved = IntroCallbackMouse;
	callbacks.MouseExit = IntroCallbackExit;
	callbacks.KeyPressed = IntroCallbackKey1;
	callbacks.KeyReleased = IntroCallbackKey2;
	callbacks.KeyRepeated = IntroCallbackKey3;
	callbacks.NetworkEvent = NetworkEvent;

	background = NewGraphic(intro->Background, 0, 0);
	LoadGraphic(background);
	ResizeGraphic(background, VideoWidth, VideoHeight);

	LibraryFileName(intro->TextFile, buf);
	if (!(file = CLopen(buf, CL_OPEN_READ))) {
		fprintf(stderr, "Can't open file `%s'\n", intro->TextFile);
		ExitFatal(-1);
	}
	l = 0;
	text = malloc(8192);
	while ((i = CLread(file, text + l, 8192)) == 8192) {
		l += 8192;
		text = realloc(text, l + 8192);
	}
	text[l + i] = '\0';
	l += i + 1;
	text = realloc(text, l);
	CLclose(file);

	CallbackMusicOff();
	PlaySectionMusic(PlaySectionBriefing);

	soundfree = -1;
	soundout = -1;
	soundcount = 0;
	if (intro->VoiceFile[0]) {
		soundfree = NextFreeChannel;
		soundout = NextSoundRequestOut;
		PlaySoundFile(intro->VoiceFile[0]);
	}

	SplitTextIntoLines(text, 320, &scrolling_text);
	for (i = 0; i < MAX_OBJECTIVES; ++i) {
		if (intro->Objectives[i]) {
			SplitTextIntoLines(intro->Objectives[i], 260 * VideoWidth / 640,
				&objectives_text[i]);
		} else {
			objectives_text[i] = NULL;
		}
	}

	line = 0;
	stage = 1;
	IntroNoEvent = 1;
	c = 0;
	while (1) {
		// FIXME: move sound specific code to the sound files
		if (soundfree != -1 && (!Channels[soundfree].Command) &&
				stage < MAX_BRIEFING_VOICES && soundout != NextSoundRequestOut &&
				intro->VoiceFile[stage]) {
			// FIXME: is there a better way to do this?
			if (soundcount == 15) {
				soundfree = NextFreeChannel;
				soundout = NextSoundRequestOut;
				PlaySoundFile(intro->VoiceFile[stage]);
				++stage;
				soundcount = 0;
			} else {
				++soundcount;
			}
		}

		//
		// Draw background
		//
		VideoDrawSubClip(background, 0, 0,
			background->Width, background->Height,
			(VideoWidth - background->Width) / 2,
			(VideoHeight - background->Height) / 2);
		//
		// Draw title
		//
		SetDefaultTextColors(FontWhite, FontYellow);
		VideoDrawTextCentered((70 + 340) / 2 * VideoWidth / 640,
			28 * VideoHeight / 480, LargeFont, intro->Title);
		//
		// Draw scrolling text
		//
		ScrollText(70 * VideoWidth / 640, 80 * VideoHeight / 480,
			70 * VideoWidth / 640 + 320, 170 * VideoHeight / 480, line, scrolling_text);

		//
		// Draw objectives
		//
		y = 306 * VideoHeight / 480;
		VideoDrawText(372 * VideoWidth / 640, y, LargeFont, "Objectives:");
		y += 30;
		for (i = 0; i < MAX_OBJECTIVES && objectives_text[i]; ++i) {
			TextLines* ptr;

			ptr = objectives_text[i];
			while (ptr) {
				VideoDrawText(372 * VideoWidth / 640, y, LargeFont, ptr->Text);
				y += 22;
				ptr = ptr->Next;
			}
		}

		SetDefaultTextColors(FontYellow, FontWhite);
		DrawContinueButton();
		DrawCursor();

		Invalidate();
		RealizeVideoMemory();

		if (!IntroNoEvent) {
			break;
		}
		WaitEventsOneFrame(&callbacks);
		if (c == 0) {
			c = 1;
		} else {
			c = 0;
			++line;
		}
	}

	FreeTextLines(&scrolling_text);
	for (i = 0; i < MAX_OBJECTIVES; ++i) {
		if (objectives_text[i]) {
			FreeTextLines(&objectives_text[i]);
		}
	}

	free(text);
	FreeGraphic(background);

	VideoClearScreen();

	VideoSyncSpeed = old_video_sync;
	SetVideoSync();

	if (soundfree != -1 && Channels[soundfree].Command) {
		FreeOneChannel(soundfree);
	}

	CallbackMusicOn();
}

/**
**  Show the credits
**
**  @param credits  Credits structure
*/
void ShowCredits(Credits* credits)
{
	EventCallback callbacks;
	Graphic* background;
	int line;
	int x;
	int y;
	int scrolling;
	TextLines* scrolling_credits;
	int old_video_sync;

	VideoClearScreen();

	old_video_sync = VideoSyncSpeed;
	VideoSyncSpeed = 100;
	SetVideoSync();

	callbacks.ButtonPressed = IntroCallbackButton1;
	callbacks.ButtonReleased = IntroCallbackButton2;
	callbacks.MouseMoved = IntroCallbackMouse;
	callbacks.MouseExit = IntroCallbackExit;
	callbacks.KeyPressed = IntroCallbackKey1;
	callbacks.KeyReleased = IntroCallbackKey2;
	callbacks.KeyRepeated = IntroCallbackKey3;
	callbacks.NetworkEvent = NetworkEvent;

	background = NULL;
	if (credits->Background) {
		background = NewGraphic(credits->Background, 0, 0);
		LoadGraphic(background);
		ResizeGraphic(background, VideoWidth, VideoHeight);
	}

	// play different music?

	scrolling_credits = NULL;
	if (credits->Names) {
		SplitTextIntoLines(credits->Names, 320, &scrolling_credits);
	}

	UseContinueButton = 1;
	InitContinueButton(TheUI.Offset640X + 455, TheUI.Offset480Y + 440);
	GameCursor = TheUI.Point.Cursor;

	x = TheUI.Offset640X;
	y = TheUI.Offset480Y;
	IntroNoEvent = 1;
	line = 0;
	scrolling = 1;
	while (1) {
		//
		// Draw background
		//
		if (background) {
			VideoDrawSubClip(background, 0, 0,
				background->Width, background->Height,
				(VideoWidth - background->Width) / 2,
				(VideoHeight - background->Height) / 2);
		}

		//
		// Draw scrolling text
		//
		if (scrolling_credits) {
			scrolling = ScrollText(x + 140, y + 80, 320, 275, line, scrolling_credits);
		}

		DrawContinueButton();
		DrawCursor();

		Invalidate();
		RealizeVideoMemory();

		if (!IntroNoEvent) {
			break;
		}

		WaitEventsOneFrame(&callbacks);

		++line;

		// Loop if we're done scrolling
		if (!scrolling) {
			line = 0;
		}
	}

	if (scrolling_credits) {
		FreeTextLines(&scrolling_credits);
	}

	FreeGraphic(background);

	VideoClearScreen();

	VideoSyncSpeed = old_video_sync;
	SetVideoSync();

#if 0
	CallbackMusicOn();
	// FIXME: should this be GameMusic?
	PlayMusic(MenuMusic);
#endif
}

/**
**  Draw text
*/
static void PictureDrawText(CampaignChapter* chapter, ChapterTextLines* chlines)
{
	ChapterPictureText* text;
	TextLines* lines;
	int x;
	int y;
	int (*draw)(int, int, unsigned, const unsigned char*);

	text = chapter->Data.Picture.Text;
	while (text) {
		if (text->Align == PictureTextAlignLeft) {
			draw = VideoDrawText;
		} else {
			draw = VideoDrawTextCentered;
		}
		x = text->X * VideoWidth / 640;
		y = text->Y * VideoHeight / 480;
		lines = chlines->Text;
		while (lines) {
			draw(x, y, text->Font, lines->Text);
			y += text->Height;
			lines = lines->Next;
		}
		text = text->Next;
		chlines = chlines->Next;
	}
}

/**
**  Show picture.
**
**  @param chapter  Pointer to CampaignChapter to show
*/
void ShowPicture(CampaignChapter* chapter)
{
	EventCallback callbacks;
	Graphic* background;
	int old_video_sync;
	int max;
	int i;
	int j;
	ChapterTextLines* lines;
	ChapterTextLines** linesptr;
	ChapterPictureText* text;

	UseContinueButton = 0;

	old_video_sync = VideoSyncSpeed;
	VideoSyncSpeed = 100;
	SetVideoSync();

	callbacks.ButtonPressed = IntroCallbackButton1;
	callbacks.ButtonReleased = IntroCallbackButton2;
	callbacks.MouseMoved = IntroCallbackMouse;
	callbacks.MouseExit = IntroCallbackExit;
	callbacks.KeyPressed = IntroCallbackKey1;
	callbacks.KeyReleased = IntroCallbackKey2;
	callbacks.KeyRepeated = IntroCallbackKey3;
	callbacks.NetworkEvent = NetworkEvent;

	background = NewGraphic(chapter->Data.Picture.Image, 0, 0);
	LoadGraphic(background);
	ResizeGraphic(background, VideoWidth, VideoHeight);
	IntroNoEvent = 1;

	text = chapter->Data.Picture.Text;
	linesptr = &lines;
	while (text) {
		(*linesptr) = calloc(sizeof(ChapterTextLines), 1);
		SplitTextIntoLines(text->Text, text->Width, &(*linesptr)->Text);
		linesptr = &((*linesptr)->Next);
		text = text->Next;
	}

	//
	// Fade in background and title
	//
	i = 0;
	max = chapter->Data.Picture.FadeIn;
	while (IntroNoEvent && i < max) {
		VideoClearScreen();
		VideoDrawSubClipTrans(background, 0, 0,
			background->Width, background->Height,
			(VideoWidth - background->Width) / 2,
			(VideoHeight - background->Height) / 2,
			255 * i / max);
		PictureDrawText(chapter, lines);

		Invalidate();
		RealizeVideoMemory();

		WaitEventsOneFrame(&callbacks);
		++i;
	}
	i = chapter->Data.Picture.FadeOut * i / (max ? max : 1);

	//
	// Draw background and title
	//
	j = 0;
	max = chapter->Data.Picture.DisplayTime;
	while (IntroNoEvent && j < max) {
		VideoDrawSubClip(background, 0, 0,
			background->Width, background->Height,
			(VideoWidth - background->Width) / 2,
			(VideoHeight - background->Height) / 2);
		PictureDrawText(chapter, lines);

		Invalidate();
		RealizeVideoMemory();

		WaitEventsOneFrame(&callbacks);
		++j;
	}

	//
	// Fade out background and title
	//
	max = chapter->Data.Picture.FadeOut;
	while (i >= 0) {
		VideoClearScreen();
		VideoDrawSubClipTrans(background, 0, 0,
			background->Width, background->Height,
			(VideoWidth - background->Width) / 2,
			(VideoHeight - background->Height) / 2,
			255 * i / max);
		PictureDrawText(chapter, lines);

		Invalidate();
		RealizeVideoMemory();

		WaitEventsOneFrame(&callbacks);
		--i;
	}

	FreeGraphic(background);

	while (lines) {
		ChapterTextLines* ptr;

		ptr = lines->Next;
		FreeTextLines(&lines->Text);
		free(lines);
		lines = ptr;
	}

	VideoClearScreen();

	VideoSyncSpeed = old_video_sync;
	SetVideoSync();
}


/**
**  Draw a box with the text inside
*/
static void DrawStatBox(int x, int y, char* text, Uint32 color, int percent)
{
	VideoFillRectangleClip(ColorBlack, x, y, 80, 24);
	VideoDrawRectangleClip(ColorYellow, x + 1, y + 1, 78, 22);
	VideoFillRectangleClip(color, x + 3, y + 3, percent * 74 / 100, 18);
	VideoDrawTextCentered(x + 40, y + 5, LargeFont, text);
}

/**
**  Draw the game stats
*/
static int GameStatsDrawFunc(int frame)
{
	int x;
	int y;
	int dodraw;
	int done;
	const int stats_pause = 30;  // Wait one second between each stat
	Player* p;
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

	done = 0;

	percent = 100;
	x = TheUI.Offset640X;
	y = TheUI.Offset480Y;
	dodraw = 99;
	if (!IntroButtonPressed) {
		dodraw = frame / stats_pause;
	}

	for (i = 0, c = 0; i < PlayerMax - 1; ++i) {
		if (Players[i].Type != PlayerNobody) {
			++c;
		}
	}
	if (c <= 4) {
		names_font = SmallTitleFont;
		top_offset = 57;
		bottom_offset = 178;
		description_offset = 30;
	} else {
		names_font = LargeFont;
		top_offset = 6;
		bottom_offset = 90;
		description_offset = 20;
	}
	line_spacing = (432 - bottom_offset - description_offset) / c;

	if (dodraw <= 10 && (frame % stats_pause) == 0) {
		PlayGameSound(SoundIdForName("statsthump"), MaxSampleVolume);
	}


	if (dodraw >= 1) {
		char* outcome;

		VideoDrawTextCentered(x + 106, y + top_offset, LargeFont, "Outcome");
		if (GameResult == GameVictory) {
			outcome = "Victory!";
		} else {
			outcome = "Defeat!";
		}
		VideoDrawTextCentered(x + 106, y + top_offset + 21, LargeTitleFont,
			outcome);
	}

	if (dodraw >= 2) {
		char* rank;
		char** ranks;
		int* scores;

		ranks = NULL;
		scores = NULL;
		for (i = 0; i < PlayerRaces.Count; ++i) {
			if (!strcmp(PlayerRaces.Name[i], PlayerRaces.Name[ThisPlayer->Race])) {
				ranks = Ranks[i].Ranks;
				scores = Ranks[i].Scores;
				break;
			}
		}
		Assert(i != PlayerRaces.Count);

		rank = ranks[0];
		i = 0;
		while (1) {
			if (ThisPlayer->Score < scores[i] || !ranks[i]) {
				break;
			}
			rank = ranks[i];
			++i;
		}

		VideoDrawTextCentered(x + 324, y + top_offset, LargeFont, "Rank");
		VideoDrawTextCentered(x + 324, y + top_offset + 21, SmallTitleFont, rank);
	}

	if (dodraw >= 3) {
		VideoDrawTextCentered(x + 540, y + top_offset, LargeFont, "Score");
		sprintf(buf, "%u", ThisPlayer->Score);
		VideoDrawTextCentered(x + 540, y + top_offset + 21, SmallTitleFont, buf);
	}

	if (dodraw >= 4) {
		max = Players[0].TotalUnits;
		for (i = 1; i < PlayerMax - 1; ++i) {
			p = &Players[i];
			if (p->Type == PlayerNobody) {
				continue;
			}
			if (p->TotalUnits > max) {
				max = p->TotalUnits;
			}
		}
		if (max == 0) {
			max = 1;
		}

		sprintf(buf, "%s - You", ThisPlayer->Name);
		VideoDrawTextCentered(x + 320, y + bottom_offset + description_offset + 26,
			names_font, buf);
		VideoDrawTextCentered(x + 50, y + bottom_offset, LargeFont, "Units");
		sprintf(buf, "%u", ThisPlayer->TotalUnits);
		percent = ThisPlayer->TotalUnits * 100 / max;
		DrawStatBox(x + 10, y + bottom_offset + description_offset, buf,
					ThisPlayer->Color, percent);
		for (i = 0, c = 1; i < PlayerMax - 1; ++i) {
			p = &Players[i];
			if (p == ThisPlayer || p->Type == PlayerNobody) {
				continue;
			}
			if (ThisPlayer->Enemy & (1 << i)) {
				sprintf(buf, "%s - Enemy", p->Name);
			} else if (ThisPlayer->Allied & (1 << i)) {
				sprintf(buf, "%s - Ally", p->Name);
			} else {
				sprintf(buf, "%s - Neutral", p->Name);
			}
			VideoDrawTextCentered(x + 320, y + bottom_offset + description_offset + 26 + line_spacing * c,
				names_font, buf);
			sprintf(buf, "%u", p->TotalUnits);
			percent = p->TotalUnits * 100 / max;
			DrawStatBox(x + 10, y + bottom_offset + description_offset + line_spacing * c,
				buf, p->Color, percent);
			++c;
		}
	}

	if (dodraw >= 5) {
		max = Players[0].TotalBuildings;
		for (i = 1; i < PlayerMax - 1; ++i) {
			p = &Players[i];
			if (p->Type == PlayerNobody) {
				continue;
			}
			if (p->TotalBuildings > max) {
				max = p->TotalBuildings;
			}
		}
		if (max == 0) {
			max = 1;
		}

		VideoDrawTextCentered(x + 140, y + bottom_offset, LargeFont, "Buildings");
		sprintf(buf, "%u", ThisPlayer->TotalBuildings);
		percent = ThisPlayer->TotalBuildings * 100 / max;
		DrawStatBox(x + 100, y + bottom_offset + description_offset, buf,
			ThisPlayer->Color, percent);
		for (i = 0, c = 1; i < PlayerMax - 1; ++i) {
			p = &Players[i];
			if (p == ThisPlayer || p->Type == PlayerNobody) {
				continue;
			}
			sprintf(buf, "%u", p->TotalBuildings);
			percent = p->TotalBuildings * 100 / max;
			DrawStatBox(x + 100, y + bottom_offset + description_offset + line_spacing * c,
				buf, p->Color, percent);
			++c;
		}
	}

	if (dodraw >= 6) {
		max = Players[0].TotalResources[GoldCost];
		for (i = 1; i < PlayerMax - 1; ++i) {
			p = &Players[i];
			if (p->Type == PlayerNobody) {
				continue;
			}
			if (p->TotalResources[GoldCost] > max) {
				max = p->TotalResources[GoldCost];
			}
		}
		if (max == 0) {
			max = 1;
		}

		VideoDrawTextCentered(x + 230, y + bottom_offset, LargeFont, DefaultResourceNames[GoldCost]);
		sprintf(buf, "%u", ThisPlayer->TotalResources[GoldCost]);
		percent = ThisPlayer->TotalResources[GoldCost] * 100 / max;
		DrawStatBox(x + 190, y + bottom_offset + description_offset, buf,
			ThisPlayer->Color, percent);
		for (i = 0, c = 1; i < PlayerMax - 1; ++i) {
			p = &Players[i];
			if (p == ThisPlayer || p->Type == PlayerNobody) {
				continue;
			}
			sprintf(buf, "%u", p->TotalResources[GoldCost]);
			percent = p->TotalResources[GoldCost] * 100 / max;
			DrawStatBox(x + 190, y + bottom_offset + description_offset + line_spacing * c,
				buf, p->Color, percent);
			++c;
		}
	}

	if (dodraw >= 7) {
		max = Players[0].TotalResources[WoodCost];
		for (i = 1; i < PlayerMax - 1; ++i) {
			p = &Players[i];
			if (p->Type == PlayerNobody) {
				continue;
			}
			if (p->TotalResources[WoodCost] > max) {
				max = p->TotalResources[WoodCost];
			}
		}
		if (max == 0) {
			max = 1;
		}

		VideoDrawTextCentered(x + 320, y + bottom_offset, LargeFont, DefaultResourceNames[WoodCost]);
		sprintf(buf, "%u", ThisPlayer->TotalResources[WoodCost]);
		percent = ThisPlayer->TotalResources[WoodCost] * 100 / max;
		DrawStatBox(x + 280, y + bottom_offset + description_offset, buf,
			ThisPlayer->Color, percent);
		for (i = 0, c = 1; i < PlayerMax - 1; ++i) {
			p = &Players[i];
			if (p == ThisPlayer || p->Type == PlayerNobody) {
				continue;
			}
			sprintf(buf, "%u", p->TotalResources[WoodCost]);
			percent = p->TotalResources[WoodCost] * 100 / max;
			DrawStatBox(x + 280, y + bottom_offset + description_offset + line_spacing * c,
				buf, p->Color,percent);
			++c;
		}
	}

	if (dodraw >= 8) {
		max = Players[0].TotalResources[OilCost];
		for (i = 1; i < PlayerMax - 1; ++i) {
			p = &Players[i];
			if (p->Type == PlayerNobody) {
				continue;
			}
			if (p->TotalResources[OilCost] > max) {
				max = p->TotalResources[OilCost];
			}
		}
		if (max == 0) {
			max = 1;
		}

		VideoDrawTextCentered(x + 410, y + bottom_offset, LargeFont, DefaultResourceNames[OilCost]);
		sprintf(buf, "%u", ThisPlayer->TotalResources[OilCost]);
		percent = ThisPlayer->TotalResources[OilCost] * 100 / max;
		DrawStatBox(x + 370, y + bottom_offset + description_offset, buf,
			ThisPlayer->Color, percent);
		for (i = 0, c = 1; i < PlayerMax - 1; ++i) {
			p = &Players[i];
			if (p == ThisPlayer || p->Type == PlayerNobody) {
				continue;
			}
			sprintf(buf, "%u", p->TotalResources[OilCost]);
			percent = p->TotalResources[OilCost] * 100 / max;
			DrawStatBox(x + 370, y + bottom_offset + description_offset + line_spacing * c,
				buf, p->Color,percent);
			++c;
		}
	}

	if (dodraw >= 9) {
		max = Players[0].TotalKills;
		for (i = 1; i < PlayerMax - 1; ++i) {
			p = &Players[i];
			if (p->Type == PlayerNobody) {
				continue;
			}
			if (p->TotalKills > max) {
				max = p->TotalKills;
			}
		}
		if (max == 0) {
			max = 1;
		}

		VideoDrawTextCentered(x + 500, y + bottom_offset, LargeFont, "Kills");
		percent = ThisPlayer->TotalKills * 100 / max;
		sprintf(buf, "%u", ThisPlayer->TotalKills);
		DrawStatBox(x + 460, y + bottom_offset + description_offset, buf,
			ThisPlayer->Color, percent);
		for (i = 0, c = 1; i < PlayerMax - 1; ++i) {
			p = &Players[i];
			if (p == ThisPlayer || p->Type == PlayerNobody) {
				continue;
			}
			sprintf(buf, "%u", p->TotalKills);
			percent = p->TotalKills * 100 / max;
			DrawStatBox(x + 460, y + bottom_offset + description_offset + line_spacing * c,
				buf, p->Color,percent);
			++c;
		}
	}

	if (dodraw >= 10) {
		max = Players[0].TotalRazings;
		for (i = 1; i < PlayerMax - 1; ++i) {
			p = &Players[i];
			if (p->Type == PlayerNobody) {
				continue;
			}
			if (p->TotalRazings > max) {
				max = p->TotalRazings;
			}
		}
		if (max == 0) {
			max = 1;
		}

		VideoDrawTextCentered(x + 590, y + bottom_offset, LargeFont, "Razings");
		sprintf(buf, "%u", ThisPlayer->TotalRazings);
		percent = ThisPlayer->TotalRazings * 100 / max;
		DrawStatBox(x + 550, y + bottom_offset + description_offset, buf,
			ThisPlayer->Color, percent);
		for (i = 0, c = 1; i < PlayerMax - 1; ++i) {
			p = &Players[i];
			if (p == ThisPlayer || p->Type == PlayerNobody) {
				continue;
			}
			sprintf(buf, "%u", p->TotalRazings);
			percent = p->TotalRazings * 100 / max;
			DrawStatBox(x + 550, y + bottom_offset + description_offset + line_spacing * c,
				buf, p->Color, percent);
			++c;
		}
		done = 1;
	}

	return done;
}

/**
**  Show the game stats
*/
void ShowStats(void)
{
	EventCallback callbacks;
	Graphic* g;
	int done;
	int old_video_sync;
	int frame;

	old_video_sync = VideoSyncSpeed;
	VideoSyncSpeed = 100;
	SetVideoSync();

	callbacks.ButtonPressed = IntroCallbackButton1;
	callbacks.ButtonReleased = IntroCallbackButton2;
	callbacks.MouseMoved = IntroCallbackMouse;
	callbacks.MouseExit = IntroCallbackExit;
	callbacks.KeyPressed = IntroCallbackKey1;
	callbacks.KeyReleased = IntroCallbackKey2;
	callbacks.KeyRepeated = IntroCallbackKey3;
	callbacks.NetworkEvent = NetworkEvent;

	VideoClearScreen();

	g = NULL;
	if (GameResult == GameVictory) {
		if (TheUI.VictoryBackgroundG) {
			g = TheUI.VictoryBackgroundG;
		}
	} else {
		if (TheUI.DefeatBackgroundG) {
			g = TheUI.DefeatBackgroundG;
		}
	}
	if (g) {
		LoadGraphic(g);
		ResizeGraphic(g, VideoWidth, VideoHeight);
	}

	UseContinueButton = 1;
	InitContinueButton(TheUI.Offset640X + 455, TheUI.Offset480Y + 440);
	GameCursor = TheUI.Point.Cursor;

	frame = 1;
	done = 0;
	IntroNoEvent = 1;
	IntroButtonPressed = 0;
	while (1) {
		if (g) {
			VideoDrawSubClip(g, 0, 0,
				g->Width, g->Height,
				(VideoWidth - g->Width) / 2,
				(VideoHeight - g->Height) / 2);
		}
		GameStatsDrawFunc(frame);
		DrawContinueButton();
		DrawCursor();

		Invalidate();
		RealizeVideoMemory();

		if (!IntroNoEvent) {
			break;
		}

		WaitEventsOneFrame(&callbacks);
		++frame;
	}

	VideoSyncSpeed = old_video_sync;
	SetVideoSync();
}

/**
**  Free Ccl Credits Memory
*/
void CleanCclCredits(void)
{
	if (GameCredits.Background) {
		free(GameCredits.Background);
		GameCredits.Background = NULL;
	}
	if (GameCredits.Names) {
		free(GameCredits.Names);
		GameCredits.Names = NULL;
	}
}

/**
**  Parse the credits configuration.
**
**  @param l  Lua state.
**
**  @todo  'comment and 'title are only parsed, but not used.
*/
static int CclCredits(lua_State* l)
{
	const char* value;
	const char* n;
	int nlen;
	int len;
	int args;
	int j;

	if (GameCredits.Background) {
		free(GameCredits.Background);
	}
	GameCredits.Background = NULL;
	if (GameCredits.Names) {
		free(GameCredits.Names);
		GameCredits.Names = malloc(1);
		GameCredits.Names[0] = '\0';
	}
	len = 0;

	args = lua_gettop(l);
	for (j = 0; j < args; ++j) {
		value = LuaToString(l, j + 1);
		++j;
		if (!strcmp(value, "background")) {
			GameCredits.Background = strdup(LuaToString(l, j + 1));
		} else if (!strcmp(value, "name") ||
				!strcmp(value, "title") ||
				!strcmp(value, "comment")) {
			n = LuaToString(l, j + 1);
			nlen = strlen(n);
			GameCredits.Names = realloc(GameCredits.Names, len + nlen + 2);
			if (len != 0) {
				GameCredits.Names[len++] = '\n';
			}
			strcpy(GameCredits.Names + len, n);
			len += nlen;
		}
	}

	return 0;
}

/**
**  Register CCL features for credits.
*/
void CreditsCclRegister(void)
{
	GameCredits.Background = NULL;
	GameCredits.Names = NULL;
	lua_register(Lua, "Credits", CclCredits);
}

/**
**  Parse the add objective ccl function
**
**  The list contains the objective text followed by an optional number
**  specifying where in the list it should be added.  If no number is
**  given it is added at the end.
*/
static int CclAddObjective(lua_State* l)
{
	int i;
	const char* obj;
	int args;
	int j;

	args = lua_gettop(l);
	j = 0;

	if (args != 1 && args != 2) {
		LuaError(l, "incorrect argument");
	}

	obj = LuaToString(l, j + 1);
	++j;

	if (j != args) {
		// Optional location number given
		int num;

		num = LuaToNumber(l, j + 1);
		if (num < 0) {
			num = 0;
		}

		i = 0;
		while (i != MAX_OBJECTIVES && GameIntro.Objectives[i]) {
			++i;
		}
		if (i == MAX_OBJECTIVES) {
			LuaError(l, "Too many objectives: %s" _C_ obj);
		}
		if (num > i) {
			num = i;
		}
		for (; i > num; --i) {
			GameIntro.Objectives[i] = GameIntro.Objectives[i - 1];
		}
		GameIntro.Objectives[num] = strdup(obj);
	} else {
		// Add objective to the end of the list
		i = 0;
		while (i != MAX_OBJECTIVES && GameIntro.Objectives[i]) {
			++i;
		}
		if (i == MAX_OBJECTIVES) {
			LuaError(l, "Too many objectives: %s" _C_ obj);
		}
		GameIntro.Objectives[i] = strdup(obj);
	}

	return 0;
}

/**
**  Parse the remove objective ccl function
*/
static int CclRemoveObjective(lua_State* l)
{
	int num;

	LuaCheckArgs(l, 1);

	num = LuaToNumber(l, 1);
	if (num < 0 || num >= MAX_OBJECTIVES) {
		LuaError(l, "remove-objective: Invalid number: %d" _C_ num);
	}
	if (!GameIntro.Objectives[num]) {
		LuaError(l, "remove-objective: No objective at location: %d" _C_ num);
	}

	free(GameIntro.Objectives[num]);

	if (num == MAX_OBJECTIVES - 1) {
		GameIntro.Objectives[num] = NULL;
	}
	for (; num < MAX_OBJECTIVES - 1 && GameIntro.Objectives[num]; ++num) {
		GameIntro.Objectives[num] = GameIntro.Objectives[num + 1];
	}

	return 0;
}

/**
**  Set the objectives
*/
static int CclSetObjectives(lua_State* l)
{
	int i;
	int args;
	int j;

	// Clean old objectives
	for (i = 0; i < MAX_OBJECTIVES && GameIntro.Objectives[i]; ++i) {
		free(GameIntro.Objectives[i]);
		GameIntro.Objectives[i] = NULL;
	}

	args = lua_gettop(l);
	for (j = 0; j < args; ++j) {
		GameIntro.Objectives[j] = strdup(LuaToString(l, j + 1));
	}

	return 0;
}

/**
**  Parse the define-ranks ccl function
*/
static int CclDefineRanks(lua_State* l)
{
	PlayerRanks* rank;
	const char* race;
	int i;
	int j;
	int len;
	int args;

	LuaCheckArgs(l, 2);
	rank = NULL;
	race = LuaToString(l, 1);
	for (i = 0; i < PlayerRaces.Count; ++i) {
		if (!strcmp(PlayerRaces.Name[i], race)) {
			rank = &Ranks[i];
			break;
		}
	}
	if (i == PlayerRaces.Count) {
		LuaError(l, "Invalid race name: %s" _C_ race);
	}

	if (rank->Ranks) {
		for (i = 0; rank->Ranks[i]; ++i) {
			free(rank->Ranks[i]);
		}
		free(rank->Ranks);
		free(rank->Scores);
	}

	args = luaL_getn(l, 2);
	len = args / 2;

	rank->Ranks = malloc((len + 1) * sizeof(char*));
	rank->Ranks[len] = NULL;
	rank->Scores = malloc(len * sizeof(int));

	i = 0;
	for (j = 0; j < args; ++j) {
		lua_rawgeti(l, 2, j + 1);
		rank->Scores[i] = LuaToNumber(l, -1);
		lua_pop(l, 1);
		++j;
		lua_rawgeti(l, 2, j + 1);
		rank->Ranks[i] = strdup(LuaToString(l, -1));
		lua_pop(l, 1);
		++i;
	}

	return 0;
}

/**
**  Register CCL functions for objectives
*/
void ObjectivesCclRegister(void)
{
	lua_register(Lua, "AddObjective", CclAddObjective);
	lua_register(Lua, "RemoveObjective", CclRemoveObjective);
	lua_register(Lua, "SetObjectives", CclSetObjectives);
	lua_register(Lua, "DefineRanks", CclDefineRanks);
}

/**
**  Save the objectives.
**
**  @param file  Output file.
*/
void SaveObjectives(CLFile* file)
{
	int i;

	if (GameIntro.Objectives[0]) {
		CLprintf(file, "SetObjectives(");
		for (i = 0; i < MAX_OBJECTIVES && GameIntro.Objectives[i]; ++i) {
			if (i) {
				CLprintf(file, ",");
			}
			CLprintf(file, "\n  \"%s\"", GameIntro.Objectives[i]);
		}
		CLprintf(file, ")\n");
	}
}

//@}
