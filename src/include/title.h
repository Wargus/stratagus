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
/**@name title.h - The title screen headerfile. */
//
//      (c) Copyright 2007 by Jimmy Salmon
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

#ifndef __TITLE_H__
#define __TITLE_H__

//@{

#include "filesystem.h"

#include <memory>
#include <string>
#include <vector>

class CFont;

enum {
	TitleFlagCenter = 1 << 0   /// Center Text
};

class TitleScreenLabel
{
public:
	TitleScreenLabel() = default;

	std::string Text;
	CFont *Font = nullptr;
	int Xofs = 0;
	int Yofs = 0;
	int Flags = 0;
};

class TitleScreen
{
public:
	TitleScreen() = default;
	~TitleScreen() = default;

	void ShowTitleImage();

private:
	void ShowLabels();
public:
	fs::path File;
	std::string Music;
	bool StretchImage = true;
	int Timeout = 0;
	int Iterations = 0;
	int Editor = 0;
	std::vector<std::unique_ptr<TitleScreenLabel>> Labels;
};

extern std::vector<std::unique_ptr<TitleScreen>> TitleScreens;          /// File for title screen

extern void ShowTitleScreens();

//@}

#endif // !__TITLE_H__
