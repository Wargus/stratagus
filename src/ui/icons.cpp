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
/**@name icons.cpp - The icons. */
//
//      (c) Copyright 1998-2012 by Lutz Sammer and Jimmy Salmon
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

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#include "icons.h"

#include "menus.h"
#include "player.h"
#include "translate.h"
#include "ui.h"
#include "unit.h"
#include "video.h"

#include <map>

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

typedef std::map<std::string, CIcon *> IconMap;
static IconMap Icons;   /// Map of ident to icon.


/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  CIcon constructor
*/
CIcon::CIcon(const std::string &ident) : G(NULL), GScale(NULL), Frame(0), Ident(ident)
{
	this->PaletteSwaps.clear();
}

/**
**  CIcon destructor
*/
CIcon::~CIcon()
{
	CPlayerColorGraphic::Free(this->G);
	CPlayerColorGraphic::Free(this->GScale);
	ClearExtraGraphics();
}

/**
**  Create a new icon
**
**  @param ident  Icon identifier
**
**  @return       New icon
*/
/* static */ CIcon *CIcon::New(const std::string &ident)
{
	CIcon *&icon = Icons[ident];

	if (icon == NULL) {
		icon = new CIcon(ident);
	}
	return icon;
}

/**
**  Get an icon
**
**  @param ident  Icon identifier
**
**  @return       The icon
*/
/* static */ CIcon *CIcon::Get(const std::string &ident)
{
	IconMap::iterator it = Icons.find(ident);
	if (it == Icons.end()) {
		DebugPrint("icon not found: %s\n" _C_ ident.c_str());
	}
	return it->second;
}

void CIcon::Load()
{
	Assert(G);
	G->Load();
	if (Preference.GrayscaleIcons) {
		GScale = G->Clone(true);
	}
	if (Frame >= G->NumFrames) {
		DebugPrint("Invalid icon frame: %s - %d\n" _C_ Ident.c_str() _C_ Frame);
		Frame = 0;
	}
	for (auto g : this->SingleSelectionG) {
		g->Load();
		if (Frame >= G->NumFrames) {
			DebugPrint("Invalid icon frame for single selection graphic: %s - %d\n" _C_ Ident.c_str() _C_ Frame);
		}
	}
	for (auto g : this->GroupSelectionG) {
		g->Load();
		if (Frame >= G->NumFrames) {
			DebugPrint("Invalid icon frame for group selection graphic: %s - %d\n" _C_ Ident.c_str() _C_ Frame);
		}
	}
	for (auto g : this->ContainedG) {
		g->Load();
		if (Frame >= G->NumFrames) {
			DebugPrint("Invalid icon frame for transport selection graphic: %s - %d\n" _C_ Ident.c_str() _C_ Frame);
		}
	}
}

/**
**  Draw icon at pos.
**
**  @param player  Player pointer used for icon colors
**  @param pos     display pixel position
*/
void CIcon::DrawIcon(const PixelPos &pos, const int player) const
{
	if (player != -1 ) {
		this->G->DrawPlayerColorFrameClip(player, this->Frame, pos.x, pos.y);
	} else {
		this->G->DrawFrameClip(this->Frame, pos.x, pos.y);
	}
}

/**
**  Draw grayscale icon at pos.
**
**  @param pos     display pixel position
*/
void CIcon::DrawGrayscaleIcon(const PixelPos &pos, const int player) const
{
	if (this->GScale) {
		if (player != -1) {
			this->GScale->DrawPlayerColorFrameClip(player, this->Frame, pos.x, pos.y);
		} else {
			this->GScale->DrawFrameClip(this->Frame, pos.x, pos.y);
		}
	}
}

/**
**  Draw cooldown spell effect on icon at pos.
**
**  @param pos       display pixel position
**  @param percent   cooldown percent
*/
void CIcon::DrawCooldownSpellIcon(const PixelPos &pos, const int percent) const
{
	// TO-DO: implement more effect types (clock-like)
	if (this->GScale) {
		this->GScale->DrawFrameClip(this->Frame, pos.x, pos.y);
		const int height = (G->Height * (100 - percent)) / 100;
		this->G->DrawSubClip(G->frame_map[Frame].x, G->frame_map[Frame].y + G->Height - height,
							 G->Width, height, pos.x, pos.y + G->Height - height);
	} else {
		DebugPrint("Enable grayscale icon drawing in your game to achieve special effects for cooldown spell icons");
		this->DrawIcon(pos);
	}
}

static void ApplyPaletteSwaps(const std::vector<PaletteSwap> &swaps, const CUnit &unit, CGraphic *graphic, bool def = false)
{
	for (PaletteSwap swap : swaps) {
		unsigned int varIdx = swap.GetUnitVariableIndex();
		if (unit.Variable[varIdx].Enable) {
			int value = def ? unit.Variable[varIdx].Max : unit.Variable[varIdx].Value;
			const SDL_Color *colors = swap.GetColorsForPercentAndAlternative(value, unit.Variable[varIdx].Max, UnitNumber(unit));
			SDL_SetPaletteColors(graphic->Surface->format->palette, colors, swap.GetColorIndexStart(), swap.GetColorCount());
		}
	}
}

static void DrawByHealthIcon(const CIcon *icon, const std::vector<CPlayerColorGraphic *> &graphics,
						const ButtonStyle &style, unsigned flags,
						const PixelPos &pos, const std::string &text, const CUnit &unit, const std::vector<PaletteSwap> &swaps) {
	int playerColor = unit.RescuedFrom
				? GameSettings.Presets[unit.RescuedFrom->Index].PlayerColor
				: GameSettings.Presets[unit.Player->Index].PlayerColor;
	int sz = graphics.size();
	if (!sz) {
		ApplyPaletteSwaps(swaps, unit, dynamic_cast<CGraphic *>(icon->G));
		icon->DrawUnitIcon(style, flags, pos, text, playerColor);
		// the normal icons are used outside this code path as well, so undo the swap immediately
		ApplyPaletteSwaps(swaps, unit, dynamic_cast<CGraphic *>(icon->G), true);
	} else {
		// TODO: we could have this more configurable?
		int graphicIdx = 0;
		if (sz > 1 && unit.Variable[HP_INDEX].Max) {
			graphicIdx = ((sz - 1) * unit.Variable[HP_INDEX].Value) / unit.Variable[HP_INDEX].Max;
		}
		ButtonStyle s(style);
		CGraphic *g = graphics[graphicIdx];
		ApplyPaletteSwaps(swaps, unit, g);
		s.Default.Sprite = s.Hover.Sprite = s.Clicked.Sprite = g;
		s.Default.Frame = s.Hover.Frame = s.Clicked.Frame = icon->Frame;
		DrawUIButton(&s, flags, pos.x, pos.y, text, playerColor);
	}
}

void CIcon::DrawSingleSelectionIcon(const ButtonStyle &style, unsigned flags,
						 const PixelPos &pos, const std::string &text, const CUnit &unit) const
{
	DrawByHealthIcon(this, this->SingleSelectionG, style, flags, pos, text, unit, this->PaletteSwaps);
}

void CIcon::DrawGroupSelectionIcon(const ButtonStyle &style, unsigned flags,
						 const PixelPos &pos, const std::string &text, const CUnit &unit) const
{
	DrawByHealthIcon(this, this->GroupSelectionG, style, flags, pos, text, unit, this->PaletteSwaps);
}

void CIcon::DrawContainedIcon(const ButtonStyle &style, unsigned flags,
						 const PixelPos &pos, const std::string &text, const CUnit &unit) const
{
	DrawByHealthIcon(this, this->ContainedG, style, flags, pos, text, unit, this->PaletteSwaps);
}

void CIcon::ClearExtraGraphics()
{
	this->SingleSelectionG.clear();
	this->GroupSelectionG.clear();
	this->ContainedG.clear();
}

void CIcon::AddSingleSelectionGraphic(CPlayerColorGraphic *g)
{
	this->SingleSelectionG.push_back(g);
}

void CIcon::AddGroupSelectionGraphic(CPlayerColorGraphic *g)
{
	this->GroupSelectionG.push_back(g);
}

void CIcon::AddContainedGraphic(CPlayerColorGraphic *g)
{
	this->ContainedG.push_back(g);
}

void CIcon::SetPaletteSwaps(std::vector<PaletteSwap> &newSwaps)
{
	this->PaletteSwaps.clear();
	for (auto s : newSwaps) {
		this->PaletteSwaps.push_back(s);
	}
}

/**
**  Draw unit icon 'icon' with border on x,y
**
**  @param style   Button style
**  @param flags   State of icon (clicked, mouse over...)
**  @param pos     display pixel position
**  @param text    Optional text to display
*/
void CIcon::DrawUnitIcon(const ButtonStyle &style, unsigned flags,
						 const PixelPos &pos, const std::string &text, int player) const
{
	ButtonStyle s(style);

	s.Default.Sprite = s.Hover.Sprite = s.Clicked.Sprite = this->G;
	s.Default.Frame = s.Hover.Frame = s.Clicked.Frame = this->Frame;
	if (!(flags & IconSelected) && (flags & IconAutoCast)) {
		s.Default.BorderColorRGB = UI.ButtonPanel.AutoCastBorderColorRGB;
		s.Default.BorderColor = 0;
		s.Default.BorderSize = 2;
	}
	if (Preference.IconsShift && Preference.IconFrameG && Preference.PressedIconFrameG) {
		int shift = 0;
		if (!(flags & IconClicked)) {
			int xoffset = (s.Width - Preference.IconFrameG->Width) / 2;
			int yoffset = (s.Height - Preference.IconFrameG->Height) / 2;
			Preference.IconFrameG->DrawClip(pos.x + xoffset, pos.y + yoffset);
		} else { // Shift the icon a bit to make it look like it's been pressed.
			shift = 1;
			int xoffset = (s.Width - Preference.PressedIconFrameG->Width) / 2 + shift;
			int yoffset = (s.Height - Preference.PressedIconFrameG->Height) / 2 + shift;
			Preference.PressedIconFrameG->DrawClip(pos.x + xoffset, pos.y + yoffset);
		}
		DrawUIButton(&s, flags, pos.x + shift, pos.y + shift, text, player);
		if (flags & IconSelected) {
			Video.DrawRectangle(ColorGreen, pos.x + shift, pos.y + shift, s.Width, s.Height);
		}
	} else if (Preference.IconsShift) {
		// Left and top edge of Icon
		Video.DrawHLine(ColorWhite, pos.x - 1, pos.y - 1, 49);
		Video.DrawVLine(ColorWhite, pos.x - 1, pos.y, 40);
		Video.DrawVLine(ColorWhite, pos.x, pos.y + 38, 2);
		Video.DrawHLine(ColorWhite, pos.x + 46, pos.y, 2);

		// Bottom and Right edge of Icon
		Video.DrawHLine(ColorGray, pos.x + 1, pos.y + 38, 47);
		Video.DrawHLine(ColorGray, pos.x + 1, pos.y + 39, 47);
		Video.DrawVLine(ColorGray, pos.x + 46, pos.y + 1, 37);
		Video.DrawVLine(ColorGray, pos.x + 47, pos.y + 1, 37);

		Video.DrawRectangle(ColorBlack, pos.x - 3, pos.y - 3, 52, 44);
		Video.DrawRectangle(ColorBlack, pos.x - 4, pos.y - 4, 54, 46);

		if (flags & IconActive) { // Code to make a border appear around the icon when the mouse hovers over it.
			Video.DrawRectangle(ColorGray, pos.x - 4, pos.y - 4, 54, 46);
			DrawUIButton(&s, flags, pos.x, pos.y, text, player);
		}

		if (flags & IconClicked) { // Shift the icon a bit to make it look like it's been pressed.
			DrawUIButton(&s, flags, pos.x + 1, pos.y + 1, text, player);
			if (flags & IconSelected) {
				Video.DrawRectangle(ColorGreen, pos.x + 1, pos.y + 1, 46, 38);
			}
			Video.DrawRectangle(ColorGray, pos.x, pos.y, 48, 40);
			Video.DrawVLine(ColorDarkGray, pos.x - 1, pos.y - 1, 40);
			Video.DrawHLine(ColorDarkGray, pos.x - 1, pos.y - 1, 49);
			Video.DrawHLine(ColorDarkGray, pos.x - 1, pos.y + 39, 2);

			Video.DrawRectangle(ColorGray, pos.x - 4, pos.y - 4, 54, 46);
		} else {
			DrawUIButton(&s, flags, pos.x, pos.y, text, player);
			if (flags & IconSelected) {
				Video.DrawRectangle(ColorGreen, pos.x, pos.y, 46, 38);
			}
		}
	} else {
		DrawUIButton(&s, flags, pos.x, pos.y, text, player);
	}
}

/**
**  Load the Icon
*/
bool IconConfig::LoadNoLog()
{
	if (Name.empty()) {
		return false;
	}

	Icon = CIcon::Get(Name);
	return Icon != NULL;
}

/**
**  Load the Icon
*/
bool IconConfig::Load()
{
	if (LoadNoLog() == true) {
		ShowLoadProgress(_("Icon %s"), this->Name.c_str());
		return true;
	} else {
		fprintf(stderr, _("Can't find icon %s\n"), this->Name.c_str());
		return false;
	}
}

/**
**  Load the graphics for the icons.
*/
void LoadIcons()
{
	for (IconMap::iterator it = Icons.begin(); it != Icons.end(); ++it) {
		CIcon &icon = *(*it).second;

		ShowLoadProgress(_("Icons %s"), icon.G->File.c_str());
		icon.Load();
	}
}

/**
**  Clean up memory used by the icons.
*/
void CleanIcons()
{
	for (IconMap::iterator it = Icons.begin(); it != Icons.end(); ++it) {
		CIcon *icon = (*it).second;
		delete icon;
	}
	Icons.clear();
}

//@}
