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
/**@name minimap.h - The minimap headerfile. */
//
//      (c) Copyright 1998-2005 by Lutz Sammer and Jimmy Salmon
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

#ifndef __MINIMAP_H__
#define __MINIMAP_H__

//@{

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CMinimap
{
	template <const int BPP>
	void UpdateMapTerrain(void *const mpixels, const int mpitch,
						  const void *const tpixels, const int tpitch);

	void UpdateTerrain();

	template <const int BPP>
	void UpdateSeen(void *const pixels, const int pitch);

public:
	CMinimap() : X(0), Y(0), W(0), H(0), XOffset(0), YOffset(0),
		WithTerrain(false), ShowSelected(false),
		Transparent(false), UpdateCache(false) {}

	void UpdateXY(const Vec2i &pos);
	void UpdateSeenXY(const Vec2i &) {}
	void Update();
	void Create();
	void FreeOpenGL();
	void Reload();
	void Destroy();
	void Draw() const;
	void DrawViewportArea(const CViewport &viewport) const;
	void AddEvent(const Vec2i &pos, Uint32 color);

	Vec2i ScreenToTilePos(const PixelPos &screenPos) const;
	int Screen2MapX(int x) const;
	int Screen2MapY(int y) const;

	bool Contains(const PixelPos &screenPos) const;
public:
	int X;
	int Y;
	int W;
	int H;
	int XOffset;
	int YOffset;
	bool WithTerrain;
	bool ShowSelected;
	bool Transparent;
	bool UpdateCache;
};

//@}

#endif // !__MINIMAP_H__
