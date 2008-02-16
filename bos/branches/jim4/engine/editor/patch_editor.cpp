#include "stratagus.h"
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


extern void DrawGuichanWidgets();

static bool PatchEditorRunning;

extern gcn::Gui *Gui;
static gcn::Container *PatchEditorContainer;

static CPatch *Patch;

static const int PatchMenuWidth = 150;

static float ScrollX;
static float ScrollY;

static CGraphic *ImpassableG;
static CGraphic *ImpassableSmallG;
static CGraphic *WaterG;
static CGraphic *WaterSmallG;

enum PatchButton {
	ButtonNone,
	ButtonImpassable,
	ButtonWater,
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
		ScrollX = Patch->getType()->getGraphic()->Width - (Video.Width - PatchMenuWidth);
	}
	if (ScrollY < 0.f ||
			Patch->getType()->getGraphic()->Height < Video.Height) {
		ScrollY = 0.f;
	} else if (ScrollY > Patch->getType()->getGraphic()->Height - Video.Height) {
		ScrollY = Patch->getType()->getGraphic()->Height - Video.Height;
	}
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
		Patch->getType()->setFlag(MouseOverTileX, MouseOverTileY, (flag ^ FlagMap[CurrentButton]));
	}
}

static void PatchEditorCallbackButtonUp(unsigned button)
{
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
		int tileX = (x - (int)ScrollX) / TileSizeX;
		int tileY = (y - (int)ScrollY) / TileSizeY;
		if (0 <= tileX && tileX < Patch->getType()->getTileWidth() &&
				0 <= tileY && tileY < Patch->getType()->getTileHeight())
		{
			MouseOverTileX = tileX;
			MouseOverTileY = tileY;
		}
	}

	// Scroll area at edge of the screen
	if (HandleMouseScrollArea(x, y)) {
		return;
	}
}

static void PatchEditorCallbackExit(void)
{
}

static void DrawPatch()
{
	const CGraphic *g = Patch->getType()->getGraphic();
	g->DrawSubClip(0, 0, g->Width, g->Height, -(int)ScrollX, -(int)ScrollY);
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
			if (flags & MapFieldUnpassable) {
				g = ImpassableSmallG;
				g->DrawSubClip(0, 0, g->Width, g->Height, x, y);
			}
			if (flags & MapFieldWaterAllowed) {
				g = WaterSmallG;
				g->DrawSubClip(0, 0, g->Width, g->Height, x + 16, y);
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
		i->G->DrawSubClip(0, 0, i->G->Width, i->G->Height, i->X, i->Y);

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

static void PatchEditorUpdateDisplay()
{
	// Patch area
	DrawPatch();
	DrawPatchTileIcons();
	DrawGrids();

	// Menu area
	Video.FillRectangle(ColorGray, Video.Width - PatchMenuWidth, 0, PatchMenuWidth, Video.Height);
	DrawIcons();

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
		Map.PatchManager.savePatchType(&file, Patch->getType());
		file.close();
		// TODO: show success
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
	delete exitButton;
}

static void AddIcon(int x, int y, CGraphic *g, PatchButton b)
{
	PatchIcon icon;
	icon.X = x;
	icon.Y = y;
	icon.G = g;
	icon.Button = b;
	PatchIcons.push_back(icon);
}

static void LoadIcons()
{
	std::string patchEditorPath = "ui/patcheditor/";

	ImpassableG = CGraphic::New(patchEditorPath + "impassable.png");
	ImpassableG->Load();
	ImpassableSmallG = CGraphic::New(patchEditorPath + "impassable-small.png");
	ImpassableSmallG->Load();
	AddIcon(Video.Width - PatchMenuWidth + 15, 100, ImpassableG, ButtonImpassable);
	FlagMap[ButtonImpassable] = MapFieldUnpassable;

	WaterG = CGraphic::New(patchEditorPath + "water.png");
	WaterG->Load();
	WaterSmallG = CGraphic::New(patchEditorPath + "water-small.png");
	WaterSmallG->Load();
	AddIcon(Video.Width - 48 - 15, 100, WaterG, ButtonWater);
	FlagMap[ButtonWater] = MapFieldWaterAllowed;
}

static void FreeIcons()
{
	CGraphic::Free(ImpassableG);
	ImpassableG = NULL;
	CGraphic::Free(ImpassableSmallG);
	ImpassableSmallG = NULL;

	CGraphic::Free(WaterG);
	WaterG = NULL;
	CGraphic::Free(WaterSmallG);
	WaterSmallG = NULL;

	PatchIcons.clear();
	FlagMap.clear();
}

void StartPatchEditor(const std::string &patchName)
{
	Patch = Map.PatchManager.add(patchName, 0, 0);
	if (!Patch) {
		fprintf(stderr, "Invalid patch name: %s\n", patchName.c_str());
		return;
	}

	LoadIcons();

	PatchEditorMainLoop();

	FreeIcons();

	Map.PatchManager.clear();
}
