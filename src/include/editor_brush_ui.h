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
/**@name editor_brush.h - Assistant for brushes in the editor. */
//
//      (c) Copyright 2023-2024 by Alyokhin
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

#pragma once

//@{
#include <cstdint>
#include "tileset.h"
#include "vec2i.h"
#include "widgets.h"

/*----------------------------------------------------------------------------
--  Declarations
----------------------------------------------------------------------------*/
class CBrushControlsUI
{
public:
	explicit CBrushControlsUI(gcn::Container* parrent, const gcn::Rectangle &rectangle)
	{ 
		Init(parrent, rectangle);
	}
	~CBrushControlsUI() = default;

	void setVisibe(const bool value)
	{ 
		for(auto &ctrl : controls) {
			ctrl->setVisible(value);
		}
	}
	void show() { setVisibe(true);  }
	void hide() { setVisibe(false);  }
	void reloadBrushes();

private:

	void Init(gcn::Container* container, const gcn::Rectangle &rectangle);
	void reloadCtrlSettings();
	void updateSizeCtrls();

private:
	gcn::Container *container = nullptr;
	std::list<gcn::Widget *> controls;

	std::unique_ptr<gcn::DropDown> brushSelect;
	std::unique_ptr<StringListModel> brushesList;
	std::unique_ptr<LambdaActionListener> brushesDropdownListener;

	std::unique_ptr<gcn::Slider> sizeSlider;
	std::unique_ptr<LambdaActionListener> brushSizeSliderListener;

	enum ResizeAllowed { cBoth = 0, cWidthOnly, cHeightOnly };
	std::map<std::string, std::unique_ptr<gcn::RadioButton>> allowResize;

	std::unique_ptr<gcn::CheckBox> enableRnd;
	std::unique_ptr<gcn::CheckBox> fixNeighbors;

	gcn::Rectangle UIRectangle {};
	int16_t verticalGap = 10;

	gcn::Color baseColor {38, 38, 78};
	gcn::Color foregroundColor {200, 200, 120};
	gcn::Color backgroundColor {40, 24, 16};

};
//@}