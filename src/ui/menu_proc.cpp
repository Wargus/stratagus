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
//      (c) Copyright 1999-2005 by Andreas Arens, Jimmy Salmon, Nehal Mistry
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

#ifdef USE_WIN32
#ifdef _MSC_VER
#undef NOUSER
#endif
#include <windows.h>
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

#include "SDL.h"
#include "SDL_syswm.h"

/*----------------------------------------------------------------------------
-- Variables
----------------------------------------------------------------------------*/

/**
**  Menu button graphics
*/
CGraphic *MenuButtonGraphics[MAX_RACES];

/**
**  Current menu button graphics
*/
CGraphic *MenuButtonG;

/**
**  Current menu
*/
Menu *CurrentMenu;

static int MenuButtonUnderCursor = -1;
static int MenuButtonCurSel = -1;

/*----------------------------------------------------------------------------
-- Menu operation functions
----------------------------------------------------------------------------*/

/**
** Find a menu by ident.
**
** @param menu_id Unique identifier for the menu.
**
** @return Pointer to the menu, NULL if menu is not found.
*/
Menu *FindMenu(const char *menu_id)
{
	return MenuMap[menu_id];
}

/**
**  Draw menu button 'button' on x,y
**
**  @param mit    Text to display.
**  @param x      X display position
**  @param y      Y display position
**  @param font   Font to use.
**  @param flag   flag of text.
*/
static void DrawMenuText(const MenuitemText *mit, int x, int y, CFont *font, int flag)
{
	char *oldnc;
	char *oldrc;
	char *nc;
	char *rc;
	char *text;
	int l;

	text = EvalString(mit->text);
	l = font->Width(text);
	GetDefaultTextColors(&oldnc, &oldrc);
	if (mit->normalcolor || mit->reversecolor) {
		nc = mit->normalcolor ? mit->normalcolor : oldnc;
		rc = mit->reversecolor ? mit->reversecolor : oldrc;
		SetDefaultTextColors(nc, rc);
	} else {
		nc = oldnc;
		rc = oldrc;
	}
	if (flag & MI_FLAGS_ACTIVE && mit->action) {
		Video.DrawRectangle(ColorGray, x - 4, y - 4, l + 5, font->Height() + 5);
		SetDefaultTextColors(nc, rc);
	}
	if (mit->Align == TextAlignCenter) {
		VideoDrawTextCentered(x, y,	font, text);
	} else if (mit->Align == TextAlignRight) {
		VideoDrawText(x - l, y,	font, text);
	} else {
		VideoDrawText(x, y,	font, text);
	}
	SetDefaultTextColors(oldnc, oldrc);
	delete[] text;
}

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
	char *nc;
	char *rc;
	char *oldnc;
	char *oldrc;
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

#if 0
	if (button == MBUTTON_SC_BUTTON || button == MBUTTON_SC_BUTTON_LEFT ||
			button == MBUTTON_SC_BUTTON_RIGHT) {
		if (flags & MI_FLAGS_DISABLED) {
			rb = button - 3;
			SetDefaultTextColors(FontGrey, FontGrey);
		} else if (flags & MI_FLAGS_CLICKED) {
			rb = button + 3;
			SetDefaultTextColors(rc, rc);
		} else {
			rb = button;
			if (flags & MI_FLAGS_ACTIVE) {
				SetDefaultTextColors(rc, rc);
			}
		}
		VideoDraw(MenuButtonG, rb - 1, x, y);
		for (s = x + 8; s < x + w - 1 - 8; s += 8) {
			VideoDraw(MenuButtonG, rb, s, y);
		}
		VideoDraw(MenuButtonG, rb + 1, x + w - 1 - 8, y);
		if (text) {
			VideoDrawTextCentered(x + w / 2, y + (h - font->Height()) / 2,
				font, text);
		}
		if (flags & MI_FLAGS_SELECTED) {
			if (flags & MI_FLAGS_DISABLED) {
				VideoDrawRectangleClip(ColorGray, x, y, w - 1, h);
			} else {
				// FIXME: wrong color
				VideoDrawRectangleClip(ColorRed, x, y, w - 1, h);
			}
		}
	} else {
	}
#endif
}

/**
** Draw pulldown 'button' on menu mx, my
**
** @param mi    menuitem pointer
** @param mx    menu X display position (offset)
** @param my    menu Y display position (offset)
*/
static void DrawPulldown(Menuitem *mi, int mx, int my)
{
	int i;
	char *nc;
	char *rc;
	char *oldnc;
	char *oldrc;
	char *text;
	unsigned flags;
	MenuButtonId rb;
	MenuButtonId db;
	int w;
	int h;
	int x;
	int y;
	int oh;

	x = mx + mi->XOfs;
	y = my + mi->YOfs;
	w = mi->D.Pulldown.xsize;
	flags = mi->Flags;
	rb = mi->D.Pulldown.button;

	GetDefaultTextColors(&oldnc, &oldrc);
	nc = oldnc;
	rc = oldrc;
	if (rb == MBUTTON_SC_PULLDOWN) {
		h = mi->D.Pulldown.ysize;
		if (flags & MI_FLAGS_CLICKED) {
			int usetop;
			int option;
			int max;

			// Check if the pulldown goes below the bottom
			if (mi->YOfs + (h + 1) * mi->D.Pulldown.noptions >= mi->Menu->Height) {
				y -= h * mi->D.Pulldown.noptions;
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
			MenuButtonG->DrawFrame(rb - 1, x, y);
			for (i = x + 16; i < x + w - 1 - 16; i += 16) {
				MenuButtonG->DrawFrame(rb, i, y);
			}
			MenuButtonG->DrawFrame(rb + 1, x + w - 1 - 16, y);
			option = 0;
			if (usetop) {
				MenuButtonG->DrawFrame(MBUTTON_SC_PULLDOWN_DOWN_ARROW,
					x + w - 1 - 16 - 3, y + 4);
				text = mi->D.Pulldown.options[mi->D.Pulldown.curopt];
				if (text) {
					VideoDrawText(x + 4, y + 2, mi->Font, text);
				}
			} else {
				if (option == mi->D.Pulldown.cursel) {
					SetDefaultTextColors(rc, rc);
				} else {
					SetDefaultTextColors(nc, rc);
				}
				text = mi->D.Pulldown.options[option];
				if (text) {
					VideoDrawText(x + 4, y + 2, mi->Font, text);
				}
				option = 1;
			}

			// Middle
			y += mi->D.Pulldown.ysize;
			rb = MBUTTON_SC_PULLDOWN_MIDDLE;
			if (usetop) {
				max = mi->D.Pulldown.noptions - 1;
			} else {
				max = mi->D.Pulldown.noptions;
			}
			for (; option < max; ++option) {
				MenuButtonG->DrawFrame(rb - 1, x, y);
				for (i = x + 16; i < x + w - 1 - 16; i += 16) {
					MenuButtonG->DrawFrame(rb, i, y);
				}
				MenuButtonG->DrawFrame(rb + 1, x + w - 1 - 16, y);
				if (option == mi->D.Pulldown.cursel) {
					SetDefaultTextColors(rc, rc);
				} else {
					SetDefaultTextColors(nc, rc);
				}
				text = mi->D.Pulldown.options[option];
				if (text) {
					VideoDrawText(x + 4, y + 2, mi->Font, text);
				}
				y += mi->D.Pulldown.ysize;
			}

			// Bottom
			SetDefaultTextColors(nc, rc);
			if (usetop) {
				rb = MBUTTON_SC_PULLDOWN_BOTTOM;
			} else {
				rb = MBUTTON_SC_PULLDOWN_BOTTOM_SELECTED;
			}
			MenuButtonG->DrawFrame(rb - 1, x, y);
			for (i = x + 16; i < x + w - 1 - 16; i += 16) {
				MenuButtonG->DrawFrame(rb, i, y);
			}
			MenuButtonG->DrawFrame(rb + 1, x + w - 1 - 16, y);
			if (usetop) {
				if (option == mi->D.Pulldown.cursel) {
					SetDefaultTextColors(rc, rc);
				} else {
					SetDefaultTextColors(nc, rc);
				}
				text = mi->D.Pulldown.options[option];
				if (text) {
					VideoDrawText(x + 4, y + 2, mi->Font, text);
				}
				option = 0;
			} else {
				MenuButtonG->DrawFrame(MBUTTON_SC_PULLDOWN_DOWN_ARROW,
					x + w - 1 - 16 - 3, y + 4);
				text = mi->D.Pulldown.options[mi->D.Pulldown.curopt];
				if (text) {
					VideoDrawText(x + 4, y + 2, mi->Font, text);
				}
				option = 1;
			}
		} else {
			h = mi->D.Pulldown.ysize;
			y = my + mi->YOfs;
			db = MBUTTON_SC_PULLDOWN_DOWN_ARROW;
			if (flags & MI_FLAGS_DISABLED) {
				rb = MBUTTON_SC_PULLDOWN_DISABLED;
				SetDefaultTextColors(FontGrey, FontGrey);
			} else {
				if (flags & MI_FLAGS_ACTIVE) {
					SetDefaultTextColors(rc, rc);
					++db;
				}
			}
			MenuButtonG->DrawFrame(rb - 1, x, y);
			for (i = x + 16; i < x + w - 1 - 16; i += 16) {
				MenuButtonG->DrawFrame(rb, i, y);
			}
			MenuButtonG->DrawFrame(rb + 1, x + w - 1 - 16, y);
			if (!(mi->Flags & MI_FLAGS_DISABLED)) {
				MenuButtonG->DrawFrame(db, x + w - 1 - 16 - 3, y + 4);
			}
			text = mi->D.Pulldown.options[mi->D.Pulldown.curopt];
			if (text) {
				VideoDrawText(x + 4, y + 2, mi->Font, text);
			}
		}
	} else {
		oh = h = mi->D.Pulldown.ysize - 2;
		if (flags & MI_FLAGS_CLICKED) {
			// Make the menu inside of the screen (TOP)
			if (y + 1 <= mi->D.Pulldown.curopt * h + CurrentMenu->Y) {
				y = 2 + CurrentMenu->Y;
			} else {
				y -= mi->D.Pulldown.curopt * h;
				// Make the menu inside the bottom of the screen
				// FIXME: can't assume bottom is always 480
				if (y + h * mi->D.Pulldown.noptions >= 480 + CurrentMenu->Y) {
					y -= y + h * mi->D.Pulldown.noptions - (480 + CurrentMenu->Y);
				}
			}
			i = mi->D.Pulldown.noptions;
			h *= i;
			while (i--) {
				PushClipping();
				SetClipping(0, 0, x + w, Video.Height - 1);
//				if (mi->transparent) {
//					VideoDrawClipTrans50(MenuButtonG, rb, x - 1, y - 1 + oh * i);
//				} else {
					MenuButtonG->DrawFrameClip(rb, x - 1, y - 1 + oh * i);
//				}
				PopClipping();
				text = mi->D.Pulldown.options[i];
				if (text) {
					if (i == mi->D.Pulldown.cursel) {
						SetDefaultTextColors(rc, rc);
					} else {
						SetDefaultTextColors(nc, rc);
					}
					VideoDrawText(x + 2, y + 2 + oh * i, mi->Font, text);
				}
			}
			w += 2;
			h += 2;
		} else {
			h = mi->D.Pulldown.ysize;
			y = my + mi->YOfs;
			if (flags & MI_FLAGS_DISABLED) {
				rb--;
				SetDefaultTextColors(FontGrey, FontGrey);
			} else {
				if (flags & MI_FLAGS_ACTIVE) {
					SetDefaultTextColors(rc, rc);
				}
			}

			PushClipping();
			if (!(mi->Flags & MI_FLAGS_DISABLED)) {
				SetClipping(0, 0, x + w - 20, Video.Height - 1);
			} else {
				SetClipping(0, 0, x + w - 1, Video.Height - 1);
			}
//			if (mi->transparent) {
//				VideoDrawClipTrans50(MenuButtonG, rb, x - 1, y - 1);
//			} else {
				MenuButtonG->DrawFrameClip(rb, x - 1, y - 1);
//			}
			PopClipping();
			if (!(mi->Flags & MI_FLAGS_DISABLED)) {
				MenuButtonG->DrawFrame(MBUTTON_DOWN_ARROW + rb - MBUTTON_PULLDOWN,
					x - 1 + w - 20, y - 2);
			}
			text = mi->D.Pulldown.options[mi->D.Pulldown.curopt];
			if (text) {
				VideoDrawText(x + 2, y + 2, mi->Font, text);
			}
		}
		if (flags & MI_FLAGS_SELECTED) {
			if (flags & MI_FLAGS_DISABLED) {
				Video.DrawRectangleClip(ColorGray, x - 2, y - 2, w, h);
			} else {
				Video.DrawRectangleClip(ColorYellow, x - 2, y - 2, w, h);
			}
		}
	}
	SetDefaultTextColors(oldnc, oldrc);
}

/**
** Draw listbox 'button' on menu mx, my
**
** @param mi    menuitem pointer
** @param mx    menu X display position (offset)
** @param my    menu Y display position (offset)
*/
static void DrawListbox(Menuitem *mi, int mx, int my)
{
	int i;
	int s;
	char *nc;
	char *rc;
	char *oldnc;
	char *oldrc;
	char *text;
	MenuButtonId rb;
	unsigned flags;
	int w;
	int h;
	int x;
	int y;

	rb = mi->D.Listbox.button;
	flags = mi->Flags;
	w = mi->D.Listbox.xsize;
	h = mi->D.Listbox.ysize;
	x = mx + mi->XOfs;
	y = my + mi->YOfs;

	GetDefaultTextColors(&oldnc, &oldrc);
	nc = oldnc;
	rc = oldrc;

	if (flags & MI_FLAGS_DISABLED) {
		rb--;
	}
	i = mi->D.Listbox.nlines;
	s = mi->D.Listbox.startline;
	while (i--) {
		PushClipping();
		SetClipping(0, 0, x + w, Video.Height - 1);
//		if (mi->transparent) {
//			VideoDrawClipTrans50(MenuButtonG, rb, x - 1, y - 1 + 18 * i);
//		} else {
			MenuButtonG->DrawFrameClip(rb, x - 1, y - 1 + 18 * i);
//		}
		PopClipping();
		if (!(flags & MI_FLAGS_DISABLED)) {
			if (i < mi->D.Listbox.noptions) {
				SetDefaultTextColors(nc, rc);
				text = (char*)(*mi->D.Listbox.retrieveopt)(mi, i + s);
				if (text) {
					if (i + s == mi->D.Listbox.curopt) {
						SetDefaultTextColors(rc, rc);
					} else {
						SetDefaultTextColors(nc, rc);
					}
					VideoDrawText(x + 2, y + 2 + 18 * i, mi->Font,text);
				}
			}
		}
	}
	if (mi->D.Listbox.curopt != -1 &&
			(mi->D.Listbox.curopt < mi->D.Listbox.startline ||
				mi->D.Listbox.curopt >= mi->D.Listbox.startline + mi->D.Listbox.nlines)) {
		(*mi->D.Listbox.retrieveopt)(mi, mi->D.Listbox.curopt);
	}

	if (flags & MI_FLAGS_SELECTED) {
		if (flags & MI_FLAGS_DISABLED) {
			Video.DrawRectangleClip(ColorGray, x - 2, y - 2, w + 1, h + 2);
		} else {
			Video.DrawRectangleClip(ColorYellow, x - 2, y - 2, w + 1, h + 2);
		}
	}
	SetDefaultTextColors(oldnc, oldrc);
}

/**
** Draw vslider 'button' on menu mx, my
**
** @param mi    menuitem pointer
** @param mx    menu X display position (offset)
** @param my    menu Y display position (offset)
*/
static void DrawVSlider(Menuitem *mi, int mx, int my)
{
	int p;
	unsigned flags;
	int w;
	int h;
	int x;
	int y;

	flags = mi->Flags;
	w = mi->D.VSlider.xsize;
	h = mi->D.VSlider.ysize;
	x = mx + mi->XOfs;
	y = my + mi->YOfs;

	if (mi->D.VSlider.style == MI_STYLE_SC_VSLIDER) {
		int upb;
		int downb;

		if (flags & MI_FLAGS_DISABLED) {
			upb = MBUTTON_SC_UP_ARROW - 1;
			downb = MBUTTON_SC_DOWN_ARROW - 1;
		} else if (flags & MI_FLAGS_CLICKED) {
			if (mi->D.VSlider.cflags&MI_CFLAGS_UP) {
				upb = MBUTTON_SC_UP_ARROW + 1;
				downb = MBUTTON_SC_DOWN_ARROW;
			} else if (mi->D.VSlider.cflags&MI_CFLAGS_DOWN) {
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
		MenuButtonG->DrawFrame(upb, x, y);
		MenuButtonG->DrawFrame(downb, x, y + h - 1 - 16);

		MenuButtonG->DrawFrame(MBUTTON_SC_S_VCONT - 1, x, y + 16 + 2);
		for (p = y + 16 + 2 + 8 + 1; p < y + h - 1 - 16 - 2 - 8; p += 8) {
			MenuButtonG->DrawFrame(MBUTTON_SC_S_VCONT, x, p);
		}
		MenuButtonG->DrawFrame(MBUTTON_SC_S_VCONT + 1, x, y + h - 1 - 16 - 2 - 8);
		p = (mi->D.VSlider.percent * (h - 54)) / 100;
		MenuButtonG->DrawFrame(MBUTTON_SC_S_KNOB, x, y + 16 + 2 + p);
	} else {
		if (flags & MI_FLAGS_DISABLED) {
			PushClipping();
			SetClipping(0, 0, Video.Width - 1, y + h - 20);
//			if (mi->transparent) {
//				VideoDrawClipTrans50(MenuButtonG, MBUTTON_S_VCONT - 1, x, y - 2);
//				VideoDrawClipTrans50(MenuButtonG, MBUTTON_S_VCONT - 1, x, y + h / 2);
//			} else {
				MenuButtonG->DrawFrameClip(MBUTTON_S_VCONT - 1, x, y - 2);
				MenuButtonG->DrawFrameClip(MBUTTON_S_VCONT - 1, x, y + h / 2);
//			}
			PopClipping();
			MenuButtonG->DrawFrame(MBUTTON_UP_ARROW - 1, x, y - 2);
			MenuButtonG->DrawFrame(MBUTTON_DOWN_ARROW - 1, x, y + h - 20);
		} else {
			PushClipping();
			SetClipping(0, 0, Video.Width - 1, y + h - 20);
//			if (mi->transparent) {
//				VideoDrawClipTrans50(MenuButtonG, MBUTTON_S_VCONT, x, y - 2);
//				VideoDrawClipTrans50(MenuButtonG, MBUTTON_S_VCONT, x, y + h / 2);
//			} else {
				MenuButtonG->DrawFrameClip(MBUTTON_S_VCONT, x, y - 2);
				MenuButtonG->DrawFrameClip(MBUTTON_S_VCONT, x, y + h / 2);
//			}
			PopClipping();
			if (mi->D.VSlider.cflags & MI_CFLAGS_UP) {
				MenuButtonG->DrawFrame(MBUTTON_UP_ARROW + 1, x, y - 2);
			} else {
				MenuButtonG->DrawFrame(MBUTTON_UP_ARROW, x, y - 2);
			}
			if (mi->D.VSlider.cflags & MI_CFLAGS_DOWN) {
				MenuButtonG->DrawFrame(MBUTTON_DOWN_ARROW + 1, x, y + h - 20);
			} else {
				MenuButtonG->DrawFrame(MBUTTON_DOWN_ARROW, x, y + h - 20);
			}
			p = (mi->D.VSlider.percent * (h - 54)) / 100;
			MenuButtonG->DrawFrame(MBUTTON_S_KNOB, x + 1, y + 18 + p);
		}

		if (flags & MI_FLAGS_SELECTED) {
			if (flags & MI_FLAGS_DISABLED) {
				Video.DrawRectangleClip(ColorGray, x, y - 2, w, h + 2);
			} else {
				Video.DrawRectangleClip(ColorYellow, x, y - 2, w, h + 2);
			}
		}
	}
}

/**
** Draw hslider 'button' on menu mx, my
**
** @param mi    menuitem pointer
** @param mx    menu X display position (offset)
** @param my    menu Y display position (offset)
*/
static void DrawHSlider(Menuitem *mi, int mx, int my)
{
	int p;
	unsigned flags;
	int w;
	int h;
	int x;
	int y;

	flags = mi->Flags;
	w = mi->D.HSlider.xsize;
	h = mi->D.HSlider.ysize;
	x = mx + mi->XOfs;
	y = my + mi->YOfs;

	if (mi->D.HSlider.style == MI_STYLE_SC_HSLIDER) {
		int leftb;
		int rightb;

		if (flags & MI_FLAGS_DISABLED) {
			leftb = MBUTTON_SC_LEFT_ARROW - 1;
			rightb = MBUTTON_SC_RIGHT_ARROW - 1;
		} else if (flags & MI_FLAGS_CLICKED) {
			if (mi->D.VSlider.cflags & MI_CFLAGS_LEFT) {
				leftb = MBUTTON_SC_LEFT_ARROW + 1;
				rightb = MBUTTON_SC_RIGHT_ARROW;
			} else if (mi->D.VSlider.cflags & MI_CFLAGS_RIGHT) {
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
		MenuButtonG->DrawFrame(leftb, x, y);
		MenuButtonG->DrawFrame(rightb, x + w - 1 - 16, y);

		MenuButtonG->DrawFrame(MBUTTON_SC_S_HCONT - 1, x + 16 + 2, y);
		for (p = x + 16 + 2 + 8 + 1; p < x + w - 1 - 16 - 2 - 8; p += 8) {
			MenuButtonG->DrawFrame(MBUTTON_SC_S_HCONT, p, y);
		}
		MenuButtonG->DrawFrame(MBUTTON_SC_S_HCONT + 1, x + w - 1 - 16 - 2 - 8, y);
		p = (mi->D.VSlider.percent * (w - 54)) / 100;
		MenuButtonG->DrawFrame(MBUTTON_SC_S_KNOB, x + 16 + 2 + p, y);
	} else {
		if (flags & MI_FLAGS_DISABLED) {
			PushClipping();
			SetClipping(0, 0, x + w - 20, Video.Height - 1);
//			if (mi->transparent) {
//				VideoDrawClipTrans50(MenuButtonG, MBUTTON_S_HCONT - 1, x - 2, y);
//				VideoDrawClipTrans50(MenuButtonG, MBUTTON_S_HCONT - 1, x + w / 2, y);
//			} else {
				MenuButtonG->DrawFrameClip(MBUTTON_S_HCONT - 1, x - 2, y);
				MenuButtonG->DrawFrameClip(MBUTTON_S_HCONT - 1, x + w / 2, y);
//			}
			PopClipping();
			MenuButtonG->DrawFrame(MBUTTON_LEFT_ARROW - 1, x - 2, y);
			MenuButtonG->DrawFrame(MBUTTON_RIGHT_ARROW - 1, x + w - 20, y);
		} else {
			PushClipping();
			SetClipping(0, 0, x + w - 20, Video.Height - 1);
//			if (mi->transparent) {
//				VideoDrawClipTrans50(MenuButtonG, MBUTTON_S_HCONT, x - 2, y);
//				VideoDrawClipTrans50(MenuButtonG, MBUTTON_S_HCONT, x + w / 2, y);
//			} else {
				MenuButtonG->DrawFrameClip(MBUTTON_S_HCONT, x - 2, y);
				MenuButtonG->DrawFrameClip(MBUTTON_S_HCONT, x + w / 2, y);
//			}
			PopClipping();
			if (mi->D.HSlider.cflags & MI_CFLAGS_LEFT) {
				MenuButtonG->DrawFrame(MBUTTON_LEFT_ARROW + 1, x - 2, y);
			} else {
				MenuButtonG->DrawFrame(MBUTTON_LEFT_ARROW, x - 2, y);
			}
			if (mi->D.HSlider.cflags & MI_CFLAGS_RIGHT) {
				MenuButtonG->DrawFrame(MBUTTON_RIGHT_ARROW + 1, x + w - 20, y);
			} else {
				MenuButtonG->DrawFrame(MBUTTON_RIGHT_ARROW, x + w - 20, y);
			}
			p = (mi->D.HSlider.percent * (w - 54)) / 100;
			MenuButtonG->DrawFrame(MBUTTON_S_KNOB, x + 18 + p, y + 1);
		}

		if (flags & MI_FLAGS_SELECTED) {
			if (flags & MI_FLAGS_DISABLED) {
				Video.DrawRectangleClip(ColorGray, x - 2, y, w + 2, h);
			} else {
				Video.DrawRectangleClip(ColorYellow, x - 2, y, w + 2, h);
			}
		}
	}
}

/**
**  Draw checkbox on menu x, y
**
**  @param style   Checkbox style
**  @param flags   State of Button (clicked, mouse over...)
**  @param checked Checked
**  @param x       X display position
**  @param y       Y display position
**  @param text    text to print on button
*/
static void DrawCheckbox(CheckboxStyle *style, unsigned flags, unsigned checked,
	int x, int y, const char *text)
{
	char *nc;
	char *rc;
	char *oldnc;
	char *oldrc;
	int i;
	ButtonStyleProperties *p;
	ButtonStyleProperties *pimage;

	if (flags & MI_FLAGS_DISABLED) {
		p = checked ? &style->CheckedDisabled : &style->Disabled;
	} else if (flags & MI_FLAGS_CLICKED) {
		p = checked ? &style->CheckedClicked : &style->Clicked;
	} else if (flags & MI_FLAGS_ACTIVE) {
		p = checked ? &style->CheckedHover : &style->Hover;
	} else if (flags & MI_FLAGS_SELECTED) {
		p = checked ? &style->CheckedSelected : &style->Selected;
	} else {
		p = checked ? &style->Checked : &style->Default;
	}

	//
	//  Image
	//
	pimage = p;
	if (!p->Sprite) {
		// No image
		if ((flags & MI_FLAGS_DISABLED)) {
			// Try unchecked disabled
			if (checked && style->Disabled.Sprite) {
				pimage = &style->Disabled;
			}
		} else {
			// Try hover, selected, then default
			if (checked) {
				if ((flags & MI_FLAGS_ACTIVE) && style->CheckedHover.Sprite) {
					pimage = &style->CheckedHover;
				} else if ((flags & MI_FLAGS_SELECTED) && style->CheckedSelected.Sprite) {
					pimage = &style->CheckedSelected;
				} else if (style->Checked.Sprite) {
					pimage = &style->Checked;
				}
			} else {
				if ((flags & MI_FLAGS_ACTIVE) && style->Hover.Sprite) {
					pimage = &style->Hover;
				} else if ((flags & MI_FLAGS_SELECTED) && style->Selected.Sprite) {
					pimage = &style->Selected;
				} else if (style->Default.Sprite) {
					pimage = &style->Default;
				}
			}
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
	if (text) {
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
		if ((flags & MI_FLAGS_SELECTED)) {
			if (checked && style->CheckedSelected.BorderSize) {
				p = &style->CheckedSelected;
			}
			if (!checked && style->Selected.BorderSize) {
				p = &style->Selected;
			}
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

/**
** Draw input 'button' on menu mx, my
**
** @param mi    menuitem pointer
** @param mx    menu X display position (offset)
** @param my    menu Y display position (offset)
*/
static void DrawInput(Menuitem *mi, int mx, int my)
{
	char *nc;
	char *rc;
	char *oldnc;
	char *oldrc;
	char *text;
	unsigned flags;
	MenuButtonId rb;
	int w;
	int h;
	int x;
	int y;
	int p;

	flags = mi->Flags;
	rb = mi->D.Input.button;
	x = mx + mi->XOfs;
	y = my + mi->YOfs;
	w = mi->D.Input.xsize;
	h = mi->D.Input.ysize;

	GetDefaultTextColors(&oldnc,&oldrc);
	if (mi->D.Input.normalcolor || mi->D.Input.reversecolor) {
		nc = mi->D.Input.normalcolor ? mi->D.Input.normalcolor : oldnc;
		rc = mi->D.Input.reversecolor ? mi->D.Input.reversecolor : oldrc;
		SetDefaultTextColors(nc, rc);
	} else {
		nc = oldnc;
		rc = oldrc;
	}
	if (mi->D.Input.button == MBUTTON_SC_PULLDOWN) {
		rb = MBUTTON_SC_INPUT;
		if (flags & MI_FLAGS_DISABLED) {
			rb -= 3;
			SetDefaultTextColors(FontGrey, FontGrey);
		}
		MenuButtonG->DrawFrame(rb - 1, x, y);
		for (p = x + 16; p < x + w - 1 - 16; p += 16) {
			MenuButtonG->DrawFrame(rb, p, y);
		}
		MenuButtonG->DrawFrame(rb + 1, x + w - 1 - 16, y);
		text = mi->D.Input.buffer;
		if (text) {
			VideoDrawText(x + 4, y + 2, mi->Font, text);
		}
	} else {
		if (flags & MI_FLAGS_DISABLED) {
			--rb;
			SetDefaultTextColors(FontGrey, FontGrey);
		}

		PushClipping();
		SetClipping(0, 0, x + w, Video.Height - 1);
//		if (mi->transparent) {
//			VideoDrawClipTrans50(MenuButtonG, rb, x - 1, y - 1);
//		} else {
			MenuButtonG->DrawFrameClip(rb, x - 1, y - 1);
//		}
		PopClipping();
		text = mi->D.Input.buffer;
		if (text) {
			if (mi->D.Input.iflags & MI_IFLAGS_PASSWORD) {
				char *p;
				p = text = new_strdup(text);
				while (*p && strcmp(p, "~!_")) {
					*p++ = '*';
				}
			}
			VideoDrawText(x + 2, y + 2, mi->Font, text);
			if (mi->D.Input.iflags & MI_IFLAGS_PASSWORD) {
				delete[] text;
			}
		}
		if (flags & MI_FLAGS_SELECTED) {
			if (flags & MI_FLAGS_DISABLED) {
				Video.DrawRectangleClip(ColorGray, x - 2, y - 2, w + 4, h);
			} else {
				Video.DrawRectangleClip(ColorYellow, x - 2, y - 2, w + 4, h);
			}
		}
	}
	SetDefaultTextColors(oldnc, oldrc);
}


/**
**  Draw a menu.
**
**  @param menu    The menu number to display (NULL allowed)
*/
void DrawMenu(Menu *menu)
{
	int i;
	int n;
	Menuitem *mi;
	Menuitem *mip;

	if (menu == NULL) {
		return;
	}

	if (menu->BackgroundG) {
		if (!menu->BackgroundG->IsLoaded()) {
			menu->BackgroundG->Load();
			menu->BackgroundG->Resize(Video.Width, Video.Height);
		}
		menu->BackgroundG->DrawFrame(0, 0, 0);
	}

	if (menu->Panel && !strcmp(menu->Panel, ScPanel)) {
		// Background
		Video.FillTransRectangle(ColorBlack, menu->X + 1,
				menu->Y + 1, menu->Width - 2, menu->Height - 2, 50);
		Video.DrawHLineClip(ColorBlue, menu->X + 3, menu->Y, menu->Width - 6);
		Video.DrawHLineClip(ColorBlue, menu->X + 3, menu->Y + menu->Height - 1, menu->Width - 6);
		Video.DrawVLineClip(ColorBlue, menu->X, menu->Y + 3, menu->Height - 6);
		Video.DrawVLineClip(ColorBlue, menu->X + menu->Width - 1, menu->Y + 3, menu->Height - 6);
		// top left
		Video.DrawPixelClip(ColorBlue, menu->X + 1, menu->Y + 1);
		Video.DrawPixelClip(ColorBlue, menu->X + 2, menu->Y + 1);
		Video.DrawPixelClip(ColorBlue, menu->X + 1, menu->Y + 2);
		// top right
		Video.DrawPixelClip(ColorBlue, menu->X + menu->Width - 3, menu->Y + 1);
		Video.DrawPixelClip(ColorBlue, menu->X + menu->Width - 2, menu->Y + 1);
		Video.DrawPixelClip(ColorBlue, menu->X + menu->Width - 2, menu->Y + 2);
		// bottom left
		Video.DrawPixelClip(ColorBlue, menu->X + 1, menu->Y + menu->Height - 3);
		Video.DrawPixelClip(ColorBlue, menu->X + 1, menu->Y + menu->Height - 2);
		Video.DrawPixelClip(ColorBlue, menu->X + 2, menu->Y + menu->Height - 2);
		// bottom right
		Video.DrawPixelClip(ColorBlue, menu->X + menu->Width - 3, menu->Y + menu->Height - 2);
		Video.DrawPixelClip(ColorBlue, menu->X + menu->Width - 2, menu->Y + menu->Height - 2);
		Video.DrawPixelClip(ColorBlue, menu->X + menu->Width - 2, menu->Y + menu->Height - 3);
	} else if (menu->Panel) {
		MenuPanel *menupanel;

		menupanel = UI.MenuPanels;
		while (menupanel) {
			if (!strcmp(menupanel->Ident, menu->Panel)) {
				break;
			}
			menupanel = menupanel->Next;
		}
		if (menupanel) {
			menupanel->G->DrawSub(0, 0,
				menupanel->G->Width, menupanel->G->Height,
				menu->X, menu->Y);
		}
	}

	n = menu->NumItems;
	mi = menu->Items;
	mip = NULL;
	for (i = 0; i < n; ++i) {
		if (mi[i].Flags & MI_FLAGS_INVISIBLE) {
			continue;
		}
		switch (mi[i].MiType) {
			case MiTypeText:
				if (!mi[i].D.Text.text) {
					break;
				}
				DrawMenuText(&mi[i].D.Text, menu->X + mi[i].XOfs, menu->Y + mi[i].YOfs,
					mi[i].Font, mi[i].Flags);
				break;
			case MiTypeButton:
				UpdateMenuItemButton(&mi[i]);
				DrawMenuButton(mi[i].D.Button.Style, mi[i].Flags,
					menu->X + mi[i].XOfs, menu->Y + mi[i].YOfs,
					mi[i].D.Button.Text);
				break;
			case MiTypePulldown:
				if (mi[i].Flags & MI_FLAGS_CLICKED) {
					mip = &mi[i]; // Delay, due to possible overlaying!
				} else {
					DrawPulldown(&mi[i], menu->X, menu->Y);
				}
				break;
			case MiTypeListbox:
				DrawListbox(&mi[i], menu->X, menu->Y);
				break;
			case MiTypeVslider:
				DrawVSlider(&mi[i], menu->X, menu->Y);
				break;
			case MiTypeHslider:
				DrawHSlider(&mi[i], menu->X, menu->Y);
				break;
			case MiTypeDrawfunc:
				if (mi[i].D.DrawFunc.draw) {
					(*mi[i].D.DrawFunc.draw)(&mi[i]);
				}
				break;
			case MiTypeInput:
				DrawInput(&mi[i], menu->X, menu->Y);
				break;
			case MiTypeCheckbox:
				DrawCheckbox(mi[i].D.Checkbox.Style, mi[i].Flags, mi[i].D.Checkbox.Checked,
					menu->X + mi[i].XOfs, menu->Y + mi[i].YOfs,
					mi[i].D.Checkbox.Text);
				break;
			default:
				break;
		}
	}
	if (mip) {
		DrawPulldown(mip, menu->X, menu->Y);
	}
}

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
	clipboard = (unsigned char*)GlobalLock(handle);
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

/**
** Handle keys in menu mode.
**
** @param key        Key scancode.
** @param keychar    ASCII character code of key.
**
** @todo FIXME: Should be MenuKeyDown(), and act on _new_ MenuKeyUp() !!!
**   to implement button animation (depress before action)
*/
static void MenuHandleKeyDown(unsigned key, unsigned keychar)
{
	int i;
	int n;
	Menuitem *mi;
	Menu *menu;

	HandleKeyModifiersDown(key, keychar);

	if (CurrentMenu == NULL) {
		return;
	}

	if (SDLK_KP0 <= key && key <= SDLK_KP9) {
		key = keychar = '0' + key - SDLK_KP0;
	} else if (key == SDLK_KP_PERIOD) {
		key = keychar = '.';
	}

	menu = CurrentMenu;
	if (MenuButtonCurSel != -1 && menu->Items[MenuButtonCurSel].MiType == MiTypeInput) {
		mi = menu->Items + MenuButtonCurSel;
		if (!(mi->Flags & (MI_FLAGS_DISABLED | MI_FLAGS_INVISIBLE))) {
inkey:
			if (key >= 0x80 && key < 0x100) {
				// FIXME ARI: ISO->WC2 Translation here!
				key = 0;
			}
			switch (key) {
				case SDLK_BACKSPACE:
					if (mi->D.Input.nch > 0) {
						if (mi->D.Input.buffer[mi->D.Input.nch - 1] == '~') {
							--mi->D.Input.nch;
						}
						strcpy(mi->D.Input.buffer + (--mi->D.Input.nch), "~!_");
					}
					break;
				case SDLK_TAB:
					goto normkey;
				case SDLK_DELETE:
					mi->D.Input.nch = 0;
					strcpy(mi->D.Input.buffer, "~!_");
					break;
				default:
					if (KeyModifiers & ModifierAlt) {
						if (key == 'x' || key == 'X') {
							goto normkey;
						}
					} else if (KeyModifiers & ModifierControl) {
						if (key == 'v' || key == 'V') {
							PasteFromClipboard(mi);
						} else if (key == 'u' || key == 'U') {
							mi->D.Input.nch = 0;
							strcpy(mi->D.Input.buffer, "~!_");
						}
					} else if (key >= 32 && key < 0x100) {
						if ((keychar == '~' && mi->D.Input.nch < mi->D.Input.maxch - 1) ||
								mi->D.Input.nch < mi->D.Input.maxch &&
									mi->Font->Width(mi->D.Input.buffer) + 8 < mi->D.Input.xsize) {
							mi->D.Input.buffer[mi->D.Input.nch++] = keychar;
							if (keychar == '~') {
								mi->D.Input.buffer[mi->D.Input.nch++] = keychar;
							}
							strcpy(mi->D.Input.buffer + mi->D.Input.nch, "~!_");
						}
					}
					break;
			}
			if (mi->D.Input.action) {
				(*mi->D.Input.action)(mi, key);
			} else if (mi->LuaHandle) {
				CallHandler(mi->LuaHandle, key);
			}
			return;
		}
	}

normkey:
	if (!(KeyModifiers & ModifierAlt)) {
		mi = menu->Items;
		i = menu->NumItems;
		while (i--) {
			switch (mi->MiType) {
				case MiTypeButton:
					if (key == mi->D.Button.HotKey) {
						if (!(mi->Flags & (MI_FLAGS_DISABLED | MI_FLAGS_INVISIBLE))) {
							if (mi->D.Button.Handler) {
								(*mi->D.Button.Handler)();
							} else if (mi->LuaHandle) {
								CallHandler(mi->LuaHandle, 0);
							}
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
		case SDLK_RETURN:
		case SDLK_KP_ENTER: // RETURN
			if (MenuButtonCurSel != -1) {
				mi = menu->Items + MenuButtonCurSel;
				switch (mi->MiType) {
					case MiTypeButton:
						if (mi->D.Button.Handler) {
							(*mi->D.Button.Handler)();
						} else if (mi->LuaHandle) {
							CallHandler(mi->LuaHandle, 0);
						}
						return;
					case MiTypeListbox:
						if (mi->D.Listbox.handler) {
							(*mi->D.Listbox.handler)();
						}
						return;
					case MiTypeVslider:
						if (mi->D.VSlider.handler) {
							(*mi->D.VSlider.handler)();
						}
						return;
					case MiTypeHslider:
						if (mi->D.HSlider.handler) {
							(*mi->D.HSlider.handler)();
						}
						return;
					case MiTypeCheckbox:
						if (mi->D.Checkbox.Action) {
							(*mi->D.Checkbox.Action)(mi);
						} else if (mi->LuaHandle) {
							CallHandler(mi->LuaHandle, mi->D.Checkbox.Checked);
						}
						return;
					default:
						break;
				}
			}
			break;
		case SDLK_UP:
		case SDLK_DOWN:
			if (MenuButtonCurSel != -1) {
				mi = menu->Items + MenuButtonCurSel;
				if (!(mi->Flags & MI_FLAGS_CLICKED)) {
					switch (mi->MiType) {
						case MiTypePulldown:
							if (key == SDLK_DOWN) {
								if (mi->D.Pulldown.curopt + 1 < mi->D.Pulldown.noptions) {
									mi->D.Pulldown.curopt++;
								} else {
									break;
								}
							} else {
								if (mi->D.Pulldown.curopt > 0) {
									mi->D.Pulldown.curopt--;
								} else {
									break;
								}
							}
							if (mi->D.Pulldown.action) {
								(*mi->D.Pulldown.action)(mi, mi->D.Pulldown.curopt);
							} else if (mi->LuaHandle) {
								CallHandler(mi->LuaHandle, mi->D.Pulldown.curopt);
							}
							break;
						case MiTypeListbox:
							if (key == SDLK_DOWN) {
								if (mi->D.Listbox.curopt < mi->D.Listbox.noptions - 1) {
									mi->D.Listbox.curopt++;
									if (mi->D.Listbox.curopt >= mi->D.Listbox.startline + mi->D.Listbox.nlines) {
										mi->D.Listbox.startline = mi->D.Listbox.curopt - mi->D.Listbox.nlines + 1;
									} else if (mi->D.Listbox.curopt < mi->D.Listbox.startline) {
										mi->D.Listbox.startline = mi->D.Listbox.curopt;
									}
								}
							} else {
								if (mi->D.Listbox.curopt > 0) {
									mi->D.Listbox.curopt--;
									if (mi->D.Listbox.curopt >= mi->D.Listbox.startline + mi->D.Listbox.nlines) {
										mi->D.Listbox.startline = mi->D.Listbox.curopt - mi->D.Listbox.nlines + 1;
									} else if (mi->D.Listbox.curopt < mi->D.Listbox.startline) {
										mi->D.Listbox.startline = mi->D.Listbox.curopt;
									}
								}
							}
							if (mi->D.Listbox.noptions > mi->D.Listbox.nlines) {
								mi[1].D.VSlider.percent = (mi->D.Listbox.startline * 100) / (mi->D.Listbox.noptions - mi->D.Listbox.nlines);
							}
							if (mi[1].D.VSlider.action) {
								(*mi[1].D.VSlider.action)(mi);
							} else if (mi[1].LuaHandle) {
								CallHandler(mi[1].LuaHandle, mi[1].D.VSlider.percent);
							}
							break;
						case MiTypeVslider:
							if (key == SDLK_DOWN) {
								mi->D.VSlider.cflags |= MI_CFLAGS_DOWN;
								// Update listbox
								if (mi > mi->Menu->Items && mi[-1].MiType == MiTypeListbox) {
									if (mi[-1].D.Listbox.startline + mi[-1].D.Listbox.nlines < mi[-1].D.Listbox.noptions) {
										mi[-1].D.Listbox.startline++;
									}
								}
							} else {
								mi->D.VSlider.cflags |= MI_CFLAGS_UP;
								// Update listbox
								if (mi > mi->Menu->Items && mi[-1].MiType == MiTypeListbox) {
									if (mi[-1].D.Listbox.startline > 0) {
										mi[-1].D.Listbox.startline--;
									}
								}
							}
							if (mi[-1].D.Listbox.noptions > mi[-1].D.Listbox.nlines) {
								mi->D.VSlider.percent = (mi[-1].D.Listbox.startline * 100) / (mi[-1].D.Listbox.noptions - mi[-1].D.Listbox.nlines);
							}
							if (mi->D.VSlider.action) {
								(*mi->D.VSlider.action)(mi);
							} else if (mi->LuaHandle) {
								CallHandler(mi->LuaHandle, mi->D.VSlider.percent);
							}
							break;
						default:
							break;
					}
				}
			}
			break;
		case SDLK_LEFT:
		case SDLK_RIGHT:
			if (MenuButtonCurSel != -1) {
				mi = menu->Items + MenuButtonCurSel;
				if (!(mi->Flags & MI_FLAGS_CLICKED)) {
					switch (mi->MiType) {
						case MiTypeHslider:
							if (key == SDLK_LEFT) {
								mi->D.HSlider.percent -= 10;
								if (mi->D.HSlider.percent < 0) {
									mi->D.HSlider.percent = 0;
								}
							} else {
								mi->D.HSlider.percent += 10;
								if (mi->D.HSlider.percent > 100) {
									mi->D.HSlider.percent = 100;
								}
							}
							if (mi->D.HSlider.action) {
								(*mi->D.HSlider.action)(mi);
							} else if (mi->LuaHandle) {
								CallHandler(mi->LuaHandle, mi->D.HSlider.percent);
							}
							break;
						default:
							break;
					}
				}
			}
			break;
		case SDLK_TAB: // TAB // FIXME: Add Shift-TAB
			if (KeyModifiers & ModifierAlt) {
				break;
			}
			if (MenuButtonCurSel != -1 && !(menu->Items[MenuButtonCurSel].Flags & MI_FLAGS_CLICKED)) {
				n = menu->NumItems;
				for (i = 0; i < n; ++i) {
					mi = menu->Items + ((MenuButtonCurSel + i + 1) % n);
					switch (mi->MiType) {
						case MiTypeButton:
						case MiTypePulldown:
						case MiTypeListbox:
						case MiTypeVslider:
						case MiTypeHslider:
						case MiTypeInput:
						case MiTypeCheckbox:
							if (mi->Flags & (MI_FLAGS_DISABLED | MI_FLAGS_INVISIBLE)) {
								break;
							}
							menu->Items[MenuButtonCurSel].Flags &= ~MI_FLAGS_SELECTED;
							mi->Flags |= MI_FLAGS_SELECTED;
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
				switch (mi->MiType) {
					case MiTypeInput:
						if (!(mi->Flags & (MI_FLAGS_DISABLED | MI_FLAGS_INVISIBLE))) {
							if (MenuButtonCurSel != -1) {
								menu->Items[MenuButtonCurSel].Flags &=
									~MI_FLAGS_SELECTED;
							}
							mi->Flags |= MI_FLAGS_SELECTED;
							MenuButtonCurSel = mi - menu->Items;
							goto inkey;
						}
					default:
						break;
				}
				++mi;
			}
			return;
	}
	return;
}

/**
**  Handle keys in menu mode.
**
**  @param key      Key scancode.
**  @param keychar  ASCII character code of key.
*/
static void MenuHandleKeyUp(unsigned key, unsigned keychar)
{
	Menuitem *mi;
	Menu *menu;

	HandleKeyModifiersUp(key, keychar);

	if (CurrentMenu == NULL) {
		return;
	}

	menu = CurrentMenu;
	if (key == SDLK_UP || key == SDLK_DOWN) {
		if (MenuButtonCurSel != -1) {
			mi = menu->Items + MenuButtonCurSel;
			if (mi->MiType == MiTypeVslider) {
				if (key == SDLK_DOWN) {
					mi->D.VSlider.cflags &= ~MI_CFLAGS_DOWN;
				} else {
					mi->D.VSlider.cflags &= ~MI_CFLAGS_UP;
				}
			}
		}
	}
}

/**
**  Handle keys repeated in menu mode.
**
**  @param key      Key scancode.
**  @param keychar  ASCII character code of key.
*/
static void MenuHandleKeyRepeat(unsigned key, unsigned keychar)
{
	Menuitem *mi;
	Menu *menu;

	HandleKeyModifiersDown(key, keychar);

	if (CurrentMenu == NULL) {
		return;
	}

	menu = CurrentMenu;
	mi = menu->Items + MenuButtonCurSel;
	if (MenuButtonCurSel != -1) {
		if (mi->MiType == MiTypeInput) {
			MenuHandleKeyDown(key, keychar);
		} else if (mi->MiType == MiTypeVslider || mi->MiType == MiTypeListbox) {
			if (key == SDLK_DOWN || key == SDLK_UP) {
				MenuHandleKeyDown(key, keychar);
			}
		}
	}
}

/**
** Handle movement of the cursor.
**
** @param x    Screen X position.
** @param y    Screen Y position.
*/
static void MenuHandleMouseMove(int x, int y)
{
	int h;
	int w;
	int i;
	int j;
	int n;
	int xs;
	int ys;
	Menuitem *mi;
	Menu *menu;
	int ox;
	int oy;
	char *tmp;

	ox = CursorX;
	oy = CursorY; // Old position for rel movement.
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
		if (!(mi->Flags & (MI_FLAGS_DISABLED | MI_FLAGS_INVISIBLE))) {
			if (mi->MiType == MiTypePulldown && (mi->Flags & MI_FLAGS_CLICKED)) {
				xs = menu->X + mi->XOfs;
				ys = menu->Y + mi->YOfs;
				if (mi->D.Pulldown.button == MBUTTON_SC_PULLDOWN) {
					int usetop;

					h = mi->D.Pulldown.ysize;
					if (mi->YOfs + (h + 1) * mi->D.Pulldown.noptions >= mi->Menu->Height) {
						ys -= h * mi->D.Pulldown.noptions;
						usetop = 0;
					} else {
						usetop = 1;
					}
					if (!(x < xs || x > xs + mi->D.Pulldown.xsize || y < ys ||
							y > ys + (h + 1) * mi->D.Pulldown.noptions)) {
						j = (y - ys) / h;
						if (usetop) {
							--j;
						} else {
							if (j == mi->D.Pulldown.noptions) {
								j = -1;
							}
						}
						if (j >= -1 && j < mi->D.Pulldown.noptions && j != mi->D.Pulldown.cursel) {
							mi->D.Pulldown.cursel = j;
							if (mi->D.Pulldown.action) {
								(*mi->D.Pulldown.action)(mi, mi->D.Pulldown.cursel);
							} else if (mi->LuaHandle) {
								CallHandler(mi->LuaHandle, mi->D.Pulldown.curopt);
							}
						}
					}
					MenuButtonUnderCursor = i;
				} else {
					h = mi->D.Pulldown.ysize - 2;
					if (ys + 1 <= mi->D.Pulldown.curopt * h + CurrentMenu->Y) {
						ys = 2 + CurrentMenu->Y;
					} else {
						ys -= mi->D.Pulldown.curopt * h;
						if (ys + h * mi->D.Pulldown.noptions >= 480 + CurrentMenu->Y) {
							ys -= ys + h * mi->D.Pulldown.noptions - (480 + CurrentMenu->Y);
						}
					}
					if (!(x < xs || x > xs + mi->D.Pulldown.xsize || y < ys ||
							y > ys + h * mi->D.Pulldown.noptions)) {
						j = (y - ys) / h;
						if (j >= 0 && j < mi->D.Pulldown.noptions && j != mi->D.Pulldown.cursel) {
							mi->D.Pulldown.cursel = j;
							if (mi->D.Pulldown.action) {
								(*mi->D.Pulldown.action)(mi, mi->D.Pulldown.cursel);
							} else if (mi->LuaHandle) {
								CallHandler(mi->LuaHandle, mi->D.Pulldown.curopt);
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
			if (!(mi->Flags & (MI_FLAGS_DISABLED | MI_FLAGS_INVISIBLE))) {
				switch (mi->MiType) {
					case MiTypeText:
						if (!mi->D.Text.text || !mi->D.Text.action)
							continue;
						xs = menu->X + mi->XOfs;
						ys = menu->Y + mi->YOfs;
						tmp = EvalString(mi->D.Text.text);
						if (x < xs - 4 || x > xs + mi->Font->Width(tmp) + 5 ||
								y < ys - 4 || y > ys + mi->Font->Height() + 5) {
							if (!(mi->Flags & MI_FLAGS_CLICKED)) {
								if (mi->Flags & MI_FLAGS_ACTIVE) {
									mi->Flags &= ~MI_FLAGS_ACTIVE;
								}
							}
							delete[] tmp;
							continue;
						}
						delete[] tmp;
						break;
					case MiTypeCheckbox:
						xs = menu->X + mi->XOfs;
						ys = menu->Y + mi->YOfs;
						if ((!mi->D.Checkbox.Text || x < xs - 1 || x > xs +
								GameFont->Width(mi->D.Checkbox.Text) + 28 ||
								y < ys - 2 ||  y > ys + GameFont->Height() + 9) &&
								(x < xs ||  x > xs + mi->D.Checkbox.Style->Width || y < ys ||
								y > ys + mi->D.Checkbox.Style->Height)) {
							if (!(mi->Flags & MI_FLAGS_CLICKED)) {
								if (mi->Flags & MI_FLAGS_ACTIVE) {
									mi->Flags &= ~MI_FLAGS_ACTIVE;
								}
							}
							continue;
						}
						break;
					case MiTypeButton:
						xs = menu->X + mi->XOfs;
						ys = menu->Y + mi->YOfs;
						if (x < xs || x > xs + mi->D.Button.Style->Width || y < ys ||
								y > ys + mi->D.Button.Style->Height) {
							if (!(mi->Flags & MI_FLAGS_CLICKED)) {
								if (mi->Flags & MI_FLAGS_ACTIVE) {
									mi->Flags &= ~MI_FLAGS_ACTIVE;
								}
							}
							continue;
						}
						break;
					case MiTypeInput:
						xs = menu->X + mi->XOfs;
						ys = menu->Y + mi->YOfs;
						if (x < xs || x > xs + mi->D.Input.xsize
								|| y < ys || y > ys + mi->D.Input.ysize) {
							if (!(mi->Flags & MI_FLAGS_CLICKED)) {
								if (mi->Flags & MI_FLAGS_ACTIVE) {
									mi->Flags &= ~MI_FLAGS_ACTIVE;
								}
							}
							continue;
						}
						break;
					case MiTypePulldown:
						// Clicked-state already checked above - there can only be one!
						xs = menu->X + mi->XOfs;
						ys = menu->Y + mi->YOfs;
						if (x < xs || x > xs + mi->D.Pulldown.xsize || y<ys ||
								y > ys + mi->D.Pulldown.ysize) {
							if (!(mi->Flags & MI_FLAGS_CLICKED)) {
								if (mi->Flags & MI_FLAGS_ACTIVE) {
									mi->Flags &= ~MI_FLAGS_ACTIVE;
								}
							}
							continue;
						}
						break;
					case MiTypeListbox:
						xs = menu->X + mi->XOfs;
						ys = menu->Y + mi->YOfs;
						if (x < xs || x > xs + mi->D.Listbox.xsize || y < ys ||
								y > ys + mi->D.Listbox.ysize) {
							if (!(mi->Flags & MI_FLAGS_CLICKED)) {
								if (mi->Flags & MI_FLAGS_ACTIVE) {
									mi->Flags &= ~MI_FLAGS_ACTIVE;
								}
							}
							continue;
						}
						j = (y - ys) / 18;
						if (j + mi->D.Listbox.startline != mi->D.Listbox.cursel) {
							mi->D.Listbox.cursel = j + mi->D.Listbox.startline; // just store for click
						}
						if (mi->Flags & MI_FLAGS_CLICKED && mi->Flags & MI_FLAGS_ACTIVE) {
							if (mi->D.Listbox.cursel != mi->D.Listbox.curopt) {
								mi->D.Listbox.dohandler = 0;
								mi->D.Listbox.curopt = mi->D.Listbox.cursel;
								if (mi->D.Listbox.action) {
									(*mi->D.Listbox.action)(mi, mi->D.Listbox.curopt);
								} else if (mi->LuaHandle) {
									CallHandler(mi->LuaHandle, mi->D.Listbox.curopt);
								}
							}
						}
						break;
					case MiTypeVslider:
					{
						int arrowsize;
						int curper;

						curper = mi->D.VSlider.percent;
						xs = menu->X + mi->XOfs;
						ys = menu->Y + mi->YOfs;
						if (x < xs || x > xs + mi->D.VSlider.xsize || y < ys ||
								y > ys + mi->D.VSlider.ysize) {
							if (!(mi->Flags & MI_FLAGS_CLICKED)) {
								if (mi->Flags & MI_FLAGS_ACTIVE) {
									mi->Flags &= ~MI_FLAGS_ACTIVE;
								}
							}
							if (y < ys || y > ys + mi->D.VSlider.ysize ||
									!(mi->Flags & MI_FLAGS_CLICKED)) {
								mi->D.VSlider.cursel = 0;
								continue;
							}
						}
						j = y - ys;
						mi->D.VSlider.cursel = 0;

						if (mi->D.VSlider.style == MI_STYLE_SC_VSLIDER) {
							arrowsize = 16;
						} else {
							arrowsize = 20;
						}

						if (j < arrowsize) {
							mi->D.VSlider.cursel |= MI_CFLAGS_UP;
						} else if (j >= mi->D.VSlider.ysize - arrowsize - 1) {
							mi->D.VSlider.cursel |= MI_CFLAGS_DOWN;
						} else {
							mi->D.VSlider.cursel &= ~(MI_CFLAGS_UP | MI_CFLAGS_DOWN);
							h = (mi->D.VSlider.percent * (mi->D.VSlider.ysize - 54)) / 100 + 18;
							if (j > h && j < h + 16) {
								mi->D.VSlider.cursel |= MI_CFLAGS_KNOB;
							} else {
								mi->D.VSlider.cursel |= MI_CFLAGS_CONT;
								if (j <= h) {
									mi->D.VSlider.cursel |= MI_CFLAGS_UP;
								} else {
									mi->D.VSlider.cursel |= MI_CFLAGS_DOWN;
								}
							}
							j -= 8;
							if (j < arrowsize) {
								j = arrowsize;
							}

							curper = ((j - arrowsize) * 100) / (mi->D.VSlider.ysize - 54);
							if (curper > 100) {
								curper = 100;
							}
						}

						// Update listbox
						if (mi > mi->Menu->Items && mi[-1].MiType == MiTypeListbox) {
							if ((mi->D.VSlider.cflags & MI_CFLAGS_KNOB) && (mi->Flags & MI_FLAGS_CLICKED)) {
								if (mi[-1].D.Listbox.noptions > mi[-1].D.Listbox.nlines) {
									mi[-1].D.Listbox.startline = (curper *
										(mi[-1].D.Listbox.noptions - mi[-1].D.Listbox.nlines) + 50) / 100;
								}
							}
							if (mi[-1].D.Listbox.noptions > mi[-1].D.Listbox.nlines) {
								mi->D.VSlider.percent = (mi[-1].D.Listbox.startline * 100) / (mi[-1].D.Listbox.noptions - mi[-1].D.Listbox.nlines);
							}
						} else {
							if ((mi->D.VSlider.cflags & MI_CFLAGS_KNOB) && (mi->Flags & MI_FLAGS_CLICKED)) {
								mi->D.VSlider.percent = curper;
							}
						}
						if ((mi->D.VSlider.cflags & MI_CFLAGS_KNOB) && (mi->Flags & MI_FLAGS_CLICKED)) {
							if (mi->D.VSlider.action) {
								(*mi->D.VSlider.action)(mi);
							} else if (mi->LuaHandle) {
								CallHandler(mi->LuaHandle, mi->D.VSlider.percent);
							}
						}
						break;
					}
					case MiTypeHslider:
					{
						int arrowsize;

						xs = menu->X + mi->XOfs;
						ys = menu->Y + mi->YOfs;
						if (x < xs || x > xs + mi->D.HSlider.xsize || y < ys ||
								y > ys + mi->D.HSlider.ysize) {
							if (!(mi->Flags & MI_FLAGS_CLICKED)) {
								if (mi->Flags & MI_FLAGS_ACTIVE) {
									mi->Flags &= ~MI_FLAGS_ACTIVE;
								}
							}
							if (x < xs || x > xs + mi->D.HSlider.xsize || !(mi->Flags & MI_FLAGS_CLICKED)) {
								mi->D.HSlider.cursel = 0;
								continue;
							}
						}
						j = x - xs;
						mi->D.HSlider.cursel = 0;

						if (mi->D.HSlider.style == MI_STYLE_SC_HSLIDER) {
							arrowsize = 16;
						} else {
							arrowsize = 20;
						}

						if (j < arrowsize) {
							mi->D.HSlider.cursel |= MI_CFLAGS_LEFT;
						} else if (j >= mi->D.HSlider.xsize - arrowsize-1) {
							mi->D.HSlider.cursel |= MI_CFLAGS_RIGHT;
						} else {
							mi->D.HSlider.cursel &= ~(MI_CFLAGS_LEFT | MI_CFLAGS_RIGHT);
							w = (mi->D.HSlider.percent * (mi->D.HSlider.xsize - 54)) / 100 + 18;
							if (j > w && j < w + 16) {
								mi->D.HSlider.cursel |= MI_CFLAGS_KNOB;
							} else {
								mi->D.HSlider.cursel |= MI_CFLAGS_CONT;
								if (j <= w) {
									mi->D.HSlider.cursel |= MI_CFLAGS_LEFT;
								} else {
									mi->D.HSlider.cursel |= MI_CFLAGS_RIGHT;
								}
							}
							j -= 8;
							if (j < arrowsize) {
								j = arrowsize;
							}

							mi->D.HSlider.curper = ((j - arrowsize) * 100) / (mi->D.HSlider.xsize - 54);
							if (mi->D.HSlider.curper > 100) {
								mi->D.HSlider.curper = 100;
							}
						}
						if ((mi->D.HSlider.cflags & MI_CFLAGS_KNOB) && (mi->Flags & MI_FLAGS_CLICKED)) {
							mi->D.HSlider.percent = mi->D.HSlider.curper;
							if (mi->D.HSlider.action) {
								(*mi->D.HSlider.action)(mi);
							} else if (mi->LuaHandle) {
								CallHandler(mi->LuaHandle, mi->D.HSlider.percent);
							}
						}
						break;
					}
					default:
						continue;
						// break;
				}
				switch (mi->MiType) {
					case MiTypeCheckbox:
					case MiTypeButton:
					case MiTypePulldown:
					case MiTypeListbox:
					case MiTypeVslider:
					case MiTypeHslider:
					case MiTypeText:
						if (!(mi->Flags & MI_FLAGS_ACTIVE)) {
							mi->Flags |= MI_FLAGS_ACTIVE;
						}
						MenuButtonUnderCursor = i;
					default:
						break;
					case MiTypeInput:
						if (!(mi->Flags & MI_FLAGS_ACTIVE)) {
							mi->Flags |= MI_FLAGS_ACTIVE;
						}
						if (MouseButtons & LeftButton
								&& mi->Flags & MI_FLAGS_SELECTED) {
							if (mi->D.Input.buffer && *mi->D.Input.buffer) {
								char *s;

								j = strtol(mi->D.Input.buffer, &s, 0);
								if ((!*s || s[0] == '~') && (j != 0 || *mi->D.Input.buffer == '0')) {
									int num;
									num = j + x - ox + (y - oy) * 1000;
									if (num < 0) {
										num = 0;
									}
									if ((mi->D.Input.maxch == 3 && num < 1000) ||
										(mi->D.Input.maxch == 4 && num < 10000) ||
										(mi->D.Input.maxch == 5 && num < 100000) ||
										(mi->D.Input.maxch == 6 && num < 1000000) ||
										(mi->D.Input.maxch >= 7)) {
										mi->D.Input.nch =
											sprintf(mi->D.Input.buffer, "%d~!_", num) - 3;
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
** Called if mouse button pressed down.
**
** @param b    button code
*/
static void MenuHandleButtonDown(unsigned b)
{
	Menuitem *mi;
	Menu *menu;

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
			if (!(mi->Flags & MI_FLAGS_CLICKED)) {
				switch (mi->MiType) {
					case MiTypeCheckbox:
					case MiTypeButton:
					case MiTypePulldown:
					case MiTypeListbox:
					case MiTypeVslider:
					case MiTypeHslider:
					case MiTypeInput:
					case MiTypeText:
						if (MenuButtonCurSel != -1) {
							menu->Items[MenuButtonCurSel].Flags &= ~MI_FLAGS_SELECTED;
						}
						MenuButtonCurSel = MenuButtonUnderCursor;
						mi->Flags |= MI_FLAGS_CLICKED | MI_FLAGS_SELECTED;
					default:
						break;
				}
			}
			PlayGameSound(GameSounds.Click.Sound, MaxSampleVolume);
			switch (mi->MiType) {
				case MiTypeVslider:
					mi->D.VSlider.cflags = mi->D.VSlider.cursel;

					// Update listbox
					if (mi > mi->Menu->Items && mi[-1].MiType == MiTypeListbox) {
						if (mi->D.VSlider.cflags & MI_CFLAGS_DOWN) {
							if (mi[-1].D.Listbox.startline + mi[-1].D.Listbox.nlines < mi[-1].D.Listbox.noptions) {
								mi[-1].D.Listbox.startline++;
							}
						} else if (mi->D.VSlider.cflags & MI_CFLAGS_UP) {
							if (mi[-1].D.Listbox.startline > 0) {
								mi[-1].D.Listbox.startline--;
							}
						}
					}
					if (mi->D.VSlider.action) {
						(*mi->D.VSlider.action)(mi);
					} else if (mi->LuaHandle) {
						CallHandler(mi->LuaHandle, mi->D.VSlider.percent);
					}
					MenuHandleMouseMove(CursorX, CursorY);
					break;
				case MiTypeHslider:
					mi->D.HSlider.cflags = mi->D.HSlider.cursel;
					if (mi->D.HSlider.cflags & MI_CFLAGS_RIGHT) {
						mi->D.HSlider.percent += 10;
						if (mi->D.HSlider.percent > 100)
							mi->D.HSlider.percent = 100;
					} else if (mi->D.HSlider.cflags & MI_CFLAGS_LEFT) {
						mi->D.HSlider.percent -= 10;
						if (mi->D.HSlider.percent < 0)
							mi->D.HSlider.percent = 0;
					}
					if (mi->D.HSlider.action) {
						(*mi->D.HSlider.action)(mi);
					} else if (mi->LuaHandle) {
						CallHandler(mi->LuaHandle, mi->D.HSlider.percent);
					}
					break;
				case MiTypePulldown:
					if (mi->D.Pulldown.curopt >= 0 &&
							mi->D.Pulldown.curopt < mi->D.Pulldown.noptions) {
						mi->D.Pulldown.cursel = mi->D.Pulldown.curopt;
					}
					break;
				case MiTypeListbox:
					if (mi->D.Listbox.cursel != mi->D.Listbox.curopt) {
						mi->D.Listbox.dohandler = 0;
						mi->D.Listbox.curopt = mi->D.Listbox.cursel;
						if (mi->D.Listbox.action) {
							(*mi->D.Listbox.action)(mi, mi->D.Listbox.curopt);
						} else if (mi->LuaHandle) {
							CallHandler(mi->LuaHandle, mi->D.Listbox.curopt);
						}
					} else {
						mi->D.Listbox.dohandler = 1;
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
			if (!(mi->Flags & MI_FLAGS_CLICKED)) {
				switch (mi->MiType) {
					case MiTypeInput:
						PasteFromClipboard(mi);
						if (mi->D.Input.action) {
							(*mi->D.Input.action)(mi, 'x');
						} else if (mi->LuaHandle) {
							CallHandler(mi->LuaHandle, 0);
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
			switch (mi->MiType) {
				case MiTypeListbox:
					if (mi->D.Listbox.startline > 0) {
						mi->D.Listbox.startline--;
					}
					if (mi->D.Listbox.noptions > mi->D.Listbox.nlines) {
						mi[1].D.VSlider.percent = (mi->D.Listbox.startline * 100) /
							(mi->D.Listbox.noptions - mi->D.Listbox.nlines);
					}
					if (mi[1].D.VSlider.action) {
						(*mi[1].D.VSlider.action)(mi);
					} else if (mi[1].LuaHandle) {
						CallHandler(mi[1].LuaHandle, mi[1].D.VSlider.percent);
					}
					MenuHandleMouseMove(CursorX, CursorY);
					break;
				case MiTypeVslider:
					mi->D.VSlider.cflags |= MI_CFLAGS_UP;

					// Update listbox
					if (mi > mi->Menu->Items && mi[-1].MiType == MiTypeListbox) {
						if (mi[-1].D.Listbox.startline > 0) {
							mi[-1].D.Listbox.startline--;
						}
					}
					if (mi->D.VSlider.action) {
						(*mi->D.VSlider.action)(mi);
					} else if (mi->LuaHandle) {
						CallHandler(mi->LuaHandle, mi->D.VSlider.percent);
					}
					mi->D.VSlider.cflags &= ~(MI_CFLAGS_DOWN | MI_CFLAGS_UP);
					break;
				case MiTypeHslider:
					mi->D.HSlider.percent -= 10;
					if (mi->D.HSlider.percent < 0) {
						mi->D.HSlider.percent = 0;
					}
					if (mi->D.HSlider.action) {
						(*mi->D.HSlider.action)(mi);
					} else if (mi->LuaHandle) {
						CallHandler(mi->LuaHandle, mi->D.HSlider.percent);
					}
					break;
				case MiTypePulldown:
					if (mi->D.Pulldown.curopt) {
						--mi->D.Pulldown.curopt;
						if (mi->D.Pulldown.action) {
							(*mi->D.Pulldown.action)(mi, mi->D.Pulldown.curopt);
						} else if (mi->LuaHandle) {
							CallHandler(mi->LuaHandle, mi->D.Pulldown.curopt);
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
			switch (mi->MiType) {
				case MiTypeListbox:
					if (mi->D.Listbox.startline + mi->D.Listbox.nlines < mi->D.Listbox.noptions) {
						mi->D.Listbox.startline++;
					}
					if (mi->D.Listbox.noptions > mi->D.Listbox.nlines) {
						mi[1].D.VSlider.percent = (mi->D.Listbox.startline * 100) /
							(mi->D.Listbox.noptions - mi->D.Listbox.nlines);
					}
					MenuHandleMouseMove(CursorX, CursorY);
					break;
				case MiTypeVslider:
					mi->D.VSlider.cflags |= MI_CFLAGS_DOWN;

					// Update listbox
					if (mi > mi->Menu->Items && mi[-1].MiType == MiTypeListbox) {
						if (mi[-1].D.Listbox.startline + mi[-1].D.Listbox.nlines < mi[-1].D.Listbox.noptions) {
							mi[-1].D.Listbox.startline++;
						}
					}
					if (mi->D.VSlider.action) {
						(*mi->D.VSlider.action)(mi);
					} else if (mi->LuaHandle) {
						CallHandler(mi->LuaHandle, mi->D.VSlider.percent);
					}
					mi->D.VSlider.cflags &= ~(MI_CFLAGS_DOWN | MI_CFLAGS_UP);
					break;
				case MiTypeHslider:
					mi->D.HSlider.percent += 10;
					if (mi->D.HSlider.percent > 100) {
						mi->D.HSlider.percent = 100;
					}
					if (mi->D.HSlider.action) {
						(*mi->D.HSlider.action)(mi);
					} else if (mi->LuaHandle) {
						CallHandler(mi->LuaHandle, mi->D.HSlider.percent);
					}
					break;
				case MiTypePulldown:
					if (mi->D.Pulldown.curopt < mi->D.Pulldown.noptions - 1) {
						++mi->D.Pulldown.curopt;
						if (mi->D.Pulldown.action) {
							(*mi->D.Pulldown.action)(mi, mi->D.Pulldown.curopt);
						} else if (mi->LuaHandle) {
							CallHandler(mi->LuaHandle, mi->D.Pulldown.curopt);
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
** Called if mouse button released.
**
** @param b    button code
*/
static void MenuHandleButtonUp(unsigned b)
{
	int i;
	int n;
	Menuitem *mi;
	Menu *menu;
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
			switch (mi->MiType) {
				case MiTypeCheckbox:
					if (mi->Flags & MI_FLAGS_CLICKED) {
						redraw_flag = 1;
						mi->Flags &= ~MI_FLAGS_CLICKED;
						if (MenuButtonUnderCursor == i) {
							MenuButtonUnderCursor = -1;
							mi->D.Checkbox.Checked = !mi->D.Checkbox.Checked;
							if (mi->D.Checkbox.Action) {
								(*mi->D.Checkbox.Action)(mi);
							} else if (mi->LuaHandle) {
								CallHandler(mi->LuaHandle, mi->D.Checkbox.Checked);
							}
						}
					}
					break;
				case MiTypeText:
					if (mi->Flags & MI_FLAGS_CLICKED) {
						redraw_flag = 1;
						mi->Flags &= ~MI_FLAGS_CLICKED;
						if (MenuButtonUnderCursor == i) {
							MenuButtonUnderCursor = -1;
							if (mi->D.Text.action) {
								(*mi->D.Text.action)(mi);
							}
						}
					}
					break;
				case MiTypeButton:
					if (mi->Flags & MI_FLAGS_CLICKED) {
						redraw_flag = 1;
						mi->Flags &= ~MI_FLAGS_CLICKED;
						if (MenuButtonUnderCursor == i) {
							MenuButtonUnderCursor = -1;
							if (mi->D.Button.Handler) {
								(*mi->D.Button.Handler)();
							} else if (mi->LuaHandle) {
								CallHandler(mi->LuaHandle, 0);
							}
						}
					}
					break;
				case MiTypePulldown:
					if (mi->Flags & MI_FLAGS_CLICKED) {
						redraw_flag = 1;
						mi->Flags &= ~MI_FLAGS_CLICKED;
						if (MenuButtonUnderCursor == i) {
							MenuButtonUnderCursor = -1;
							if (mi->D.Pulldown.cursel != mi->D.Pulldown.curopt &&
									mi->D.Pulldown.cursel >= 0 &&
									mi->D.Pulldown.cursel < mi->D.Pulldown.noptions) {
								mi->D.Pulldown.curopt = mi->D.Pulldown.cursel;
								if (mi->D.Pulldown.action) {
									(*mi->D.Pulldown.action)(mi, mi->D.Pulldown.curopt);
								} else if (mi->LuaHandle) {
									CallHandler(mi->LuaHandle, mi->D.Pulldown.curopt);
								}
							}
						}
						mi->D.Pulldown.cursel = 0;
					}
					break;
				case MiTypeListbox:
					if (mi->Flags & MI_FLAGS_CLICKED) {
						redraw_flag = 1;
						mi->Flags &= ~MI_FLAGS_CLICKED;
						if (MenuButtonUnderCursor == i) {
							MenuButtonUnderCursor = -1;
							if (mi->D.Listbox.dohandler && mi->D.Listbox.handler) {
								(*mi->D.Listbox.handler)();
							}
						}
					}
					break;
				case MiTypeInput:
					if (mi->Flags & MI_FLAGS_CLICKED) {
						redraw_flag = 1;
						mi->Flags &= ~MI_FLAGS_CLICKED;
						// MAYBE ADD HERE
					}
					break;
				case MiTypeVslider:
					if (mi->Flags & MI_FLAGS_CLICKED) {
						redraw_flag = 1;
						mi->Flags &= ~MI_FLAGS_CLICKED;
						mi->D.VSlider.cflags = 0;
					}
					break;
				case MiTypeHslider:
					if (mi->Flags & MI_FLAGS_CLICKED) {
						redraw_flag = 1;
						mi->Flags &= ~MI_FLAGS_CLICKED;
						mi->D.HSlider.cflags = 0;
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
	struct _menu_* Menu;
	int CurSel;
	struct _menu_stack_* Next;
} MenuStack;

MenuStack *Menus;             /// FIXME : docu

/**
** Push the current menu onto the stack.
*/
static void PushMenu(void)
{
	MenuStack *menu;

	menu = new MenuStack;
	menu->Menu = CurrentMenu;
	menu->CurSel = MenuButtonCurSel;
	menu->Next = Menus;
	Menus = menu;
}

/**
** Pop the stack and set the current menu
*/
static void PopMenu(void)
{
	MenuStack *menu;

	if (Menus && Menus->Menu == CurrentMenu) {
		if (CurrentMenu->ExitFunc) {
			CurrentMenu->ExitFunc(CurrentMenu); // action/destructor
		}

		MenuButtonUnderCursor = -1;
		MenuButtonCurSel = -1;
		CurrentMenu = NULL;

		MenuButtonCurSel = Menus->CurSel;
		menu = Menus;
		Menus = Menus->Next;
		delete menu;
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
void CloseMenu(void)
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
** Process a menu.
**
** @param menu_id The menu number to process
** @param loop Indicates to setup handlers and really 'Process'
**
** @todo FIXME: This function is called from the event handler!!
*/
void ProcessMenu(const char *menu_id, int loop)
{
	int i;
	int oldncr;
	Menuitem *mi;
	Menu *menu;
	Menu *CurrentMenuSave;
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
	GameCursor = UI.Point.Cursor;
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
		switch (mi->MiType) {
			case MiTypeButton:
			case MiTypePulldown:
			case MiTypeListbox:
			case MiTypeVslider:
			case MiTypeHslider:
			case MiTypeInput:
				mi->Flags &= ~(MI_FLAGS_CLICKED | MI_FLAGS_ACTIVE | MI_FLAGS_SELECTED);
				if (i == menu->DefSel) {
					mi->Flags |= MI_FLAGS_SELECTED;
					MenuButtonCurSel = i;
				}
				break;
			default:
				break;
			
		}
		switch (mi->MiType) {
			case MiTypePulldown:
				mi->D.Pulldown.cursel = 0;
				if (mi->D.Pulldown.defopt != -1) {
					mi->D.Pulldown.curopt = mi->D.Pulldown.defopt;
				}
				break;
			case MiTypeListbox:
				mi->D.Listbox.cursel = -1;
				mi->D.Listbox.startline = 0;
				if (mi->D.Listbox.defopt != -1) {
					mi->D.Listbox.curopt = mi->D.Listbox.defopt;
				}
				break;
			case MiTypeVslider:
				mi->D.VSlider.cflags = 0;
				if (mi->D.VSlider.defper != -1) {
					mi->D.VSlider.percent = mi->D.VSlider.defper;
				}
				break;
			case MiTypeHslider:
				mi->D.HSlider.cflags = 0;
				if (mi->D.HSlider.defper != -1) {
					mi->D.HSlider.percent = mi->D.HSlider.defper;
				}
				break;
			default:
				break;
		}
	}
	if (menu->InitFunc) {
		menu->InitFunc(menu);
	}
	MenuButtonUnderCursor = -1;

	if (loop) {
		SetVideoSync();
		MenuHandleMouseMove(CursorX,CursorY); // This activates buttons as appropriate!
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
					menu->NetAction();
				}
			}
		}
	}

	if (loop) {
		if (menu->ExitFunc) {
			menu->ExitFunc(menu); // action/destructor
		}
		CurrentMenu = CurrentMenuSave;
		MenuButtonUnderCursor = MenuButtonUnderCursorSave;
		MenuButtonCurSel = MenuButtonCurSelSave;
	}

	// FIXME: should ExitMenus() be called instead?!?
}

/**
**  Init Menus for a specific race
**
**  @param race    The Race to set-up for
*/
void InitMenus(int race)
{
	static int last_race = -1;

	InitMenuData();

#ifndef USE_OPENGL
	if (race == last_race) { // same race? already loaded!
		return;
	}
#endif

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
		CGraphic::Free(MenuButtonG);
	}
	last_race = race;
	MenuButtonG = MenuButtonGraphics[race];
	MenuButtonG->Load();

	CurrentMenu = NULL;
}

/**
**  Exit Menus code (freeing data)
*/
void ExitMenus(void)
{
}

//@}
