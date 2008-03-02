//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name minimap.h - The minimap headerfile. */
//
//      (c) Copyright 1998-2008 by Lutz Sammer and Jimmy Salmon
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

#ifndef __MINIMAP_H__
#define __MINIMAP_H__

//@{

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/

class CMinimap
{
public:
	CMinimap() : X(0), Y(0), W(0), H(0), XOffset(0), YOffset(0),
		WithTerrain(false), ShowSelected(false), Transparent(false) {}

	void UpdateTerrain();
	void Update();
	void Create();
	void FreeOpenGL();
	void Reload();
	void Destroy();
	void Draw(int vx, int vy);
	void DrawCursor(int vx, int vy);
	void AddEvent(int x, int y);

	int Screen2MapX(int x);
	int Screen2MapY(int y);

	int X;
	int Y;
	int W;
	int H;
	int XOffset;
	int YOffset;
	bool WithTerrain;
	bool ShowSelected;
	bool Transparent;
};

//@}

#endif // !__MINIMAP_H__
