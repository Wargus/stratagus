//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name patch.h - The patch header. */
//
//      (c) Copyright 2008 by Jimmy Salmon
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

#ifndef _PATCH_H_
#define _PATCH_H_

//@{

class CPatchType;


class CPatch
{
public:
	/**
	**  Patch constructor
	*/
	CPatch(CPatchType *type, int x, int y) :
		type(type), x(x), y(y)
	{
	}

	/**
	**  Patch destructor
	*/
	~CPatch()
	{
	}

	/**
	**  Get the patch type
	*/
	inline CPatchType *getType() const { return this->type; }

	/**
	**  Set the position of the patch
	*/
	inline void setPos(int x, int y) { this->x = x; this->y = y; }

	/**
	**  Set the X position of the patch
	*/
	inline void setX(int x) { this->x = x; }
	/**
	**  Get the X position of the patch
	*/
	inline int getX() const { return this->x; }

	/**
	**  Set the Y position of the patch
	*/
	inline void setY(int y) { this->y = y; }
	/**
	**  Get the Y position of the patch
	*/
	inline int getY() const { return this->y; }

private:
	CPatchType *type;
	int x;
	int y;
};

//@}

#endif /* _PATCH_H_ */

