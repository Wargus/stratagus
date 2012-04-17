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
/**@name map_draw.cpp - The map drawing. */
//
//      (c) Copyright 1999-2005 by Lutz Sammer and Jimmy Salmon
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

#include "stratagus.h"
#include "unit.h"
#include "tileset.h"
#include "video.h"
#include "map.h"
#include "player.h"
#include "pathfinder.h"
#include "ui.h"
#include "missile.h"
#include "unittype.h"
#include "font.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Global functions
----------------------------------------------------------------------------*/

#if 1 // for parallell drawing

#include "actions.h"
#include "action/action_built.h"
#include "action/action_upgradeto.h"

extern void DrawConstructionShadow(const CUnitType &type, const CConstructionFrame *cframe,
								   int frame, int x, int y);
extern void DrawConstruction(const int player, const CConstructionFrame *cframe,
							 const CUnitType &type, int frame, int x, int y);
class MissileDrawProxy
{
public:
	void DrawMissile(const CViewport &vp) const;

	void operator=(const Missile* missile);
public:
	const MissileType *Type;  /// missile-type pointer
	union {
		int Damage;  /// direct damage that missile applies
		int SpriteFrame; /// sprite frame counter
	} data;
	PixelPos pixelPos;
};

void MissileDrawProxy::DrawMissile(const CViewport &vp) const
{
	const PixelPos sceenPixelPos = vp.MapToScreenPixelPos(this->pixelPos);

	switch (this->Type->Class) {
		case MissileClassHit:
			CLabel(GetGameFont()).DrawClip(sceenPixelPos.x, sceenPixelPos.y, this->data.Damage);
			break;
		default:
			this->Type->DrawMissileType(this->data.SpriteFrame, sceenPixelPos);
			break;
	}
}

void MissileDrawProxy::operator=(const Missile *missile)
{
	this->Type = missile->Type;
	this->pixelPos = missile->position;
	if (missile->Type->Class == MissileClassHit) {
		this->data.Damage = missile->Damage;
	} else {
		this->data.SpriteFrame = missile->SpriteFrame;
	}
}

int FindAndSortMissiles(const CViewport &vp, MissileDrawProxy table[], const int tablesize)
{
	Assert(tablesize <= MAX_MISSILES * 9);

	Missile *buffer[MAX_MISSILES * 9];
	const int n = FindAndSortMissiles(vp, buffer, MAX_MISSILES * 9);

	for (int i = 0; i < n; ++i) {
		table[i] = buffer[i];
	}
	return n;
}

class CUnitDrawProxy {

	void DrawSelectionAt(int x, int y) const;
	void DrawDecorationAt(int x, int y) const;
public:

	CUnitDrawProxy(): Variable(NULL) {}
	~CUnitDrawProxy() {
		delete[] Variable;
	}

	Vec2i tilePos;
	int frame;
	int TeamSelected; //unit->TeamSelected
	int GroupId; //unit->GroupId

	signed char IX;
	signed char IY;
	unsigned char CurrentResource;

	unsigned int IsAlive:1;
	unsigned int Selected:1; //unit->Selected
	unsigned int ResourcesHeld:1;      /// isResources Held by a unit
	unsigned int state: 2;
	unsigned int Blink: 3; //unit->Blink

	const CConstructionFrame *cframe;
	const CUnitType *Type;
	const CPlayer *Player;

	CVariable *Variable;

	void operator=(const CUnit *unit);
	void Draw(const CViewport *vp) const;
};

void CUnitDrawProxy::operator=(const CUnit *unit)
{
	int action = unit->CurrentAction();
	bool IsVisible = unit->IsVisible(*ThisPlayer);

	IsAlive = action != UnitActionDie;
	Player = unit->RescuedFrom ? unit->RescuedFrom : unit->Player;
	cframe = NULL;
	CurrentResource = unit->CurrentResource;
	Selected = unit->Selected;
	TeamSelected = unit->TeamSelected;
	Blink = unit->Blink;
	ResourcesHeld = unit->ResourcesHeld > 0;

	if (unit->GroupId) {
		if (unit->Player->AiEnabled) {
			GroupId = unit->GroupId;
		} else {
			int num = 0;
			while (!(unit->GroupId & (1 << num))) {
				++num;
			}
			GroupId = num + 1;
		}
	} else {
		GroupId = 0;
	}

	if (unit->Variable) {
		const unsigned int num_dec = UnitTypeVar.DecoVar.size();
		if (!Variable) {
			Variable = new CVariable[num_dec];
		}
		for (unsigned int i = 0; i < num_dec; ++i) {
			Variable[i] = unit->Variable[UnitTypeVar.DecoVar[i]->Index];
		}
	} else {
		if (Variable) {
			const unsigned int num_dec = UnitTypeVar.DecoVar.size();
			for (unsigned int i = 0; i < num_dec; ++i) {
				Variable[i] = Type->Variable[UnitTypeVar.DecoVar[i]->Index];
			}
		}
	}

	if (ReplayRevealMap || IsVisible) {
		Type = unit->Type;
		frame = unit->Frame;
		IY = unit->IY;
		IX = unit->IX;
		tilePos = unit->tilePos;

		state = (action == UnitActionBuilt) |
				((action == UnitActionUpgradeTo) << 1);

		// Reset Type to the type being upgraded to
		if (action == UnitActionUpgradeTo) {
			const COrder_UpgradeTo &order = *static_cast<COrder_UpgradeTo *>(unit->CurrentOrder());

			Type = const_cast<CUnitType *>(&order.GetUnitType());
		}

		if (unit->Constructed) {
			if (unit->CurrentAction() == UnitActionBuilt) {
				COrder_Built &order = *static_cast<COrder_Built *>(unit->CurrentOrder());

				cframe = &order.GetFrame();
			} else {
				cframe = NULL;
			}
		}
	} else {
		IY = unit->Seen.IY;
		IX = unit->Seen.IX;
		tilePos = unit->Seen.tilePos;
		frame = unit->Seen.Frame;
		Type = unit->Seen.Type;
		state = unit->Seen.State;

		if (unit->Seen.Constructed) {
			cframe = unit->Seen.CFrame;
		}
	}

#ifdef DYNAMIC_LOAD
	if (!type->Sprite) {
		LoadUnitTypeSprite(type);
	}
#endif

}

/**
**  Draw decoration (invis, for the unit.)
**
**  @param unit  Pointer to the unit.
**  @param type  Type of the unit.
**  @param x     Screen X position of the unit.
**  @param y     Screen Y position of the unit.
*/
void CUnitDrawProxy::DrawDecorationAt(int x, int y) const
{

	// Now show decoration for each variable.
	if (Variable) {
		const unsigned int num_dec = UnitTypeVar.DecoVar.size();
		for (unsigned int i = 0; i < num_dec; ++i) {
			const CDecoVar *var = UnitTypeVar.DecoVar[i];
			int value = Variable[i].Value;
			int max = Variable[i].Max;
			Assert(value <= max);

			if (!((value == 0 && !var->ShowWhenNull) || (value == max && !var->ShowWhenMax)
				|| (var->HideHalf && value != 0 && value != max)
				//|| (!var->ShowIfNotEnable && !Variable[var->Index].Enable) ||
				|| (!var->ShowIfNotEnable && !Variable[i].Enable)
				|| (var->ShowOnlySelected && !Selected)
				|| (Player != ThisPlayer && ((Player->Type == PlayerNeutral && var->HideNeutral)
											|| (ThisPlayer->IsEnemy(*Player) && !var->ShowOpponent)
											|| (ThisPlayer->IsAllied(*Player) && var->HideAllied)))
				|| max == 0)) {
				var->Draw(
					x + var->OffsetX + var->OffsetXPercent * Type->TileWidth * PixelTileSize.x / 100,
					y + var->OffsetY + var->OffsetYPercent * Type->TileHeight * PixelTileSize.y / 100,
					Type, Variable[i]);
			}
		}
	}

	//
	// Draw group number
	//
	if (Selected && GroupId != 0
#ifndef DEBUG
		&& Player == ThisPlayer
#endif
	   ) {
		int width = GetGameFont()->Width(GroupId - 1);
		x += (Type->TileWidth * PixelTileSize.x + Type->BoxWidth) / 2 - width;
		width = GetGameFont()->Height();
		y += (Type->TileHeight * PixelTileSize.y + Type->BoxHeight) / 2 - width;
		CLabel(GetGameFont()).DrawClip(x, y, GroupId - 1);
	}
}

void CUnitDrawProxy::DrawSelectionAt(int x, int y) const
{
	Uint32 color;

	// FIXME: make these colors customizable with scripts.
	if (Selected || TeamSelected || (Blink & 1)) {
		if (Player->Index == PlayerNumNeutral) {
			color = ColorYellow;
		} else if ((Selected || (Blink & 1))
					&& (Player == ThisPlayer || ThisPlayer->IsTeamed(*Player))) {
			color = ColorGreen;
		} else if (ThisPlayer->IsEnemy(*Player)) {
			color = ColorRed;
		} else {
			int i;

			for (i = 0; i < PlayerMax; ++i) {
				if (TeamSelected & (1 << i)) {
					break;
				}
			}
			if (i == PlayerMax) {
				color = Player->Color;
			} else {
				color = Players[i].Color;
			}
		}
	} else if (CursorBuilding && Type->Building &&	IsAlive
				&& (Player == ThisPlayer || ThisPlayer->IsTeamed(*Player))) {
		// If building mark all own buildings
		color = ColorGray;
	} else {
		return;
	}

	int xx = x + Type->TileWidth * PixelTileSize.x / 2 - Type->BoxWidth / 2 -
			 (Type->Width - Type->Sprite->Width) / 2;
	int yy = y + Type->TileHeight * PixelTileSize.y / 2 - Type->BoxHeight / 2 -
			 (Type->Height - Type->Sprite->Height) / 2;

	DrawSelection(color, xx, yy, xx + Type->BoxWidth, yy + Type->BoxHeight);
}

void CUnitDrawProxy::Draw(const CViewport *vp) const
{
	const PixelPos screenPos = vp->TilePosToScreen_TopLeft(this->tilePos);
	const int x = screenPos.x + this->IX;
	const int y = screenPos.y + this->IY;

	/* FIXME: check if we have to push real type here?*/
	if (state == 1 && cframe) {
		DrawConstructionShadow(*Type, cframe, frame, x, y);
	} else {
		if (IsAlive) {
			DrawShadow(*Type, frame, x, y);
		}
	}

	//
	// Show that the unit is selected
	//
	DrawSelectionAt(x, y);

	//
	// Adjust sprite for Harvesters.
	//
	CPlayerColorGraphic *sprite = Type->Sprite;
	if (Type->Harvester && this->CurrentResource) {
		ResourceInfo *resinfo = Type->ResInfo[this->CurrentResource];
		if (this->ResourcesHeld) {
			if (resinfo->SpriteWhenLoaded) {
				sprite = resinfo->SpriteWhenLoaded;
			}
		} else {
			if (resinfo->SpriteWhenEmpty) {
				sprite = resinfo->SpriteWhenEmpty;
			}
		}
	}

	//
	// Now draw!
	// Buildings under construction/upgrade/ready.
	//
	if (state == 1) {
		if (cframe) {
			DrawConstruction(Player->Index, cframe, *Type, frame,
							 x + (Type->TileWidth * PixelTileSize.x) / 2,
							 y + (Type->TileHeight * PixelTileSize.y) / 2);
		}
		//
		// Draw the future unit type, if upgrading to it.
		//
	} else if (state == 2) {
		// FIXME: this frame is hardcoded!!!
		DrawUnitType(*Type, sprite, Player->Index, frame < 0 ? /*-1*/ - 1 : 1, x, y);
	} else {
		DrawUnitType(*Type, sprite, Player->Index, frame, x, y);
	}

	// Unit's extras not fully supported.. need to be decorations themselves.
	// FIXME: johns: ugly check here, should be removed!
	if (IsAlive /*unit->IsVisible(ThisPlayer)*/) {
		DrawDecorationAt(x, y);
	}

	//DrawInformations(this, type, x, y);
}

/**
**  Compare what order 2 units should be drawn on the map
**
**  @param c1  First Unit to compare (*Unit)
**  @param c2  Second Unit to compare (*Unit)
**
*/
static inline bool DrawLevelCompare(const CUnit *c1, const CUnit *c2)
{
	int drawlevel1 = c1->GetDrawLevel();
	int drawlevel2 = c2->GetDrawLevel();

	if (drawlevel1 == drawlevel2) {
		// diffpos compares unit's Y positions (bottom of sprite) on the map
		// and uses X position in case Y positions are equal.
		// FIXME: Use BoxHeight?
		const int pos1 = (c1->tilePos.y * PixelTileSize.y + c1->IY + c1->Type->Height);
		const int pos2 = (c2->tilePos.y * PixelTileSize.y + c2->IY + c2->Type->Height);
		return pos1 == pos2 ?
			   (c1->tilePos.x - c2->tilePos.x ? c1->tilePos.x < c2->tilePos.x : c1->Slot < c2->Slot) : pos1 < pos2;
	} else {
		return drawlevel1 < drawlevel2;
	}
}

int FindAndSortUnits(const CViewport *vp, CUnitDrawProxy table[])
{
	//  Select all units touching the viewpoint.
	const Vec2i minPos = {vp->MapX - 1, vp->MapY - 1};
	const Vec2i maxPos = {vp->MapX + vp->MapWidth + 1, vp->MapY + vp->MapHeight + 1};
	std::vector<CUnit *> buffer;

	Map.Select(minPos, maxPos, buffer);
	int n = static_cast<int>(buffer.size());

	for (int i = 0; i < n; ++i) {
		if (!buffer[i]->IsVisibleInViewport(vp)
			|| buffer[i]->Destroyed || buffer[i]->Container
			|| buffer[i]->Type->Revealer) {
			buffer[i--] = buffer[--n];
			buffer.pop_back();
		}
	}

	if (n > 1) {
		std::sort(buffer.begin(), buffer.begin() + n, DrawLevelCompare);
		for (int i = 0; i < n; ++i) {
			UpdateUnitVariables(*buffer[i]);
			table[i] = buffer[i];
		}
	} else if (n == 1) {
		UpdateUnitVariables(*buffer[0]);
		table[0] = buffer[0];
	}

	return n;
}

class CDrawProxy
{
public:
	CDrawProxy() : nunits(0), nmissiles(0) {}

	void Update(const CViewport &vp) {
		// We find and sort units after draw level.
		if (lock.TryLock()) {
			nunits = FindAndSortUnits(&vp, unittable);
			nmissiles = FindAndSortMissiles(vp, missiletable, MAX_MISSILES * 9);
			lock.UnLock();
		}
	}

	void Draw(const CViewport &vp) {
		int i = 0, j = 0;
		lock.Lock();
		while (i < nunits && j < nmissiles) {
			if (unittable[i].Type->DrawLevel <= missiletable[j].Type->DrawLevel) {
				unittable[i].Draw(&vp);
				++i;
			} else {
				missiletable[j].DrawMissile(vp);
				++j;
			}
		}
		for (; i < nunits; ++i) {
			unittable[i].Draw(&vp);
		}
		for (; j < nmissiles; ++j) {
			missiletable[j].DrawMissile(vp);
		}
		lock.UnLock();
	}
private:
	CMutex lock;
	CUnitDrawProxy unittable[UnitMax];
	MissileDrawProxy missiletable[MAX_MISSILES * 9];
	int nunits;
	int nmissiles;
};

void CViewport::UpdateParallelProxy()
{
	if (!Proxy) {
		Proxy = new CDrawProxy();
	}
	Proxy->Update(*this);
}
#endif


/**
**  Check if any part of an area is visible in a viewport.
**
**  @param boxmin  map tile position of area in map to be checked.
**  @param boxmax  map tile position of area in map to be checked.
**
**  @return    True if any part of area is visible, false otherwise
*/
bool CViewport::AnyMapAreaVisibleInViewport(const Vec2i &boxmin, const Vec2i &boxmax) const
{
	Assert(boxmin.x <= boxmax.x && boxmin.y <= boxmax.y);

	if (boxmax.x < this->MapX
		|| boxmax.y < this->MapY
		|| boxmin.x >= this->MapX + this->MapWidth
		|| boxmin.y >= this->MapY + this->MapHeight) {
		return false;
	}
	return true;
}

bool CViewport::IsInsideMapArea(const PixelPos &screenPixelPos) const
{
	const Vec2i tilePos = ScreenToTilePos(screenPixelPos);

	return Map.Info.IsPointOnMap(tilePos);
}

// Convert viewport coordinates into map pixel coordinates
PixelPos CViewport::ScreenToMapPixelPos(const PixelPos &screenPixelPos) const
{
	const int x = screenPixelPos.x - this->X + this->MapX * PixelTileSize.x + this->OffsetX;
	const int y = screenPixelPos.y - this->Y + this->MapY * PixelTileSize.y + this->OffsetY;
	const PixelPos mapPixelPos = {x, y};

	return mapPixelPos;
}

// Convert map pixel coordinates into viewport coordinates
PixelPos CViewport::MapToScreenPixelPos(const PixelPos &mapPixelPos) const
{
	PixelPos screenPixelPos = {
		mapPixelPos.x + this->X - (this->MapX * PixelTileSize.x + this->OffsetX),
		mapPixelPos.y + this->Y - (this->MapY * PixelTileSize.y + this->OffsetY)
	};
	return screenPixelPos;
}

/// convert screen coordinate into tilepos
Vec2i CViewport::ScreenToTilePos(const PixelPos &screenPixelPos) const
{
	const PixelPos mapPixelPos = ScreenToMapPixelPos(screenPixelPos);
	const Vec2i tilePos = {mapPixelPos.x / PixelTileSize.x, mapPixelPos.y / PixelTileSize.y};

	return tilePos;
}

/// convert tilepos coordonates into screen (take the top left of the tile)
PixelPos CViewport::TilePosToScreen_TopLeft(const Vec2i &tilePos) const
{
	const PixelPos mapPos = {tilePos.x * PixelTileSize.x, tilePos.y * PixelTileSize.y};

	return MapToScreenPixelPos(mapPos);
}

/// convert tilepos coordonates into screen (take the center of the tile)
PixelPos CViewport::TilePosToScreen_Center(const Vec2i &tilePos) const
{
	const PixelPos topLeft = TilePosToScreen_TopLeft(tilePos);

	return topLeft + PixelTileSize / 2;
}

/**
**  Change viewpoint of map viewport v to tilePos.
**
**  @param tilePos  map tile position.
**  @param offset   offset in tile.
*/
void CViewport::Set(const PixelPos &mapPos)
{
	int x = mapPos.x;
	int y = mapPos.y;

	x = std::max(x, -UI.MapArea.ScrollPaddingLeft);
	y = std::max(y, -UI.MapArea.ScrollPaddingTop);

	x = std::min(x, Map.Info.MapWidth * PixelTileSize.x - (this->EndX - this->X) - 1 + UI.MapArea.ScrollPaddingRight);
	y = std::min(y, Map.Info.MapHeight * PixelTileSize.y - (this->EndY - this->Y) - 1 + UI.MapArea.ScrollPaddingBottom);

	this->MapX = x / PixelTileSize.x;
	if (x < 0 && x % PixelTileSize.x) {
		this->MapX--;
	}
	this->MapY = y / PixelTileSize.y;
	if (y < 0 && y % PixelTileSize.y) {
		this->MapY--;
	}
	this->OffsetX = x % PixelTileSize.x;
	if (this->OffsetX < 0) {
		this->OffsetX += PixelTileSize.x;
	}
	this->OffsetY = y % PixelTileSize.y;
	if (this->OffsetY < 0) {
		this->OffsetY += PixelTileSize.y;
	}
	this->MapWidth = ((this->EndX - this->X) + this->OffsetX - 1) / PixelTileSize.x + 1;
	this->MapHeight = ((this->EndY - this->Y) + this->OffsetY - 1) / PixelTileSize.y + 1;
}

/**
**  Change viewpoint of map viewport v to tilePos.
**
**  @param tilePos  map tile position.
**  @param offset   offset in tile.
*/
void CViewport::Set(const Vec2i &tilePos, const PixelDiff &offset)
{
	const int x = tilePos.x * PixelTileSize.x + offset.x;
	const int y = tilePos.y * PixelTileSize.y + offset.y;
	const PixelPos mapPixelPos = {x, y};

	this->Set(mapPixelPos);
}

/**
**  Center map viewport v on map tile (pos).
**
**  @param pos     map tile position.
**  @param offset  offset in tile.
*/
void CViewport::Center(const Vec2i &pos, const PixelDiff &offset)
{
	const int x = pos.x * PixelTileSize.x + offset.x - (this->EndX - this->X) / 2;
	const int y = pos.y * PixelTileSize.y + offset.y - (this->EndY - this->Y) / 2;
	const PixelPos mapPixelPos = {x, y};

	this->Set(mapPixelPos);
}

/**
**  Draw the map backgrounds.
**
** StephanR: variables explained below for screen:<PRE>
** *---------------------------------------*
** |                                       |
** |        *-----------------------*      |<-TheUi.MapY,dy (in pixels)
** |        |   |   |   |   |   |   |      |        |
** |        |   |   |   |   |   |   |      |        |
** |        |---+---+---+---+---+---|      |        |
** |        |   |   |   |   |   |   |      |        |MapHeight (in tiles)
** |        |   |   |   |   |   |   |      |        |
** |        |---+---+---+---+---+---|      |        |
** |        |   |   |   |   |   |   |      |        |
** |        |   |   |   |   |   |   |      |        |
** |        *-----------------------*      |<-ey,UI.MapEndY (in pixels)
** |                                       |
** |                                       |
** *---------------------------------------*
**          ^                       ^
**        dx|-----------------------|ex,UI.MapEndX (in pixels)
**            UI.MapX MapWidth (in tiles)
** (in pixels)
** </PRE>
*/
void CViewport::DrawMapBackgroundInViewport() const
{
	int ex = this->EndX;
	int sy = this->MapY;
	int dy = this->Y - this->OffsetY;
	int ey = this->EndY;
	const int map_max = Map.Info.MapWidth * Map.Info.MapHeight;
	unsigned short int tile;

	while (sy  < 0) {
		sy++;
		dy += PixelTileSize.y;
	}
	sy *=  Map.Info.MapWidth;

	while (dy <= ey && sy  < map_max) {

		/*
			if (sy / Map.Info.MapWidth < 0) {
				sy += Map.Info.MapWidth;
				dy += PixelTileSize.y;
				continue;
			}
		*/
		int sx = this->MapX + sy;
		int dx = this->X - this->OffsetX;
		while (dx <= ex && (sx - sy < Map.Info.MapWidth)) {
			if (sx - sy < 0) {
				++sx;
				dx += PixelTileSize.x;
				continue;
			}

			if (ReplayRevealMap) {
				tile = Map.Fields[sx].Tile;
			} else {
				tile = Map.Fields[sx].SeenTile;
			}
			Map.TileGraphic->DrawFrameClip(tile, dx, dy);

#ifdef DEBUG
			int my_mask = 0;
			unsigned int color = 0;
			if (Map.CheckMask(sx, MapFieldUnpassable)) {
				my_mask = 1;
			}
			if (Map.CheckMask(sx, MapFieldNoBuilding)) {
				my_mask |= 2;
			}
			switch (my_mask) {
				case 1://tile only Unpassable
					color = 0xFF0000;
					break;
				case 2://tile only NoBuilding
					color = 0x00FF00;
					break;
				case 3://tile Unpassable and NoBuilding
					color = 0xFF;
					break;
				default:
					break;
			}

			Video.DrawHLineClip(color, dx, dy, PixelTileSize.x);
			Video.DrawVLineClip(color, dx, dy, PixelTileSize.y);
			if (0 && my_mask) {
				CLabel label(GetSmallFont());
				label.Draw(dx + 2, dy + 2, tile);
				label.Draw(dx + 2, dy + GetSmallFont()->Height() + 4,
						   Map.Fields[sx].TilesetTile);

			}
#endif
			++sx;
			dx += PixelTileSize.x;
		}
		sy += Map.Info.MapWidth;
		dy += PixelTileSize.y;
	}
}

/**
**  Draw a map viewport.
*/
void CViewport::Draw() const
{
	PushClipping();
	SetClipping(this->X, this->Y, this->EndX, this->EndY);

	/* this may take while */
	this->DrawMapBackgroundInViewport();

	CurrentViewport = this;
#if 1 // for parallell drawing
	if (Proxy) {
		Proxy->Draw(*this);
	} else {
#else
	{
#endif
		std::vector<CUnit *> unittable;
		Missile *missiletable[MAX_MISSILES * 9];

		// We find and sort units after draw level.
		FindAndSortUnits(this, unittable);
		const int nunits = static_cast<int>(unittable.size());
		const int nmissiles = FindAndSortMissiles(*this, missiletable, MAX_MISSILES * 9);
		int i = 0;
		int j = 0;

		while (i < nunits && j < nmissiles) {
			if (unittable[i]->Type->DrawLevel <= missiletable[j]->Type->DrawLevel) {
				unittable[i]->Draw(this);
				++i;
			} else {
				missiletable[j]->DrawMissile(*this);
				++j;
			}
		}
		for (; i < nunits; ++i) {
			unittable[i]->Draw(this);
		}
		for (; j < nmissiles; ++j) {
			missiletable[j]->DrawMissile(*this);
		}
	}

	this->DrawMapFogOfWar();

	//
	// Draw orders of selected units.
	// Drawn here so that they are shown even when the unit is out of the screen.
	//
	//FIXME: This is still unsecure during parallel
	if (!Preference.ShowOrders) {
	} else if (Preference.ShowOrders < 0
				|| (ShowOrdersCount >= GameCycle) || (KeyModifiers & ModifierShift)) {
		for (int i = 0; i < NumSelected; ++i) {
			ShowOrder(*Selected[i]);
		}
	}
	DrawBorder();
	PopClipping();
}

/**
**  Draw border around the viewport
*/
void CViewport::DrawBorder() const
{
	// if we a single viewport, no need to denote the "selected" one
	if (UI.NumViewports == 1) {
		return;
	}

	Uint32 color = ColorBlack;
	if (this == UI.SelectedViewport) {
		color = ColorOrange;
	}

	Video.DrawRectangle(color, this->X, this->Y, this->EndX - this->X + 1,
						this->EndY - this->Y + 1);
}

CViewport::~CViewport()
{
#if 1 // for parallell drawing
	delete Proxy;
#endif
}

//@}
