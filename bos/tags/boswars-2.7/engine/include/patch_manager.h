//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name patch.h - The patch manager header. */
//
//      (c) Copyright 2008-2010 by Jimmy Salmon
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

#ifndef _PATCH_MANAGER_H_
#define _PATCH_MANAGER_H_

//@{

#include <list>
#include <map>
#include <vector>
#include <string>

class CPatchType;
class CPatch;
class CFile;


class CPatchManager
{
public:
	/**
	**  Patch manager constructor
	*/
	CPatchManager();

	/**
	**  Patch manager destructor
	*/
	~CPatchManager();

	/**
	**  Create a new patch of type typeName at location x, y
	*/
	CPatch *add(const std::string &typeName, int x, int y);

	/**
	**  Remove a patch
	*/
	void remove(CPatch *patch);

	/**
	**  Move a patch's location
	*/
	void move(CPatch *patch, int x, int y);

	/**
	**  Move a patch to the top
	*/
	void moveToTop(CPatch *patch);

	/**
	**  Move a patch to the bottom
	*/
	void moveToBottom(CPatch *patch);

	/**
	**  Get the patch at location x, y
	*/
	CPatch *getPatch(int x, int y, int *xOffset = NULL, int *yOffset = NULL) const;

	/**
	**  Get all of the patches
	*/
	const std::list<CPatch *> &getPatches() const;

	/**
	**  Get all of the patch names
	*/
	std::vector<std::string> getPatchTypeNames() const;

	/**
	**  Get the names of the patch types that use the graphic in
	**  the specified file.
	*/
	std::vector<std::string> getPatchTypeNamesUsingGraphic(
		const std::string &graphicFile) const;

	/**
	**  Get all of the patch themes
	*/
	std::vector<std::string> getPatchTypeThemes() const;

	/**
	**  Load the patches used in the map
	*/
	void load();

	/**
	**  Load all patches
	*/
	void loadAll();

	/**
	**  Clear the patches used in the map
	*/
	void clear();

	/**
	**  Define a new patch type.
	**  Types should be created only once and last for the duration of the game.
	*/
	CPatchType *newPatchType(const std::string &name, const std::string &file,
		int tileWidth, int tileHeight, int *flags, const std::string &theme = "");

	/**
	**  Define a new patch type.
	**  Types should be created only once and last for the duration of the game.
	*/
	CPatchType *newPatchType(const std::string &name, const std::string &file,
		int tileWidth, int tileHeight, unsigned short *flags, const std::string &theme = "");

	/**
	**  Get a patch type
	*/
	CPatchType *getPatchType(const std::string &name);

	/**
	**  Save all of the patches
	*/
	std::string savePatches(bool patchesOnly = false) const;

	/**
	**  Save a patch type
	*/
	std::string savePatchType(CPatchType *patchType) const;

	/**
	**  Compute how large the patch type should be if one were
	**  created for the graphic in the specified file.
	**  This function is intended to be called from Lua.
	*/
	bool computePatchSize(const std::string &graphicFile,
	                      int *width, int *height) const;

private:
	void updateMapFlags(int x1, int y1, int x2, int y2);

	std::list<CPatch *> patches;
	std::map<std::string, CPatchType *> patchTypesMap;

	std::list<CPatch *> removedPatches;

	bool loadedAll;
};

//@}

#endif /* _PATCH_MANAGER_H_ */

