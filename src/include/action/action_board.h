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
/**@name action_board.h - The actions headerfile. */
//
//      (c) Copyright 1998-2012 by Lutz Sammer and Jimmy Salmon
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

#ifndef __ACTION_BOARD_H__
#define __ACTION_BOARD_H__

#include "actions.h"

//@{

class COrder_Board : public COrder
{
	friend std::unique_ptr<COrder> COrder::NewActionBoard(CUnit &unit);
public:
	COrder_Board() : COrder(UnitAction::Board) {}

	std::unique_ptr<COrder> Clone() const override { return std::make_unique<COrder_Board>(*this); }

	bool IsValid() const override;

	void Save(CFile &file, const CUnit &unit) const override;
	bool ParseSpecificData(lua_State *l, int &j, std::string_view value, const CUnit &unit) override;

	void Execute(CUnit &unit) override;
	PixelPos Show(const CViewport &vp, const PixelPos &lastScreenPos) const override;
	void UpdatePathFinderData(PathFinderInput &input) override;
	const Vec2i GetGoalPos() const override;

private:
	bool WaitForTransporter(CUnit &unit);
	int MoveToTransporter(CUnit &unit);
private:
	int State = 0;
	int Range = 0;
	Vec2i goalPos{-1, -1};
};


//@}

#endif // !__ACTION_BOARD_H__
