//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name editor.h - The editor file. */
//
//      (c) Copyright 2002-2010 by Lutz Sammer and Jimmy Salmon
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

#ifndef __EDITOR_H__
#define __EDITOR_H__

//@{

#include <vector>
#include <string>
#include "icons.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CUnitType;
class CPatchType;

enum EditorRunningType {
	EditorNotRunning = 0,    /// Not Running
	EditorStarted = 1,       /// Editor Enabled at all
	EditorCommandLine = 2,   /// Called from Command Line
	EditorEditing = 4,       /// Editor is fully running
};

enum EditorStateType {
	EditorSelecting,         /// Select
	EditorEditPatch,         /// Edit patch
	EditorEditUnit,          /// Edit units
	EditorSetStartLocation   /// Set the start location
};

struct CPatchIcon
{
	CPatchIcon(CPatchType *patchType, CGraphic *g) :
		PatchType(patchType), G(g)
	{}

	CPatchType *PatchType;
	CGraphic *G;
};

class CEditor {
public:
	CEditor() : StartUnit(NULL),
		UnitIndex(0), CursorUnitIndex(-1), SelectedUnitIndex(-1),
		PatchIndex(0), CursorPatchIndex(-1), SelectedPatchIndex(-1),
		CursorPlayer(-1), SelectedPlayer(-1), MapLoaded(false),
		ShowPatchOutlines(false)
		{};
	~CEditor() {};

	void Init();

	std::vector<std::string> UnitTypes;             /// Sorted editor unit-type table.
	std::vector<const CUnitType *> ShownUnitTypes;  /// Shown editor unit-type table.

	std::vector<CPatchIcon *> ShownPatchTypes;         /// Shown editor patch-type table.
	std::map<std::string, CPatchIcon *> AllPatchTypes; /// All editor patch-type table.

	IconConfig Select;           /// Editor's select icon.
	IconConfig Units;            /// Editor's units icon.
	IconConfig Patch;            /// Editor's patch icon.
	std::string StartUnitName;   /// name of the Unit used to display the start location.
	const CUnitType *StartUnit;  /// Unit used to display the start location.

	int UnitIndex;               /// Unit icon draw index.
	int CursorUnitIndex;         /// Unit icon under cursor.
	int SelectedUnitIndex;       /// Unit type to draw.

	int PatchIndex;              /// Patch icon draw index.
	int CursorPatchIndex;        /// Patch icon under cursor.
	int SelectedPatchIndex;      /// Patch type to draw.

	int CursorPlayer;            /// Player under the cursor.
	int SelectedPlayer;          /// Player selected for draw.

	bool MapLoaded;              /// Map loaded in editor

	bool ShowPatchOutlines;      /// Show outlines around patches
	Uint32 PatchOutlineColor;    /// Patch outline color

	bool ShowTerrainFlags;       /// Show CMapField::Flags

	EditorRunningType Running;   /// Editor is running

	EditorStateType State;       /// Current editor state

	void TileSelectedPatch();
};

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

extern CEditor Editor;

extern const char *EditorStartFile;  /// Editor CCL start file

extern bool PatchEditorRunning;

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

extern void SetEditorSelectIcon(const std::string &icon);
extern void SetEditorUnitsIcon(const std::string &icon);
extern void SetEditorPatchIcon(const std::string &icon);
extern void SetEditorStartUnit(const std::string &name);

	/// Start the editor
extern void StartEditor(const std::string &filename);

	/// Save a map from editor
extern int EditorSaveMap(const std::string &file);

	/// Start the patch editor
extern void StartPatchEditor(const std::string &patchName);

//@}

#endif // !__EDITOR_H__
