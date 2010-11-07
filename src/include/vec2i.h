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
/**@name vec2i.h - Vec2i headerfile. */
//
//      (c) Copyright 2010 by Joris Dauphin
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

#ifndef __VEC2I_H__
#define __VEC2I_H__

//@{

class Vec2i
{
public:
	short int x;
	short int y;
};


inline bool operator == (const Vec2i &lhs, const Vec2i & rhs)
{
	return lhs.x == rhs.x && lhs.y == rhs.y;
}

inline bool operator != (const Vec2i &lhs, const Vec2i & rhs)
{
	return !(lhs == rhs);
}

inline const Vec2i& operator += (Vec2i &lhs, const Vec2i& rhs)
{
	lhs.x += rhs.x;
	lhs.y += rhs.y;
	return lhs;
}

inline const Vec2i& operator -= (Vec2i &lhs, const Vec2i& rhs)
{
	lhs.x -= rhs.x;
	lhs.y -= rhs.y;
	return lhs;
}

inline const Vec2i& operator *= (Vec2i &lhs, int rhs)
{
	lhs.x *= rhs;
	lhs.y *= rhs;
	return lhs;
}

inline const Vec2i& operator /= (Vec2i &lhs, int rhs)
{
	lhs.x /= rhs;
	lhs.y /= rhs;
	return lhs;
}

inline Vec2i operator + (const Vec2i &lhs, const Vec2i& rhs)
{
	Vec2i res(lhs);

	res += rhs;
	return res;
}

inline Vec2i operator - (const Vec2i &lhs, const Vec2i& rhs)
{
	Vec2i res(lhs);

	res -= rhs;
	return res;
}

inline Vec2i operator * (const Vec2i &lhs, int rhs)
{
	Vec2i res(lhs);

	res *= rhs;
	return res;
}

inline Vec2i operator * (int lhs, const Vec2i &rhs)
{
	Vec2i res(rhs);

	res *= lhs;
	return res;
}


inline Vec2i operator / (const Vec2i &lhs, int rhs)
{
	Vec2i res(lhs);

	res /= rhs;
	return res;
}


//@}

#endif // !__VEC2I_H__
