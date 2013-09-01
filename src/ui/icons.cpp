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
}

/**
**  CIcon destructor
*/
CIcon::~CIcon()
{
	CGraphic::Free(this->G);
	CGraphic::Free(this->GScale);
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
	CIcon *icon = Icons[ident];
	if (icon) {
		return icon;
	} else {
		icon = new CIcon(ident);
		Icons[ident] = icon;
		return icon;
	}
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
	CIcon *icon = Icons[ident];
	if (!icon) {
		DebugPrint("icon not found: %s\n" _C_ ident.c_str());
	}
	return icon;
}

void CIcon::Load()
{
	Assert(G);
	G->Load();
	GScale = G->Clone(true);
	if (Frame >= G->NumFrames) {
		DebugPrint("Invalid icon frame: %s - %d\n" _C_ Ident.c_str() _C_ Frame);
		Frame = 0;
	}
}

/**
**  Draw icon at pos.
**
**  @param player  Player pointer used for icon colors
**  @param pos     display pixel position
*/
void CIcon::DrawIcon(const CPlayer &player, const PixelPos &pos) const
{
	CPlayerColorGraphic *g = dynamic_cast<CPlayerColorGraphic *>(this->G);
	if (g) {
		g->DrawPlayerColorFrameClip(player.Index, this->Frame, pos.x, pos.y);
	} else {
		this->G->DrawFrameClip(this->Frame, pos.x, pos.y);
	}
}

/**
**  Draw grayscale icon at pos.
**
**  @param pos     display pixel position
*/
void CIcon::DrawGrayscaleIcon(const PixelPos &pos) const
{
	this->GScale->DrawFrameClip(this->Frame, pos.x, pos.y);
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
	this->GScale->DrawFrameClip(this->Frame, pos.x, pos.y);
	const int height = (G->Height * (100 - percent)) / 100;
	this->G->DrawSubClip(G->frame_map[Frame].x, G->frame_map[Frame].y + G->Height - height, G->Width,
						 height, pos.x, pos.y + G->Height - height);
}

/**
**  Draw unit icon 'icon' with border on x,y
**
**  @param style   Button style
**  @param flags   State of icon (clicked, mouse over...)
**  @param pos     display pixel position
**  @param text    Optional text to display
*/
void CIcon::DrawUnitIcon(const ButtonStyle &style,
						 unsigned flags, const PixelPos &pos, const std::string &text) const
{
	ButtonStyle s(style);

	s.Default.Sprite = s.Hover.Sprite = s.Clicked.Sprite = this->G;
	s.Default.Frame = s.Hover.Frame = s.Clicked.Frame = this->Frame;
	if (!(flags & IconSelected) && (flags & IconAutoCast)) {
		s.Default.BorderColorRGB = UI.ButtonPanel.AutoCastBorderColorRGB;
		s.Default.BorderColor = 0;
	}
	// FIXME: player colors
	DrawUIButton(&s, flags, pos.x, pos.y, text);
}

/**
**  Load the Icon
*/
bool IconConfig::LoadNoLog()
{
	Assert(!Name.empty());

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
