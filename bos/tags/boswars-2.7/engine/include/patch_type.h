//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name patch_type.h - The patch type header. */
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

#ifndef _PATCH_TYPE_H_
#define _PATCH_TYPE_H_

//@{

#include "video.h"


class CPatchType
{
public:
	/**
	**  Patch type constructor
	*/
	CPatchType(const std::string &name, const std::string &file,
	           int tileWidth, int tileHeight, unsigned short *flags, bool customPatch, const std::string &theme = "") :
		name(name), file(file), graphic(NULL), tileWidth(tileWidth), tileHeight(tileHeight), customPatch(customPatch), theme(theme)
	{
		this->flags = new unsigned short[this->tileWidth * this->tileHeight];
		memcpy(this->flags, flags, this->tileWidth * this->tileHeight * sizeof(unsigned short));
	}

	/**
	**  Patch type destructor
	*/
	~CPatchType()
	{
		CGraphic::Free(this->graphic);
		delete[] this->flags;
	}

	/**
	**  Load the patch type
	*/
	void load()
	{
		if (!this->graphic) {
			this->graphic = CGraphic::New(this->file);
			this->graphic->Load();
		}
	}

	/**
	**  Clean the allocated memory
	*/
	void clean()
	{
		CGraphic::Free(this->graphic);
		this->graphic = NULL;
	}

	/**
	**  Get the name
	*/
	inline const std::string &getName() const { return this->name; }

	/**
	**  Get the file name
	*/
	inline const std::string &getFile() const { return this->file; }

	/**
	**  Get the graphic
	*/
	inline const CGraphic *getGraphic() const { return this->graphic; }

	/**
	**  Get the tile width of the patch
	*/
	inline int getTileWidth() const { return this->tileWidth; }

	/**
	**  Get the tile height of the patch
	*/
	inline int getTileHeight() const { return this->tileHeight; }

	/**
	**  Get the tile flag at a tile location
	*/
	unsigned short getFlag(int x, int y) const
	{
		Assert(0 <= x && x < this->tileWidth && 0 <= y && y < this->tileHeight);
		return flags[y * this->tileWidth + x];
	}

	/**
	**  Set the tile flag at a tile location
	*/
	void setFlag(int x, int y, unsigned short flag)
	{
		Assert(0 <= x && x < this->tileWidth && 0 <= y && y < this->tileHeight);
		flags[y * this->tileWidth + x] = flag;
	}

	/**
	**  Get all of the tile flags
	*/
	inline unsigned short *getFlags()
	{
		return this->flags;
	}

	/**
	**  Set custom patch
	*/
	inline void setCustomPatch(bool customPatch)
	{
		this->customPatch = customPatch;
	}

	/**
	**  Is this a custom patch
	*/
	inline bool isCustomPatch()
	{
		return this->customPatch;
	}

	/**
	**  Get the theme
	*/
	inline const std::string &getTheme() const { return this->theme; }

private:
	std::string name;
	std::string file;
	CGraphic *graphic;
	int tileWidth;
	int tileHeight;
	unsigned short *flags;
	bool customPatch;
	std::string theme;
};

//@}

#endif /* _PATCH_TYPE_H_ */

