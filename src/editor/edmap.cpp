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
/**@name edmap.cpp - Editor map functions. */
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

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include <chrono>
#include <random>
#include <cmath>

#include "stratagus.h"
#include "editor.h"
#include "map.h"
#include "tileset.h"
#include "ui.h"
#include "player.h"
#include "unit.h"
#include "unit_manager.h"
#include "unittype.h"


/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/


/**
**  Get quad from tile.
**
**  A quad is a 32 bit value defining the content of the tile.
**
**  A tile is split into 4 parts, the basic tile type of this part
**    is stored as 8bit value in the quad.
**
**  ab
**  cd -> abcd
*/
static uint32_t QuadFromTile(const Vec2i &pos)
{
	const tile_index idx = Map.Field(pos)->getTileIndex();
	return Map.Tileset.getQuadFromTile(idx);
}

std::optional<tile_index> CTileIconsSet::getTile(size_t iconNo) const
{
	if (iconNo < icons.size()) {
		return icons[iconNo];
	}
	return std::nullopt;
}

std::optional<tile_index> CTileIconsSet::getSelectedTile() const
{
	if (isSelected()) {
		return getTile(selected);
	}
	return std::nullopt;
}

void CTileIconsSet::rebuild(bool manualMode /* = false */, bool firstOfKindOnly /* = true */)
{
	resetSelected();
	if (manualMode) {
		icons = firstOfKindOnly ? Map.Tileset.queryFirstOfItsKindTiles()
								: Map.Tileset.queryAllTiles();
	} else {
		/// FIXME: The extended tileset can be edited only in manual mode _yet_,
		/// so only the icons of the basic tileset should be left in the icon palette.
		icons.clear();
		for (auto icon : Map.Tileset.querySolidTiles()) {
			if (icon < ExtendedTilesetBeginIdx) {
				icons.push_back(icon);
			}
		}
	}
	updateSliderCtrl();
	recalcDisplayed();
}

void CTileIconsSet::updateSliderCtrl()
{
	if (!sliderCtrl) {
		return;
	}
	/// TODO: Add page|line scrolling for horizontal|vertical slider
	sliderCtrl->setScale(0, icons.size() - displayedNum + 1);
	sliderCtrl->setStepLength(displayedNum);
	sliderCtrl->setValue(0);
}

void CTileIconsSet::recalcDisplayed()
{
	if (!sliderCtrl || !displayedNum) {
		displayFrom(0);
		return;
	}
	/// TODO: Add page|line scrolling for horizontal|vertical slider
	displayFrom(sliderCtrl->getValue());
};

void CTileIconsSet::setDisplayedNum(uint16_t number)
{
	displayedNum = number;
	updateSliderCtrl();
	recalcDisplayed();
}

/**
**  Set tile.
**
**  @param pos   map tile coordinate.
**  @param tileIdx  Tile type to edit
*/
void CEditor::SetTile(const Vec2i &pos, tile_index tileIdx)
{
 	Assert(Map.Info.IsPointOnMap(pos));

	CMapField &mf = *Map.Field(pos);

	mf.setTileIndex(Map.Tileset, tileIdx, 0, mf.getElevation());
	mf.playerInfo.SeenTile = mf.getGraphicTile();

	UI.Minimap.UpdateSeenXY(pos);
	UI.Minimap.UpdateXY(pos);

	UpdateMinimap = true;
}

/**
**  Apply brush
**
**  @param pos   map tile coordinate.
*/
void CEditor::applyCurentBrush(const Vec2i &pos)
{
	auto editTile = [this, &pos](const TilePos &tileOffset,
								 tile_index tileIdx,
								 bool fixNeighbors,
								 bool decorative) -> void {
		const TilePos tilePos(pos + tileOffset);
		if (tilePos.x < 0
			|| tilePos.x >= Map.Info.MapWidth
			|| tilePos.y < 0
			|| tilePos.y >= Map.Info.MapHeight) {
			return;
		}
		SetTile(tilePos, tileIdx);
		if (Map.Info.IsHighgroundsEnabled()) {
			Map.Field(tilePos)->setElevation(SelectedElevationLevel);
		}
		CMapField &mapField = *Map.Field(tilePos);
		if (decorative) {
			mapField.setFlag(MapFieldNonMixing);
		} else {
			if (mapField.isFlag(MapFieldNonMixing)) {
				mapField.resetFlag(MapFieldNonMixing);
			}
			if (fixNeighbors) {
				ChangeSurrounding(tilePos, tilePos);
			}
		}
	};

	const auto &brush = brushes.getCurrentBrush();
	brush.applyAt(pos, editTile);

	if (!MirrorEdit) {
		return;
	}

	TilePos maxPos(Map.Info.MapWidth - 1, Map.Info.MapHeight - 1);

	if (brush.getAlign() == CBrush::EBrushAlign::UpperLeft) {
		maxPos.x -= brush.getWidth() - 1;
		maxPos.y -= brush.getHeight() - 1;
	}
	const TilePos mirror = maxPos - pos;
	const TilePos mirrorv(mirror.x, pos.y);

	brush.applyAt(mirrorv, editTile);

	if (MirrorEdit == 1) {
		return;
	}
	const TilePos mirrorh(pos.x, mirror.y);

	brush.applyAt(mirrorh, editTile);
	brush.applyAt(mirror, editTile);
}

/**
**  Change tile.
**
**  @param pos   map tile coordinate.
**  @param tileIndex  Tile type to edit.
**  @param lock_pos   map tile coordinate, that should not be changed in callback.
**  @param changeSurroundings   fix the neighbors tiles.
**  @param randomizeTile   do the tile randomization (get random tile of the same kind).
*/
void CEditor::ChangeTile(const Vec2i &pos,
						 tile_index tileIndex,
						 const Vec2i &lock_pos,
						 bool changeSurroundings,
						 bool randomizeTile)
{
	const tile_index tileIdx = randomizeTile ? Map.Tileset.getRandomTileOfTheSameKindAs(tileIndex)
											 : tileIndex;
	SetTile(pos, tileIdx);
	if (Map.Info.IsHighgroundsEnabled()) {
		Map.Field(pos)->setElevation(SelectedElevationLevel);
	}
	// Change the flags
	if (changeSurroundings
		&& !Map.Field(pos)->isFlag(MapFieldDecorative | MapFieldNonMixing)) {

		ChangeSurrounding(pos, lock_pos);
	}
}

/**
**  Update surroundings for tile changes.
**
**  @param pos  Map tile position of change.
**  @param lock_pos  Original change position, that should not be altered.
*/
void CEditor::ChangeSurrounding(const Vec2i &pos, const Vec2i &lock_pos)
{
	CMapField &mf = *Map.Field(pos);

	if (mf.isFlag(MapFieldDecorative | MapFieldNonMixing)) {
		return;
	}

	// Special case 1) Walls.
	if (mf.isAWall()) {
		Map.SetWall(pos, mf.isHuman());
		return;
	}

	const unsigned int quad = QuadFromTile(pos);
	const unsigned int TH_QUAD_M = 0xFFFF0000; // Top half quad mask
	const unsigned int BH_QUAD_M = 0x0000FFFF; // Bottom half quad mask
	const unsigned int LH_QUAD_M = 0xFF00FF00; // Left half quad mask
	const unsigned int RH_QUAD_M = 0x00FF00FF; // Right half quad mask

	bool did_change = false; // if we changed any tiles

	// How this works:
	//  first get the quad of the neighbouring tile,
	//  then check if the margin matches.
	//  Otherwise, call ChangeTile again.
	if (pos.y) {
		const Vec2i offset(0, -1);
		// Insert into the bottom the new tile.
		CMapField *f = Map.Field(pos + offset);
		if (f->isAWall()) {
			ChangeSurrounding(pos + offset, pos);
		} else if (!f->isFlag(MapFieldDecorative | MapFieldNonMixing)) {
			unsigned q2 = QuadFromTile(pos + offset);
			unsigned u = (q2 & TH_QUAD_M) | ((quad >> 16) & BH_QUAD_M);
			if (u != q2 && (pos + offset) != lock_pos) {
				int tile = Map.Tileset.tileFromQuad(u & BH_QUAD_M, u);
				if (tile) {
					did_change = true;
					ChangeTile(pos + offset, tile, lock_pos, true, true);
				}
			}
		}
	}
	if (pos.y < Map.Info.MapHeight - 1) {
		const Vec2i offset(0, 1);
		// Insert into the top the new tile.
		CMapField *f = Map.Field(pos + offset);
		if (f->isAWall()) {
			ChangeSurrounding(pos + offset, pos);
		} else if (!f->isFlag(MapFieldDecorative | MapFieldNonMixing)) {
			unsigned q2 = QuadFromTile(pos + offset);
			unsigned u = (q2 & BH_QUAD_M) | ((quad << 16) & TH_QUAD_M);
			if (u != q2 && (pos + offset) != lock_pos) {
				int tile = Map.Tileset.tileFromQuad(u & TH_QUAD_M, u);
				if (tile) {
					did_change = true;
					ChangeTile(pos + offset, tile, lock_pos, true, true);
				}
			}
		}
	}
	if (pos.x) {
		const Vec2i offset(-1, 0);
		// Insert into the left the new tile.
		CMapField *f = Map.Field(pos + offset);
		if (f->isAWall()) {
			ChangeSurrounding(pos + offset, pos);
		} else if (!f->isFlag(MapFieldDecorative | MapFieldNonMixing)) {
			unsigned q2 = QuadFromTile(pos + offset);
			unsigned u = (q2 & LH_QUAD_M) | ((quad >> 8) & RH_QUAD_M);
			if (u != q2 && (pos + offset) != lock_pos) {
				int tile = Map.Tileset.tileFromQuad(u & RH_QUAD_M, u);
				if (tile) {
					did_change = true;
					ChangeTile(pos + offset, tile, lock_pos, true, true);
				}
			}
		}
	}
	if (pos.x < Map.Info.MapWidth - 1) {
		const Vec2i offset(1, 0);
		// Insert into the right the new tile.
		CMapField *f = Map.Field(pos + offset);
		if (f->isAWall()) {
			ChangeSurrounding(pos + offset, pos);
		} else if (!f->isFlag(MapFieldDecorative | MapFieldNonMixing)) {
			unsigned q2 = QuadFromTile(pos + offset);
			unsigned u = (q2 & RH_QUAD_M) | ((quad << 8) & LH_QUAD_M);
			if (u != q2 && (pos + offset) != lock_pos) {
				int tile = Map.Tileset.tileFromQuad(u & LH_QUAD_M, u);
				if (tile) {
					did_change = true;
					ChangeTile(pos + offset, tile, lock_pos, true, true);
				}
			}
		}
	}

	if (did_change) {
		ChangeSurrounding(pos, lock_pos);
	}
}

/**
**  Make random map
**  FIXME: vladi: we should have parameters control here...
*/

/**
**  TileFill
**
**  @param pos   map tile coordinate for area center.
**  @param tile  Tile type to edit.
**  @param size  Size of surrounding rectangle.
**
**  TileFill(centerx, centery, tile_type_water, map_width)
**  will fill map with water...
*/
void CEditor::TileFill(const Vec2i &pos, int tile, int size)
{
	const Vec2i diag(size / 2, size / 2);
	Vec2i ipos = pos - diag;
	Vec2i apos = pos + diag;

	Map.FixSelectionArea(ipos, apos);

	// change surroundings unless the fill covers the entire map
	bool changeSurroundings = (ipos.x > 0 || ipos.y > 0 || Map.Info.MapWidth - 1 > apos.x
							   || Map.Info.MapHeight - 1 > apos.y);

	Vec2i itPos;
	for (itPos.x = ipos.x; itPos.x <= apos.x; ++itPos.x) {
		for (itPos.y = ipos.y; itPos.y <= apos.y; ++itPos.y) {
			ChangeTile(itPos, tile, itPos, changeSurroundings, true);
		}
	}
}

static std::mt19937 MersenneTwister(std::chrono::steady_clock::now().time_since_epoch().count());
static int rng() {
	return std::abs((int)MersenneTwister());
}

/**
**  Randomize tiles and fill in map
**
**  @param tile      tile number to use
**  @param count     number of times to apply randomization
**  @param max_size  maximum size of the fill rectangle
*/
void CEditor::RandomizeTile(int tile, int count, int max_size)
{
	const Vec2i mpos(Map.Info.MapWidth - 1, Map.Info.MapHeight - 1);

	for (int i = 0; i < count; ++i) {
		const Vec2i rpos(rng() % ((1 + mpos.x) / 2), rng() % ((1 + mpos.y) / 2));
		const Vec2i mirror = mpos - rpos;
		const Vec2i mirrorh(rpos.x, mirror.y);
		const Vec2i mirrorv(mirror.x, rpos.y);
		const int rz = rng() % max_size + 1;

		TileFill(rpos, tile, rz);
		TileFill(mirrorh, tile, rz);
		TileFill(mirrorv, tile, rz);
		TileFill(mirror, tile, rz);
	}
}

/**
**  Add a unit to random locations on the map, unit will be neutral
**
**  @param unit_type  unit type to add to map as a character string
**  @param count      the number of times to add the unit
**  @param value      resources to be stored in that unit
*/
void CEditor::RandomizeUnit(const std::string_view unit_type, int count, int value, int tileIndexUnderUnit)
{
	const Vec2i mpos(Map.Info.MapWidth, Map.Info.MapHeight);
	CUnitType &type = UnitTypeByIdent(unit_type);
	const Vec2i tpos(type.TileWidth, type.TileHeight);

	for (int i = 0; i < count; ++i) {
		const Vec2i rpos(rng() % (mpos.x / 2 - tpos.x + 1), rng() % (mpos.y / 2 - tpos.y + 1));
		const Vec2i mirror(mpos.x - rpos.x - 1, mpos.y - rpos.y - 1);
		const Vec2i mirrorh(rpos.x, mirror.y);
		const Vec2i mirrorv(mirror.x, rpos.y);
		const Vec2i tmirror(mpos.x - rpos.x - tpos.x, mpos.y - rpos.y - tpos.y);
		const Vec2i tmirrorh(rpos.x, tmirror.y);
		const Vec2i tmirrorv(tmirror.x, rpos.y);
		int tile = tileIndexUnderUnit;
		const int z = type.TileHeight;

		// FIXME: vladi: the idea is simple: make proper land for unit(s) :)
		// FIXME: handle units larger than 1 square
		TileFill(rpos, tile, z * 2);
		TileFill(mirrorh, tile, z * 2);
		TileFill(mirrorv, tile, z * 2);
		TileFill(mirror, tile, z * 2);

		// FIXME: can overlap units
		CUnit *unit = MakeUnitAndPlace(rpos, type, &Players[PlayerNumNeutral]);
		if (unit == nullptr) {
			DebugPrint("Unable to allocate Unit");
		} else {
			unit->ResourcesHeld = value;
		}

		unit = MakeUnitAndPlace(tmirrorh, type, &Players[PlayerNumNeutral]);
		if (unit == nullptr) {
			DebugPrint("Unable to allocate Unit");
		} else {
			unit->ResourcesHeld = value;
		}

		unit = MakeUnitAndPlace(tmirrorv, type, &Players[PlayerNumNeutral]);
		if (unit == nullptr) {
			DebugPrint("Unable to allocate Unit");
		} else {
			unit->ResourcesHeld = value;
		}

		unit = MakeUnitAndPlace(tmirror, type, &Players[PlayerNumNeutral]);
		if (unit == nullptr) {
			DebugPrint("Unable to allocate Unit");
		} else {
			unit->ResourcesHeld = value;
		}
	}
}

/**
**  Destroy all units
*/
static void EditorDestroyAllUnits()
{
	while (UnitManager->empty() == false) {
		CUnit &unit = **UnitManager->GetUnits().begin();

		unit.Remove(nullptr);
		UnitLost(unit);
		UnitClearOrders(unit);
		unit.Release();
	}
}

void CEditor::RandomizeTransition(int x, int y)
{
	CMapField &mf = *Map.Field(x, y);
	const CTileset &tileset = Map.Tileset;
	terrain_typeIdx baseTileIndex = tileset.tiles[mf.getTileIndex()].tileinfo.BaseTerrain;
	terrain_typeIdx mixTerrainIdx = tileset.tiles[mf.getTileIndex()].tileinfo.MixTerrain;
	if (mixTerrainIdx != 0) {
		if (rng() % 8 == 0) {
			// change only in ~12% of cases
			const tile_index tileIdx = tileset.findTileIndex(rng() % 2 ? baseTileIndex : mixTerrainIdx, 0);
			ChangeTile(Vec2i(x, y), tileIdx, Vec2i(x, y), true, true);
		}
	}
}

/**
**  Create a random map
*/
void CEditor::CreateRandomMap(bool shuffleTranslitions)
{
	const int mz = std::max(Map.Info.MapHeight, Map.Info.MapWidth);

	// remove all units
	EditorDestroyAllUnits();
	// make water-base
	const Vec2i zeros(0, 0);
	TileFill(zeros, BaseTileIndex, mz * 3);
	UI.Minimap.Update();
	EditorUpdateDisplay();


	Editor.BuildingRandomMap = true;
	for (std::tuple<int, int, int> t : RandomTiles) {
		RandomizeTile(std::get<0>(t), mz / 64 * std::get<1>(t), std::get<2>(t));
		UI.Minimap.Update();
		EditorUpdateDisplay();
	}

	if (shuffleTranslitions) {
		// shuffle transitions in all directions
		// from top left to bottom right
		for (int x = 0; x < Map.Info.MapWidth; x++) {
			for (int y = 0; y < Map.Info.MapHeight; y++) {
				RandomizeTransition(x, y);
			}
		}
		UI.Minimap.Update();
		EditorUpdateDisplay();
		// from bottom right to top left
		for (int x = Map.Info.MapWidth - 1; x >= 0; x--) {
			for (int y = Map.Info.MapHeight - 1; y >= 0; y--) {
				RandomizeTransition(x, y);
			}
		}
		UI.Minimap.Update();
		EditorUpdateDisplay();
	}

	Editor.BuildingRandomMap = false;

	for (std::tuple<std::string, int, int, int> t : RandomUnits) {
		RandomizeUnit(std::get<0>(t), mz / 64 * std::get<1>(t), std::get<2>(t), std::get<3>(t));
		UI.Minimap.Update();
		EditorUpdateDisplay();
	}
}

//@}
