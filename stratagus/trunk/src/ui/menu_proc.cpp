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
/**@name menu_proc.c - The menu processing code. */
//
//      (c) Copyright 1999-2004 by Andreas Arens, Jimmy Salmon, Nehal Mistry
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
--		Includes
----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"

#ifdef _MSC_VER
#undef NOUSER
#endif
#include "video.h"
#include "font.h"
#include "interface.h"
#include "menus.h"
#include "cursor.h"
#include "network.h"
#include "netconnect.h"
#include "ui.h"
#include "sound_server.h"
#include "sound.h"
#include "script.h"
#include "campaign.h"

#ifdef USE_WIN32
#ifdef _MSC_VER
#undef NOUSER
#endif
#define DrawIcon WinDrawIcon
#define EndMenu WinEndMenu
#include <windows.h>
#undef EndMenu
#undef DrawIcon
#endif

#ifdef USE_SDL
#include "SDL.h"
#include "SDL_syswm.h"
#endif

/*----------------------------------------------------------------------------
--		Variables
----------------------------------------------------------------------------*/

/**
**		Menu button graphics
*/
global MenuGraphics MenuButtonGfx;

/**
**		The currently processed menu
*/
global Menu* CurrentMenu;

/**
**		The background picture used by menus
*/
local Graphic* Menusbgnd;

/**
**		X, Y, Width, and Height of menu are to redraw
*/
local int MenuRedrawX;
local int MenuRedrawY;
local int MenuRedrawW;
local int MenuRedrawH;

local int MenuButtonUnderCursor = -1;
local int MenuButtonCurSel = -1;

/*----------------------------------------------------------------------------
--		Menu operation functions
----------------------------------------------------------------------------*/

/**
**	  Convert menu button style to a string for saving.
*/
global char* MenuButtonStyle(int style)
{
   switch (style) {
	   case MBUTTON_MAIN:
		   return "main";
	   case MBUTTON_NETWORK:
		   return "network";
	   case MBUTTON_GM_HALF:
		   return "gm-half";
	   case MBUTTON_132:
		   return "132";
	   case MBUTTON_GM_FULL:
		   return "gm-full";
	   case MBUTTON_GEM_ROUND:
		   return "gem-round";
	   case MBUTTON_GEM_SQUARE:
		   return "gem-square";
	   case MBUTTON_UP_ARROW:
		   return "up-arrow";
	   case MBUTTON_DOWN_ARROW:
		   return "down-arrow";
	   case MBUTTON_LEFT_ARROW:
		   return "left-arrow";
	   case MBUTTON_RIGHT_ARROW:
		   return "right-arrow";
	   case MBUTTON_S_KNOB:
		   return "s-knob";
	   case MBUTTON_S_VCONT:
		   return "s-vcont";
	   case MBUTTON_S_HCONT:
		   return "s-hcont";
	   case MBUTTON_PULLDOWN:
		   return "pulldown";
	   case MBUTTON_VTHIN:
		   return "vthin";
	   case MBUTTON_FOLDER:
		   return "folder";
   }
   fprintf(stderr, "MenuButtonStyle not found: %d\n", style);
   return "";
}

/**
**		Find a menu by ident.
**
**		@param menu_id		Unique identifier for the menu.
**
**		@return				Pointer to the menu, NULL if menu is not found.
*/
global Menu* FindMenu(const char* menu_id)
{
	Menu** menu;

	if (!(menu = (Menu**)hash_find(MenuHash, (char*)menu_id))) {
		return NULL;
	} else {
		return *menu;
	}
}

/**
**		Set menu backgound and draw it.
*/
global void MenusSetBackground(void)
{
	if (!Menusbgnd) {
		Menusbgnd = LoadGraphic(MenuBackground);
		ResizeGraphic(Menusbgnd, VideoWidth, VideoHeight);
	}

	VideoDrawSubClip(Menusbgnd, 0, 0,
		Menusbgnd->Width, Menusbgnd->Height,
		(VideoWidth - Menusbgnd->Width) / 2, (VideoHeight - Menusbgnd->Height) / 2);

	MenuRedrawX = 0;
	MenuRedrawY = 0;
	MenuRedrawW = VideoWidth;
	MenuRedrawH = VideoHeight;
}

/**
**		Draw menu button 'button' on x,y
**
**		@param button		Button identifier
**		@param flags		State of Button (clicked, mouse over...)
**		@param transparent		State of button transparency: 0=No, 1=Yes
**		@param w		Button width (for border)
**		@param h		Button height (for border)
**		@param x		X display position
**		@param y		Y display position
**		@param font		font number for text
**		@param text		text to print on button
**		@param normalcolor
**		@param reversecolor
*/
global void DrawMenuButton(MenuButtonId button, unsigned flags, int transparent, int w, int h,
	int x, int y, const int font, const unsigned char* text,
	char* normalcolor, char* reversecolor)
{
	MenuButtonId rb;
	int s;
	char* nc;
	char* rc;
	char* oldnc;
	char* oldrc;

	GetDefaultTextColors(&oldnc, &oldrc);
	if (normalcolor || reversecolor) {
		nc = normalcolor ? normalcolor : oldnc;
		rc = reversecolor ? reversecolor : oldrc;
		SetDefaultTextColors(nc, rc);
	} else {
		nc = oldnc;
		rc = oldrc;
	}
	if (button == MBUTTON_SC_BUTTON || button == MBUTTON_SC_BUTTON_LEFT ||
			button == MBUTTON_SC_BUTTON_RIGHT) {
		if (flags & MenuButtonDisabled) {
			rb = button - 3;
			SetDefaultTextColors(FontGrey, FontGrey);
		} else if (flags & MenuButtonClicked) {
			rb = button + 3;
			SetDefaultTextColors(rc, rc);
		} else {
			rb = button;
			if (flags & MenuButtonActive) {
				SetDefaultTextColors(rc, rc);
			}
		}
		VideoDraw(MenuButtonGfx.Sprite, rb - 1, x, y);
		for (s = x + 8; s < x + w - 1 - 8; s += 8) {
			VideoDraw(MenuButtonGfx.Sprite, rb, s, y);
		}
		VideoDraw(MenuButtonGfx.Sprite, rb + 1, x + w - 1 - 8, y);
		if (text) {
			VideoDrawTextCentered(x + w / 2, y + (h - VideoTextHeight(font)) / 2,
				font, text);
		}
		if (flags & MenuButtonSelected) {
			if (flags & MenuButtonDisabled) {
				VideoDrawRectangleClip(ColorGray, x, y, w - 1, h);
			} else {
				// FIXME: wrong color
				VideoDrawRectangleClip(ColorRed, x, y, w - 1, h);
			}
		}
	} else {
		if (flags & MenuButtonDisabled) {
			rb = button - 1;
			s = 0;
			SetDefaultTextColors(FontGrey, FontGrey);
		} else if (flags & MenuButtonClicked) {
			rb = button + 1;
			s = 2;
			SetDefaultTextColors(rc, rc);
		} else {
			rb = button;
			s = 0;
			if (flags & MenuButtonActive) {
				SetDefaultTextColors(rc, rc);
			}
		}
		if (rb < MenuButtonGfx.Sprite->NumFrames) {
			if (transparent) {
				VideoDrawClipTrans50(MenuButtonGfx.Sprite, rb, x, y);
			} else {
				VideoDrawClip(MenuButtonGfx.Sprite, rb, x, y);
			}

		} else {
			if (rb < button) {
				VideoDrawRectangleClip(ColorGray, x + 1, y + 1, w - 2, h - 2);
				VideoDrawRectangleClip(ColorGray, x + 2, y + 2, w - 4, h - 4);
			} else {
				// FIXME: Temp-workaround for missing folder button in non-expansion gfx
				VideoDrawRectangleClip(ColorYellow, x + 1, y + 1, w - 2, h - 2);
				VideoDrawRectangleClip(ColorYellow, x + 2, y + 2, w - 4, h - 4);
			}
		}
		if (text) {
			if (button != MBUTTON_FOLDER) {
				VideoDrawTextCentered(s + x + w / 2,
					s + y + (h - VideoTextHeight(font)) / 2 + 2, font, text);
			} else {
				SetDefaultTextColors(nc, rc);
				VideoDrawText(x + 44, y + 6, font, text);
			}
		}
		if (flags & MenuButtonSelected) {
			if (flags & MenuButtonDisabled) {
				VideoDrawRectangleClip(ColorGray, x, y, w - 1, h);
			} else {
				VideoDrawRectangleClip(ColorYellow, x, y, w - 1, h);
			}
		}
	}
	SetDefaultTextColors(oldnc, oldrc);
}

/**
**		Draw pulldown 'button' on menu mx, my
**
**		@param mi		menuitem pointer
**		@param mx		menu X display position (offset)
**		@param my		menu Y display position (offset)
*/
local void DrawPulldown(Menuitem* mi, int mx, int my)
{
	int i;
	char* nc;
	char* rc;
	char* oldnc;
	char* oldrc;
	char* text;
	unsigned flags;
	MenuButtonId rb;
	MenuButtonId db;
	int w;
	int h;
	int x;
	int y;
	int oh;

	x = mx + mi->xofs;
	y = my + mi->yofs;
	w = mi->d.pulldown.xsize;
	flags = mi->flags;
	rb = mi->d.pulldown.button;

	GetDefaultTextColors(&oldnc, &oldrc);
	if (mi->d.pulldown.normalcolor || mi->d.pulldown.reversecolor) {
		nc = mi->d.pulldown.normalcolor ? mi->d.pulldown.normalcolor : oldnc;
		rc = mi->d.pulldown.reversecolor ? mi->d.pulldown.reversecolor : oldrc;
		SetDefaultTextColors(nc, rc);
	} else {
		nc = oldnc;
		rc = oldrc;
	}
	if (rb == MBUTTON_SC_PULLDOWN) {
		h = mi->d.pulldown.ysize;
		if (flags & MenuButtonClicked) {
			int usetop;
			int option;
			int max;

			// Check if the pulldown goes below the bottom
			if (mi->yofs + (h + 1) * mi->d.pulldown.noptions >= mi->menu->Height) {
				y -= h * mi->d.pulldown.noptions;
				usetop = 0;
			} else {
				usetop = 1;
			}

			// Draw top
			if (usetop) {
				rb = MBUTTON_SC_PULLDOWN_TOP_SELECTED;
			} else {
				rb = MBUTTON_SC_PULLDOWN_TOP;
			}
			VideoDraw(MenuButtonGfx.Sprite, rb - 1, x, y);
			for (i = x + 16; i < x + w - 1 - 16; i += 16) {
				VideoDraw(MenuButtonGfx.Sprite, rb, i, y);
			}
			VideoDraw(MenuButtonGfx.Sprite, rb + 1, x + w - 1 - 16, y);
			option = 0;
			if (usetop) {
				VideoDraw(MenuButtonGfx.Sprite, MBUTTON_SC_PULLDOWN_DOWN_ARROW,
					x + w - 1 - 16 - 3, y + 4);
				text = mi->d.pulldown.options[mi->d.pulldown.curopt];
				if (text) {
					VideoDrawText(x + 4, y + 2, mi->font, text);
				}
			} else {
				if (option == mi->d.pulldown.cursel) {
					SetDefaultTextColors(rc, rc);
				} else {
					SetDefaultTextColors(nc, rc);
				}
				text = mi->d.pulldown.options[option];
				if (text) {
					VideoDrawText(x + 4, y + 2, mi->font, text);
				}
				option = 1;
			}

			// Middle
			y += mi->d.pulldown.ysize;
			rb = MBUTTON_SC_PULLDOWN_MIDDLE;
			if (usetop) {
				max = mi->d.pulldown.noptions - 1;
			} else {
				max = mi->d.pulldown.noptions;
			}
			for (; option < max; ++option) {
				VideoDraw(MenuButtonGfx.Sprite, rb - 1, x, y);
				for (i = x + 16; i < x + w - 1 - 16; i += 16) {
					VideoDraw(MenuButtonGfx.Sprite, rb, i, y);
				}
				VideoDraw(MenuButtonGfx.Sprite, rb + 1, x + w - 1 - 16, y);
				if (option == mi->d.pulldown.cursel) {
					SetDefaultTextColors(rc, rc);
				} else {
					SetDefaultTextColors(nc, rc);
				}
				text = mi->d.pulldown.options[option];
				if (text) {
					VideoDrawText(x + 4, y + 2, mi->font, text);
				}
				y += mi->d.pulldown.ysize;
			}

			// Bottom
			SetDefaultTextColors(nc, rc);
			if (usetop) {
				rb = MBUTTON_SC_PULLDOWN_BOTTOM;
			} else {
				rb = MBUTTON_SC_PULLDOWN_BOTTOM_SELECTED;
			}
			VideoDraw(MenuButtonGfx.Sprite, rb - 1, x, y);
			for (i = x + 16; i < x + w - 1 - 16; i += 16) {
				VideoDraw(MenuButtonGfx.Sprite, rb, i, y);
			}
			VideoDraw(MenuButtonGfx.Sprite, rb + 1, x + w - 1 - 16, y);
			if (usetop) {
				if (option == mi->d.pulldown.cursel) {
					SetDefaultTextColors(rc, rc);
				} else {
					SetDefaultTextColors(nc, rc);
				}
				text = mi->d.pulldown.options[option];
				if (text) {
					VideoDrawText(x + 4, y + 2, mi->font, text);
				}
				option = 0;
			} else {
				VideoDraw(MenuButtonGfx.Sprite, MBUTTON_SC_PULLDOWN_DOWN_ARROW,
					x + w - 1 - 16 - 3, y + 4);
				text = mi->d.pulldown.options[mi->d.pulldown.curopt];
				if (text) {
					VideoDrawText(x + 4, y + 2, mi->font, text);
				}
				option = 1;
			}
		} else {
			h = mi->d.pulldown.ysize;
			y = my + mi->yofs;
			db = MBUTTON_SC_PULLDOWN_DOWN_ARROW;
			if (flags & MenuButtonDisabled) {
				rb = MBUTTON_SC_PULLDOWN_DISABLED;
				SetDefaultTextColors(FontGrey, FontGrey);
			} else {
				if (flags & MenuButtonActive) {
					SetDefaultTextColors(rc, rc);
					++db;
				}
			}
			VideoDraw(MenuButtonGfx.Sprite, rb - 1, x, y);
			for (i = x + 16; i < x + w - 1 - 16; i += 16) {
				VideoDraw(MenuButtonGfx.Sprite, rb, i, y);
			}
			VideoDraw(MenuButtonGfx.Sprite, rb + 1, x + w - 1 - 16, y);
			if (!(mi->d.pulldown.state & MI_PSTATE_PASSIVE)) {
				VideoDraw(MenuButtonGfx.Sprite, db, x + w - 1 - 16 - 3, y + 4);
			}
			text = mi->d.pulldown.options[mi->d.pulldown.curopt];
			if (text) {
				VideoDrawText(x + 4, y + 2, mi->font, text);
			}
		}
	} else {
		oh = h = mi->d.pulldown.ysize - 2;
		if (flags & MenuButtonClicked) {
			// Make the menu inside of the screen (TOP)
			if (y + 1 <= mi->d.pulldown.curopt * h + CurrentMenu->Y) {
				y = 2 + CurrentMenu->Y;
			} else {
				y -= mi->d.pulldown.curopt * h;
				// Make the menu inside the bottom of the screen
				// FIXME: can't assume bottom is always 480
				if (y + h * mi->d.pulldown.noptions >= 480 + CurrentMenu->Y) {
					y -= y + h * mi->d.pulldown.noptions - (480 + CurrentMenu->Y);
				}
			}
			i = mi->d.pulldown.noptions;
			h *= i;
			while (i--) {
				PushClipping();
				SetClipping(0, 0, x + w, VideoHeight - 1);
				if (mi->transparent) {
					VideoDrawClipTrans50(MenuButtonGfx.Sprite, rb, x - 1, y - 1 + oh * i);
				} else {
					VideoDrawClip(MenuButtonGfx.Sprite, rb, x - 1, y - 1 + oh * i);
				}
				PopClipping();
				text = mi->d.pulldown.options[i];
				if (text) {
					if (i == mi->d.pulldown.cursel) {
						SetDefaultTextColors(rc, rc);
					} else {
						SetDefaultTextColors(nc, rc);
					}
					VideoDrawText(x + 2, y + 2 + oh * i, mi->font, text);
				}
			}
			w += 2;
			h += 2;
		} else {
			h = mi->d.pulldown.ysize;
			y = my + mi->yofs;
			if (flags & MenuButtonDisabled) {
				rb--;
				SetDefaultTextColors(FontGrey, FontGrey);
			} else {
				if (flags & MenuButtonActive) {
					SetDefaultTextColors(rc, rc);
				}
			}

			PushClipping();
			if (!(mi->d.pulldown.state & MI_PSTATE_PASSIVE)) {
				SetClipping(0, 0, x + w - 20, VideoHeight - 1);
			} else {
				SetClipping(0, 0, x + w - 1, VideoHeight - 1);
			}
			if (mi->transparent) {
				VideoDrawClipTrans50(MenuButtonGfx.Sprite, rb, x - 1, y - 1);
			} else {
				VideoDrawClip(MenuButtonGfx.Sprite, rb, x - 1, y - 1);
			}
			PopClipping();
			if (!(mi->d.pulldown.state & MI_PSTATE_PASSIVE)) {
				VideoDraw(MenuButtonGfx.Sprite, MBUTTON_DOWN_ARROW + rb - MBUTTON_PULLDOWN,
					x - 1 + w - 20, y - 2);
			}
			text = mi->d.pulldown.options[mi->d.pulldown.curopt];
			if (text) {
				VideoDrawText(x + 2, y + 2, mi->font, text);
			}
		}
		if (flags & MenuButtonSelected) {
			if (flags & MenuButtonDisabled) {
				VideoDrawRectangleClip(ColorGray, x - 2, y - 2, w, h);
			} else {
				VideoDrawRectangleClip(ColorYellow, x - 2, y - 2, w, h);
			}
		}
	}
	SetDefaultTextColors(oldnc, oldrc);
}

/**
**		Draw listbox 'button' on menu mx, my
**
**		@param mi		menuitem pointer
**		@param mx		menu X display position (offset)
**		@param my		menu Y display position (offset)
*/
local void DrawListbox(Menuitem* mi, int mx, int my)
{
	int i;
	int s;
	char* nc;
	char* rc;
	char* oldnc;
	char* oldrc;
	char* text;
	MenuButtonId rb;
	unsigned flags;
	int w;
	int h;
	int x;
	int y;

	rb = mi->d.listbox.button;
	flags = mi->flags;
	w = mi->d.listbox.xsize;
	h = mi->d.listbox.ysize;
	x = mx + mi->xofs;
	y = my + mi->yofs;

	GetDefaultTextColors(&oldnc, &oldrc);
	if (mi->d.listbox.normalcolor || mi->d.listbox.reversecolor) {
		nc = mi->d.listbox.normalcolor ? mi->d.listbox.normalcolor : oldnc;
		rc = mi->d.listbox.reversecolor ? mi->d.listbox.reversecolor : oldrc;
		SetDefaultTextColors(nc, rc);
	} else {
		nc = oldnc;
		rc = oldrc;
	}

	if (flags & MenuButtonDisabled) {
		rb--;
	}
	i = mi->d.listbox.nlines;
	s = mi->d.listbox.startline;
	while (i--) {
		PushClipping();
		SetClipping(0, 0, x + w, VideoHeight - 1);
		if (mi->transparent) {
			VideoDrawClipTrans50(MenuButtonGfx.Sprite, rb, x - 1, y - 1 + 18 * i);
		} else {
			VideoDrawClip(MenuButtonGfx.Sprite, rb, x - 1, y - 1 + 18 * i);
		}
		PopClipping();
		if (!(flags & MenuButtonDisabled)) {
			if (i < mi->d.listbox.noptions) {
				SetDefaultTextColors(nc, rc);
				text = (*mi->d.listbox.retrieveopt)(mi, i + s);
				if (text) {
					if (i == mi->d.listbox.curopt) {
						SetDefaultTextColors(rc, rc);
					} else {
						SetDefaultTextColors(nc, rc);
					}
					VideoDrawText(x + 2, y + 2 + 18 * i, mi->font,text);
				}
			}
		}
	}

	if (flags & MenuButtonSelected) {
		if (flags & MenuButtonDisabled) {
			VideoDrawRectangleClip(ColorGray, x - 2, y - 2, w + 1, h + 2);
		} else {
			VideoDrawRectangleClip(ColorYellow, x - 2, y - 2, w + 1, h + 2);
		}
	}
	SetDefaultTextColors(oldnc, oldrc);
}

/**
**		Draw vslider 'button' on menu mx, my
**
**		@param mi		menuitem pointer
**		@param mx		menu X display position (offset)
**		@param my		menu Y display position (offset)
*/
local void DrawVSlider(Menuitem* mi, int mx, int my)
{
	int p;
	unsigned flags;
	int w;
	int h;
	int x;
	int y;

	flags = mi->flags;
	w = mi->d.vslider.xsize;
	h = mi->d.vslider.ysize;
	x = mx + mi->xofs;
	y = my + mi->yofs;

	if (mi->d.vslider.style == MI_STYLE_SC_VSLIDER) {
		int upb;
		int downb;

		if (flags & MenuButtonDisabled) {
			upb = MBUTTON_SC_UP_ARROW - 1;
			downb = MBUTTON_SC_DOWN_ARROW - 1;
		} else if (flags & MenuButtonClicked) {
			if (mi->d.vslider.cflags&MI_CFLAGS_UP) {
				upb = MBUTTON_SC_UP_ARROW + 1;
				downb = MBUTTON_SC_DOWN_ARROW;
			} else if (mi->d.vslider.cflags&MI_CFLAGS_DOWN) {
				upb = MBUTTON_SC_UP_ARROW;
				downb = MBUTTON_SC_DOWN_ARROW + 1;
			} else {
				upb = MBUTTON_SC_UP_ARROW;
				downb = MBUTTON_SC_DOWN_ARROW;
			}
		} else {
			upb = MBUTTON_SC_UP_ARROW;
			downb = MBUTTON_SC_DOWN_ARROW;
		}
		VideoDraw(MenuButtonGfx.Sprite, upb, x, y);
		VideoDraw(MenuButtonGfx.Sprite, downb, x, y + h - 1 - 16);

		VideoDraw(MenuButtonGfx.Sprite, MBUTTON_SC_S_VCONT - 1, x, y + 16 + 2);
		for (p = y + 16 + 2 + 8 + 1; p < y + h - 1 - 16 - 2 - 8; p += 8) {
			VideoDraw(MenuButtonGfx.Sprite, MBUTTON_SC_S_VCONT, x, p);
		}
		VideoDraw(MenuButtonGfx.Sprite, MBUTTON_SC_S_VCONT + 1, x, y + h - 1 - 16 - 2 - 8);
		p = (mi->d.vslider.percent * (h - 54)) / 100;
		VideoDraw(MenuButtonGfx.Sprite, MBUTTON_SC_S_KNOB, x, y + 16 + 2 + p);
	} else {
		if (flags & MenuButtonDisabled) {
			PushClipping();
			SetClipping(0, 0, VideoWidth - 1, y + h - 20);
			if (mi->transparent) {
				VideoDrawClipTrans50(MenuButtonGfx.Sprite, MBUTTON_S_VCONT - 1, x, y - 2);
				VideoDrawClipTrans50(MenuButtonGfx.Sprite, MBUTTON_S_VCONT - 1, x, y + h / 2);
			} else {
				VideoDrawClip(MenuButtonGfx.Sprite, MBUTTON_S_VCONT - 1, x, y - 2);
				VideoDrawClip(MenuButtonGfx.Sprite, MBUTTON_S_VCONT - 1, x, y + h / 2);
			}
			PopClipping();
			VideoDraw(MenuButtonGfx.Sprite, MBUTTON_UP_ARROW - 1, x, y - 2);
			VideoDraw(MenuButtonGfx.Sprite, MBUTTON_DOWN_ARROW - 1, x, y + h - 20);
		} else {
			PushClipping();
			SetClipping(0, 0, VideoWidth - 1, y + h - 20);
			if (mi->transparent) {
				VideoDrawClipTrans50(MenuButtonGfx.Sprite, MBUTTON_S_VCONT, x, y - 2);
				VideoDrawClipTrans50(MenuButtonGfx.Sprite, MBUTTON_S_VCONT, x, y + h / 2);
			} else {
				VideoDrawClip(MenuButtonGfx.Sprite, MBUTTON_S_VCONT, x, y - 2);
				VideoDrawClip(MenuButtonGfx.Sprite, MBUTTON_S_VCONT, x, y + h / 2);
			}
			PopClipping();
			if (mi->d.vslider.cflags & MI_CFLAGS_UP) {
				VideoDraw(MenuButtonGfx.Sprite, MBUTTON_UP_ARROW + 1, x, y - 2);
			} else {
				VideoDraw(MenuButtonGfx.Sprite, MBUTTON_UP_ARROW, x, y - 2);
			}
			if (mi->d.vslider.cflags & MI_CFLAGS_DOWN) {
				VideoDraw(MenuButtonGfx.Sprite, MBUTTON_DOWN_ARROW + 1, x, y + h - 20);
			} else {
				VideoDraw(MenuButtonGfx.Sprite, MBUTTON_DOWN_ARROW, x, y + h - 20);
			}
			p = (mi->d.vslider.percent * (h - 54)) / 100;
			VideoDraw(MenuButtonGfx.Sprite, MBUTTON_S_KNOB, x + 1, y + 18 + p);
		}

		if (flags & MenuButtonSelected) {
			if (flags & MenuButtonDisabled) {
				VideoDrawRectangleClip(ColorGray, x, y - 2, w, h + 2);
			} else {
				VideoDrawRectangleClip(ColorYellow, x, y - 2, w, h + 2);
			}
		}
	}
}

/**
**		Draw hslider 'button' on menu mx, my
**
**		@param mi		menuitem pointer
**		@param mx		menu X display position (offset)
**		@param my		menu Y display position (offset)
*/
local void DrawHSlider(Menuitem* mi, int mx, int my)
{
	int p;
	unsigned flags;
	int w;
	int h;
	int x;
	int y;

	flags = mi->flags;
	w = mi->d.hslider.xsize;
	h = mi->d.hslider.ysize;
	x = mx + mi->xofs;
	y = my + mi->yofs;

	if (mi->d.hslider.style == MI_STYLE_SC_HSLIDER) {
		int leftb;
		int rightb;

		if (flags & MenuButtonDisabled) {
			leftb = MBUTTON_SC_LEFT_ARROW - 1;
			rightb = MBUTTON_SC_RIGHT_ARROW - 1;
		} else if (flags & MenuButtonClicked) {
			if (mi->d.vslider.cflags & MI_CFLAGS_LEFT) {
				leftb = MBUTTON_SC_LEFT_ARROW + 1;
				rightb = MBUTTON_SC_RIGHT_ARROW;
			} else if (mi->d.vslider.cflags & MI_CFLAGS_RIGHT) {
				leftb = MBUTTON_SC_LEFT_ARROW;
				rightb = MBUTTON_SC_RIGHT_ARROW + 1;
			} else {
				leftb = MBUTTON_SC_LEFT_ARROW;
				rightb = MBUTTON_SC_RIGHT_ARROW;
			}
		} else {
			leftb = MBUTTON_SC_LEFT_ARROW;
			rightb = MBUTTON_SC_RIGHT_ARROW;
		}
		VideoDraw(MenuButtonGfx.Sprite, leftb, x, y);
		VideoDraw(MenuButtonGfx.Sprite, rightb, x + w - 1 - 16, y);

		VideoDraw(MenuButtonGfx.Sprite, MBUTTON_SC_S_HCONT - 1, x + 16 + 2, y);
		for (p = x + 16 + 2 + 8 + 1; p < x + w - 1 - 16 - 2 - 8; p += 8) {
			VideoDraw(MenuButtonGfx.Sprite, MBUTTON_SC_S_HCONT, p, y);
		}
		VideoDraw(MenuButtonGfx.Sprite, MBUTTON_SC_S_HCONT + 1, x + w - 1 - 16 - 2 - 8, y);
		p = (mi->d.vslider.percent * (w - 54)) / 100;
		VideoDraw(MenuButtonGfx.Sprite, MBUTTON_SC_S_KNOB, x + 16 + 2 + p, y);
	} else {
		if (flags & MenuButtonDisabled) {
			PushClipping();
			SetClipping(0, 0, x + w - 20, VideoHeight - 1);
			if (mi->transparent) {
				VideoDrawClipTrans50(MenuButtonGfx.Sprite, MBUTTON_S_HCONT - 1, x - 2, y);
				VideoDrawClipTrans50(MenuButtonGfx.Sprite, MBUTTON_S_HCONT - 1, x + w / 2, y);
			} else {
				VideoDrawClip(MenuButtonGfx.Sprite, MBUTTON_S_HCONT - 1, x - 2, y);
				VideoDrawClip(MenuButtonGfx.Sprite, MBUTTON_S_HCONT - 1, x + w / 2, y);
			}
			PopClipping();
			VideoDraw(MenuButtonGfx.Sprite, MBUTTON_LEFT_ARROW - 1, x - 2, y);
			VideoDraw(MenuButtonGfx.Sprite, MBUTTON_RIGHT_ARROW - 1, x + w - 20, y);
		} else {
			PushClipping();
			SetClipping(0, 0, x + w - 20, VideoHeight - 1);
			if (mi->transparent) {
				VideoDrawClipTrans50(MenuButtonGfx.Sprite, MBUTTON_S_HCONT, x - 2, y);
				VideoDrawClipTrans50(MenuButtonGfx.Sprite, MBUTTON_S_HCONT, x + w / 2, y);
			} else {
				VideoDrawClip(MenuButtonGfx.Sprite, MBUTTON_S_HCONT, x - 2, y);
				VideoDrawClip(MenuButtonGfx.Sprite, MBUTTON_S_HCONT, x + w / 2, y);
			}
			PopClipping();
			if (mi->d.hslider.cflags & MI_CFLAGS_LEFT) {
				VideoDraw(MenuButtonGfx.Sprite, MBUTTON_LEFT_ARROW + 1, x - 2, y);
			} else {
				VideoDraw(MenuButtonGfx.Sprite, MBUTTON_LEFT_ARROW, x - 2, y);
			}
			if (mi->d.hslider.cflags & MI_CFLAGS_RIGHT) {
				VideoDraw(MenuButtonGfx.Sprite, MBUTTON_RIGHT_ARROW + 1, x + w - 20, y);
			} else {
				VideoDraw(MenuButtonGfx.Sprite, MBUTTON_RIGHT_ARROW, x + w - 20, y);
			}
			p = (mi->d.hslider.percent * (w - 54)) / 100;
			VideoDraw(MenuButtonGfx.Sprite, MBUTTON_S_KNOB, x + 18 + p, y + 1);
		}

		if (flags & MenuButtonSelected) {
			if (flags & MenuButtonDisabled) {
				VideoDrawRectangleClip(ColorGray, x - 2, y, w + 2, h);
			} else {
				VideoDrawRectangleClip(ColorYellow, x - 2, y, w + 2, h);
			}
		}
	}
}

/**
**		Draw gem 'button' on menu mx, my
**
**		@param mi		menuitem pointer
**		@param mx		menu X display position (offset)
**		@param my		menu Y display position (offset)
*/
local void DrawGem(Menuitem* mi, int mx, int my)
{
	unsigned flags;
	MenuButtonId rb;
	int x;
	int y;
	char* nc;
	char* rc;
	char* oldnc;
	char* oldrc;

	flags = mi->flags;
	rb = mi->d.gem.button;
	x = mx+mi->xofs;
	y = my+mi->yofs;
	if ((mi->d.gem.state & MI_GSTATE_INVISIBLE)) {
		return;
	}
	if (flags & MenuButtonDisabled) {
		rb--;
	}
	else {
		if (flags & MenuButtonClicked) {
			++rb;
		}
		if ((mi->d.gem.state & MI_GSTATE_CHECKED)) {
			rb += 2;
		}
	}
	VideoDraw(MenuButtonGfx.Sprite, rb, x, y);

	if (mi->d.gem.text) {
		GetDefaultTextColors(&oldnc,&oldrc);
		if (mi->d.gem.normalcolor || mi->d.gem.reversecolor) {
			nc = mi->d.gem.normalcolor ? mi->d.gem.normalcolor : oldnc;
			rc = mi->d.gem.reversecolor ? mi->d.gem.reversecolor : oldrc;
			SetDefaultTextColors(nc,rc);
		} else {
			nc = oldnc;
			rc = oldrc;
		}
		VideoDrawText(x + 24, y + 4, GameFont, mi->d.gem.text);
		if (mi->flags & MenuButtonActive) {
			SetDefaultTextColors(rc, rc);
			VideoDrawRectangleClip(ColorGray, mx + mi->xofs - 4, my + mi->yofs - 4,
				VideoTextLength(GameFont, mi->d.gem.text) + 30, VideoTextHeight(GameFont) + 12);
		}
		SetDefaultTextColors(oldnc, oldrc);
	}
}

/**
**		Draw input 'button' on menu mx, my
**
**		@param mi		menuitem pointer
**		@param mx		menu X display position (offset)
**		@param my		menu Y display position (offset)
*/
local void DrawInput(Menuitem* mi, int mx, int my)
{
	char* nc;
	char* rc;
	char* oldnc;
	char* oldrc;
	char* text;
	unsigned flags;
	MenuButtonId rb;
	int w;
	int h;
	int x;
	int y;
	int p;

	flags = mi->flags;
	rb = mi->d.input.button;
	x = mx+mi->xofs;
	y = my+mi->yofs;
	w = mi->d.input.xsize;
	h = mi->d.input.ysize;

	GetDefaultTextColors(&oldnc,&oldrc);
	if (mi->d.input.normalcolor || mi->d.input.reversecolor) {
		nc = mi->d.input.normalcolor ? mi->d.input.normalcolor : oldnc;
		rc = mi->d.input.reversecolor ? mi->d.input.reversecolor : oldrc;
		SetDefaultTextColors(nc, rc);
	} else {
		nc = oldnc;
		rc = oldrc;
	}
	if (mi->d.input.button == MBUTTON_SC_PULLDOWN) {
		rb = MBUTTON_SC_INPUT;
		if (flags & MenuButtonDisabled) {
			rb -= 3;
			SetDefaultTextColors(FontGrey, FontGrey);
		}
		VideoDraw(MenuButtonGfx.Sprite, rb - 1, x, y);
		for (p = x + 16; p < x + w - 1 - 16; p += 16) {
			VideoDraw(MenuButtonGfx.Sprite, rb, p, y);
		}
		VideoDraw(MenuButtonGfx.Sprite, rb + 1, x + w - 1 - 16, y);
		text = mi->d.input.buffer;
		if (text) {
			VideoDrawText(x + 4, y + 2, mi->font, text);
		}
	} else {
		if (flags & MenuButtonDisabled) {
			--rb;
			SetDefaultTextColors(FontGrey, FontGrey);
		}

		PushClipping();
		SetClipping(0, 0, x + w, VideoHeight - 1);
		if (mi->transparent) {
			VideoDrawClipTrans50(MenuButtonGfx.Sprite, rb, x - 1, y - 1);
		} else {
			VideoDrawClip(MenuButtonGfx.Sprite, rb, x - 1, y - 1);
		}
		PopClipping();
		text = mi->d.input.buffer;
		if (text) {
			VideoDrawText(x + 2, y + 2, mi->font, text);
		}
		if (flags & MenuButtonSelected) {
			if (flags & MenuButtonDisabled) {
				VideoDrawRectangleClip(ColorGray, x - 2, y - 2, w + 4, h);
			} else {
				VideoDrawRectangleClip(ColorYellow, x - 2, y - 2, w + 4, h);
			}
		}
	}
	SetDefaultTextColors(oldnc, oldrc);
}


/**
**		Draw a menu.
**
**		@param menu		The menu number to display (NULL allowed)
*/
global void DrawMenu(Menu* menu)
{
	int i;
	int n;
	int l;
	char* nc;
	char* rc;
	char* oldnc;
	char* oldrc;
	Menuitem* mi;
	Menuitem* mip;

	if (menu == NULL) {
		return;
	}

	MenuRedrawX = menu->X;
	MenuRedrawY = menu->Y;
	MenuRedrawW = menu->Width;
	MenuRedrawH = menu->Height;

	if (menu->Panel && !strcmp(menu->Panel, ScPanel)) {
		// Background
		VideoFillTransRectangle(ColorBlack, MenuRedrawX + 1,
				MenuRedrawY + 1, MenuRedrawW - 2, MenuRedrawH - 2, 50);
		VideoDrawHLineClip(ColorBlue, MenuRedrawX + 3, MenuRedrawY, MenuRedrawW - 6);
		VideoDrawHLineClip(ColorBlue, MenuRedrawX + 3, MenuRedrawY + MenuRedrawH - 1, MenuRedrawW - 6);
		VideoDrawVLineClip(ColorBlue, MenuRedrawX, MenuRedrawY + 3, MenuRedrawH - 6);
		VideoDrawVLineClip(ColorBlue, MenuRedrawX + MenuRedrawW - 1, MenuRedrawY + 3, MenuRedrawH - 6);
		// top left
		VideoDrawPixelClip(ColorBlue, MenuRedrawX + 1, MenuRedrawY + 1);
		VideoDrawPixelClip(ColorBlue, MenuRedrawX + 2, MenuRedrawY + 1);
		VideoDrawPixelClip(ColorBlue, MenuRedrawX + 1, MenuRedrawY + 2);
		// top right
		VideoDrawPixelClip(ColorBlue, MenuRedrawX + MenuRedrawW - 3, MenuRedrawY + 1);
		VideoDrawPixelClip(ColorBlue, MenuRedrawX + MenuRedrawW - 2, MenuRedrawY + 1);
		VideoDrawPixelClip(ColorBlue, MenuRedrawX + MenuRedrawW - 2, MenuRedrawY + 2);
		// bottom left
		VideoDrawPixelClip(ColorBlue, MenuRedrawX + 1, MenuRedrawY + MenuRedrawH - 3);
		VideoDrawPixelClip(ColorBlue, MenuRedrawX + 1, MenuRedrawY + MenuRedrawH - 2);
		VideoDrawPixelClip(ColorBlue, MenuRedrawX + 2, MenuRedrawY + MenuRedrawH - 2);
		// bottom right
		VideoDrawPixelClip(ColorBlue, MenuRedrawX + MenuRedrawW - 3, MenuRedrawY + MenuRedrawH - 2);
		VideoDrawPixelClip(ColorBlue, MenuRedrawX + MenuRedrawW - 2, MenuRedrawY + MenuRedrawH - 2);
		VideoDrawPixelClip(ColorBlue, MenuRedrawX + MenuRedrawW - 2, MenuRedrawY + MenuRedrawH - 3);
	} else if (menu->Panel) {
		MenuPanel* menupanel;

		menupanel = TheUI.MenuPanels;
		while (menupanel) {
			if (!strcmp(menupanel->Ident, menu->Panel)) {
				break;
			}
			menupanel = menupanel->Next;
		}
		if (menupanel) {
			VideoDrawSub(menupanel->Panel.Graphic, 0, 0,
				VideoGraphicWidth(menupanel->Panel.Graphic),
				VideoGraphicHeight(menupanel->Panel.Graphic),
				menu->X, menu->Y);
		}
	}

	n = menu->NumItems;
	mi = menu->Items;
	mip = NULL;
	for (i = 0; i < n; ++i) {
		switch (mi->mitype) {
			case MI_TYPE_TEXT:
				if (!mi->d.text.text) {
					break;
				}
				GetDefaultTextColors(&oldnc, &oldrc);
				if (mi->d.text.normalcolor || mi->d.text.reversecolor) {
					nc = mi->d.text.normalcolor ? mi->d.text.normalcolor : oldnc;
					rc = mi->d.text.reversecolor ? mi->d.text.reversecolor : oldrc;
					SetDefaultTextColors(nc, rc);
				} else {
					nc = oldnc;
					rc = oldrc;
				}
				if (mi->flags & MenuButtonActive && mi->d.text.action) {
					VideoDrawRectangleClip(ColorGray, menu->X + mi->xofs - 4, menu->Y + mi->yofs - 4,
											VideoTextLength(mi->font, mi->d.text.text) + 5,
											VideoTextHeight(mi->font) + 5);
					SetDefaultTextColors(rc, rc);
				}
				if (mi->d.text.align & MI_TFLAGS_CENTERED) {
					VideoDrawTextCentered(menu->X + mi->xofs, menu->Y + mi->yofs,
							mi->font, mi->d.text.text);
				} else if (mi->d.text.align & MI_TFLAGS_RALIGN) {
					l = VideoTextLength(mi->font, mi->d.text.text);
					VideoDrawText(menu->X + mi->xofs-l,menu->Y + mi->yofs,
							mi->font, mi->d.text.text);
				} else {
					VideoDrawText(menu->X + mi->xofs, menu->Y + mi->yofs,
							mi->font, mi->d.text.text);
				}
				SetDefaultTextColors(oldnc, oldrc);
				break;
			case MI_TYPE_BUTTON:
				DrawMenuButton(mi->d.button.button, mi->flags, mi->transparent,
					mi->d.button.xsize, mi->d.button.ysize,
					menu->X + mi->xofs, menu->Y + mi->yofs,
					mi->font, mi->d.button.text,
					mi->d.button.normalcolor, mi->d.button.reversecolor);
				break;
			case MI_TYPE_PULLDOWN:
				if (mi->flags & MenuButtonClicked) {
					mip = mi;		// Delay, due to possible overlaying!
				} else {
					DrawPulldown(mi, menu->X, menu->Y);
				}
				break;
			case MI_TYPE_LISTBOX:
				DrawListbox(mi, menu->X, menu->Y);
				break;
			case MI_TYPE_VSLIDER:
				DrawVSlider(mi, menu->X, menu->Y);
				break;
			case MI_TYPE_HSLIDER:
				DrawHSlider(mi, menu->X, menu->Y);
				break;
			case MI_TYPE_DRAWFUNC:
				if (mi->d.drawfunc.draw) {
					(*mi->d.drawfunc.draw)(mi);
				}
				break;
			case MI_TYPE_INPUT:
				DrawInput(mi, menu->X, menu->Y);
				break;
			case MI_TYPE_GEM:
				DrawGem(mi, menu->X, menu->Y);
				break;
			default:
				break;
		}
		++mi;
	}
	if (mip) {
		DrawPulldown(mip, menu->X, menu->Y);
	}
}

/**
**		Paste text from the clipboard
*/
local void PasteFromClipboard(Menuitem* mi)
{
#if defined(USE_WIN32) || defined(_XLIB_H_)
	int i;
	unsigned char* clipboard;
#ifdef USE_WIN32
	HGLOBAL handle;
#elif defined(_XLIB_H_)
	Display* display;
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
	clipboard = GlobalLock(handle);
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
	for (i = 0; mi->d.input.nch < mi->d.input.maxch && clipboard[i] &&
			VideoTextLength(mi->font, mi->d.input.buffer) + 8 < mi->d.input.xsize; ++i) {
		if (clipboard[i] >= 32 && clipboard[i] != '~') {
			mi->d.input.buffer[mi->d.input.nch] = clipboard[i];
			++mi->d.input.nch;
		}
	}
	strcpy(mi->d.input.buffer + mi->d.input.nch, "~!_");
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

/**
**		Handle keys in menu mode.
**
**		@param key		Key scancode.
**		@param keychar		ASCII character code of key.
**
**		@todo FIXME: Should be MenuKeyDown(), and act on _new_ MenuKeyUp() !!!
**	  to implement button animation (depress before action)
*/
local void MenuHandleKeyDown(unsigned key, unsigned keychar)
{
	int i;
	int n;
	Menuitem* mi;
	Menu* menu;

	HandleKeyModifiersDown(key, keychar);

	if (CurrentMenu == NULL) {
		return;
	}

	if (KeyCodeKP0 <= key && key <= KeyCodeKP9) {
		key = keychar = '0' + key - KeyCodeKP0;
	} else if (key == KeyCodeKPPeriod) {
		key = keychar = '.';
	}

	menu = CurrentMenu;
	if (MenuButtonCurSel != -1 && menu->Items[MenuButtonCurSel].mitype == MI_TYPE_INPUT) {
		mi = menu->Items + MenuButtonCurSel;
		if (!(mi->flags & MenuButtonDisabled)) {
inkey:
			if (key >= 0x80 && key < 0x100) {
				// FIXME ARI: ISO->WC2 Translation here!
				key = 0;
			}
			switch(key) {
				case '\b': case '\177':
					if (mi->d.input.nch > 0) {
						strcpy(mi->d.input.buffer + (--mi->d.input.nch), "~!_");
					}
					break;
				case 9:
					goto normkey;
				case '~':				// ~ are quotes
					return;				// Just ignore them
				case KeyCodeDelete:
					mi->d.input.nch = 0;
					strcpy(mi->d.input.buffer, "~!_");
					break;
				default:
					if (KeyModifiers&ModifierAlt) {
						if (key == 'x' || key == 'X') {
							goto normkey;
						}
					} else if (KeyModifiers&ModifierControl) {
						if (key == 'v' || key == 'V') {
							PasteFromClipboard(mi);
						} else if (key == 'u' || key == 'U') {
							mi->d.input.nch = 0;
							strcpy(mi->d.input.buffer, "~!_");
						}
					} else if (key >= 32 && key < 0x100) {
						if (mi->d.input.nch < mi->d.input.maxch &&
							VideoTextLength(mi->font, mi->d.input.buffer) + 8 < mi->d.input.xsize) {
							mi->d.input.buffer[mi->d.input.nch++] = keychar;
							strcpy(mi->d.input.buffer + mi->d.input.nch, "~!_");
						}
					}
					break;
			}
			if (mi->d.input.action) {
				(*mi->d.input.action)(mi, key);
			}
			return;
		}
	}

normkey:
	if (!(KeyModifiers & ModifierAlt)) {
		mi = menu->Items;
		i = menu->NumItems;
		while (i--) {
			switch (mi->mitype) {
				case MI_TYPE_BUTTON:
					if (key == mi->d.button.hotkey) {
						if (!(mi->flags & MenuButtonDisabled) && mi->d.button.handler) {
							(*mi->d.button.handler)();
						}
						return;
					}
				default:
					break;
			}
			++mi;
		}
	}
	switch (key) {
		case 10: case 13:						// RETURN
			if (MenuButtonCurSel != -1) {
				mi = menu->Items + MenuButtonCurSel;
				switch (mi->mitype) {
					case MI_TYPE_BUTTON:
						if (mi->d.button.handler) {
							(*mi->d.button.handler)();
						}
						return;
					case MI_TYPE_LISTBOX:
						if (mi->d.listbox.handler) {
							(*mi->d.listbox.handler)();
						}
						return;
					case MI_TYPE_VSLIDER:
						if (mi->d.vslider.handler) {
							(*mi->d.vslider.handler)();
						}
						return;
					case MI_TYPE_HSLIDER:
						if (mi->d.hslider.handler) {
							(*mi->d.hslider.handler)();
						}
						return;
					default:
						break;
				}
			}
			break;
		case KeyCodeUp: case KeyCodeDown:
			if (MenuButtonCurSel != -1) {
				mi = menu->Items + MenuButtonCurSel;
				if (!(mi->flags & MenuButtonClicked)) {
					switch (mi->mitype) {
						case MI_TYPE_PULLDOWN:
							if (key == KeyCodeDown) {
								if (mi->d.pulldown.curopt + 1 < mi->d.pulldown.noptions) {
									mi->d.pulldown.curopt++;
								} else {
									break;
								}
							} else {
								if (mi->d.pulldown.curopt > 0) {
									mi->d.pulldown.curopt--;
								} else {
									break;
								}
							}
							if (mi->d.pulldown.action) {
								(*mi->d.pulldown.action)(mi, mi->d.pulldown.curopt);
							}
							break;
						case MI_TYPE_LISTBOX:
							if (key == KeyCodeDown) {
								if (mi->d.listbox.curopt+mi->d.listbox.startline+1 < mi->d.pulldown.noptions) {
									mi->d.listbox.curopt++;
									if (mi->d.listbox.curopt >= mi->d.listbox.nlines) {
										mi->d.listbox.curopt--;
										mi->d.listbox.startline++;
									}
								} else {
									break;
								}
							} else {
								if (mi->d.listbox.curopt+mi->d.listbox.startline > 0) {
									mi->d.listbox.curopt--;
									if (mi->d.listbox.curopt < 0) {
										mi->d.listbox.curopt++;
										mi->d.listbox.startline--;
									}
								} else {
									break;
								}
							}
							if (mi->d.listbox.action) {
								(*mi->d.listbox.action)(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
							}
							break;
						case MI_TYPE_VSLIDER:
							if (key == KeyCodeDown) {
								mi->d.vslider.cflags |= MI_CFLAGS_DOWN;
							} else {
								mi->d.vslider.cflags |= MI_CFLAGS_UP;
							}
							if (mi->d.vslider.action) {
								(*mi->d.vslider.action)(mi, 2);
							}
							break;
						default:
							break;
					}
				}
			}
			break;
		case KeyCodeLeft:
		case KeyCodeRight:
			if (MenuButtonCurSel != -1) {
				mi = menu->Items + MenuButtonCurSel;
				if (!(mi->flags & MenuButtonClicked)) {
					switch (mi->mitype) {
						case MI_TYPE_HSLIDER:
							if (key == KeyCodeLeft) {
								mi->d.hslider.percent -= 10;
								if (mi->d.hslider.percent < 0) {
									mi->d.hslider.percent = 0;
								}
							} else {
								mi->d.hslider.percent += 10;
								if (mi->d.hslider.percent > 100) {
									mi->d.hslider.percent = 100;
								}
							}
							if (mi->d.hslider.action) {
								(*mi->d.hslider.action)(mi);
							}
							break;
						default:
							break;
					}
				}
			}
			break;
		case 9:								// TAB						// FIXME: Add Shift-TAB
			if (KeyModifiers & ModifierAlt) {
				break;
			}
			if (MenuButtonCurSel != -1 && !(menu->Items[MenuButtonCurSel].flags & MenuButtonClicked)) {
				n = menu->NumItems;
				for (i = 0; i < n; ++i) {
					mi = menu->Items + ((MenuButtonCurSel + i + 1) % n);
					switch (mi->mitype) {
						case MI_TYPE_PULLDOWN:
							if ((mi->d.pulldown.state & MI_PSTATE_PASSIVE)) {
								continue;
							}
							/* FALL THROUGH */
						case MI_TYPE_BUTTON:
						case MI_TYPE_LISTBOX:
						case MI_TYPE_VSLIDER:
						case MI_TYPE_HSLIDER:
						case MI_TYPE_INPUT:
							if (mi->flags & MenuButtonDisabled) {
								break;
							}
							mi->flags |= MenuButtonSelected;
							menu->Items[MenuButtonCurSel].flags &= ~MenuButtonSelected;
							MenuButtonCurSel = mi - menu->Items;
							return;
						default:
							break;
					}
				}
			}
			break;
		case 'x':
		case 'X':
			if ((KeyModifiers & ModifierAlt)) {
				Exit(0);
			}
		default:
			mi = menu->Items;
			i = menu->NumItems;
			while (i--) {
				switch (mi->mitype) {
					case MI_TYPE_INPUT:
						if (!(mi->flags & MenuButtonDisabled)) {
							if (MenuButtonCurSel != -1) {
								menu->Items[MenuButtonCurSel].flags &=
									~MenuButtonSelected;
							}
							mi->flags |= MenuButtonSelected;
							MenuButtonCurSel = mi - menu->Items;
							goto inkey;
						}
					default:
						break;
				}
				++mi;
			}
			DebugLevel3("Key %d\n" _C_ key);
			return;
	}
	return;
}

/**
**		Handle keys in menu mode.
**
**		@param key		Key scancode.
**		@param keychar		ASCII character code of key.
*/
local void MenuHandleKeyUp(unsigned key, unsigned keychar)
{
	HandleKeyModifiersUp(key, keychar);
}

/**
**		Handle keys repeated in menu mode.
**
**		@param key		Key scancode.
**		@param keychar		ASCII character code of key.
*/
local void MenuHandleKeyRepeat(unsigned key, unsigned keychar)
{
	HandleKeyModifiersDown(key, keychar);

	if (CurrentMenu == NULL) {
		return;
	}

	if (MenuButtonCurSel != -1 && CurrentMenu->Items[MenuButtonCurSel].mitype == MI_TYPE_INPUT) {
		MenuHandleKeyDown(key, keychar);
	}
}

/**
**		Handle movement of the cursor.
**
**		@param x		Screen X position.
**		@param y		Screen Y position.
*/
local void MenuHandleMouseMove(int x, int y)
{
	int h;
	int w;
	int i;
	int j;
	int n;
	int xs;
	int ys;
	Menuitem* mi;
	Menu* menu;
	int ox;
	int oy;

	ox = CursorX;
	oy = CursorY;						// Old position for rel movement.
	HandleCursorMove(&x, &y);

	if (CurrentMenu == NULL) {
		return;
	}

	menu = CurrentMenu;

	n = menu->NumItems;
	MenuButtonUnderCursor = -1;

	// check active (popped-up) pulldown first, as it may overlay other menus!
	mi = menu->Items;
	for (i = 0; i < n; ++i) {
		if (!(mi->flags & MenuButtonDisabled)) {
			if (mi->mitype == MI_TYPE_PULLDOWN && (mi->flags & MenuButtonClicked)) {
				xs = menu->X + mi->xofs;
				ys = menu->Y + mi->yofs;
				if (mi->d.pulldown.button == MBUTTON_SC_PULLDOWN) {
					int usetop;

					h = mi->d.pulldown.ysize;
					if (mi->yofs + (h + 1) * mi->d.pulldown.noptions >= mi->menu->Height) {
						ys -= h * mi->d.pulldown.noptions;
						usetop = 0;
					} else {
						usetop = 1;
					}
					if (!(x < xs || x > xs + mi->d.pulldown.xsize || y < ys ||
							y > ys + (h + 1) * mi->d.pulldown.noptions)) {
						j = (y - ys) / h;
						if (usetop) {
							--j;
						} else {
							if (j == mi->d.pulldown.noptions) {
								j = -1;
							}
						}
						if (j >= -1 && j < mi->d.pulldown.noptions && j != mi->d.pulldown.cursel) {
							mi->d.pulldown.cursel = j;
							if (mi->d.pulldown.action) {
								(*mi->d.pulldown.action)(mi, mi->d.pulldown.cursel);
							}
						}
					}
					MenuButtonUnderCursor = i;
				} else {
					h = mi->d.pulldown.ysize - 2;
					if (ys + 1 <= mi->d.pulldown.curopt * h + CurrentMenu->Y) {
						ys = 2 + CurrentMenu->Y;
					} else {
						ys -= mi->d.pulldown.curopt * h;
						if (ys + h * mi->d.pulldown.noptions >= 480 + CurrentMenu->Y) {
							ys -= ys + h * mi->d.pulldown.noptions - (480 + CurrentMenu->Y);
						}
					}
					if (!(x < xs || x > xs + mi->d.pulldown.xsize || y < ys ||
							y > ys + h * mi->d.pulldown.noptions)) {
						j = (y - ys) / h;
						if (j >= 0 && j < mi->d.pulldown.noptions && j != mi->d.pulldown.cursel) {
							mi->d.pulldown.cursel = j;
							if (mi->d.pulldown.action) {
								(*mi->d.pulldown.action)(mi, mi->d.pulldown.cursel);
							}
						}
					}
					MenuButtonUnderCursor = i;
				}
				break;
			}
		}
		++mi;
	}
	if (MenuButtonUnderCursor == -1) {
		for (i = 0; i < n; ++i) {
			mi = menu->Items + i;
			if (!(mi->flags & MenuButtonDisabled)) {
				switch (mi->mitype) {
					case MI_TYPE_TEXT:
						if (!mi->d.text.text || !mi->d.text.action)
							continue;
						xs = menu->X + mi->xofs;
						ys = menu->Y + mi->yofs;
						if (x < xs - 4 || x > xs + VideoTextLength(mi->font, mi->d.text.text) + 5 ||
								y < ys - 4 || y > ys + VideoTextHeight(mi->font) + 5) {
							if (!(mi->flags & MenuButtonClicked)) {
								if (mi->flags & MenuButtonActive) {
									mi->flags &= ~MenuButtonActive;
								}
							}
							continue;
						}
						break;
					case MI_TYPE_GEM:
						xs = menu->X + mi->xofs;
						ys = menu->Y + mi->yofs;
						if ((!mi->d.gem.text || x < xs - 1 || x > xs +
								VideoTextLength(GameFont, mi->d.gem.text) + 28 ||
								y < ys - 2 ||  y > ys + VideoTextHeight(GameFont) + 9) &&
								(x < xs ||  x > xs + mi->d.gem.xsize || y < ys ||
								y > ys + mi->d.gem.ysize)) {
							if (!(mi->flags & MenuButtonClicked)) {
								if (mi->flags & MenuButtonActive) {
									mi->flags &= ~MenuButtonActive;
								}
							}
							continue;
						}
						break;
					case MI_TYPE_BUTTON:
						xs = menu->X + mi->xofs;
						ys = menu->Y + mi->yofs;
						if (x < xs || x > xs + mi->d.button.xsize || y < ys ||
								y > ys + mi->d.button.ysize) {
							if (!(mi->flags & MenuButtonClicked)) {
								if (mi->flags & MenuButtonActive) {
									mi->flags &= ~MenuButtonActive;
								}
							}
							continue;
						}
						break;
					case MI_TYPE_INPUT:
						xs = menu->X + mi->xofs;
						ys = menu->Y + mi->yofs;
						if (x < xs || x > xs + mi->d.input.xsize
								|| y < ys || y > ys + mi->d.input.ysize) {
							if (!(mi->flags & MenuButtonClicked)) {
								if (mi->flags & MenuButtonActive) {
									mi->flags &= ~MenuButtonActive;
								}
							}
							continue;
						}
						break;
					case MI_TYPE_PULLDOWN:
						if ((mi->d.pulldown.state & MI_PSTATE_PASSIVE)) {
							continue;
						}
						// Clicked-state already checked above - there can only be one!
						xs = menu->X + mi->xofs;
						ys = menu->Y + mi->yofs;
						if (x < xs || x > xs + mi->d.pulldown.xsize || y<ys ||
								y > ys + mi->d.pulldown.ysize) {
							if (!(mi->flags & MenuButtonClicked)) {
								if (mi->flags & MenuButtonActive) {
									mi->flags &= ~MenuButtonActive;
								}
							}
							continue;
						}
						break;
					case MI_TYPE_LISTBOX:
						xs = menu->X + mi->xofs;
						ys = menu->Y + mi->yofs;
						if (x < xs || x > xs + mi->d.listbox.xsize || y < ys ||
								y > ys + mi->d.listbox.ysize) {
							if (!(mi->flags & MenuButtonClicked)) {
								if (mi->flags & MenuButtonActive) {
									mi->flags &= ~MenuButtonActive;
								}
							}
							continue;
						}
						j = (y - ys) / 18;
						if (j != mi->d.listbox.cursel) {
							mi->d.listbox.cursel = j;		// just store for click
						}
						if (mi->flags & MenuButtonClicked && mi->flags & MenuButtonActive) {
							if (mi->d.listbox.cursel != mi->d.listbox.curopt) {
								mi->d.listbox.dohandler = 0;
								mi->d.listbox.curopt = mi->d.listbox.cursel;
								if (mi->d.listbox.action) {
									(*mi->d.listbox.action)(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
								}
							}
						}
						break;
					case MI_TYPE_VSLIDER:
					{
						int arrowsize;

						xs = menu->X + mi->xofs;
						ys = menu->Y + mi->yofs;
						if (x < xs || x > xs + mi->d.vslider.xsize || y < ys ||
								y > ys + mi->d.vslider.ysize) {
							if (!(mi->flags & MenuButtonClicked)) {
								if (mi->flags & MenuButtonActive) {
									mi->flags &= ~MenuButtonActive;
								}
							}
							if (y < ys || y > ys + mi->d.vslider.ysize ||
									!(mi->flags & MenuButtonClicked)) {
								mi->d.vslider.cursel = 0;
								continue;
							}
						}
						j = y - ys;
						mi->d.vslider.cursel = 0;

						if (mi->d.vslider.style == MI_STYLE_SC_VSLIDER) {
							arrowsize = 16;
						} else {
							arrowsize = 20;
						}

						if (j < arrowsize) {
							mi->d.vslider.cursel |= MI_CFLAGS_UP;
						} else if (j >= mi->d.vslider.ysize - arrowsize - 1) {
							mi->d.vslider.cursel |= MI_CFLAGS_DOWN;
						} else {
							mi->d.vslider.cursel &= ~(MI_CFLAGS_UP | MI_CFLAGS_DOWN);
							h = (mi->d.vslider.percent * (mi->d.vslider.ysize - 54)) / 100 + 18;
							if (j > h && j < h + 16) {
								mi->d.vslider.cursel |= MI_CFLAGS_KNOB;
							} else {
								mi->d.vslider.cursel |= MI_CFLAGS_CONT;
								if (j <= h) {
									mi->d.vslider.cursel |= MI_CFLAGS_UP;
								} else {
									mi->d.vslider.cursel |= MI_CFLAGS_DOWN;
								}
							}
							j -= 8;
							if (j < arrowsize) {
								j = arrowsize;
							}

							mi->d.vslider.curper = ((j - arrowsize) * 100) / (mi->d.vslider.ysize - 54);
							if (mi->d.vslider.curper > 100) {
								mi->d.vslider.curper = 100;
							}
						}
						if (mi->d.vslider.action) {
							(*mi->d.vslider.action)(mi, 1);				// 1 indicates move
						}
						break;
					}
					case MI_TYPE_HSLIDER:
					{
						int arrowsize;

						xs = menu->X + mi->xofs;
						ys = menu->Y + mi->yofs;
						if (x < xs || x > xs + mi->d.hslider.xsize || y < ys ||
								y > ys + mi->d.hslider.ysize) {
							if (!(mi->flags & MenuButtonClicked)) {
								if (mi->flags & MenuButtonActive) {
									mi->flags &= ~MenuButtonActive;
								}
							}
							if (x < xs || x > xs + mi->d.hslider.xsize || !(mi->flags & MenuButtonClicked)) {
								mi->d.hslider.cursel = 0;
								continue;
							}
						}
						j = x - xs;
						mi->d.hslider.cursel = 0;

						if (mi->d.hslider.style == MI_STYLE_SC_HSLIDER) {
							arrowsize = 16;
						} else {
							arrowsize = 20;
						}

						if (j < arrowsize) {
							mi->d.hslider.cursel |= MI_CFLAGS_LEFT;
						} else if (j >= mi->d.hslider.xsize - arrowsize-1) {
							mi->d.hslider.cursel |= MI_CFLAGS_RIGHT;
						} else {
							mi->d.hslider.cursel &= ~(MI_CFLAGS_LEFT | MI_CFLAGS_RIGHT);
							w = (mi->d.hslider.percent * (mi->d.hslider.xsize - 54)) / 100 + 18;
							if (j > w && j < w + 16) {
								mi->d.hslider.cursel |= MI_CFLAGS_KNOB;
							} else {
								mi->d.hslider.cursel |= MI_CFLAGS_CONT;
								if (j <= w) {
									mi->d.hslider.cursel |= MI_CFLAGS_LEFT;
								} else {
									mi->d.hslider.cursel |= MI_CFLAGS_RIGHT;
								}
							}
							j -= 8;
							if (j < arrowsize) {
								j = arrowsize;
							}

							mi->d.hslider.curper = ((j - arrowsize) * 100) / (mi->d.hslider.xsize - 54);
							if (mi->d.hslider.curper > 100) {
								mi->d.hslider.curper = 100;
							}
						}
						if ((mi->d.hslider.cflags & MI_CFLAGS_KNOB) && (mi->flags & MenuButtonClicked)) {
							mi->d.hslider.percent = mi->d.hslider.curper;
							if (mi->d.hslider.action) {
								(*mi->d.hslider.action)(mi);
							}
						}
						break;
					}
					default:
						continue;
						// break;
				}
				switch (mi->mitype) {
					case MI_TYPE_GEM:
						if ((mi->d.gem.state & (MI_GSTATE_PASSIVE | MI_GSTATE_INVISIBLE))) {
							break;
						}
						/* FALL THROUGH */
					case MI_TYPE_BUTTON:
					case MI_TYPE_PULLDOWN:
					case MI_TYPE_LISTBOX:
					case MI_TYPE_VSLIDER:
					case MI_TYPE_HSLIDER:
					case MI_TYPE_TEXT:
						if (!(mi->flags & MenuButtonActive)) {
							mi->flags |= MenuButtonActive;
						}
						MenuButtonUnderCursor = i;
					default:
						break;
					case MI_TYPE_INPUT:
						if (!(mi->flags & MenuButtonActive)) {
							mi->flags |= MenuButtonActive;
						}
						if (MouseButtons & LeftButton
								&& mi->flags & MenuButtonSelected) {
							if (mi->d.input.buffer && *mi->d.input.buffer) {
								char* s;

								j = strtol(mi->d.input.buffer, &s, 0);
								if ((!*s || s[0] == '~') && (j != 0 || *mi->d.input.buffer == '0')) {
									int num;
									num = j + x - ox + (y - oy) * 1000;
									if (num < 0) {
										num = 0;
									}
									if ((mi->d.input.maxch == 3 && num < 1000) ||
										(mi->d.input.maxch == 4 && num < 10000) ||
										(mi->d.input.maxch == 5 && num < 100000) ||
										(mi->d.input.maxch == 6 && num < 1000000) ||
										(mi->d.input.maxch >= 7)) {
										mi->d.input.nch =
											sprintf(mi->d.input.buffer, "%d~!_", num) - 3;
									}
								}
							}
						}
						MenuButtonUnderCursor = i;
						break;
				}
			}
		}
	}
}

/**
**		Called if mouse button pressed down.
**
**		@param b		button code
*/
local void MenuHandleButtonDown(unsigned b __attribute__((unused)))
{
	Menuitem* mi;
	Menu* menu;

	if (CurrentMenu == NULL) {
		return;
	}

	menu = CurrentMenu;

	if (MouseButtons & (LeftButton << MouseHoldShift)) {
		return;
	}

	if (MouseButtons & LeftButton) {
		if (MenuButtonUnderCursor != -1) {
			mi = menu->Items + MenuButtonUnderCursor;
			if (!(mi->flags & MenuButtonClicked)) {
				switch (mi->mitype) {
					case MI_TYPE_GEM:
					case MI_TYPE_BUTTON:
					case MI_TYPE_PULLDOWN:
					case MI_TYPE_LISTBOX:
					case MI_TYPE_VSLIDER:
					case MI_TYPE_HSLIDER:
					case MI_TYPE_INPUT:
					case MI_TYPE_TEXT:
						if (MenuButtonCurSel != -1) {
							menu->Items[MenuButtonCurSel].flags &= ~MenuButtonSelected;
						}
						MenuButtonCurSel = MenuButtonUnderCursor;
						mi->flags |= MenuButtonClicked | MenuButtonSelected;
					default:
						break;
				}
			}
			PlayGameSound(GameSounds.Click.Sound, MaxSampleVolume);
			switch (mi->mitype) {
				case MI_TYPE_VSLIDER:
					mi->d.vslider.cflags = mi->d.vslider.cursel;
					if (mi->d.vslider.action) {
						(*mi->d.vslider.action)(mi, 0);				// 0 indicates down
					}
					break;
				case MI_TYPE_HSLIDER:
					mi->d.hslider.cflags = mi->d.hslider.cursel;
					if (mi->d.hslider.cflags & MI_CFLAGS_RIGHT) {
						mi->d.hslider.percent += 10;
						if (mi->d.hslider.percent > 100)
							mi->d.hslider.percent = 100;
					} else if (mi->d.hslider.cflags & MI_CFLAGS_LEFT) {
						mi->d.hslider.percent -= 10;
						if (mi->d.hslider.percent < 0)
							mi->d.hslider.percent = 0;
					}
					if (mi->d.hslider.action) {
						(*mi->d.hslider.action)(mi);
					}
					break;
				case MI_TYPE_PULLDOWN:
					if (mi->d.pulldown.curopt >= 0 &&
							mi->d.pulldown.curopt < mi->d.pulldown.noptions) {
						mi->d.pulldown.cursel = mi->d.pulldown.curopt;
					}
					break;
				case MI_TYPE_LISTBOX:
					if (mi->d.listbox.cursel != mi->d.listbox.curopt) {
						mi->d.listbox.dohandler = 0;
						mi->d.listbox.curopt = mi->d.listbox.cursel;
						if (mi->d.listbox.action) {
							(*mi->d.listbox.action)(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
						}
					}
					else {
						mi->d.listbox.dohandler = 1;
					}
					break;
				default:
					break;
			}
		}
	}

	if (MouseButtons & MiddleButton) {
		if (MenuButtonUnderCursor != -1) {
			mi = menu->Items + MenuButtonUnderCursor;
			if (!(mi->flags & MenuButtonClicked)) {
				switch (mi->mitype) {
					case MI_TYPE_INPUT:
						PasteFromClipboard(mi);
						if (mi->d.input.action) {
							(*mi->d.input.action)(mi, 'x');
						}
						break;
					default:
						break;
				}
			}
		}
	}

	// mousewheel up
	if (MouseButtons & UpButton) {
		if (MenuButtonUnderCursor != -1) {
			mi = menu->Items + MenuButtonUnderCursor;
			switch (mi->mitype) {
				case MI_TYPE_LISTBOX:
					if (mi->d.listbox.curopt < 0) {
						mi->d.listbox.curopt = 0;
					}
					if (mi->d.listbox.startline > 0) {
						mi->d.listbox.startline--;
						if (mi->d.listbox.curopt != mi->d.listbox.nlines - 1) {
							mi->d.listbox.curopt++;
						}
					} else {
						if (mi->d.listbox.curopt != 0) {
							mi->d.listbox.curopt--;
						}
					}
					if (mi->d.listbox.action) {
						(*mi->d.listbox.action)(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
					}
					mi[1].d.vslider.percent = 100 * (mi->d.listbox.curopt + mi->d.listbox.startline)
						/ (mi->d.listbox.noptions - 1);
					break;
				case MI_TYPE_VSLIDER:
					mi->d.vslider.cflags |= MI_CFLAGS_UP;
					if (mi->d.vslider.action) {
						(*mi->d.vslider.action)(mi, 2);
					}
					break;
				case MI_TYPE_HSLIDER:
					mi->d.hslider.percent -= 10;
					if (mi->d.hslider.percent < 0) {
						mi->d.hslider.percent = 0;
					}
					if (mi->d.hslider.action) {
						(*mi->d.hslider.action)(mi);
					}
					break;
				case MI_TYPE_PULLDOWN:
					if (mi->d.pulldown.curopt) {
						--mi->d.pulldown.curopt;
						if (mi->d.pulldown.action) {
							(*mi->d.pulldown.action)(mi, mi->d.pulldown.curopt);
						}
					}
					break;
				default:
					break;
			}
		}
	}

	// mousewheel down
	if (MouseButtons & DownButton) {
		if (MenuButtonUnderCursor != -1) {
			mi = menu->Items + MenuButtonUnderCursor;
			switch (mi->mitype) {
				case MI_TYPE_LISTBOX:
					if (mi->d.listbox.curopt < 0)
						mi->d.listbox.curopt = 0;
					if (mi->d.listbox.startline < mi->d.listbox.noptions - mi->d.listbox.nlines) {
						mi->d.listbox.startline++;
						if (mi->d.listbox.curopt != 0)
							mi->d.listbox.curopt--;
					} else {
						if (mi->d.listbox.curopt != mi->d.listbox.nlines - 1 &&
							mi->d.listbox.curopt != mi->d.listbox.noptions - 1)
								mi->d.listbox.curopt++;
					}
					if (mi->d.listbox.action) {
						(*mi->d.listbox.action)(mi, mi->d.listbox.curopt + mi->d.listbox.startline);
					}
					mi[1].d.vslider.percent = 100 * (mi->d.listbox.curopt + mi->d.listbox.startline)
						/ (mi->d.listbox.noptions - 1);
					break;
				case MI_TYPE_VSLIDER:
					mi->d.vslider.cflags |= MI_CFLAGS_DOWN;
					if (mi->d.vslider.action) {
						(*mi->d.vslider.action)(mi, 2);
					}
					break;
				case MI_TYPE_HSLIDER:
					mi->d.hslider.percent += 10;
					if (mi->d.hslider.percent > 100) {
						mi->d.hslider.percent = 100;
					}
					if (mi->d.hslider.action) {
						(*mi->d.hslider.action)(mi);
					}
					break;
				case MI_TYPE_PULLDOWN:
					if (mi->d.pulldown.curopt < mi->d.pulldown.noptions - 1) {
						++mi->d.pulldown.curopt;
						if (mi->d.pulldown.action) {
							(*mi->d.pulldown.action)(mi, mi->d.pulldown.curopt);
						}
					}
					break;
				default:
					break;
			}
		}
	}
}

/**
**		Called if mouse button released.
**
**		@param b		button code
*/
local void MenuHandleButtonUp(unsigned b)
{
	int i;
	int n;
	Menuitem* mi;
	Menu* menu;
	int redraw_flag;

	if (CurrentMenu == NULL) {
		return;
	}

	redraw_flag = 0;
	menu = CurrentMenu;

	if ((1 << b) == LeftButton) {
		n = menu->NumItems;
		for (i = 0; i < n; ++i) {
			mi = menu->Items + i;
			switch (mi->mitype) {
				case MI_TYPE_GEM:
					if (mi->flags & MenuButtonClicked) {
						redraw_flag = 1;
						mi->flags &= ~MenuButtonClicked;
						if (MenuButtonUnderCursor == i) {
							MenuButtonUnderCursor = -1;
							if ((mi->d.gem.state & MI_GSTATE_CHECKED)) {
								mi->d.gem.state &= ~MI_GSTATE_CHECKED;
							} else {
								mi->d.gem.state |= MI_GSTATE_CHECKED;
							}
							if (mi->d.gem.action) {
								(*mi->d.gem.action)(mi);
							}
						}
					}
					break;
				case MI_TYPE_TEXT:
					if (mi->flags & MenuButtonClicked) {
						redraw_flag = 1;
						mi->flags &= ~MenuButtonClicked;
						if (MenuButtonUnderCursor == i) {
							MenuButtonUnderCursor = -1;
							if (mi->d.text.action) {
								(*mi->d.text.action)(mi);
							}
						}
					}
					break;
				case MI_TYPE_BUTTON:
					if (mi->flags & MenuButtonClicked) {
						redraw_flag = 1;
						mi->flags &= ~MenuButtonClicked;
						if (MenuButtonUnderCursor == i) {
							MenuButtonUnderCursor = -1;
							if (mi->d.button.handler) {
								(*mi->d.button.handler)();
							}
						}
					}
					break;
				case MI_TYPE_PULLDOWN:
					if (mi->flags & MenuButtonClicked) {
						redraw_flag = 1;
						mi->flags &= ~MenuButtonClicked;
						if (MenuButtonUnderCursor == i) {
							MenuButtonUnderCursor = -1;
							if (mi->d.pulldown.cursel != mi->d.pulldown.curopt &&
									mi->d.pulldown.cursel >= 0 &&
									mi->d.pulldown.cursel < mi->d.pulldown.noptions) {
								mi->d.pulldown.curopt = mi->d.pulldown.cursel;
								if (mi->d.pulldown.action) {
									(*mi->d.pulldown.action)(mi, mi->d.pulldown.curopt);
								}
							}
						}
						mi->d.pulldown.cursel = 0;
					}
					break;
				case MI_TYPE_LISTBOX:
					if (mi->flags & MenuButtonClicked) {
						redraw_flag = 1;
						mi->flags &= ~MenuButtonClicked;
						if (MenuButtonUnderCursor == i) {
							MenuButtonUnderCursor = -1;
							if (mi->d.listbox.dohandler && mi->d.listbox.handler) {
								(*mi->d.listbox.handler)();
							}
						}
					}
					break;
				case MI_TYPE_INPUT:
					if (mi->flags & MenuButtonClicked) {
						redraw_flag = 1;
						mi->flags &= ~MenuButtonClicked;
						// MAYBE ADD HERE
					}
					break;
				case MI_TYPE_VSLIDER:
					if (mi->flags & MenuButtonClicked) {
						redraw_flag = 1;
						mi->flags &= ~MenuButtonClicked;
						mi->d.vslider.cflags = 0;
					}
					break;
				case MI_TYPE_HSLIDER:
					if (mi->flags & MenuButtonClicked) {
						redraw_flag = 1;
						mi->flags &= ~MenuButtonClicked;
						mi->d.hslider.cflags = 0;
					}
					break;
				default:
					break;
			}
		}
	}
	if (redraw_flag) {
		MenuHandleMouseMove(CursorX, CursorY);
	}
}

typedef struct _menu_stack_ {
	Menu* Menu;
	int CurSel;
	struct _menu_stack_* Next;
} MenuStack;

MenuStack* Menus;

/**
**		Push the current menu onto the stack.
*/
local void PushMenu(void)
{
	MenuStack* menu;

	menu = malloc(sizeof(MenuStack));
	menu->Menu = CurrentMenu;
	menu->CurSel = MenuButtonCurSel;
	menu->Next = Menus;
	Menus = menu;
}

/**
**		Pop the stack and set the current menu
*/
local void PopMenu(void)
{
	MenuStack* menu;
	Menuitem* mi;
	int i;

	if (Menus && Menus->Menu == CurrentMenu) {
		for (i = 0; i < CurrentMenu->NumItems; ++i) {
			mi = CurrentMenu->Items + i;
			if (mi->exitfunc) {
				(*mi->exitfunc)(mi);				// action/destructor
			}
		}

		MenuButtonUnderCursor = -1;
		MenuButtonCurSel = -1;
		CurrentMenu = NULL;

		MenuButtonCurSel = Menus->CurSel;
		menu = Menus;
		Menus = Menus->Next;
		free(menu);
		if (Menus) {
			CurrentMenu = Menus->Menu;
		}
	} else {
		CurrentMenu = NULL;
	}
}

/**
**  End process menu
*/
global void EndMenu(void)
{
	CursorOn = CursorOnUnknown;
	PopMenu();

	if (!CurrentMenu && Callbacks != &GameCallbacks &&
			(GameRunning || GameResult != GameNoResult)) {
		InterfaceState = IfaceStateNormal;
		Callbacks = &GameCallbacks;
		GamePaused = 0;
		UIHandleMouseMove(CursorX, CursorY);
	}
}

/**
**		Process a menu.
**
**		@param menu_id		The menu number to process
**		@param loop		Indicates to setup handlers and really 'Process'
**
**		@todo FIXME: This function is called from the event handler!!
*/
global void ProcessMenu(const char* menu_id, int loop)
{
	int i;
	int oldncr;
	Menuitem* mi;
	Menu* menu;
	Menu* CurrentMenuSave;
	int MenuButtonUnderCursorSave;
	int MenuButtonCurSelSave;

	CurrentMenuSave = NULL;
	MenuButtonUnderCursorSave = -1;
	MenuButtonCurSelSave = -1;

	CancelBuildingMode();

	// Recursion protection:
	if (loop) {
		CurrentMenuSave = CurrentMenu;
		MenuButtonUnderCursorSave = MenuButtonUnderCursor;
		MenuButtonCurSelSave = MenuButtonCurSel;
		InterfaceState = IfaceStateMenu;
	}

	ButtonUnderCursor = -1;
	CursorState = CursorStatePoint;
	GameCursor = TheUI.Point.Cursor;
	menu = FindMenu(menu_id);
	if (menu == NULL) {
		return;
	}
	CurrentMenu = menu;

	if (!loop) {
		Callbacks = &MenuCallbacks;
		PushMenu();
	}

	MenuButtonCurSel = -1;
	for (i = 0; i < menu->NumItems; ++i) {
		mi = menu->Items + i;
		switch (mi->mitype) {
			case MI_TYPE_BUTTON:
			case MI_TYPE_PULLDOWN:
			case MI_TYPE_LISTBOX:
			case MI_TYPE_VSLIDER:
			case MI_TYPE_HSLIDER:
			case MI_TYPE_INPUT:
				mi->flags &= ~(MenuButtonClicked | MenuButtonActive | MenuButtonSelected);
				if (i == menu->DefSel) {
					mi->flags |= MenuButtonSelected;
					MenuButtonCurSel = i;
				}
				break;
		}
		switch (mi->mitype) {
			case MI_TYPE_PULLDOWN:
				mi->d.pulldown.cursel = 0;
				if (mi->d.pulldown.defopt != -1) {
					mi->d.pulldown.curopt = mi->d.pulldown.defopt;
				}
				break;
			case MI_TYPE_LISTBOX:
				mi->d.listbox.cursel = -1;
				mi->d.listbox.startline = 0;
				if (mi->d.listbox.defopt != -1) {
					mi->d.listbox.curopt = mi->d.listbox.defopt;
				}
				break;
			case MI_TYPE_VSLIDER:
				mi->d.vslider.cflags = 0;
				if (mi->d.vslider.defper != -1) {
					mi->d.vslider.percent = mi->d.vslider.defper;
				}
				break;
			case MI_TYPE_HSLIDER:
				mi->d.hslider.cflags = 0;
				if (mi->d.hslider.defper != -1) {
					mi->d.hslider.percent = mi->d.hslider.defper;
				}
				break;
			default:
				break;
		}
		if (mi->initfunc) {
			(*mi->initfunc)(mi);
		}
	}
	MenuButtonUnderCursor = -1;

	if (loop) {
		SetVideoSync();
		MenuHandleMouseMove(CursorX,CursorY);		// This activates buttons as appropriate!
	}

	if (loop) {
		while (CurrentMenu != NULL) {
			PlayListAdvance();
			if (!(FrameCounter % ((VideoSyncSpeed * CYCLES_PER_SECOND) / 50))) {
				PlaySectionMusic(PlaySectionUnknown);
			}

			InterfaceState = IfaceStateNormal;
			UpdateDisplay();
			InterfaceState = IfaceStateMenu;

			RealizeVideoMemory();
			oldncr = NetConnectRunning;
			WaitEventsOneFrame(&MenuCallbacks);
			if (NetConnectRunning == 2) {
				NetworkProcessClientRequest();
			}
			if (NetConnectRunning == 1) {
				NetworkProcessServerRequest();
			}
			// stopped by network activity?
			if (oldncr == 2 && NetConnectRunning == 0) {
				if (menu->NetAction) {
					(*menu->NetAction)();
				}
			}
		}
	}

	if (loop) {
		for (i = 0; i < menu->NumItems; ++i) {
			mi = menu->Items + i;
			if (mi->exitfunc) {
				(*mi->exitfunc)(mi);				// action/destructor
			}
		}
		CurrentMenu = CurrentMenuSave;
		MenuButtonUnderCursor = MenuButtonUnderCursorSave;
		MenuButtonCurSel = MenuButtonCurSelSave;
	}

	// FIXME: should ExitMenus() be called instead?!?
	if (Menusbgnd) {
		VideoFree(Menusbgnd);
		Menusbgnd = NULL;
	}
}

/**
**		Init Menus for a specific race
**
**		@param race		The Race to set-up for
*/
global void InitMenus(int race)
{
	static int last_race = -1;
	const char* file;
	char* buf;
	int width;
	int height;

	InitMenuData();
	InitMenuFunctions();

	if (race == last_race) {		// same race? already loaded!
		return;
	}

	if (last_race == -1) {
		MenuCallbacks.ButtonPressed = &MenuHandleButtonDown;
		MenuCallbacks.ButtonReleased = &MenuHandleButtonUp;
		MenuCallbacks.MouseMoved = &MenuHandleMouseMove;
		MenuCallbacks.MouseExit = &HandleMouseExit;
		MenuCallbacks.KeyPressed = &MenuHandleKeyDown;
		MenuCallbacks.KeyReleased = &MenuHandleKeyUp;
		MenuCallbacks.KeyRepeated = &MenuHandleKeyRepeat;
		MenuCallbacks.NetworkEvent = NetworkEvent;
	} else {
		// free previous sprites for different race
		VideoFree(MenuButtonGfx.Sprite);
	}
	last_race = race;
	file = MenuButtonGfx.File[race];
	buf = alloca(strlen(file) + 9 + 1);
	file = strcat(strcpy(buf, "graphics/"), file);
	width = MenuButtonGfx.Width[race];
	height = MenuButtonGfx.Height[race];
	MenuButtonGfx.Sprite = LoadSprite(file, width, height);

	CurrentMenu = NULL;
}

/**
**		Exit Menus code (freeing data)
*/
global void ExitMenus(void)
{
	if (Menusbgnd) {
		VideoFree(Menusbgnd);
		Menusbgnd = NULL;
	}
}
