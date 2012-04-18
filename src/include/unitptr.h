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
/**@name unitptr.h - The unitptr headerfile. */
//
//      (c) Copyright 2012 by Joris Dauphin
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

#ifndef UNIT_PTR_H
#define UNIT_PTR_H

//@{

class CUnit;


/**
**  Class to ease the ref counting of each CUnit instance.
*/
class CUnitPtr
{
public:
	CUnitPtr() : unit(NULL) {}
	CUnitPtr(CUnit *u);
	CUnitPtr(const CUnitPtr &u);
	~CUnitPtr() { Reset(); }

	void Reset();

	operator CUnit *() { return unit; }
	operator CUnit *() const { return unit; }

	CUnit &operator*() { return *unit; }
	CUnit *operator->() const { return unit; }

	CUnitPtr &operator= (CUnit *u);

	bool operator== (CUnit *u) const { return this->unit == u; }
	bool operator!= (CUnit *u) const { return this->unit != u; }

private:
	CUnit *unit;
};

//@}

#endif // UNIT_PTR_H
