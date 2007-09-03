//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name editor.cpp - Editor functions. */
//
//      (c) Copyright 2002-2005 by Lutz Sammer
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

//----------------------------------------------------------------------------
//  Documentation
//----------------------------------------------------------------------------

/**
**  @page EditorModule Module - Editor
**
**  This is a very simple editor for the Stratagus engine.
**
**  @section Missing Missing features
**
**    @li Edit upgrade section
**    @li Edit allow section
**    @li Edit .cm files
**    @li Upgraded unit-types should be shown different on map
**    @li Good keyboard bindings
**    @li Script support
**    @li Commandline support
**    @li Cut&Paste
**    @li More random map functions.
*/

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <stdio.h>

#include "stratagus.h"

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

const char *EditorStartFile;  /// Editor CCL start file

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

//@}
