//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name icons.cpp - The icons. */
//
//      (c) Copyright 1998-2006 by Lutz Sammer and Jimmy Salmon
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stratagus.h"

#include <string>
#include <vector>
#include <map>

#include "video.h"
#include "icons.h"
#include "player.h"
#include "ui.h"
#include "menus.h"


/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

static std::vector<CIcon *> AllIcons;          /// Vector of all icons.
std::map<std::string, CIcon *> Icons;          /// Map of ident to icon.


/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  CIcon constructor
*/
CIcon::CIcon(const std::string &ident) : G(NULL), Frame(0), Ident(ident)
{
}

/**
**  CIcon destructor
*/
CIcon::~CIcon()
{
	CGraphic::Free(this->G);
}

/**
**  Create a new icon
**
**  @param ident  Icon identifier
**
**  @return       New icon
*/
CIcon *CIcon::New(const std::string &ident)
{
	CIcon *icon = Icons[ident];
	if (icon) {
		return icon;
	} else {
		icon = new CIcon(ident);
		Icons[ident] = icon;
		AllIcons.push_back(icon);
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
CIcon *CIcon::Get(const std::string &ident)
{
	CIcon *icon = Icons[ident];
	if (!icon) {
		DebugPrint("icon not found: %s\n" _C_ ident.c_str());
	}
	return icon;
}

/**
**  Load the Icon
**
**
*/
void IconConfig::Load()
{
	Assert(!Name.empty());

	Icon = CIcon::Get(Name);
	if (!Icon) {
		fprintf(stderr, "Can't find icon %s\n", Name.c_str());
		ExitFatal(-1);
	}
};

/**
**  Init the icons.
**
**  Add the short name and icon aliases to hash table.
*/
void InitIcons(void)
{
}

/**
**  Load the graphics for the icons.
*/
void LoadIcons(void)
{
	for (size_t i = 0; i < AllIcons.size(); ++i) {
		CIcon *icon = AllIcons[i];
		icon->G->Load();
		ShowLoadProgress("Icons %s", icon->G->File.c_str());
		if (icon->Frame >= icon->G->NumFrames) {
			DebugPrint("Invalid icon frame: %s - %d\n" _C_
				icon->GetIdent().c_str() _C_ icon->Frame);
			icon->Frame = 0;
		}
	}
}

/**
**  Clean up memory used by the icons.
*/
void CleanIcons(void)
{
	std::vector<CIcon *>::iterator i;
	for (i = AllIcons.begin(); i != AllIcons.end(); ++i) {
		delete *i;
	}
	AllIcons.clear();
	Icons.clear();
}

/**
**  Draw icon on x,y.
**
**  @param player  Player pointer used for icon colors
**  @param x       X display pixel position
**  @param y       Y display pixel position
*/
void CIcon::DrawIcon(const CPlayer *player, int x, int y) const
{
	CPlayerColorGraphic *g = dynamic_cast<CPlayerColorGraphic *>(this->G);
	if (g) {
		g->DrawPlayerColorFrameClip(player->Index, this->Frame, x, y);
	} else {
		this->G->DrawFrameClip(this->Frame, x, y);
	}
}

/**
**  Draw unit icon 'icon' with border on x,y
**
**  @param player  Player pointer used for icon colors
**  @param style   Button style
**  @param flags   State of icon (clicked, mouse over...)
**  @param x       X display pixel position
**  @param y       Y display pixel position
**  @param text    Optional text to display
*/
void CIcon::DrawUnitIcon(const CPlayer *player, ButtonStyle *style,
	unsigned flags, int x, int y, const std::string &text) const
{
	static ButtonStyle s;
	s = *style;

	s.Default.Sprite = s.Hover.Sprite = s.Clicked.Sprite = this->G;
	s.Default.Frame = s.Hover.Frame = s.Clicked.Frame = this->Frame;
	if (!(flags & IconSelected) && (flags & IconAutoCast)) {
		s.Default.BorderColorRGB = UI.ButtonPanel.AutoCastBorderColorRGB;
		s.Default.BorderColor = 0;
	}
	// FIXME: player colors
	DrawMenuButton(&s, flags, x, y, text);
}

/**
**  Register CCL features for icons.
*/
void IconCclRegister(void)
{
}

//@}
