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
/**@name ui.cpp - The user interface globals. */
//
//      (c) Copyright 1999-2011 by Lutz Sammer, Andreas Arens,
//                                 Jimmy Salmon and Pali Rohár
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

#include "ui.h"

#include "font.h"
#include "interface.h"
#include "iolib.h"
#include "map.h"
#include "menus.h"
#include "title.h"
#include "ui/contenttype.h"
#include "ui/popup.h"
#include "unit.h"
#include "video.h"

#include <stdarg.h>

/*----------------------------------------------------------------------------
-- Variables
----------------------------------------------------------------------------*/

bool RightButtonAttacks;                   /// right button attacks

static ViewportModeType NewViewportMode = VIEWPORT_SINGLE;

bool FancyBuildings;                       /// Mirror buildings 1 yes, 0 now.

/**
**  The user interface configuration
*/
CUserInterface UI;

/*----------------------------------------------------------------------------
-- Functions
----------------------------------------------------------------------------*/

/**
**  Show load progress.
**
**  @param fmt  printf format string.
*/
void ShowLoadProgress(const char *fmt, ...)
{
	va_list va;
	char temp[4096];

	va_start(va, fmt);
	vsnprintf(temp, sizeof(temp) - 1, fmt, va);
	temp[sizeof(temp) - 1] = '\0';
	va_end(va);

	if (Video.Depth && IsGameFontReady() && GetGameFont().IsLoaded()) {
		// Remove non printable chars
		for (unsigned char *s = (unsigned char*)temp; *s; ++s) {
			if (*s < 32) {
				*s = ' ';
			}
		}
		Video.FillRectangle(ColorBlack, 5, Video.Height - 18, Video.Width - 10, 18);
		CLabel(GetGameFont()).DrawCentered(Video.Width / 2, Video.Height - 16, temp);
		InvalidateArea(5, Video.Height - 18, Video.Width - 10, 18);
		RealizeVideoMemory();
	} else {
		DebugPrint("!!!!%s\n" _C_ temp);
	}
}

CUnitInfoPanel::~CUnitInfoPanel()
{
	for (std::vector<CContentType *>::iterator content = Contents.begin();
		 content != Contents.end(); ++content) {
		delete *content;
	}
	delete Condition;
}


CUserInterface::CUserInterface() :
	MouseScroll(false), KeyScroll(false), MouseScrollSpeed(1),
	MouseScrollSpeedDefault(0), MouseScrollSpeedControl(0),
	SingleSelectedButton(NULL),
	MaxSelectedFont(NULL), MaxSelectedTextX(0), MaxSelectedTextY(0),
	SingleTrainingButton(NULL),
	SingleTrainingFont(NULL), SingleTrainingTextX(0), SingleTrainingTextY(0),
	TrainingFont(NULL), TrainingTextX(0), TrainingTextY(0),
	CompletedBarColor(0), CompletedBarShadow(0),
	ViewportMode(VIEWPORT_SINGLE), MouseViewport(NULL),
	SelectedViewport(NULL), NumViewports(0),
	MessageFont(NULL), MessageScrollSpeed(5),
	ViewportCursorColor(0), Offset640X(0), Offset480Y(0),
	VictoryBackgroundG(NULL), DefeatBackgroundG(NULL)
{
	MouseWarpPos.x = MouseWarpPos.y = 0;

	Point.Name = "cursor-point";
	Glass.Name = "cursor-glass";
	Cross.Name = "cursor-cross";
	YellowHair.Name = "cursor-yellow-hair";
	GreenHair.Name = "cursor-green-hair";
	RedHair.Name = "cursor-red-hair";
	Scroll.Name = "cursor-scroll";

	ArrowE.Name = "cursor-arrow-e";
	ArrowNE.Name = "cursor-arrow-ne";
	ArrowN.Name = "cursor-arrow-n";
	ArrowNW.Name = "cursor-arrow-nw";
	ArrowW.Name = "cursor-arrow-w";
	ArrowSW.Name = "cursor-arrow-sw";
	ArrowS.Name = "cursor-arrow-s";
	ArrowSE.Name = "cursor-arrow-se";

	NormalFontColor = "light-blue";
	ReverseFontColor = "yellow";
}

/**
**  Get popup class pointer by string identifier.
**
**  @param ident  Popup identifier.
**
**  @return       popup class pointer.
*/
CPopup *PopupByIdent(const std::string &ident)
{
	for (std::vector<CPopup *>::iterator i = UI.ButtonPopups.begin(); i < UI.ButtonPopups.end(); ++i) {
		if ((*i)->Ident == ident) {
			return *i;
		}
	}
	return NULL;
}

/**
**  Initialize the user interface.
*/
void InitUserInterface()
{
	UI.Offset640X = (Video.Width - 640) / 2;
	UI.Offset480Y = (Video.Height - 480) / 2;

	//
	// Calculations
	//
	if (Map.Info.MapWidth) {
		UI.MapArea.EndX = std::min<int>(UI.MapArea.EndX, Map.Info.MapWidth * PixelTileSize.x - 1);
		UI.MapArea.EndY = std::min<int>(UI.MapArea.EndY, Map.Info.MapHeight * PixelTileSize.y - 1);
	}

	UI.SelectedViewport = UI.Viewports;

	SetViewportMode(VIEWPORT_SINGLE);

	UI.CompletedBarColor = Video.MapRGB(TheScreen->format, UI.CompletedBarColorRGB);
	UI.ViewportCursorColor = ColorWhite;
}

/**
**  Load Cursor.
*/
void CursorConfig::Load()
{
	Assert(!Name.empty());
	Cursor = CursorByIdent(Name);
	if (Cursor == NULL) {
		return ;
	}
	Assert(Name == Cursor->Ident);
}

/**
**  Load the user interface graphics.
*/
void CUserInterface::Load()
{
	//  Load graphics
	const int size = (int)Fillers.size();
	for (int i = 0; i < size; ++i) {
		Fillers[i].Load();
	}

	for (int i = 0; i <= FreeWorkersCount; ++i) {
		if (Resources[i].G) {
			Resources[i].G->Load();
			Resources[i].G->UseDisplayFormat();
		}
	}

	if (InfoPanel.G) {
		InfoPanel.G->Load();
		InfoPanel.G->UseDisplayFormat();
	}
	if (ButtonPanel.G) {
		ButtonPanel.G->Load();
		ButtonPanel.G->UseDisplayFormat();
	}
	if (PieMenu.G) {
		PieMenu.G->Load();
		PieMenu.G->UseDisplayFormat();
	}

	//  Resolve cursors
	Point.Load();
	Glass.Load();
	Cross.Load();
	YellowHair.Load();
	GreenHair.Load();
	RedHair.Load();
	Scroll.Load();

	ArrowE.Load();
	ArrowNE.Load();
	ArrowN.Load();
	ArrowNW.Load();
	ArrowW.Load();
	ArrowSW.Load();
	ArrowS.Load();
	ArrowSE.Load();
}


bool CMapArea::Contains(const PixelPos &screenPos) const
{
	return this->X <= screenPos.x && screenPos.x <= this->EndX
		   && this->Y <= screenPos.y && screenPos.y <= this->EndY;
}

/**
**  Save the viewports.
**
**  @param file  Save file handle
**  @param ui    User interface to save
*/
static void SaveViewports(CFile &file, const CUserInterface &ui)
{
	// FIXME: don't save the number
	file.printf("DefineViewports(\"mode\", %d", ui.ViewportMode);
	for (int i = 0; i < ui.NumViewports; ++i) {
		const CViewport &vp = ui.Viewports[i];
		file.printf(",\n  \"viewport\", {%d, %d, %d}", vp.MapPos.x, vp.MapPos.y,
					vp.Unit ? UnitNumber(*vp.Unit) : -1);
	}
	file.printf(")\n\n");
}

/**
**  Save the user interface module.
**
**  @param file  Save file handle
*/
void SaveUserInterface(CFile &file)
{
	SaveViewports(file, UI);
}

/**
**  Clean up a user interface.
*/
CUserInterface::~CUserInterface()
{
}

/**
**  Clean up the user interface module.
*/
void CleanUserInterface()
{
	// Filler
	for (int i = 0; i < (int)UI.Fillers.size(); ++i) {
		CGraphic::Free(UI.Fillers[i].G);
	}
	UI.Fillers.clear();

	// Resource Icons
	for (int i = 0; i <= FreeWorkersCount; ++i) {
		CGraphic::Free(UI.Resources[i].G);
	}

	// Info Panel
	CGraphic::Free(UI.InfoPanel.G);
	for (std::vector<CUnitInfoPanel *>::iterator panel = UI.InfoPanelContents.begin();
		 panel != UI.InfoPanelContents.end(); ++panel) {
		delete *panel;
	}
	UI.InfoPanelContents.clear();

	// Button Popups
	for (std::vector<CPopup *>::iterator popup = UI.ButtonPopups.begin();
		 popup != UI.ButtonPopups.end(); ++popup) {
		delete *popup;
	}
	UI.ButtonPopups.clear();

	delete UI.SingleSelectedButton;
	UI.SelectedButtons.clear();
	delete UI.SingleTrainingButton;
	UI.SingleTrainingText.clear();
	UI.TrainingButtons.clear();
	UI.TrainingText.clear();
	delete UI.UpgradingButton;
	delete UI.ResearchingButton;
	UI.TransportingButtons.clear();
	UI.UserButtons.clear();

	// Button Panel
	CGraphic::Free(UI.ButtonPanel.G);

	// Pie Menu
	CGraphic::Free(UI.PieMenu.G);

	// Backgrounds
	CGraphic::Free(UI.VictoryBackgroundG);
	CGraphic::Free(UI.DefeatBackgroundG);

	// Title Screens
	if (TitleScreens) {
		for (int i = 0; TitleScreens[i]; ++i) {
			delete TitleScreens[i];
		}
		delete[] TitleScreens;
		TitleScreens = NULL;
	}
}

#ifdef DEBUG
void FreeButtonStyles()
{
	std::map<std::string, ButtonStyle *>::iterator i;
	for (i = ButtonStyleHash.begin(); i != ButtonStyleHash.end(); ++i) {
		delete(*i).second;
	}
	ButtonStyleHash.clear();
}
#endif

/**
**  Takes coordinates of a pixel in stratagus's window and computes
**  the map viewport which contains this pixel.
**
**  @param screenPos  pixel coordinate with origin at UL corner of screen
**
**  @return viewport pointer or NULL if this pixel is not inside
**  any of the viewports.
**
**  @note This functions only works with rectangular viewports, when
**  we support shaped map window, this must be rewritten.
*/
CViewport *GetViewport(const PixelPos &screenPos)
{
	for (CViewport *vp = UI.Viewports; vp < UI.Viewports + UI.NumViewports; ++vp) {
		if (vp->Contains(screenPos)) {
			return vp;
		}
	}
	return NULL;
}

/**
**  Takes an array of new Viewports which are supposed to have their
**  pixel geometry (CViewport::[XY] and CViewport::End[XY]) already
**  computed. Using this information as well as old viewport's
**  parameters fills in new viewports' CViewport::Map* parameters.
**  Then it replaces the old viewports with the new ones and finishes
**  the set-up of the new mode.
**
**  @param new_vps  The array of the new viewports
**  @param num_vps  The number of elements in the new_vps[] array.
*/
static void FinishViewportModeConfiguration(CViewport new_vps[], int num_vps)
{
	//  Compute location of the viewport using oldviewport
	for (int i = 0; i < num_vps; ++i) {
		new_vps[i].MapPos.x = 0;
		new_vps[i].MapPos.y = 0;
		const CViewport *vp = GetViewport(new_vps[i].GetTopLeftPos());
		if (vp) {
			const PixelDiff relDiff = new_vps[i].GetTopLeftPos() - vp->GetTopLeftPos();

			new_vps[i].Offset = relDiff + Map.TilePosToMapPixelPos_TopLeft(vp->MapPos) + vp->Offset;
		} else {
			new_vps[i].Offset.x = 0;
			new_vps[i].Offset.y = 0;
		}
	}

	// Affect the old viewport.
	for (int i = 0; i < num_vps; ++i) {
		CViewport &vp = UI.Viewports[i];

		vp.TopLeftPos = new_vps[i].TopLeftPos;
		vp.BottomRightPos = new_vps[i].BottomRightPos;
		vp.Set(new_vps[i].MapPos, new_vps[i].Offset);
	}
	UI.NumViewports = num_vps;

	//
	//  Update the viewport pointers
	//
	UI.MouseViewport = GetViewport(CursorScreenPos);
	UI.SelectedViewport = std::min(UI.Viewports + UI.NumViewports - 1, UI.SelectedViewport);
}

/**
**  Takes a viewport which is supposed to have its CViewport::[XY]
**  correctly filled-in and computes CViewport::End[XY] attributes
**  according to clipping information passed in other two arguments.
**
**  @param vp     The viewport.
**  @param ClipX  Maximum x-coordinate of the viewport's right side
**                as dictated by current UI's geometry and ViewportMode.
**  @param ClipY  Maximum y-coordinate of the viewport's bottom side
**                as dictated by current UI's geometry and ViewportMode.
**
**  @note It is supposed that values passed in Clip[XY] will
**  never be greater than UI::MapArea::End[XY].
**  However, they can be smaller according to the place
**  the viewport vp takes in context of current ViewportMode.
*/
static void ClipViewport(CViewport &vp, int ClipX, int ClipY)
{
	// begin with maximum possible viewport size
	vp.BottomRightPos.x = vp.TopLeftPos.x + Map.Info.MapWidth * PixelTileSize.x - 1;
	vp.BottomRightPos.y = vp.TopLeftPos.y + Map.Info.MapHeight * PixelTileSize.y - 1;

	// first clip it to MapArea size if necessary
	vp.BottomRightPos.x = std::min<int>(vp.BottomRightPos.x, ClipX);
	vp.BottomRightPos.y = std::min<int>(vp.BottomRightPos.y, ClipY);

	Assert(vp.BottomRightPos.x <= UI.MapArea.EndX);
	Assert(vp.BottomRightPos.y <= UI.MapArea.EndY);
}

/**
**  Compute viewport parameters for single viewport mode.
**
**  The parameters include viewport's width and height expressed
**  in pixels, its position with respect to Stratagus's window
**  origin, and the corresponding map parameters expressed in map
**  tiles with origin at map origin (map tile (0,0)).
*/
static void SetViewportModeSingle()
{
	CViewport new_vps[MAX_NUM_VIEWPORTS];

	DebugPrint("Single viewport set\n");

	new_vps[0].TopLeftPos.x = UI.MapArea.X;
	new_vps[0].TopLeftPos.y = UI.MapArea.Y;
	ClipViewport(new_vps[0], UI.MapArea.EndX, UI.MapArea.EndY);

	FinishViewportModeConfiguration(new_vps, 1);
}

/**
**  Compute viewport parameters for horizontally split viewport mode.
**  This mode splits the UI::MapArea with a horizontal line to
**  2 (approximately) equal parts.
**
**  The parameters include viewport's width and height expressed
**  in pixels, its position with respect to Stratagus's window
**  origin, and the corresponding map parameters expressed in map
**  tiles with origin at map origin (map tile (0,0)).
*/
static void SetViewportModeSplitHoriz()
{
	CViewport new_vps[MAX_NUM_VIEWPORTS];

	DebugPrint("Two horizontal viewports set\n");

	new_vps[0].TopLeftPos.x = UI.MapArea.X;
	new_vps[0].TopLeftPos.y = UI.MapArea.Y;
	ClipViewport(new_vps[0], UI.MapArea.EndX,
				 UI.MapArea.Y + (UI.MapArea.EndY - UI.MapArea.Y + 1) / 2);

	new_vps[1].TopLeftPos.x = UI.MapArea.X;
	new_vps[1].TopLeftPos.y = new_vps[0].BottomRightPos.y + 1;
	ClipViewport(new_vps[1], UI.MapArea.EndX, UI.MapArea.EndY);

	FinishViewportModeConfiguration(new_vps, 2);
}

/**
**  Compute viewport parameters for horizontal 3-way split viewport mode.
**  This mode splits the UI::MapArea with a horizontal line to
**  2 (approximately) equal parts, then splits the bottom part vertically
**  to another 2 parts.
**
**  The parameters include viewport's width and height expressed
**  in pixels, its position with respect to Stratagus's window
**  origin, and the corresponding map parameters expressed in map
**  tiles with origin at map origin (map tile (0,0)).
*/
static void SetViewportModeSplitHoriz3()
{
	CViewport new_vps[MAX_NUM_VIEWPORTS];

	DebugPrint("Horizontal 3-way viewport division set\n");

	new_vps[0].TopLeftPos.x = UI.MapArea.X;
	new_vps[0].TopLeftPos.y = UI.MapArea.Y;
	ClipViewport(new_vps[0], UI.MapArea.EndX,
				 UI.MapArea.Y + (UI.MapArea.EndY - UI.MapArea.Y + 1) / 2);

	new_vps[1].TopLeftPos.x = UI.MapArea.X;
	new_vps[1].TopLeftPos.y = new_vps[0].BottomRightPos.y + 1;
	ClipViewport(new_vps[1],
				 UI.MapArea.X + (UI.MapArea.EndX - UI.MapArea.X + 1) / 2,
				 UI.MapArea.EndY);

	new_vps[2].TopLeftPos.x = new_vps[1].BottomRightPos.x + 1;
	new_vps[2].TopLeftPos.y = new_vps[0].BottomRightPos.y + 1;
	ClipViewport(new_vps[2], UI.MapArea.EndX, UI.MapArea.EndY);

	FinishViewportModeConfiguration(new_vps, 3);
}

/**
**  Compute viewport parameters for vertically split viewport mode.
**  This mode splits the UI::MapArea with a vertical line to
**  2 (approximately) equal parts.
**
**  The parameters  include viewport's width and height expressed
**  in pixels, its position with respect to Stratagus's window
**  origin, and the corresponding map parameters expressed in map
**  tiles with origin at map origin (map tile (0,0)).
*/
static void SetViewportModeSplitVert()
{
	CViewport new_vps[MAX_NUM_VIEWPORTS];

	DebugPrint("Two vertical viewports set\n");

	new_vps[0].TopLeftPos.x = UI.MapArea.X;
	new_vps[0].TopLeftPos.y = UI.MapArea.Y;
	ClipViewport(new_vps[0],
				 UI.MapArea.X + (UI.MapArea.EndX - UI.MapArea.X + 1) / 2,
				 UI.MapArea.EndY);

	new_vps[1].TopLeftPos.x = new_vps[0].BottomRightPos.x + 1;
	new_vps[1].TopLeftPos.y = UI.MapArea.Y;
	ClipViewport(new_vps[1], UI.MapArea.EndX, UI.MapArea.EndY);

	FinishViewportModeConfiguration(new_vps, 2);
}

/**
**  Compute viewport parameters for 4-way split viewport mode.
**  This mode splits the UI::MapArea vertically *and* horizontally
**  to 4 (approximately) equal parts.
**
**  The parameters  include viewport's width and height expressed
**  in pixels, its position with respect to Stratagus's window
**  origin, and the corresponding map parameters expressed in map
**  tiles with origin at map origin (map tile (0,0)).
*/
static void SetViewportModeQuad()
{
	CViewport new_vps[MAX_NUM_VIEWPORTS];

	DebugPrint("Four viewports set\n");

	new_vps[0].TopLeftPos.x = UI.MapArea.X;
	new_vps[0].TopLeftPos.y = UI.MapArea.Y;
	ClipViewport(new_vps[0],
				 UI.MapArea.X + (UI.MapArea.EndX - UI.MapArea.X + 1) / 2,
				 UI.MapArea.Y + (UI.MapArea.EndY - UI.MapArea.Y + 1) / 2);

	new_vps[1].TopLeftPos.x = new_vps[0].BottomRightPos.x + 1;
	new_vps[1].TopLeftPos.y = UI.MapArea.Y;
	ClipViewport(new_vps[1],
				 UI.MapArea.EndX,
				 UI.MapArea.Y + (UI.MapArea.EndY - UI.MapArea.Y + 1) / 2);

	new_vps[2].TopLeftPos.x = UI.MapArea.X;
	new_vps[2].TopLeftPos.y = new_vps[0].BottomRightPos.y + 1;
	ClipViewport(new_vps[2],
				 UI.MapArea.X + (UI.MapArea.EndX - UI.MapArea.X + 1) / 2,
				 UI.MapArea.EndY);

	new_vps[3].TopLeftPos.x = new_vps[1].TopLeftPos.x;
	new_vps[3].TopLeftPos.y = new_vps[2].TopLeftPos.y;
	ClipViewport(new_vps[3], UI.MapArea.EndX, UI.MapArea.EndY);

	FinishViewportModeConfiguration(new_vps, 4);
}

/**
**  Sets up (calls geometry setup routines for) a new viewport mode.
**
**  @param new_mode  New mode's number.
*/
void SetViewportMode(ViewportModeType new_mode)
{
	switch (UI.ViewportMode = new_mode) {
		case VIEWPORT_SINGLE:
			SetViewportModeSingle();
			break;
		case VIEWPORT_SPLIT_HORIZ:
			SetViewportModeSplitHoriz();
			break;
		case VIEWPORT_SPLIT_HORIZ3:
			SetViewportModeSplitHoriz3();
			break;
		case VIEWPORT_SPLIT_VERT:
			SetViewportModeSplitVert();
			break;
		case VIEWPORT_QUAD:
			SetViewportModeQuad();
			break;
		default:
			DebugPrint("trying to set an unknown mode!!\n");
			break;
	}
}

/**
**  Cycles through predefined viewport modes (geometry configurations)
**  in order defined by the ViewportMode enumerated type.
**
**  @param step   The size of step used for cycling. Values that
**               make sense are mostly 1 (next viewport mode) and
**               -1 (previous viewport mode).
*/
void CycleViewportMode(int step)
{
	NewViewportMode = (ViewportModeType)(UI.ViewportMode + step);
	if (NewViewportMode >= NUM_VIEWPORT_MODES) {
		NewViewportMode = VIEWPORT_SINGLE;
	}
	if (NewViewportMode < 0) {
		NewViewportMode = (ViewportModeType)(NUM_VIEWPORT_MODES - 1);
	}
}

void CheckViewportMode()
{
	if (NewViewportMode != UI.ViewportMode) {
		SetViewportMode(NewViewportMode);
	}
}

/**
**  Check if mouse scrolling is enabled
*/
bool GetMouseScroll()
{
	return UI.MouseScroll;
}

/**
**  Enable/disable scrolling with the mouse
**
**  @param enabled  True to enable mouse scrolling, false to disable
*/
void SetMouseScroll(bool enabled)
{
	UI.MouseScroll = enabled;
}

/**
**  Check if keyboard scrolling is enabled
*/
bool GetKeyScroll()
{
	return UI.KeyScroll;
}

/**
**  Enable/disable scrolling with the keyboard
**
**  @param enabled  True to enable keyboard scrolling, false to disable
*/
void SetKeyScroll(bool enabled)
{
	UI.KeyScroll = enabled;
}

/**
**  Check if mouse grabbing is enabled
*/
bool GetGrabMouse()
{
	return SdlGetGrabMouse();
}

/**
**  Enable/disable grabbing the mouse
**
**  @param enabled  True to enable mouse grabbing, false to disable
*/
void SetGrabMouse(bool enabled)
{
	ToggleGrabMouse(enabled ? 1 : -1);
}

/**
**  Check if scrolling stops when leaving the window
*/
bool GetLeaveStops()
{
	return LeaveStops;
}

/**
**  Enable/disable leaving the window stops scrolling
**
**  @param enabled  True to stop scrolling, false to disable
*/
void SetLeaveStops(bool enabled)
{
	LeaveStops = enabled;
}

//@}
