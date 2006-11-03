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
/**@name menu_proc.cpp - The menu processing code. */
//
//      (c) Copyright 1999-2006 by Andreas Arens, Jimmy Salmon, Nehal Mistry
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

#include "stratagus.h"

#include "video.h"
#include "font.h"
#include "menus.h"
#include "netconnect.h"
#include "sound.h"

/*----------------------------------------------------------------------------
-- Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
-- Menu operation functions
----------------------------------------------------------------------------*/

/**
**  Draw menu button 'button' on x,y
**
**  @param style  Button style
**  @param flags  State of Button (clicked, mouse over...)
**  @param x      X display position
**  @param y      Y display position
**  @param text   text to print on button
*/
void DrawMenuButton(ButtonStyle *style, unsigned flags, int x, int y,
	const char *text)
{
	const char *nc;
	const char *rc;
	const char *oldnc;
	const char *oldrc;
	int i;
	ButtonStyleProperties *p;
	ButtonStyleProperties *pimage;

	if (flags & MI_FLAGS_DISABLED) {
		p = &style->Disabled;
	} else if (flags & MI_FLAGS_CLICKED) {
		p = &style->Clicked;
	} else if (flags & MI_FLAGS_ACTIVE) {
		p = &style->Hover;
	} else if (flags & MI_FLAGS_SELECTED) {
		p = &style->Selected;
	} else {
		p = &style->Default;
	}

	//
	//  Image
	//
	pimage = p;
	if (!p->Sprite && !(flags & MI_FLAGS_INVISIBLE)) {
		// No image.  Try hover, selected, then default
		if ((flags & MI_FLAGS_ACTIVE) && style->Hover.Sprite) {
			pimage = &style->Hover;
		} else if ((flags & MI_FLAGS_SELECTED) && style->Selected.Sprite) {
			pimage = &style->Selected;
		} else if (style->Default.Sprite) {
			pimage = &style->Default;
		}
	}
	if (pimage->Sprite) {
		pimage->Sprite->Load();
	}
	if (pimage->Sprite) {
		pimage->Sprite->DrawFrame(pimage->Frame, x, y);
	}

	//
	//  Text
	//
	if (text && text[0]) {
		GetDefaultTextColors(&oldnc, &oldrc);
		nc = p->TextNormalColor ? p->TextNormalColor :
			style->TextNormalColor ? style->TextNormalColor : oldnc;
		rc = p->TextReverseColor ? p->TextReverseColor :
			style->TextReverseColor ? style->TextReverseColor : oldrc;
		SetDefaultTextColors(nc, rc);

		if (p->TextAlign == TextAlignCenter || p->TextAlign == TextAlignUndefined) {
			VideoDrawTextCentered(x + p->TextX, y + p->TextY,
				style->Font, text);
		} else if (p->TextAlign == TextAlignLeft) {
			VideoDrawText(x + p->TextX, y + p->TextY, style->Font, text);
		} else {
			VideoDrawText(x + p->TextX - style->Font->Width(text), y + p->TextY,
				style->Font, text);
		}

		SetDefaultTextColors(oldnc, oldrc);
	}

	//
	//  Border
	//
	if (!p->BorderSize) {
		// No border, check if there's a Selected border
		if ((flags & MI_FLAGS_SELECTED) && style->Selected.BorderSize) {
			p = &style->Selected;
		}
	}
	if (!p->BorderColor) {
		p->BorderColor = Video.MapRGB(TheScreen->format,
			p->BorderColorRGB.r, p->BorderColorRGB.g, p->BorderColorRGB.b);
	}
	if (p->BorderSize) {
		for (i = 0; i < p->BorderSize; ++i) {
			Video.DrawRectangleClip(p->BorderColor, x - i, y - i,
				style->Width + 2 * i, style->Height + 2 * i);
		}
	}
}

#if 0
/**
** Paste text from the clipboard
*/
static void PasteFromClipboard(Menuitem *mi)
{
#if defined(USE_WIN32) || defined(_XLIB_H_)
	int i;
	unsigned char *clipboard;
#ifdef USE_WIN32
	HGLOBAL handle;
#elif defined(_XLIB_H_)
	Display *display;
	Window window;
	Atom rettype;
	unsigned long nitem;
	unsigned long dummy;
	int retform;
	XEvent event;
#endif

#ifdef USE_WIN32
	if (!IsClipboardFormatAvailable(CF_TEXT) || !OpenClipboard(NULL)) {
		return;
	}
	handle = GetClipboardData(CF_TEXT);
	if (!handle) {
		CloseClipboard();
		return;
	}
	clipboard = (unsigned char *)GlobalLock(handle);
	if (!clipboard) {
		CloseClipboard();
		return;
	}
#elif defined(_XLIB_H_)
	if (!(display = XOpenDisplay(NULL))) {
		return;
	}

	// Creates a non maped temporary X window to hold the selection
	if (!(window = XCreateSimpleWindow(display,
			DefaultRootWindow(display), 0, 0, 1, 1, 0, 0, 0))) {
		XCloseDisplay(display);
		return;
	}

	XConvertSelection(display, XA_PRIMARY, XA_STRING, XA_STRING,
		window, CurrentTime);

	XNextEvent(display, &event);

	if (event.type != SelectionNotify ||
			event.xselection.property != XA_STRING) {
		return;
	}

	XGetWindowProperty(display, window, XA_STRING, 0, 1024, False,
		XA_STRING, &rettype, &retform, &nitem, &dummy, &clipboard);

	XDestroyWindow(display, window);
	XCloseDisplay(display);

	if (rettype != XA_STRING || retform != 8) {
		if (clipboard != NULL) {
			XFree(clipboard);
		}
		clipboard = NULL;
	}

	if (clipboard == NULL) {
		return;
	}
#endif
	for (i = 0; mi->D.Input.nch < mi->D.Input.maxch && clipboard[i] &&
			mi->Font->Width(mi->D.Input.buffer) + 8 < mi->D.Input.xsize; ++i) {
		if (clipboard[i] >= 32 && clipboard[i] != '~') {
			mi->D.Input.buffer[mi->D.Input.nch] = clipboard[i];
			++mi->D.Input.nch;
		}
	}
	strcpy(mi->D.Input.buffer + mi->D.Input.nch, "~!_");
#ifdef USE_WIN32
	GlobalUnlock(handle);
	CloseClipboard();
#elif defined(_XLIB_H_)
	if (clipboard != NULL) {
		XFree(clipboard);
	}
#endif
#endif
}
#endif

#if 0
/**
** Process a menu.
**
** @param menu_id The menu number to process
** @param loop Indicates to setup handlers and really 'Process'
**
** @todo FIXME: This function is called from the event handler!!
*/
void ProcessMenu(const char *menu_id, int loop)
{
	int oldncr;

	CancelBuildingMode();

	// Recursion protection:
	if (loop) {
		InterfaceState = IfaceStateMenu;
	}

	ButtonUnderCursor = -1;

	if (loop) {
		while (CurrentMenu != NULL) {
			CheckMusicFinished();

			InterfaceState = IfaceStateNormal;
			UpdateDisplay();
			InterfaceState = IfaceStateMenu;

			RealizeVideoMemory();
			oldncr = NetConnectRunning;
//			WaitEventsOneFrame(&MenuCallbacks);
			if (NetConnectRunning == 2) {
				NetworkProcessClientRequest();
			}
			if (NetConnectRunning == 1) {
				NetworkProcessServerRequest();
			}
			// stopped by network activity?
			if (oldncr == 2 && NetConnectRunning == 0) {
				//menu->NetAction();
			}
		}
	}
}
#endif

//@}
