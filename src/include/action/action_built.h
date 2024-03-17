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
/**@name actions_built.h - The actions headerfile. */
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

#ifndef __ACTION_BUILT_H__
#define __ACTION_BUILT_H__

#include "actions.h"

//@{

class COrder_Built : public COrder
{
	friend std::unique_ptr<COrder> COrder::NewActionBuilt(CUnit &builder, CUnit &unit);
public:
	COrder_Built() : COrder(UnitAction::Built) {}
	~COrder_Built();

	std::unique_ptr<COrder> Clone() const override { return std::make_unique<COrder_Built>(*this); }

	bool IsValid() const override;

	void Save(CFile &file, const CUnit &unit) const override;
	bool
	ParseSpecificData(lua_State *l, int &j, std::string_view value, const CUnit &unit) override;

	void Execute(CUnit &unit) override;
	void Cancel(CUnit &unit) override;
	PixelPos Show(const CViewport &vp, const PixelPos &lastScreenPos) const override;
	void UpdatePathFinderData(PathFinderInput &input) override
	{
		UpdatePathFinderData_NotCalled(input);
	}

	void UpdateUnitVariables(CUnit &unit) const override;
	void FillSeenValues(CUnit &unit) const override;
	void AiUnitKilled(CUnit &unit) override;

	void Progress(CUnit &unit, int amount);
	void ProgressHp(CUnit &unit, int amount);

	std::size_t GetFrameIndex() const { return Frame; }
	const CUnitPtr &GetWorker() const { return Worker; }
	CUnit *GetWorkerPtr() { return Worker; }

private:
	void Boost(CUnit &building, int amount, int varIndex) const;
	void UpdateConstructionFrame(CUnit &unit);

private:
	CUnitPtr Worker = nullptr; /// Worker building this unit
	int ProgressCounter = 0;   /// Progress counter, in 1/100 cycles.
	bool IsCancelled = false;  /// Cancel construction
	std::size_t Frame = -1;    /// Construction frame
};

//@}

#endif // !__ACTION_BUILT_H__
