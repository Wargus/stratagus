/*
**	A clone of a famous game.
*/
/**@name button.h	-	The unit button file. */
/*
**	(c) Copyright 1999 by Lutz Sammer
**
**	$Id$
*/

#ifndef __BUTTON_H__
#define __BUTTON_H__

//@{

typedef void ButtonConfig;
#define InitUnitButtons()
#define UnitButtonCclRegister()

#if 0
// Unused

/*----------------------------------------------------------------------------
--	Includes
----------------------------------------------------------------------------*/

#include "icons.h"

/*----------------------------------------------------------------------------
--	Definitons
----------------------------------------------------------------------------*/

/**
**	Defines an unit button.
*/
typedef struct _unit_button_ {
    void*	OType;			/// Object type (future extensions)

    char*	Ident;			/// identifier

    IconConfig	Icon;			/// icon to display
    int		ReqFn;			/// requirements function to appear
    int		ReqArg;			/// requirements argument
    int		ActFn;			/// action to perfrom on button press
    int		ActArg;			/// action function argument
    int		Key;			/// keyboard hotkey
    char*	Text;			/// tip text in status line
//  char*	BalloonTip;		/// Balloon help
} UnitButton;

/**
**	Button definition
*/
typedef struct _button_config_ {
    char*	Name;			/// config icon name
    UnitButton*	Button;			/// identifier to use to run time
} ButtonConfig;

#if 0
/**
**	Defines an unit button panel.
*/
typedef struct _unit_panel_ {
    void*	OType;			/// Object type (future extensions)

    char*	Ident;			/// identifier
} UnitPanel;
#endif

/*----------------------------------------------------------------------------
--	Variables
----------------------------------------------------------------------------*/

extern char	  UnitButtonType;	/// unit button type
extern UnitButton UnitButtons[];	/// all unit buttons

/*----------------------------------------------------------------------------
--	Functions
----------------------------------------------------------------------------*/

extern void UnitButtonCclRegister(void);	/// register ccl features

extern UnitButton* ButtonByIdent(const char*);	/// get button by ident
extern void InitUnitButtons(void);		/// setup unit buttons

#endif

//@}

#endif	// !__BUTTON_H__
