//       _________ __                 __                               
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/ 
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   Stratagus - A free fantasy real time strategy game engine
//
/**@name menus_defs.inc	-	The menu and buttons definitions. */
//
//	(c) Copyright 1999-2002 by Andreas Arens
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
//	$Id$

//@{

/*----------------------------------------------------------------------------
--	Definitions
----------------------------------------------------------------------------*/

#ifdef SAVE_MENU_CCL	// {

typedef char char30[30];
global hashtable(char30,MENUS_MAXFUNC) MenuFuncHash2;

#undef HASHADD
#define HASHADD(x,y) { \
    *(void **)hash_add(MenuFuncHash,(y)) = (void *)(x); \
    sprintf(buf,"%p",(x)); \
    strcpy((char*)hash_add(MenuFuncHash2,buf), (y)); \
}

local char *MenuNames[] = {
    "menu-game",
    "menu-victory",
    "menu-defeated",
    "menu-select-scenario",
    "menu-program-start",
    "menu-global-options",
    "menu-custom-game",
    "menu-enter-name",
    "menu-create-join-menu",
    "menu-multi-setup",
    "menu-enter-server",
    "menu-net-multi-client",
    "menu-net-connecting",
    "menu-campaign-select",
    "menu-campaign-continue",
    "menu-objectives",
    "menu-end-scenario",
    "menu-sound-options",
    "menu-preferences",
    "menu-settings-speed",
    "menu-game-options",
    "menu-net-error",
    "menu-tips",
    "menu-help",
    "menu-keystroke-help",
    "menu-save-game",
    "menu-load-game",
    "menu-save-confirm",
    "menu-delete-confirm",
    "menu-editor-select",
    "menu-editor-new",
    "menu-editor-main-load-map",
    "menu-editor",
    "menu-editor-map-properties",
    "menu-editor-player-properties",
    "menu-editor-tips",
    "menu-editor-edit-resource",
    "menu-editor-error",
    "menu-editor-edit-ai-properties",
    "menu-editor-save",
    "menu-editor-save-confirm",
    "menu-diplomacy",
};

local char *MenuFlags[] = {
    "",
    "'flags '(active)",
    "'flags '(clicked)",
    "",
    "'flags '(selected)",
    "",
    "",
    "",
    "'flags '(disabled)",
};

local char *MenuImages[] = {
    "none",
    "panel1",
    "panel2",
    "panel3",
    "panel4",
    "panel5",
};

/**
**	FIXME: should use the names of the real fonts.
*/
local char *MenuFontNames[] = {
    "small",
    "game",
    "large",
    "small-title",
    "large-title",
};

local char *MenuTextFlags[] = {
    "none",
    "center",
    "right",
    "",
    "left",
};

local char *MenuGemState[] = {
    "unchecked",
    "passive",
    "invisible",
    "",
    "checked",
};

local char *MenuHotKeyToStr(int key, char *buf)
{
    if (!key) {
	buf[0] = '\0';
    } else if (('a' <= key && key <= 'z') || ('A' <= key && key <= 'Z')) {
	buf[0] = key;
	buf[1] = '\0';
    } else if (KeyCodeF1 <= key && key <= KeyCodeF12) {
	buf[0] = 'f';
	if (key >= KeyCodeF10) {
	    buf[1] = '1';
	    buf[2] = key-10-KeyCodeF1+1 + '0';
	    buf[3] = '\0';
	} else {
	    buf[1] = key-KeyCodeF1+1 + '0';
	    buf[2] = '\0';
	}
    } else if (key == '\033') {
	strcpy(buf,"esc");
    }
    return buf;
}

/**
**	Save state of units to file.
**
**	@param file	Output file.
**
**	@todo Better readable formating of output.
*/
global void SaveMenus(FILE* file)
{
    Menu *menu;
    int i;
    int j;
    int n;
    int offsetX;
    int offsetY;
    char hotkey[10];
    char func[10];
    char func2[10];
    char func3[10];
    char initfunc[40];
    char exitfunc[40];
    char netaction[40];

    offsetX = (VideoWidth - 640) / 2;
    offsetY = (VideoHeight - 480) / 2;

    fprintf(file,"\n;;; -----------------------------------------\n");
    fprintf(file,";;; $Id$\n\n");

    for (i=0; i<sizeof(MenuNames)/sizeof(*MenuNames); ++i) {
	menu = FindMenu(MenuNames[i]);
	fprintf(file,";;\n;; %s\n;;\n", MenuNames[i]);
	if (menu->netaction) {
	    sprintf(func,"%p",menu->netaction);
	    sprintf(netaction," 'netaction '%s",(char*)hash_find(MenuFuncHash2,func));
	} else {
	    netaction[0] = '\0';
	}
	fprintf(file,"(define-menu 'name '%s 'geometry '(%d %d %d %d)\n"
		     "    'image '%s 'default '%d%s)\n",
		     MenuNames[i],
		     menu->x - offsetX, menu->y - offsetY,
		     menu->xsize, menu->ysize,
		     MenuImages[menu->image],
		     menu->defsel,
		     netaction);
	for (j=0; j<menu->nitems; ++j) {
	    if (menu->items[j].initfunc) {
		sprintf(func,"%p",menu->items[j].initfunc);
		sprintf(initfunc," 'init '%s",(char*)hash_find(MenuFuncHash2,func));
	    } else {
		initfunc[0] = '\0';
	    }
	    if (menu->items[j].exitfunc) {
		sprintf(func,"%p",menu->items[j].exitfunc);
		sprintf(exitfunc," 'exit '%s",(char*)hash_find(MenuFuncHash2,func));
	    } else {
		exitfunc[0] = '\0';
	    }

	    fprintf(file,"(define-menu-item 'pos (list %d %d) 'font '%s %s%s%s\n",
			 menu->items[j].xofs, menu->items[j].yofs,
			 MenuFontNames[menu->items[j].font],
			 MenuFlags[menu->items[j].flags],
			 initfunc,
			 exitfunc);
	    switch (menu->items[j].mitype) {
		case MI_TYPE_TEXT:
		    if (menu->items[j].d.text.text) {
			fprintf(file,"    'text '(\"%s\" %s)\n",
			    (char*)menu->items[j].d.text.text,
			    MenuTextFlags[menu->items[j].d.text.tflags]);
		    } else {
			fprintf(file,"    'text '(() %s)\n",
			    MenuTextFlags[menu->items[j].d.text.tflags]);
		    }
		    break;
		case MI_TYPE_BUTTON:
		    sprintf(func,"%p",menu->items[j].d.button.handler);
		    // FIXME: Johns "null" is not good, use nil or ()
		    //		or use no style item.
		    fprintf(file,"    'button '(size (%d %d)\n"
			         "            caption %c%s%c\n"
				 "            hotkey \"%s\"\n"
				 "            func %s\n"
				 "            style %s)\n",
				 menu->items[j].d.button.xsize,
				 menu->items[j].d.button.ysize,
				 menu->items[j].d.button.text ? '"' : '(',
				 menu->items[j].d.button.text
				    ?  menu->items[j].d.button.text
				    : (unsigned char*)NULL,
				 menu->items[j].d.button.text ? '"' : ')',
				 MenuHotKeyToStr(menu->items[j].d.button.hotkey,
					hotkey),
				 (char*)hash_find(MenuFuncHash2,func),
				 MenuButtonStyle(menu->items[j].d.button.button));
		    break;
		case MI_TYPE_PULLDOWN:
		    sprintf(func,"%p",menu->items[j].d.pulldown.action);
		    fprintf(file,"    'pulldown '(size (%d %d)\n"
			         "              style %s\n"
				 "              func %s\n",
				 menu->items[j].d.pulldown.xsize,
				 menu->items[j].d.pulldown.ysize,
				 MenuButtonStyle(menu->items[j].d.pulldown.button),
				 (char*)hash_find(MenuFuncHash2,func));
		    fprintf(file,"              options (");
		    for (n=0; n<menu->items[j].d.pulldown.noptions; ++n) {
			fprintf(file,"\"%s\" ", menu->items[j].d.pulldown.options[n]);
		    }

		    fprintf(file,")\n");
		    if (menu->items[j].d.pulldown.state & MI_PSTATE_PASSIVE) {
			fprintf(file,"              state passive\n");
		    }
		    fprintf(file,"              default %d\n"
				 "              current %d)\n",
				 menu->items[j].d.pulldown.defopt,
				 menu->items[j].d.pulldown.curopt);
		    break;
		case MI_TYPE_LISTBOX:
		    sprintf(func,"%p",menu->items[j].d.listbox.action);
		    sprintf(func2,"%p",menu->items[j].d.listbox.retrieveopt);
		    sprintf(func3,"%p",menu->items[j].d.listbox.handler);
		    fprintf(file,"    'listbox '(size (%d %d)\n"
				 "             style %s\n"
				 "             func %s\n"
				 "             retopt %s\n"
				 "             handler %s\n"
				 "             nlines %d)\n",
				 menu->items[j].d.listbox.xsize,
				 menu->items[j].d.listbox.ysize,
				 MenuButtonStyle(menu->items[j].d.listbox.button),
				 (char*)hash_find(MenuFuncHash2,func),
				 (char*)hash_find(MenuFuncHash2,func2),
				 (char*)hash_find(MenuFuncHash2,func3),
				 menu->items[j].d.listbox.nlines);
		    break;
		case MI_TYPE_VSLIDER:
		    sprintf(func,"%p",menu->items[j].d.vslider.action);
		    sprintf(func2,"%p",menu->items[j].d.vslider.handler);
		    fprintf(file,"    'vslider '(size (%d %d)\n"
			         "             func %s\n"
				 "             handler %s)\n",
				 menu->items[j].d.vslider.xsize,
				 menu->items[j].d.vslider.ysize,
				 (char*)hash_find(MenuFuncHash2,func),
				 (char*)hash_find(MenuFuncHash2,func2));
		    break;
		case MI_TYPE_DRAWFUNC:
		    sprintf(func,"%p",menu->items[j].d.drawfunc.draw);
		    fprintf(file,"    'drawfunc '%s\n",
			         (char*)hash_find(MenuFuncHash2,func));
		    break;
		case MI_TYPE_INPUT:
		    sprintf(func,"%p",menu->items[j].d.input.action);
		    fprintf(file,"    'input '(size (%d %d)\n"
			         "           func %s\n"
				 "           style %s)\n",
				 menu->items[j].d.input.xsize,
				 menu->items[j].d.input.ysize,
				 (char*)hash_find(MenuFuncHash2,func),
				 MenuButtonStyle(menu->items[j].d.input.button));
		    break;
		case MI_TYPE_GEM:
		    sprintf(func,"%p",menu->items[j].d.gem.action);
		    fprintf(file,"    'gem '(size (%d %d)\n"
			         "         state %s\n"
				 "         func %s\n"
				 "         style %s)\n",
				 menu->items[j].d.gem.xsize,
				 menu->items[j].d.gem.ysize,
				 MenuGemState[menu->items[j].d.gem.state],
				 (char*)hash_find(MenuFuncHash2,func),
				 MenuButtonStyle(menu->items[j].d.gem.button));
		    break;
		case MI_TYPE_HSLIDER:
		    sprintf(func,"%p",menu->items[j].d.hslider.action);
		    sprintf(func2,"%p",menu->items[j].d.hslider.handler);
		    fprintf(file,"    'hslider '(size (%d %d)\n"
			         "             func %s\n"
				 "             handler %s)\n",
				 menu->items[j].d.hslider.xsize,
				 menu->items[j].d.hslider.ysize,
				 (char*)hash_find(MenuFuncHash2,func),
				 (char*)hash_find(MenuFuncHash2,func2));
		    break;
		default:
		    abort();
	    }
	    fprintf(file,"    'menu '%s)\n",MenuNames[i]);
	}
	fprintf(file,"\n\n");
    }
}
#endif	// } SAVE_MENU_CCL


    /// Offsets into NetMultiSetupMenuItems
#define SERVER_PLAYER_STATE	5
    /// Offsets into NetMultiSetupMenuItems
#define SERVER_PLAYER_READY	32
    /// Offsets into NetMultiSetupMenuItems
#define SERVER_PLAYER_LAG	46

    /// Offsets into NetMultiClientMenuItems
#define CLIENT_PLAYER_STATE	5
    /// Offsets into NetMultiClientMenuItems
#define CLIENT_RACE		21
    /// Offsets into NetMultiClientMenuItems
#define CLIENT_RESOURCE		23
    /// Offsets into NetMultiClientMenuItems
#define CLIENT_UNITS		25
    /// Offsets into NetMultiClientMenuItems
#define CLIENT_FOG_OF_WAR	27
    /// Offsets into NetMultiClientMenuItems
#define CLIENT_TILESET		29
	/// Offsets into NetMultiClientMenuItems
#define CLIENT_GAMETYPE		31
    /// Offsets into NetMultiClientMenuItems
#define CLIENT_PLAYER_READY	32

//@}
