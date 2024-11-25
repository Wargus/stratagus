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

//@{

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/


#include "editor_brush_ui.h"

#include "editor.h"


/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/
void CBrushControlsUI::reloadBrushes()
{
	const auto prevSelectedItem = brushesList->getElementAt(brushSelect->getSelected());
	const auto newBrushesList = std::make_unique<StringListModel>(Editor.brushes.getBrushesNames());
	brushesList.reset(newBrushesList.get());
	brushSelect->setListModel(brushesList.get());

	const std::string currentBrush = Editor.brushes.getCurrentBrush().getName();
	if (const auto idx = brushesList->getIdxOfElement(currentBrush) != -1) {
		brushSelect->setSelected(idx);
	} else {
		brushSelect->setSelected(0);
	}
	reloadCtrlSettings();
}

void CBrushControlsUI::Init(gcn::Container* parrent, const gcn::Rectangle &rectangle)
{
	UIRectangle = rectangle;

	brushesList = std::make_unique<StringListModel>(Editor.brushes.getBrushesNames());
	brushSelect = std::make_unique<gcn::DropDown>(brushesList.get());
	brushSelect->setFont(&GetGameFont());
	brushSelect->setWidth(UIRectangle.width - 10);
	brushSelect->setBaseColor(baseColor);
	brushSelect->setForegroundColor(foregroundColor);
	brushSelect->setBackgroundColor(backgroundColor);

	controls.push_back(dynamic_cast<gcn::Widget*>(brushSelect.get()));
	parrent->add(brushSelect.get(), UIRectangle.x + 5, UIRectangle.y + 5);

	brushesDropdownListener = 
	std::make_unique<LambdaActionListener>([this](const std::string&) {
		const int selected = brushSelect->getSelected();
		if (Editor.brushes.setCurrentBrush(brushesList->getElementAt(selected))) {
			reloadCtrlSettings();
		}
		DebugPrint("Current brush is: %s\n", brushesList->getElementAt(selected).c_str());
	});
	brushSelect->addActionListener(brushesDropdownListener.get());


	sizeSlider = std::make_unique<gcn::Slider>();
	sizeSlider->setWidth(UIRectangle.width - 32);
	sizeSlider->setHeight(GetSmallFont().getHeight());
	sizeSlider->setBaseColor(baseColor);
	sizeSlider->setForegroundColor(foregroundColor);
	sizeSlider->setBackgroundColor(backgroundColor);

	controls.push_back(dynamic_cast<gcn::Widget*>(sizeSlider.get()));
	parrent->add(sizeSlider.get(),
				 16,
				 brushSelect->getY() + brushSelect->getHeight() + verticalGap);


	allowResize.insert({"Both",
						 std::make_unique<gcn::RadioButton>("Size", "BrushSize", true)});
	allowResize.insert({"WidthOnly",
						 std::make_unique<gcn::RadioButton>("Width", "BrushSize", false)});
	allowResize.insert({"HeightOnly",
						 std::make_unique<gcn::RadioButton>("Height", "BrushSize", false)});

	for (auto &[name, radioButton] : allowResize) {
		radioButton->setHeight(14);
		radioButton->setFont(&GetGameFont());
	}

	controls.push_back(dynamic_cast<gcn::Widget*>(allowResize["Both"].get()));
	parrent->add(allowResize["Both"].get(),
				 UIRectangle.x + UIRectangle.width / 2 - allowResize["Both"]->getWidth () - 5,
				 sizeSlider->getY() + sizeSlider->getHeight() + verticalGap / 2);

	controls.push_back(dynamic_cast<gcn::Widget*>(allowResize["WidthOnly"].get()));
	parrent->add(allowResize["WidthOnly"].get(),
				 UIRectangle.x + UIRectangle.width / 2 + 5,
				 allowResize["Both"]->getY());

	controls.push_back(dynamic_cast<gcn::Widget*>(allowResize["HeightOnly"].get()));
	parrent->add(allowResize["HeightOnly"].get(),
				 UIRectangle.x + UIRectangle.width / 2 + 5,
				 allowResize["WidthOnly"]->getY() 
				 + allowResize["WidthOnly"]->getHeight()
				 + verticalGap / 2);
	updateSizeCtrls();

	brushSizeSliderListener =
	std::make_unique<LambdaActionListener>([this](const std::string&) {

		const uint8_t size = sizeSlider->getValue();
		DebugPrint("Brush size: %d\n", int(size));

		if (allowResize["Both"]->isSelected()) {
			Editor.brushes.getCurrentBrush().resize(size, size);
		} else if (allowResize["Width"]->isSelected()) {
			Editor.brushes.getCurrentBrush().resizeW(size);
		} else {
			Editor.brushes.getCurrentBrush().resizeH(size);
		}

	});
	sizeSlider->addActionListener(brushSizeSliderListener.get());

	
	enableRnd = std::make_unique<gcn::CheckBox>("Random");
	enableRnd->setHeight(14);
	enableRnd->setFont(&GetGameFont());

	controls.push_back(dynamic_cast<gcn::Widget*>(enableRnd.get()));
	parrent->add(enableRnd.get(),
				 UIRectangle.x + 5,
				 allowResize["HeightOnly"]->getY()
				 + allowResize["HeightOnly"]->getHeight()
				 + verticalGap);

	fixNeighbors = std::make_unique<gcn::CheckBox>("Fix neighbors");
	fixNeighbors->setHeight(14);
	fixNeighbors->setFont(&GetGameFont());

	controls.push_back(dynamic_cast<gcn::Widget*>(fixNeighbors.get()));
	parrent->add(fixNeighbors.get(),
				 enableRnd->getX(),
				 enableRnd->getY() + enableRnd->getHeight() + verticalGap);

	reloadCtrlSettings();
}

void CBrushControlsUI::reloadCtrlSettings()
{
	const auto brush = Editor.brushes.getCurrentBrush();
	enableRnd->setSelected(brush.getAutoRandomizable());
	enableRnd->setVisible(brush.isRandomizeAllowed());

	fixNeighbors->setSelected(brush.getFixNeighbors());
	fixNeighbors->setVisible(brush.isNeighborsFixAllowed());

	updateSizeCtrls();
}

void CBrushControlsUI::updateSizeCtrls()
{
	const auto brush = Editor.brushes.getCurrentBrush();

	sizeSlider->setVisible(brush.isResizeble());
	for (auto &[name, radioButton] : allowResize) {
		radioButton->setVisible(brush.isResizeble());
	}
	if (!brush.isResizeble()) {
		return;
	}

	const auto resizeSteps = brush.getResizeSteps();

	const auto allowed = resizeSteps.x && resizeSteps.y ? ResizeAllowed::cBoth
					   : resizeSteps.x					? ResizeAllowed::cWidthOnly
														: ResizeAllowed::cHeightOnly;

	const auto selected = allowed == ResizeAllowed::cBoth      ? "Both"
						: allowed == ResizeAllowed::cWidthOnly ? "WidthOnly"
															   : "HeightOnly";
	allowResize[selected]->setSelected(true);

	allowResize["Both"]->setEnabled(allowed == cBoth);
	allowResize["WidthOnly"]->setEnabled(allowed == cBoth || allowed == cWidthOnly);
	allowResize["HeightOnly"]->setEnabled(allowed == cBoth || allowed == cHeightOnly);

	const auto maxSizes = brush.getMaxSize();

	if (allowed == ResizeAllowed::cBoth || allowed == ResizeAllowed::cWidthOnly) {
		sizeSlider->setScale(brush.getMinSize().x, maxSizes.x);
		sizeSlider->setStepLength(brush.getResizeSteps().x);
		sizeSlider->setValue(brush.getWidth());
	} else {
		sizeSlider->setScale(brush.getMinSize().y, maxSizes.y);
		sizeSlider->setStepLength(brush.getResizeSteps().y);
		sizeSlider->setValue(brush.getHeight());
	}
}
//@}
