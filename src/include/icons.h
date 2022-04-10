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
/**@name icons.h - The icons headerfile. */
//
//      (c) Copyright 1998-2005 by Lutz Sammer
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

#ifndef __ICONS_H__
#define __ICONS_H__

//@{

#include "vec2i.h"
#include "color.h"
#include <string>

/*----------------------------------------------------------------------------
--  Documentation
----------------------------------------------------------------------------*/

/**
**  @class CIcon icons.h
**
**  \#include "icons.h"
**
**  This structure contains all information about an icon.
**
**  The icon structure members:
**
**  CIcon::Ident
**
**    Unique identifier of the icon, used to reference it in config
**    files and during startup.  Don't use this in game, use instead
**    the pointer to this structure.
**
**  CIcon::G
**
**    Graphic image containing the loaded graphics. Loaded by
**    LoadIcons(). All icons belonging to the same icon file shares
**    this structure.
**
**  CIcon::Frame
**
**    Frame number in the graphic to display.
*/

/**
**  @class IconConfig icons.h
**
**  \#include "icons.h"
**
**  This structure contains all configuration information about an icon.
**
**  IconConfig::Name
**
**    Unique identifier of the icon, used to reference icons in config
**    files and during startup.  The name is resolved during game
**    start and the pointer placed in the next field.
**    @see CIcon::Ident
**
**  IconConfig::Icon
**
**    Pointer to an icon. This pointer is resolved during game start.
*/

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

#define IconActive   1  /// cursor on icon
#define IconClicked  2  /// mouse button down on icon
#define IconSelected 4  /// this the selected icon
#define IconDisabled 8  /// icon disabled
#define IconAutoCast 16 /// auto cast icon

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CUnit;
class CGraphic;
class CPlayerColorGraphic;
class CPlayer;
class ButtonStyle;

/// Icon: rectangle image used in menus
class CIcon
{
public:
	CIcon(const std::string &ident);
	~CIcon();

	static CIcon *New(const std::string &ident);
	static CIcon *Get(const std::string &ident);

	void Load();

	/// Draw icon
	void DrawIcon(const PixelPos &pos, const int player = -1) const;
	/// Draw grayscale icon
	void DrawGrayscaleIcon(const PixelPos &pos, const int player = -1) const;
	/// Draw cooldown spell
	void DrawCooldownSpellIcon(const PixelPos &pos, const int percent) const;
	/// Draw icon of a unit
	void DrawUnitIcon(const ButtonStyle &style,
					  unsigned flags, const PixelPos &pos, const std::string &text, const int playerIndex = -1) const;
	void DrawSingleSelectionIcon(const ButtonStyle &style,
					  unsigned flags, const PixelPos &pos, const std::string &text, const CUnit &unit) const;
	void DrawGroupSelectionIcon(const ButtonStyle &style,
					  unsigned flags, const PixelPos &pos, const std::string &text, const CUnit &unit) const;
	void DrawContainedIcon(const ButtonStyle &style,
					  unsigned flags, const PixelPos &pos, const std::string &text, const CUnit &unit) const;

	const std::string &GetIdent() const { return this->Ident; }

	/// Modify the extra icon graphics
	void ClearExtraGraphics();
	void AddSingleSelectionGraphic(CPlayerColorGraphic *g);
	void AddGroupSelectionGraphic(CPlayerColorGraphic *g);
	void AddContainedGraphic(CPlayerColorGraphic *g);

	void SetPaletteSwaps(std::vector<PaletteSwap> &newSwaps);

public:
	CPlayerColorGraphic *G;              /// Graphic data
	CPlayerColorGraphic *GScale;         /// Icon when drawn grayscaled
	int Frame;                /// Frame number in graphic
private:
	std::string Ident;        /// Icon identifier

	/*
	 * These following lists of icons are used to allow different icons for single and multi-selection as
	 * well as when the unit is contained in some container like a transport or a bunker.
	 * Additionally, these are lists, and the health percentage of the unit is mapped into this list. this
	 * way, units can have different (damaged) icons. all of these are optional, the default is to just use
	 * the graphic stored in the *G field above.
	 */
	std::vector<CPlayerColorGraphic *> SingleSelectionG; /// graphics by health status for single-selection
	std::vector<CPlayerColorGraphic *> GroupSelectionG;  /// graphics by health status for multi-selection
	std::vector<CPlayerColorGraphic *> ContainedG;       /// graphics by health status when in a container

	/*
	 * These following lists are used to map percentages of arbitrary unit variables (e.g. health, shield,
	 * mana, ...) to palette swaps in the selection icons.
	 */
	std::vector<PaletteSwap> PaletteSwaps;
};

/// Icon reference (used in config tables)
class IconConfig
{
public:
	IconConfig() : Icon(NULL) {}

	bool LoadNoLog();
	bool Load();
public:
	std::string Name;    /// config icon name
	CIcon *Icon;         /// icon pointer to use to run time
};

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

extern void LoadIcons();   /// Load icons
extern void CleanIcons();  /// Cleanup icons

//@}

#endif // !__ICONS_H__
