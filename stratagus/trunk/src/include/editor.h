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
/**@name editor.h	-	The editor file. */
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

#ifndef __EDITOR_H__
#define __EDITOR_H__

//@{

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

    /// Editor is running
extern char EditorRunning;

    /// Editor CCL start file
extern const char* EditorStartFile;

extern char** EditorUnitTypes;		/// Sorted editor unit-type table

extern int MaxUnitIndex;		/// Max unit icon draw index

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

    ///	Editor main event loop
extern void EditorMainLoop(void);
    /// Update editor display
extern void EditorUpdateDisplay(void);

    /// Register ccl features
extern void EditorCclRegister(void);

    /// Update surroundings for tile changes.
extern void EditorTileChanged(int x, int y);

//@}

#endif	// !__EDITOR_H__
