//   ___________		     _________		      _____  __
//   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
//    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \\   __\\   __\ 
//    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
//    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
//	  \/		    \/	   \/	     \/		   \/
//  ______________________                           ______________________
//			  T H E   W A R   B E G I N S
//	   FreeCraft - A free fantasy real time strategy game engine
//
/**@name X11.c		-	XWindows support. */
/*
**	(c) Copyright 1998-2000 by Lutz Sammer and Valery Shchedrin
**
**	$Id$
*/

//@{

#ifdef USE_X11

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>

#include <errno.h>

#include "freecraft.h"
#include "video.h"
#include "tileset.h"
#include "sound_id.h"
#include "unitsound.h"
#include "unittype.h"
#include "player.h"
#include "unit.h"
#include "map.h"
#include "minimap.h"
#include "font.h"
#include "image.h"
#include "sound_server.h"
#include "missile.h"
#include "sound.h"
#include "cursor.h"
#include "interface.h"
#include "network.h"
#include "ui.h"
#include "new_video.h"

/**
**	Architecture-dependant video memory. Set by InitVideoXXX.
**
**	@see InitVideo
*/
global void* VideoMemory;

/**
**	Architecture-dependant video depth. Set by InitVideoXXX.
**
**	@see InitVideo
*/
global int VideoDepth;

global VMemType8 * Pixels8;
global VMemType16 * Pixels16;
global VMemType32 * Pixels32;
global Palette GlobalPalette[256];

local Display* TheDisplay;
local int TheScreen;
local Pixmap TheMainDrawable;
local Window TheMainWindow;
local GC GcLine;

    /// Atom for WM_DELETE_WINDOW
local Atom WmDeleteWindowAtom;


/*----------------------------------------------------------------------------
--	Sync
----------------------------------------------------------------------------*/

global int VideoSyncSpeed=100;		// 0 disable interrupts
volatile int VideoInterrupts;		// be happy, were are quicker

/**
**	Called from SIGALRM.
*/
local void VideoSyncHandler(int unused)
{
    DebugLevel3("Interrupt\n");
    ++VideoInterrupts;
}

/**
**	Initialise video sync.
*/
global void InitVideoSync(void)
{
    struct sigaction sa;
    struct itimerval itv;

    if( !VideoSyncSpeed ) {
	return;
    }

    sa.sa_handler=VideoSyncHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags=SA_RESTART;
    if( sigaction(SIGALRM,&sa,NULL) ) {
	fprintf(stderr,"Can't set signal\n");
    }

    itv.it_interval.tv_sec=itv.it_value.tv_sec=
	(100/FRAMES_PER_SECOND)/VideoSyncSpeed;
    itv.it_interval.tv_usec=itv.it_value.tv_usec=
	(100000000/FRAMES_PER_SECOND)/VideoSyncSpeed-
	itv.it_value.tv_sec*100000;
    if( setitimer(ITIMER_REAL,&itv,NULL) ) {
	fprintf(stderr,"Can't set itimer\n");
    }

    // DebugLevel1("Timer installed\n");
}

/**
**	Watch opening/closing of X11 connections
*/
local void MyConnectionWatch
	(Display* display,XPointer client,int fd,Bool flag,XPointer* data)
{
    DebugLevel0(__FUNCTION__": fildes %d flag %d\n",fd,flag);
    if( flag ) {			// file handle opened
    } else {				// file handle closed
    }
}

/**
**	X11 initialize.
*/
global void GameInitDisplay(void)
{
    int i;
    Window window;
    XGCValues gcvalue;
    XSizeHints hints;
    XWMHints wmhints;
    XClassHint classhint;
    XSetWindowAttributes attributes;
    int shm_major,shm_minor;
    Bool pixmap_support;
    XShmSegmentInfo shminfo;
    XVisualInfo xvi;
    XPixmapFormatValues *xpfv;

    if( !(TheDisplay=XOpenDisplay(NULL)) ) {
	fprintf(stderr,"Cannot connect to X-Server.\n");
	exit(-1);
    }

    TheScreen=DefaultScreen(TheDisplay);

    //	I need shared memory pixmap extension.

    if( !XShmQueryVersion(TheDisplay,&shm_major,&shm_minor,&pixmap_support) ) {
	fprintf(stderr,"SHM-Extensions required.\n");
	exit(-1);
    }
    if( !pixmap_support ) {
	fprintf(stderr,"SHM-Extensions with pixmap supported required.\n");
	exit(-1);
    }

    //  Look for a nice visual

    if( VideoDepth && XMatchVisualInfo(TheDisplay,
	    TheScreen,VideoDepth,TrueColor,&xvi) )
	goto foundvisual;
    if(XMatchVisualInfo(TheDisplay, TheScreen, 16, TrueColor, &xvi))
	goto foundvisual;
    if(XMatchVisualInfo(TheDisplay, TheScreen, 15, TrueColor, &xvi))
	goto foundvisual;
    if(XMatchVisualInfo(TheDisplay, TheScreen, 24, TrueColor, &xvi))
	goto foundvisual;
    if(XMatchVisualInfo(TheDisplay, TheScreen, 8, TrueColor, &xvi))
	goto foundvisual;
    if(XMatchVisualInfo(TheDisplay, TheScreen, 8, PseudoColor, &xvi))
	goto foundvisual;
    fprintf(stderr,"Sorry, I couldn't find an 8, 15 , 16 or 24 bit visual.\n");
    exit(-1);

foundvisual:

    xpfv=XListPixmapFormats(TheDisplay, &i);
    for(i--;i>=0;i--)  {
	DebugLevel0("pixmap %d\n", xpfv[i].depth);
	if(xpfv[i].depth==xvi.depth)
	    break;
    }
    if(i<0)  {
	fprintf(stderr,"No Pixmap format for visual depth?\n");
	exit(-1);
    }
    if( !VideoDepth ) {
	VideoDepth=xvi.depth;
    }

    if( !VideoWidth ) {
	VideoWidth = DEFAULT_VIDEO_WIDTH;
	VideoHeight = DEFAULT_VIDEO_HEIGHT;
    }

    shminfo.shmid=shmget(IPC_PRIVATE,
	    (VideoWidth*xpfv[i].bits_per_pixel+xpfv[i].scanline_pad-1) /
	    xpfv[i].scanline_pad * xpfv[i].scanline_pad * VideoHeight / 8,
	    IPC_CREAT|0777);

    XFree(xpfv);

    if( !shminfo.shmid==-1 ) {
	fprintf(stderr,"shmget failed.\n");
	exit(-1);
    }
    VideoMemory=shminfo.shmaddr=shmat(shminfo.shmid,0,0);
    if( shminfo.shmaddr==(void*)-1 ) {
	shmctl(shminfo.shmid,IPC_RMID,0);
	fprintf(stderr,"shmat failed.\n");
	exit(-1);
    }
    shminfo.readOnly=False;

    if( !XShmAttach(TheDisplay,&shminfo) ) {
	shmctl(shminfo.shmid,IPC_RMID,0);
	fprintf(stderr,"XShmAttach failed.\n");
	exit(-1);
    }
    // Mark segment as deleted as soon as both clone and the X server have
    // attached to it.  The POSIX spec says that a segment marked as deleted
    // can no longer have addition processes attach to it, but Linux will let
    // them anyway.
    shmctl(shminfo.shmid,IPC_RMID,0);

    TheMainDrawable=attributes.background_pixmap=
	    XShmCreatePixmap(TheDisplay,DefaultRootWindow(TheDisplay)
		,shminfo.shmaddr,&shminfo
		,VideoWidth,VideoHeight
		,xvi.depth);
    attributes.cursor = XCreateFontCursor(TheDisplay,XC_tcross-1);
    attributes.backing_store = NotUseful;
    attributes.save_under = False;
    attributes.event_mask = KeyPressMask|KeyReleaseMask|/*ExposureMask|*/
	FocusChangeMask|ButtonPressMask|PointerMotionMask|ButtonReleaseMask;
    i = CWBackPixmap|CWBackingStore|CWSaveUnder|CWEventMask|CWCursor;

    if(xvi.class==PseudoColor)  {
	i|=CWColormap;
	attributes.colormap =
	    XCreateColormap( TheDisplay, xvi.screen, xvi.visual, AllocNone);
	// FIXME:  Really should fill in the colormap right now
    }
    window=XCreateWindow(TheDisplay,DefaultRootWindow(TheDisplay)
	    ,0,0,VideoWidth,VideoHeight,3
	    ,xvi.depth,InputOutput,xvi.visual ,i,&attributes);
    TheMainWindow=window;

    gcvalue.graphics_exposures=False;
    GcLine=XCreateGC(TheDisplay,window,GCGraphicsExposures,&gcvalue);

    //
    //	Clear initial window.
    //
    XSetForeground(TheDisplay,GcLine,BlackPixel(TheDisplay,TheScreen));
    XFillRectangle(TheDisplay,TheMainDrawable,GcLine,0,0
	    ,VideoWidth,VideoHeight);

    WmDeleteWindowAtom=XInternAtom(TheDisplay,"WM_DELETE_WINDOW",False);

    //
    //	Set some usefull min/max sizes as well as a 1.3 aspect
    //
#if 0
    if( geometry ) {
	hints.flags=0;
	f=XParseGeometry(geometry
		,&hints.x,&hints.y,&hints.width,&hints.height);

	if( f&XValue ) {
	    if( f&XNegative ) {
		hints.x+=DisplayWidth-hints.width;
	    }
	    hints.flags|=USPosition;
	    // FIXME: win gravity
	}
	if( f&YValue ) {
	    if( f&YNegative ) {
		hints.y+=DisplayHeight-hints.height;
	    }
	    hints.flags|=USPosition;
	    // FIXME: win gravity
	}
	if( f&WidthValue ) {
	    hints.flags|=USSize;
	}
	if( f&HeightValue ) {
	    hints.flags|=USSize;
	}
    } else {
#endif
	hints.width=VideoWidth;
	hints.height=VideoHeight;
	hints.flags=PSize;
#if 0
    }
#endif
    hints.min_width=VideoWidth;
    hints.min_height=VideoHeight;
    hints.max_width=VideoWidth;
    hints.max_height=VideoHeight;
    hints.min_aspect.x=4;
    hints.min_aspect.y=3;

    hints.max_aspect.x=4;
    hints.max_aspect.y=3;
    hints.width_inc=4;
    hints.height_inc=3;

    hints.flags|=PMinSize|PMaxSize|PAspect|PResizeInc;

    wmhints.input=True;
    wmhints.initial_state=NormalState;
    wmhints.window_group=window;
    wmhints.flags=InputHint|StateHint|WindowGroupHint;

    classhint.res_name="freecraft";
    classhint.res_class="FreeCraft";

    XSetStandardProperties(TheDisplay,window
	,"FreeCraft (formerly known as ALE Clone)"
	,"FreeCraft",None,(char**)0,0,&hints);
    XSetClassHint(TheDisplay,window,&classhint);
    XSetWMHints(TheDisplay,window,&wmhints);

    XSetWMProtocols(TheDisplay,window,&WmDeleteWindowAtom,1);

    XMapWindow(TheDisplay,window);

    //
    //	Input handling.
    //
    XAddConnectionWatch(TheDisplay,MyConnectionWatch,NULL);

    XFlush(TheDisplay);
}

/**
**	Change video mode to new width.
*/
global int SetVideoMode(int width)
{
    if (width == 640) return 1;
    return 0;
}

/**
**	Invalidate some area
*/
global void InvalidateArea(int x,int y,int w,int h)
{
    // FIXME: This checks should be done at hight level
    if( x<0 ) {
	w+=x;
	x=0;
    }
    if( y<0 ) {
	h+=y;
	y=0;
    }
    if( !w<=0 && !h<=0 ) {
	DebugLevel3("X %d,%d -> %d,%d\n",x,y,w,h);
	XClearArea(TheDisplay,TheMainWindow,x,y,w,h,False);
    }
}

/**
**	Invalidate whole window
*/
global void Invalidate(void)
{
    XClearWindow(TheDisplay,TheMainWindow);
}

/**
**      Handle keyboard modifiers
*/
local void X11HandleModifiers(XKeyEvent* keyevent)
{
    int mod=keyevent->state;

    // Here we use an ideous hack to avoid X keysyms mapping.
    // What we need is to know that the player hit key 'x' with
    // the control modifier; we don't care if he typed `key mapped
    // on Ctrl-x` with control modifier...
    // Note that we don't use this hack for "shift", because shifted
    // keys can be useful (to get numbers on my french keybord
    // for exemple :)).
    if( mod&ShiftMask ) {
	    /* Do Nothing */;
    }
    if( mod&ControlMask ) {
        keyevent->state&=~ControlMask;  // Hack Attack!
    }
    if( mod&Mod1Mask ) {
        keyevent->state&=~Mod1Mask;     // Hack Attack!
    }
}

/**
**	Handle keyboard!
*/
local void X11HandleKey(KeySym code)
{
    int icode;

    /*
    **	Convert X11 keycodes into internal keycodes.
    */
    switch( (icode=code) ) {
	case XK_Escape:
	    icode='\e';
	    break;
	case XK_Return:
	    icode='\r';
	    break;
	case XK_BackSpace:
	    icode='\b';
	    break;
	case XK_Tab:
	    icode='\t';
	    break;
	case XK_Up:
	    icode=KeyCodeUp;
	    break;
	case XK_Down:
	    icode=KeyCodeDown;
	    break;
	case XK_Left:
	    icode=KeyCodeLeft;
	    break;
	case XK_Right:
	    icode=KeyCodeRight;
	    break;
	case XK_Pause:
	    icode=KeyCodePause;
	    break;
	case XK_F1:
	    icode=KeyCodeF1;
	    break;
	case XK_F2:
	    icode=KeyCodeF2;
	    break;
	case XK_F3:
	    icode=KeyCodeF3;
	    break;
	case XK_F4:
	    icode=KeyCodeF4;
	    break;
	case XK_F5:
	    icode=KeyCodeF5;
	    break;
	case XK_F6:
	    icode=KeyCodeF6;
	    break;
	case XK_F7:
	    icode=KeyCodeF7;
	    break;
	case XK_F8:
	    icode=KeyCodeF8;
	    break;
	case XK_F9:
	    icode=KeyCodeF9;
	    break;
	case XK_F10:
	    icode=KeyCodeF10;
	    break;
	case XK_F11:
	    icode=KeyCodeF11;
	    break;
	case XK_F12:
	    icode=KeyCodeF12;
	    break;
        // We need these because if you only hit a modifier key,
        // X doesn't set its state (modifiers) field in the keyevent.
	case XK_Shift_L:
	case XK_Shift_R:
	    icode = KeyCodeShift;
	    break;
	case XK_Control_L:
	case XK_Control_R:
	    icode = KeyCodeControl;
	    break;
	case XK_Alt_L:
	case XK_Alt_R:
	case XK_Meta_L:
	case XK_Meta_R:
	    icode = KeyCodeAlt;
	    break;
	case XK_Super_L:
	case XK_Super_R:
	    icode = KeyCodeSuper;
	    break;
	case XK_Hyper_L:
	case XK_Hyper_R:
	    icode = KeyCodeHyper;
	    break;
	default:
	    break;
    }
    if( HandleKeyDown(icode) ) {
	return;
    }
    DoButtonPanelKey(icode);
}

/**
**	Handle keyboard! (release)
*/
local void X11HandleKeyUp(KeySym code)
{
    int icode;

    switch( (icode=code) ) {
	case XK_Shift_L:
	case XK_Shift_R:
	    icode = KeyCodeShift;
	    break;
	case XK_Control_L:
	case XK_Control_R:
	    icode = KeyCodeControl;
	    break;
	case XK_Alt_L:
	case XK_Alt_R:
	case XK_Meta_L:
	case XK_Meta_R:
	    icode = KeyCodeAlt;
	    break;
	case XK_Super_L:
	case XK_Super_R:
	    icode = KeyCodeHyper;
	    break;

	case XK_Up:
	    icode = KeyCodeUp;
	    break;
	case XK_Down:
	    icode = KeyCodeDown;
	    break;
	case XK_Left:
	    icode = KeyCodeLeft;
	    break;
	case XK_Right:
	    icode = KeyCodeRight;
	    break;

	default:
	    DebugLevel3("\tUnknown key %x\n",code);
	    break;
    }

    HandleKeyUp(icode);
}

/**
**	Handle interactive input event.
*/
local void DoEvent(void)
{
    XEvent event;
    int xw, yw;

    XNextEvent(TheDisplay,&event);

    switch( event.type ) {
	case ButtonPress:
	    DebugLevel3("\tbutton press %d\n",event.xbutton.button);
	    HandleButtonDown(event.xbutton.button);
	    break;

	case ButtonRelease:
	    DebugLevel3("\tbutton release %d\n",event.xbutton.button);
	    HandleButtonUp(event.xbutton.button);
	    break;

	case Expose:
	    DebugLevel1("\texpose\n");
	    MustRedraw=-1;
	    break;

	case MotionNotify:
	    DebugLevel3("\tmotion notify %d,%d\n"
		,event.xbutton.x,event.xbutton.y);
	    HandleMouseMove(event.xbutton.x,event.xbutton.y);
	    if ( (TheUI.WarpX != -1 || TheUI.WarpY != -1)
		    && (event.xbutton.x!=TheUI.WarpX
			 || event.xbutton.y!=TheUI.WarpY)
		    ) {
		xw = TheUI.WarpX;
		yw = TheUI.WarpY;
		TheUI.WarpX = -1;
		TheUI.WarpY = -1;

		XWarpPointer(TheDisplay,TheMainWindow,TheMainWindow,0,0
			,0,0,xw,yw);
	    }
	    MustRedraw|=RedrawCursor;
	    break;

	case FocusIn:
	    DebugLevel3("\tfocus in\n");
	    break;

	case FocusOut:
	    DebugLevel3("\tfocus out\n");
	    CursorOn=-1;
	    break;

	case ClientMessage:
	    DebugLevel3("\tclient message\n");
            if (event.xclient.format == 32) {
                if ((Atom)event.xclient.data.l[0] == WmDeleteWindowAtom) {
		    Exit(0);
		}
	    }
	    break;

	case KeyPress:
	    DebugLevel3("\tKey press\n");
{
	    char buf[128];
	    int num;
	    KeySym keysym;

            X11HandleModifiers((XKeyEvent*)&event);
	    num=XLookupString((XKeyEvent*)&event,buf,sizeof(buf),&keysym,0);
	    DebugLevel3("\tKey %lx `%s'\n",keysym,buf);
	    if( num==1 ) {
		X11HandleKey(*buf);
	    } else {
		X11HandleKey(keysym);
	    }
}
	    break;

	case KeyRelease:
	    DebugLevel3("\tKey release\n");
	    X11HandleKeyUp(XLookupKeysym((XKeyEvent*)&event,0));
	    break;

	default:
	    DebugLevel0("\tUnkown event\n");
	    break;
    }
}

/**
**	Wait for interactive input event.
**
**	Handles X11 events, keyboard, mouse.
**	Video interrupt for sync.
**	Network messages.
**	Sound queue.
**
**	We must handle atlast one X11 event
**
**	FIXME:	the initialition could be moved out of the loop
*/
global void WaitEventsAndKeepSync(void)
{
    struct timeval tv;
    fd_set rfds;
    fd_set wfds;
    int maxfd;
    int* xfd;
    int n;
    int i;
    int morex;
    int connection;

    connection=ConnectionNumber(TheDisplay);

    for( ;; ) {
#ifdef SLOW_INPUT
	while( XPending(TheDisplay) ) {
	   DoEvent();
	}
#endif

	//
	//	Prepare select
	//
	tv.tv_sec=tv.tv_usec=0;

	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	maxfd=0;

	//
	//	X11 how many events already in queue
	//
	xfd=NULL;
	morex=QLength(TheDisplay);
	if( !morex ) {
	    //
	    //	X11 connections number
	    //
	    maxfd=connection;
	    FD_SET(connection,&rfds);

	    //
	    //	Get all X11 internal connections
	    //
	    if( !XInternalConnectionNumbers(TheDisplay,&xfd,&n) ) {
		DebugLevel0(__FUNCTION__": out of memory\n");
		abort();
	    }
	    for( i=n; i--; ) {
		FD_SET(xfd[i],&rfds);
		if( xfd[i]>maxfd ) {
		    maxfd=xfd[i];
		}
	    }
	}

	//
	//	Sound
	//
	if( !SoundOff && !SoundThreadRunning ) {
	    if( SoundFildes>maxfd ) {
		maxfd=SoundFildes;
	    }
	    FD_SET(SoundFildes,&wfds);
	}

	//
	//	Network
	//
	if( NetworkFildes!=-1 ) {
	    if( NetworkFildes>maxfd ) {
		maxfd=NetworkFildes;
	    }
	    FD_SET(NetworkFildes,&rfds);
	    if( !NetworkInSync ) {
		NetworkRecover();	// recover network
	    }
	}

	maxfd=select(maxfd+1,&rfds,&wfds,NULL
		,(morex|VideoInterrupts) ? &tv : NULL);

	//
	//	X11
	//
	if( maxfd>0 ) {
	    if( !morex ) {		// look if new events
		if (xfd) {
		    for( i=n; i--; ) {
			if( FD_ISSET(xfd[i],&rfds) ) {
			    XProcessInternalConnection(TheDisplay,xfd[i]);
			}
		    }
		}
		if( FD_ISSET(connection,&rfds) ) {
		    morex=XEventsQueued(TheDisplay,QueuedAfterReading);
		} else {
		    morex=QLength(TheDisplay);
		}
	    }
	}
	if( xfd) {
	    XFree(xfd);
	}

	for( i=morex; i--; ) {		// handle new + *OLD* x11 events
	    DoEvent();
	}

	if( maxfd>0 ) {
	    //
	    //	Sound
	    //
	    if( !SoundOff && !SoundThreadRunning
			&& FD_ISSET(SoundFildes,&wfds) ) {
		WriteSound();
	    }

	    //
	    //	Network in sync and time for frame over: return
	    //
	    if( !morex && NetworkInSync && VideoInterrupts ) {
		return;
	    }

	    //
	    //	Network
	    //
	    if( NetworkFildes!=-1 && FD_ISSET(NetworkFildes,&rfds) ) {
		NetworkEvent();
	    }
	}

	//
	//	Network in sync and time for frame over: return
	//
	if( !morex && NetworkInSync && VideoInterrupts ) {
	    return;
	}
    }
}

/**
**	Create a new hardware dependend palette palette.
**
**	@param palette	Hardware independend palette.
**
**	@returns	A hardware dependend pixel table.
*/
global GraphicData * VideoCreateNewPalette(const Palette *palette)
{
    XColor color;
    XWindowAttributes xwa;
    int i;
    void* pixels;

    if( !TheDisplay || !TheMainWindow ) {	// no init
	return NULL;
    }

    switch( VideoDepth ) {
    case 8:
	pixels=malloc(256*sizeof(VMemType8));
	break;
    case 15:
    case 16:
	pixels=malloc(256*sizeof(VMemType16));
	break;
    case 24:
    case 32:
	pixels=malloc(256*sizeof(VMemType32));
	break;
    default:
	DebugLevel0(__FUNCTION__": Unknown depth\n");
	return NULL;
    }

    XGetWindowAttributes(TheDisplay,TheMainWindow,&xwa);

    //
    //	Convert each palette entry into hardware format.
    //
    for( i=0; i<256; ++i ) {
	int r;
	int g;
	int b;
	int v;

	r=(palette[i].r)&0xFF;
	g=(palette[i].g)&0xFF;
	b=(palette[i].b)&0xFF;
	v=r+g+b;

	// Apply global saturation,contrast and brightness
	r= ((((r*3-v)*TheUI.Saturation + v*100)
	    *TheUI.Contrast)
	    +TheUI.Brightness*25600*3)/30000;
	g= ((((g*3-v)*TheUI.Saturation + v*100)
	    *TheUI.Contrast)
	    +TheUI.Brightness*25600*3)/30000;
	b= ((((b*3-v)*TheUI.Saturation + v*100)
	    *TheUI.Contrast)
	    +TheUI.Brightness*25600*3)/30000;

	// Boundings
	r= r<0 ? 0 : r>255 ? 255 : r;
	g= g<0 ? 0 : g>255 ? 255 : g;
	b= b<0 ? 0 : b>255 ? 255 : b;

	// -> Video
	color.red=r<<8;
	color.green=g<<8;
	color.blue=b<<8;
	color.flags=DoRed|DoGreen|DoBlue;
	if( !XAllocColor(TheDisplay,xwa.colormap,&color) ) {
	    fprintf(stderr,"Cannot allocate color\n");
	    exit(-1);
	}

	switch( VideoDepth ) {
	case 8:
	    ((VMemType8*)pixels)[i]=color.pixel;
	    break;
	case 15:
	case 16:
	    ((VMemType16*)pixels)[i]=color.pixel;
	    break;
	case 24:
	case 32:
	    ((VMemType32*)pixels)[i]=color.pixel;
	    break;
	}
    }

    return pixels;
}

/**
**	Color cycle.
*/
global void ColorCycle(void)
{
    int i;
    int x;

    // FIXME: this isn't 100% correct
    // Color cycling info - forest:
    // 3	flash red/green	(attacked building on minimap)
    // 38-47	cycle		(water)
    // 48-56	cycle		(water-coast boundary)
    // 202	pulsates red	(Circle of Power)
    // 240-244	cycle		(water around ships, Runestone, Dark Portal)
    // Color cycling info - swamp:
    // 3	flash red/green	(attacked building on minimap)
    // 4	pulsates red	(Circle of Power)
    // 5-9	cycle		(Runestone, Dark Portal)
    // 38-47	cycle		(water)
    // 88-95	cycle		(waterholes in coast and ground)
    // 240-244	cycle		(water around ships)
    // Color cycling info - wasteland:
    // 3	flash red/green	(attacked building on minimap)
    // 38-47	cycle		(water)
    // 64-70	cycle		(coast)
    // 202	pulsates red	(Circle of Power)
    // 240-244	cycle		(water around ships, Runestone, Dark Portal)
    // Color cycling info - winter:
    // 3	flash red/green	(attacked building on minimap)
    // 40-47	cycle		(water)
    // 48-54	cycle		(half-sunken ice-floe)
    // 202	pulsates red	(Circle of Power)
    // 205-207	cycle		(lights on christmas tree)
    // 240-244	cycle		(water around ships, Runestone, Dark Portal)

    // FIXME: function pointer
    switch( VideoDepth ) {
    case 8:
      x = ((VMemType8*)TheMap.TileData->Pixels)[38];
      for(i = 38; i < 47; ++i){
	((VMemType8*)TheMap.TileData->Pixels)[i] = 
	  ((VMemType8*)TheMap.TileData->Pixels)[i+1];
      }
      ((VMemType8*)TheMap.TileData->Pixels)[47] = x;

	x=Pixels8[38];
	for( i=38; i<47; ++i ) {	// tileset color cycle
	    Pixels8[i]=Pixels8[i+1];
	}
	Pixels8[47]=x;

	x=Pixels8[240];
	for( i=240; i<244; ++i ) {	// units/icons color cycle
	    Pixels8[i]=Pixels8[i+1];
	}
	Pixels8[244]=x;
	break;
    case 15:
    case 16:
      x = ((VMemType16*)TheMap.TileData->Pixels)[38];
      for(i = 38; i < 47; ++i){
	((VMemType16*)TheMap.TileData->Pixels)[i] = 
	  ((VMemType16*)TheMap.TileData->Pixels)[i+1];
      }
      ((VMemType16*)TheMap.TileData->Pixels)[47] = x;

	x=Pixels16[38];
	for( i=38; i<47; ++i ) {	// tileset color cycle
	    Pixels16[i]=Pixels16[i+1];
	}
	Pixels16[47]=x;

	x=Pixels16[240];
	for( i=240; i<244; ++i ) {	// units/icons color cycle
	    Pixels16[i]=Pixels16[i+1];
	}
	Pixels16[244]=x;
	break;
    case 24:
    case 32:
      x = ((VMemType32*)TheMap.TileData->Pixels)[38];
      for(i = 38; i < 47; ++i){
	((VMemType32*)TheMap.TileData->Pixels)[i] = 
	  ((VMemType32*)TheMap.TileData->Pixels)[i+1];
      }
      ((VMemType32*)TheMap.TileData->Pixels)[47] = x;



	x=Pixels32[38];
	for( i=38; i<47; ++i ) {	// tileset color cycle
	    Pixels32[i]=Pixels32[i+1];
	}
	Pixels32[47]=x;

	x=Pixels32[240];
	for( i=240; i<244; ++i ) {	// units/icons color cycle
	    Pixels32[i]=Pixels32[i+1];
	}
	Pixels32[244]=x;
	break;
    }

    MapColorCycle();		// FIXME: could be little more informativer
    MustRedraw|=RedrawMap|RedrawInfoPanel;
}

/**
**	Check video interrupt.
**
**	Display and count too slow frames.
*/
global void CheckVideoInterrupts(void)
{
    if( VideoInterrupts ) {
        //DebugLevel1("Slow frame\n");
	IfDebug(
	    DrawText(TheUI.MapX+10,TheUI.MapY+10,GameFont,"SLOW FRAME!!");
	    XClearArea(TheDisplay,TheMainWindow
		,TheUI.MapX+10,TheUI.MapX+10,13*13,13
		,False);
	);
        ++SlowFrameCounter;
#if 0
    } else {
        struct itimerval itv;

        getitimer(ITIMER_REAL,&itv);
        DebugLevel1("Remain %d\n",itv.it_value.tv_usec);
#endif
    }
}

/**
**	Realize video memory.
*/
global void RealizeVideoMemory(void)
{
    // in X11 it does flushing the output queue
    XFlush(TheDisplay);
    //XSync(TheDisplay,False);
}

/**
**	Toggle grab mouse.
*/
global void ToggleGrabMouse(void)
{
    static int grabbed;

    if( grabbed ) {
	XUngrabPointer(TheDisplay,CurrentTime);
	grabbed=0;
    } else {
	if( XGrabPointer(TheDisplay,TheMainWindow,True,0
		,GrabModeAsync,GrabModeAsync
		,TheMainWindow, None, CurrentTime)==GrabSuccess ) {
	    grabbed=1;
	}

    }
}

#endif	// USE_X11

//@}
