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


static void PatchEditorCallbackButtonDown(unsigned button)
{
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
	HandleCursorMove(&x, &y); // Reduce to screen

	GameCursor = UI.Point.Cursor;

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
	g->DrawSubClip(0, 0, g->Width, g->Height, 0, 0);
}

static void DrawGrids()
{
	int width = Patch->getType()->getTileWidth();
	int height = Patch->getType()->getTileHeight();
	int i;

	for (i = 1; i < width; ++i) {
		Video.DrawVLine(ColorBlack, i * TileSizeX, 0, height * TileSizeY);
	}
	for (i = 1; i < height; ++i) {
		Video.DrawHLine(ColorBlack, 0, i * TileSizeY, width * TileSizeX);
	}
}

static void PatchEditorUpdateDisplay()
{
	DrawPatch();
	DrawGrids();

	Video.FillRectangle(ColorGray, Video.Width - PatchMenuWidth, 0, PatchMenuWidth, Video.Height);

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
	}
};

class PatchExitButtonListener : public gcn::ActionListener
{
public:
	virtual void action(const std::string &eventId) {
		PatchEditorRunning = false;
	}
};

static void PatchEditorMainLoop()
{
	const EventCallback *old_callbacks = GetCallbacks();
	EventCallback callbacks;
	gcn::Button *saveButton;
	gcn::Button *exitButton;
	PatchSaveButtonListener *saveButtonListener;
	PatchExitButtonListener *exitButtonListener;

	callbacks.ButtonPressed = PatchEditorCallbackButtonDown;
	callbacks.ButtonReleased = PatchEditorCallbackButtonUp;
	callbacks.MouseMoved = PatchEditorCallbackMouse;
	callbacks.MouseExit = PatchEditorCallbackExit;
	callbacks.KeyPressed = PatchEditorCallbackKeyDown;
	callbacks.KeyReleased = PatchEditorCallbackKeyUp;
	callbacks.KeyRepeated = PatchEditorCallbackKeyRepeated;
	callbacks.NetworkEvent = NetworkEvent;

	SetCallbacks(&callbacks);

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

		PatchEditorUpdateDisplay();

		WaitEventsOneFrame();
	}

	Map.PatchManager.clear();

	SetCallbacks(old_callbacks);

	Gui->setTop(oldTop);
	delete PatchEditorContainer;
	delete saveButton;
	delete exitButton;
}

void StartPatchEditor(const std::string &patchName)
{
	Patch = Map.PatchManager.add(patchName, 0, 0);
	if (!Patch) {
		fprintf(stderr, "Invalid patch name: %s\n", patchName.c_str());
		return;
	}

	PatchEditorMainLoop();
}
