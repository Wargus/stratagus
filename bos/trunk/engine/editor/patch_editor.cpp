//     ____                _       __               
//    / __ )____  _____   | |     / /___ ___________
//   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
//  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
// /_____/\____/____/     |__/|__/\__,_/_/  /____/  
//                                              
//       A futuristic real-time strategy game.
//          This file is part of Bos Wars.
//
/**@name patch_editor.cpp - The patch editor. */
//
//      (c) Copyright 2008 by Jimmy Salmon
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

#include "stratagus.h"

#include <sstream>

#include "sound.h"
#include "iolib.h"
#include "network.h"
#include "interface.h"
#include "cursor.h"
#include "map.h"
#include "guichan.h"
#include "patch_type.h"
#include "patch.h"
#include "patch_manager.h"
#include "ui.h"
#include "font.h"


extern void DrawGuichanWidgets();

bool PatchEditorRunning;

extern gcn::Gui *Gui;
static gcn::Container *PatchEditorContainer;

static CPatch *Patch;

static const int PatchMenuWidth = 150;

static float ScrollX;
static float ScrollY;

static CGraphic *TransparentG;
static CGraphic *TransparentSmallG;
static CGraphic *ImpassableG;
static CGraphic *ImpassableSmallG;
static CGraphic *CoastG;
static CGraphic *CoastSmallG;
static CGraphic *ShallowWaterG;
static CGraphic *ShallowWaterSmallG;
static CGraphic *DeepWaterG;
static CGraphic *DeepWaterSmallG;
#define NumSpeeds 7
static CGraphic *SpeedG[NumSpeeds + 1];
static CGraphic *SpeedSmallG[NumSpeeds + 1];

enum PatchButton {
	ButtonNone,
	ButtonImpassable,
	ButtonCoast,
	ButtonShallowWater,
	ButtonDeepWater,
	ButtonSpeed0,
	ButtonSpeed1,
	ButtonSpeed2,
	ButtonSpeed3,
	ButtonSpeed4,
	ButtonSpeed5,
	ButtonSpeed6,
	ButtonSpeed7,
	ButtonTransparent,
};

struct PatchIcon
{
	int X;
	int Y;
	CGraphic *G;
	PatchButton Button;
};

static std::vector<PatchIcon> PatchIcons;

static std::map<int, unsigned short> FlagMap;
static PatchButton CurrentButton;
static PatchButton MouseOverButton;
static int MouseOverTileX;
static int MouseOverTileY;

static unsigned short DraggingClearFlags;
static unsigned short DraggingSetFlags; // overrides DraggingClearFlags


static void DoScroll()
{
	int state = MouseScrollState | KeyScrollState;
	static Uint32 lastTicks = GetTicks() - 1;

	Uint32 ticks = GetTicks();
	float speed = (ticks - lastTicks) / 1000.f * 400;
	lastTicks = ticks;

	if (state == ScrollNone) {
		return;
	}

	if ((state & (ScrollLeft | ScrollRight)) &&
			(state & (ScrollLeft | ScrollRight)) != (ScrollLeft | ScrollRight)) {
		if (state & ScrollRight) {
			ScrollX += speed;
		} else {
			ScrollX -= speed;
		}
	}
	if ((state & (ScrollUp | ScrollDown)) &&
			(state & (ScrollUp | ScrollDown)) != (ScrollUp | ScrollDown)) {
		if (state & ScrollDown) {
			ScrollY += speed;
		} else {
			ScrollY -= speed;
		}
	}

	if (ScrollX < 0.f ||
			Patch->getType()->getGraphic()->Width < (Video.Width - PatchMenuWidth)) {
		ScrollX = 0.f;
	} else if (ScrollX > Patch->getType()->getGraphic()->Width - (Video.Width - PatchMenuWidth)) {
		ScrollX = (float)(Patch->getType()->getGraphic()->Width - (Video.Width - PatchMenuWidth));
	}
	if (ScrollY < 0.f ||
			Patch->getType()->getGraphic()->Height < Video.Height) {
		ScrollY = 0.f;
	} else if (ScrollY > Patch->getType()->getGraphic()->Height - Video.Height) {
		ScrollY = (float)(Patch->getType()->getGraphic()->Height - Video.Height);
	}
}

/**
**  Change the patch-tile flags at (::MouseOverTileX, ::MouseOverTileY),
**  as specified with ::DraggingClearFlags and ::DraggingSetFlags.
**
**  The patch editor has no real map, so this function does not
**  attempt to update any CMapField.
*/
static void ChangeFlagsAtMouse()
{
	CPatchType *patchType = Patch->getType();
	unsigned short flag = patchType->getFlag(MouseOverTileX, MouseOverTileY);
	flag &= ~DraggingClearFlags;
	flag |= DraggingSetFlags;
	patchType->setFlag(MouseOverTileX, MouseOverTileY, flag);
}

static void PatchEditorCallbackButtonDown(unsigned button)
{
	if ((1 << button) != LeftButton) {
		return;
	}

	// Icons
	if (MouseOverButton != ButtonNone) {
		CurrentButton = MouseOverButton;
	}

	// Patch area
	if (MouseOverTileX != -1) {
		unsigned short flag = Patch->getType()->getFlag(MouseOverTileX, MouseOverTileY);
		if (ButtonSpeed0 <= CurrentButton && CurrentButton <= ButtonSpeed7) {
			DraggingClearFlags = MapFieldSpeedMask;
			DraggingSetFlags = FlagMap[CurrentButton];
		} else if (CurrentButton == ButtonCoast
			|| CurrentButton == ButtonShallowWater
			|| CurrentButton == ButtonDeepWater) {
			// clear existing land/water flags then set the new flag
			DraggingClearFlags = (MapFieldLandAllowed
					      | MapFieldCoastAllowed
					      | MapFieldShallowWater
					      | MapFieldDeepWater
					      | MapFieldNoBuilding);
			if (flag & FlagMap[CurrentButton]) {
				// set default to land
				DraggingSetFlags = MapFieldLandAllowed;
			} else {
				DraggingSetFlags = FlagMap[CurrentButton];
			}
		} else {
			// toggle flag
			DraggingClearFlags = flag & FlagMap[CurrentButton];
			DraggingSetFlags = DraggingClearFlags ^ FlagMap[CurrentButton];
		}

		ChangeFlagsAtMouse();
	}
}

static void PatchEditorCallbackButtonUp(unsigned button)
{
	DraggingClearFlags = 0;
	DraggingSetFlags = 0;
}

static void PatchEditorCallbackKeyDown(unsigned key, unsigned keychar)
{
	if (HandleKeyModifiersDown(key, keychar)) {
		return;
	}

	switch (key) {
		case 'f': // ALT+F, CTRL+F toggle fullscreen
			if (!(KeyModifiers & (ModifierAlt | ModifierControl))) {
				break;
			}
			ToggleFullScreen();
			break;

		case 'x': // ALT+X, CTRL+X: Exit editor
			if (!(KeyModifiers & (ModifierAlt | ModifierControl))) {
				break;
			}
			Exit(0);

		case SDLK_UP: // Keyboard scrolling
		case SDLK_KP8:
			KeyScrollState |= ScrollUp;
			break;
		case SDLK_DOWN:
		case SDLK_KP2:
			KeyScrollState |= ScrollDown;
			break;
		case SDLK_LEFT:
		case SDLK_KP4:
			KeyScrollState |= ScrollLeft;
			break;
		case SDLK_RIGHT:
		case SDLK_KP6:
			KeyScrollState |= ScrollRight;
			break;

		default:
			HandleCommandKey(key);
			return;
	}
}

static void PatchEditorCallbackKeyUp(unsigned key, unsigned keychar)
{
	if (HandleKeyModifiersUp(key, keychar)) {
		return;
	}

	switch (key) {
		case SDLK_UP: // Keyboard scrolling
		case SDLK_KP8:
			KeyScrollState &= ~ScrollUp;
			break;
		case SDLK_DOWN:
		case SDLK_KP2:
			KeyScrollState &= ~ScrollDown;
			break;
		case SDLK_LEFT:
		case SDLK_KP4:
			KeyScrollState &= ~ScrollLeft;
			break;
		case SDLK_RIGHT:
		case SDLK_KP6:
			KeyScrollState &= ~ScrollRight;
			break;
		default:
			break;
	}
}

static void PatchEditorCallbackKeyRepeated(unsigned dummy1, unsigned dummy2)
{
}

static void PatchEditorCallbackMouse(int x, int y)
{
	HandleCursorMove(&x, &y);

	MouseScrollState = ScrollNone;
	GameCursor = UI.Point.Cursor;
	MouseOverButton = ButtonNone;
	MouseOverTileX = -1;

	// Icons
	std::vector<PatchIcon>::iterator i;
	for (i = PatchIcons.begin(); i != PatchIcons.end(); ++i) {
		if (i->X <= x && x <= i->X + i->G->Width &&
				i->Y <= y && y <= i->Y + i->G->Height)
		{
			MouseOverButton = i->Button;
		}
	}

	// Patch area
	if (0 <= x && x < Video.Width - PatchMenuWidth &&
		0 <= y && y < Video.Height)
	{
		int tileX = (x + (int)ScrollX) / TileSizeX;
		int tileY = (y + (int)ScrollY) / TileSizeY;
		if (0 <= tileX && tileX < Patch->getType()->getTileWidth() &&
			0 <= tileY && tileY < Patch->getType()->getTileHeight())
		{
			MouseOverTileX = tileX;
			MouseOverTileY = tileY;

			// If dragging did not begin in the patch area,
			// then DraggingClearFlags and DraggingSetFlags
			// are both zero, and this call doesn't change
			// anything.
			ChangeFlagsAtMouse();
		}
	}

	// Scroll area at edge of the screen
	if (HandleMouseScrollArea(x, y)) {
		return;
	}
}

static void PatchEditorCallbackExit(void)
{
	// Disabled
	if (!LeaveStops) {
		return;
	}

	// Prevent scrolling while out of focus (on other applications)
	KeyScrollState = MouseScrollState = ScrollNone;
}

static void DrawPatch()
{
	const CGraphic *g = Patch->getType()->getGraphic();
	g->DrawClip(-(int)ScrollX, -(int)ScrollY);
}

static void DrawPatchTileIcons()
{
	CGraphic *g;
	int x, y;

	for (int j = 0; j < Patch->getType()->getTileHeight(); ++j) {
		y = j * TileSizeY - (int)ScrollY;

		for (int i = 0; i < Patch->getType()->getTileWidth(); ++i) {
			x = i * TileSizeX - (int)ScrollX;

			unsigned short flags = Patch->getType()->getFlag(i, j);
			if (flags & MapFieldTransparent) {
				g = TransparentSmallG;
				g->DrawClip(x, y);
				continue;
			}
			if (flags & MapFieldUnpassable) {
				g = ImpassableSmallG;
				g->DrawClip(x, y);
			}
			if (flags & MapFieldCoastAllowed) {
				g = CoastSmallG;
				g->DrawClip(x + 16, y);
			}
			if (flags & MapFieldShallowWater) {
				g = ShallowWaterSmallG;
				g->DrawClip(x + 16, y);
			}
			if (flags & MapFieldDeepWater) {
				g = DeepWaterSmallG;
				g->DrawClip(x + 16, y);
			}
			if ((flags & MapFieldSpeedMask) != 3) {
				g = SpeedSmallG[(flags & MapFieldSpeedMask)];
				g->DrawClip(x + 16, y + 16);
			}
		}
	}
}

static void DrawGrids()
{
	int i;
	int width = Patch->getType()->getTileWidth();
	int height = Patch->getType()->getTileHeight();

	for (i = 1; i < width; ++i) {
		Video.DrawVLineClip(ColorBlack, i * TileSizeX - (int)ScrollX, 0, height * TileSizeY);
	}
	for (i = 1; i < height; ++i) {
		Video.DrawHLineClip(ColorBlack, 0, i * TileSizeY - (int)ScrollY, width * TileSizeX);
	}
}

static void DrawIcons()
{
	Uint32 color;
	std::vector<PatchIcon>::iterator i;

	for (i = PatchIcons.begin(); i != PatchIcons.end(); ++i) {
		i->G->DrawClip(i->X, i->Y);

		if (i->Button == MouseOverButton) {
			color = ColorBlue;
		} else if (i->Button == CurrentButton) {
			color = ColorGreen;
		} else {
			color = ColorBlack;
		}

		Video.DrawRectangleClip(color, i->X - 1, i->Y - 1, 50, 50);
		Video.DrawRectangleClip(color, i->X - 2, i->Y - 2, 52, 52);
	}
}

static void DrawCoordinates()
{
	if (MouseOverTileX == -1) {
		return;
	}

	std::ostringstream o;
	o << MouseOverTileX << "," << MouseOverTileY;
	VideoDrawTextCentered(Video.Width - PatchMenuWidth / 2, Video.Height - 15, GameFont, o.str());
}

static void PatchEditorUpdateDisplay()
{
	// Patch area
	DrawPatch();
	DrawPatchTileIcons();
	DrawGrids();

	// Menu area
	Video.FillRectangle(ColorGray, Video.Width - PatchMenuWidth, 0, PatchMenuWidth, Video.Height);
	DrawIcons();
	DrawCoordinates();

	DrawGuichanWidgets();

	DrawCursor();

	Invalidate();
	RealizeVideoMemory();
}

static gcn::Button *PatchNewButton(const std::string &caption)
{
	gcn::Color darkColor(38, 38, 78, 128);
	gcn::Button *button;
	
	button = new gcn::Button(caption);
	button->setSize(106, 28);
	button->setBackgroundColor(darkColor);
	button->setBaseColor(darkColor);
	return button;
}

class PatchSaveButtonListener : public gcn::ActionListener
{
public:
	virtual void action(const std::string &eventId) {
		CFile file;
		std::string name = UserDirectory + "patches/" + Patch->getType()->getName() + ".lua";
		if (file.open(name.c_str(), CL_OPEN_WRITE) == -1) {
			// TODO: show error
			return;
		}
		file.printf("%s", Map.PatchManager.savePatchType(Patch->getType()).c_str());
		file.close();
		// TODO: show success

		Patch->getType()->setCustomPatch(false);
	}
};

class PatchExitButtonListener : public gcn::ActionListener
{
public:
	virtual void action(const std::string &eventId) {
		// TODO: prompt to save changes
		PatchEditorRunning = false;
	}
};

static void InitCallbacks(EventCallback *callbacks)
{
	callbacks->ButtonPressed = PatchEditorCallbackButtonDown;
	callbacks->ButtonReleased = PatchEditorCallbackButtonUp;
	callbacks->MouseMoved = PatchEditorCallbackMouse;
	callbacks->MouseExit = PatchEditorCallbackExit;
	callbacks->KeyPressed = PatchEditorCallbackKeyDown;
	callbacks->KeyReleased = PatchEditorCallbackKeyUp;
	callbacks->KeyRepeated = PatchEditorCallbackKeyRepeated;
	callbacks->NetworkEvent = NetworkEvent;
}

static void PatchEditorMainLoop()
{
	const EventCallback *old_callbacks = GetCallbacks();
	EventCallback callbacks;
	gcn::Button *saveButton;
	gcn::Button *exitButton;
	PatchSaveButtonListener *saveButtonListener;
	PatchExitButtonListener *exitButtonListener;

	InitCallbacks(&callbacks);
	SetCallbacks(&callbacks);

	ScrollX = 0.f;
	ScrollY = 0.f;

	CurrentButton = ButtonImpassable;
	MouseOverButton = ButtonNone;

	gcn::Widget *oldTop = Gui->getTop();

	PatchEditorContainer = new gcn::Container();
	PatchEditorContainer->setDimension(gcn::Rectangle(0, 0, Video.Width, Video.Height));
	PatchEditorContainer->setOpaque(false);
	Gui->setTop(PatchEditorContainer);

	saveButton = PatchNewButton(_("Save"));
	saveButtonListener = new PatchSaveButtonListener();
	saveButton->addActionListener(saveButtonListener);
	PatchEditorContainer->add(saveButton,
		Video.Width - PatchMenuWidth / 2 - saveButton->getWidth() / 2, 20);

	exitButton = PatchNewButton(_("Exit"));
	exitButtonListener = new PatchExitButtonListener();
	exitButton->addActionListener(exitButtonListener);
	PatchEditorContainer->add(exitButton,
		Video.Width - PatchMenuWidth / 2 - saveButton->getWidth() / 2, Video.Height - exitButton->getHeight() - 20);

	Map.PatchManager.load();

	PatchEditorRunning = true;
	while (PatchEditorRunning) {
		CheckMusicFinished();
		DoScroll();

		PatchEditorUpdateDisplay();

		WaitEventsOneFrame();
	}

	SetCallbacks(old_callbacks);

	Gui->setTop(oldTop);
	delete PatchEditorContainer;
	delete saveButton;
	delete saveButtonListener;
	delete exitButton;
	delete exitButtonListener;
}

static void PatchAddIcon(int x, int y, CGraphic *g, PatchButton b)
{
	PatchIcon icon;
	icon.X = x;
	icon.Y = y;
	icon.G = g;
	icon.Button = b;
	PatchIcons.push_back(icon);
}

static void PatchLoadIcons()
{
	std::string patchEditorPath = "ui/patcheditor/";
	int spacing = 5;
	int leftX = Video.Width - PatchMenuWidth + spacing;
	int rightX = Video.Width - 48 - spacing;
	int topY = 60;

	ImpassableG = CGraphic::New(patchEditorPath + "impassable.png");
	ImpassableG->Load();
	ImpassableSmallG = CGraphic::New(patchEditorPath + "impassable-small.png");
	ImpassableSmallG->Load();
	PatchAddIcon(leftX, topY, ImpassableG, ButtonImpassable);
	FlagMap[ButtonImpassable] = MapFieldUnpassable;

	CoastSmallG = CGraphic::New(patchEditorPath + "coast-small.png");
	CoastSmallG->Load();
	CoastG = CGraphic::New(patchEditorPath + "coast.png");
	CoastG->Load();
	PatchAddIcon(rightX, topY, CoastG, ButtonCoast);
	FlagMap[ButtonCoast] = MapFieldCoastAllowed;

	ShallowWaterG = CGraphic::New(patchEditorPath + "water.png");
	ShallowWaterG->Load();
	ShallowWaterSmallG = CGraphic::New(patchEditorPath + "water-small.png");
	ShallowWaterSmallG->Load();
	PatchAddIcon(leftX, topY + (48 + spacing), ShallowWaterG, ButtonShallowWater);
	FlagMap[ButtonShallowWater] = MapFieldShallowWater;

	DeepWaterG = CGraphic::New(patchEditorPath + "deep-sea.png");
	DeepWaterG->Load();
	DeepWaterSmallG = CGraphic::New(patchEditorPath + "deep-sea-small.png");
	DeepWaterSmallG->Load();
	PatchAddIcon(rightX, topY + (48 + spacing), DeepWaterG, ButtonDeepWater);
	FlagMap[ButtonDeepWater] = MapFieldDeepWater;

	for (int i = 0; i <= NumSpeeds; ++i) {
		std::ostringstream o;
		o << patchEditorPath << "speed" << i << ".png";
		SpeedG[i] = CGraphic::New(o.str());
		SpeedG[i]->Load();
		o.str(std::string());
		o.clear();
		o << patchEditorPath << "speed" << i << "-small.png";
		SpeedSmallG[i] = CGraphic::New(o.str());
		SpeedSmallG[i]->Load();
		PatchAddIcon((!(i & 1) ? leftX : rightX), topY + (48 + spacing) * ((i + 4) / 2), SpeedG[i], (PatchButton)(ButtonSpeed0 + i));
		FlagMap[ButtonSpeed0 + i] = i;
	}

	TransparentG = CGraphic::New(patchEditorPath + "transparent.png");
	TransparentG->Load();
	TransparentSmallG = CGraphic::New(patchEditorPath + "transparent-small.png");
	TransparentSmallG->Load();
	PatchAddIcon(leftX, topY + (48 + spacing) * ((NumSpeeds + 5) / 2), TransparentG, ButtonTransparent);
	FlagMap[ButtonTransparent] = MapFieldTransparent;
}

static void PatchFreeIcons()
{
	CGraphic::Free(ImpassableG);
	ImpassableG = NULL;
	CGraphic::Free(ImpassableSmallG);
	ImpassableSmallG = NULL;

	CGraphic::Free(CoastG);
	CoastG = NULL;
	CGraphic::Free(CoastSmallG);
	CoastSmallG = NULL;

	CGraphic::Free(ShallowWaterG);
	ShallowWaterG = NULL;
	CGraphic::Free(ShallowWaterSmallG);
	ShallowWaterSmallG = NULL;

	CGraphic::Free(DeepWaterG);
	DeepWaterG = NULL;
	CGraphic::Free(DeepWaterSmallG);
	DeepWaterSmallG = NULL;

	PatchIcons.clear();
	FlagMap.clear();
}

void StartPatchEditor(const std::string &patchName)
{
	std::string name;
	if (patchName.substr(0, 5) != "user-") {
		name = "user-" + patchName;
		if (Map.PatchManager.getPatchType(name) == NULL) {
			CPatchType *patchType = Map.PatchManager.getPatchType(patchName);
			Map.PatchManager.newPatchType(name,
				patchType->getFile(),
				patchType->getTileWidth(), patchType->getTileHeight(),
				patchType->getFlags());
		}
	} else {
		name = patchName;
	}

	Patch = Map.PatchManager.add(name, 0, 0);
	if (!Patch) {
		fprintf(stderr, "Invalid patch name: %s\n", patchName.c_str());
		return;
	}

	PatchLoadIcons();

	PatchEditorMainLoop();

	PatchFreeIcons();

	Map.PatchManager.clear();
}

//@}
