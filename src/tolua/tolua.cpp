/*
** Lua binding: stratagus
** Generated automatically by tolua++-1.0.7 on Sun Dec 18 12:14:39 2005.
*/

#ifndef __cplusplus
#include "stdlib.h"
#endif
#include "string.h"

#include "tolua++.h"

/* Exported function */
TOLUA_API int tolua_stratagus_open (lua_State* tolua_S);

#include "stratagus.h"
#include "ui.h"
#include "minimap.h"
#include "player.h"
#include "unittype.h"
#include "unit.h"
#include "video.h"
#include "font.h"
#include "widgets.h"
#include "sound.h"
#include "sound_server.h"
#ifdef _MSC_VER
#pragma warning(disable:4800)
#endif
using namespace gcn;
void StartMap(const char *str);
void StartEditor(const char *str);
void StartReplay(const char *str);
void StartSavedGame(const char *str);

/* function to release collected object via destructor */
#ifdef __cplusplus

static int tolua_collect_ImageSlider (lua_State* tolua_S)
{
 ImageSlider* self = (ImageSlider*) tolua_tousertype(tolua_S,1,0);
 delete self;
 return 0;
}

static int tolua_collect_Label (lua_State* tolua_S)
{
 Label* self = (Label*) tolua_tousertype(tolua_S,1,0);
 delete self;
 return 0;
}

static int tolua_collect_Color (lua_State* tolua_S)
{
 Color* self = (Color*) tolua_tousertype(tolua_S,1,0);
 delete self;
 return 0;
}

static int tolua_collect_DropDownWidget (lua_State* tolua_S)
{
 DropDownWidget* self = (DropDownWidget*) tolua_tousertype(tolua_S,1,0);
 delete self;
 return 0;
}

static int tolua_collect_Windows (lua_State* tolua_S)
{
 Windows* self = (Windows*) tolua_tousertype(tolua_S,1,0);
 delete self;
 return 0;
}

static int tolua_collect_ButtonWidget (lua_State* tolua_S)
{
 ButtonWidget* self = (ButtonWidget*) tolua_tousertype(tolua_S,1,0);
 delete self;
 return 0;
}

static int tolua_collect_MenuScreen (lua_State* tolua_S)
{
 MenuScreen* self = (MenuScreen*) tolua_tousertype(tolua_S,1,0);
 delete self;
 return 0;
}

static int tolua_collect_TextField (lua_State* tolua_S)
{
 TextField* self = (TextField*) tolua_tousertype(tolua_S,1,0);
 delete self;
 return 0;
}

static int tolua_collect_ImageCheckBox (lua_State* tolua_S)
{
 ImageCheckBox* self = (ImageCheckBox*) tolua_tousertype(tolua_S,1,0);
 delete self;
 return 0;
}

static int tolua_collect_ScrollingWidget (lua_State* tolua_S)
{
 ScrollingWidget* self = (ScrollingWidget*) tolua_tousertype(tolua_S,1,0);
 delete self;
 return 0;
}

static int tolua_collect_LuaActionListener (lua_State* tolua_S)
{
 LuaActionListener* self = (LuaActionListener*) tolua_tousertype(tolua_S,1,0);
 delete self;
 return 0;
}

static int tolua_collect_CheckBox (lua_State* tolua_S)
{
 CheckBox* self = (CheckBox*) tolua_tousertype(tolua_S,1,0);
 delete self;
 return 0;
}

static int tolua_collect_ImageButton (lua_State* tolua_S)
{
 ImageButton* self = (ImageButton*) tolua_tousertype(tolua_S,1,0);
 delete self;
 return 0;
}

static int tolua_collect_CPlayer (lua_State* tolua_S)
{
 CPlayer* self = (CPlayer*) tolua_tousertype(tolua_S,1,0);
 delete self;
 return 0;
}

static int tolua_collect_StatBoxWidget (lua_State* tolua_S)
{
 StatBoxWidget* self = (StatBoxWidget*) tolua_tousertype(tolua_S,1,0);
 delete self;
 return 0;
}

static int tolua_collect_ImageRadioButton (lua_State* tolua_S)
{
 ImageRadioButton* self = (ImageRadioButton*) tolua_tousertype(tolua_S,1,0);
 delete self;
 return 0;
}

static int tolua_collect_RadioButton (lua_State* tolua_S)
{
 RadioButton* self = (RadioButton*) tolua_tousertype(tolua_S,1,0);
 delete self;
 return 0;
}

static int tolua_collect_CColor (lua_State* tolua_S)
{
 CColor* self = (CColor*) tolua_tousertype(tolua_S,1,0);
 delete self;
 return 0;
}

static int tolua_collect_ImageWidget (lua_State* tolua_S)
{
 ImageWidget* self = (ImageWidget*) tolua_tousertype(tolua_S,1,0);
 delete self;
 return 0;
}

static int tolua_collect_ListBoxWidget (lua_State* tolua_S)
{
 ListBoxWidget* self = (ListBoxWidget*) tolua_tousertype(tolua_S,1,0);
 delete self;
 return 0;
}

static int tolua_collect_Slider (lua_State* tolua_S)
{
 Slider* self = (Slider*) tolua_tousertype(tolua_S,1,0);
 delete self;
 return 0;
}
#endif


/* function to register type */
static void tolua_reg_types (lua_State* tolua_S)
{
 tolua_usertype(tolua_S,"Label");
 tolua_usertype(tolua_S,"ImageRadioButton");
 tolua_usertype(tolua_S,"CFontColor");
 tolua_usertype(tolua_S,"CUserInterface");
 tolua_usertype(tolua_S,"CIcon");
 tolua_usertype(tolua_S,"TextField");
 tolua_usertype(tolua_S,"CheckBox");
 tolua_usertype(tolua_S,"ImageWidget");
 tolua_usertype(tolua_S,"CPlayer");
 tolua_usertype(tolua_S,"CUnit");
 tolua_usertype(tolua_S,"CColor");
 tolua_usertype(tolua_S,"CButtonPanel");
 tolua_usertype(tolua_S,"CMinimap");
 tolua_usertype(tolua_S,"ImageSlider");
 tolua_usertype(tolua_S,"ListBoxWidget");
 tolua_usertype(tolua_S,"Color");
 tolua_usertype(tolua_S,"DropDownWidget");
 tolua_usertype(tolua_S,"Windows");
 tolua_usertype(tolua_S,"CInfoPanel");
 tolua_usertype(tolua_S,"Container");
 tolua_usertype(tolua_S,"ButtonWidget");
 tolua_usertype(tolua_S,"CPreference");
 tolua_usertype(tolua_S,"CVideo");
 tolua_usertype(tolua_S,"ImageCheckBox");
 tolua_usertype(tolua_S,"ScrollingWidget");
 tolua_usertype(tolua_S,"CGraphic");
 tolua_usertype(tolua_S,"StatBoxWidget");
 tolua_usertype(tolua_S,"CUnitType");
 tolua_usertype(tolua_S,"CUpgrade");
 tolua_usertype(tolua_S,"LuaActionListener");
 tolua_usertype(tolua_S,"CFont");
 tolua_usertype(tolua_S,"RadioButton");
 tolua_usertype(tolua_S,"MenuScreen");
 tolua_usertype(tolua_S,"ImageButton");
 tolua_usertype(tolua_S,"Widget");
 tolua_usertype(tolua_S,"Slider");
}

/* get function: X of class  CMinimap */
#ifndef TOLUA_DISABLE_tolua_get_CMinimap_X
static int tolua_get_CMinimap_X(lua_State* tolua_S)
{
  CMinimap* self = (CMinimap*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'X'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->X);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: X of class  CMinimap */
#ifndef TOLUA_DISABLE_tolua_set_CMinimap_X
static int tolua_set_CMinimap_X(lua_State* tolua_S)
{
  CMinimap* self = (CMinimap*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'X'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->X = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Y of class  CMinimap */
#ifndef TOLUA_DISABLE_tolua_get_CMinimap_Y
static int tolua_get_CMinimap_Y(lua_State* tolua_S)
{
  CMinimap* self = (CMinimap*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Y'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->Y);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Y of class  CMinimap */
#ifndef TOLUA_DISABLE_tolua_set_CMinimap_Y
static int tolua_set_CMinimap_Y(lua_State* tolua_S)
{
  CMinimap* self = (CMinimap*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Y'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Y = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: W of class  CMinimap */
#ifndef TOLUA_DISABLE_tolua_get_CMinimap_W
static int tolua_get_CMinimap_W(lua_State* tolua_S)
{
  CMinimap* self = (CMinimap*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'W'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->W);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: W of class  CMinimap */
#ifndef TOLUA_DISABLE_tolua_set_CMinimap_W
static int tolua_set_CMinimap_W(lua_State* tolua_S)
{
  CMinimap* self = (CMinimap*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'W'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->W = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: H of class  CMinimap */
#ifndef TOLUA_DISABLE_tolua_get_CMinimap_H
static int tolua_get_CMinimap_H(lua_State* tolua_S)
{
  CMinimap* self = (CMinimap*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'H'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->H);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: H of class  CMinimap */
#ifndef TOLUA_DISABLE_tolua_set_CMinimap_H
static int tolua_set_CMinimap_H(lua_State* tolua_S)
{
  CMinimap* self = (CMinimap*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'H'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->H = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: WithTerrain of class  CMinimap */
#ifndef TOLUA_DISABLE_tolua_get_CMinimap_WithTerrain
static int tolua_get_CMinimap_WithTerrain(lua_State* tolua_S)
{
  CMinimap* self = (CMinimap*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'WithTerrain'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->WithTerrain);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: WithTerrain of class  CMinimap */
#ifndef TOLUA_DISABLE_tolua_set_CMinimap_WithTerrain
static int tolua_set_CMinimap_WithTerrain(lua_State* tolua_S)
{
  CMinimap* self = (CMinimap*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'WithTerrain'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->WithTerrain = ((bool)  tolua_toboolean(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: ShowSelected of class  CMinimap */
#ifndef TOLUA_DISABLE_tolua_get_CMinimap_ShowSelected
static int tolua_get_CMinimap_ShowSelected(lua_State* tolua_S)
{
  CMinimap* self = (CMinimap*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ShowSelected'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->ShowSelected);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: ShowSelected of class  CMinimap */
#ifndef TOLUA_DISABLE_tolua_set_CMinimap_ShowSelected
static int tolua_set_CMinimap_ShowSelected(lua_State* tolua_S)
{
  CMinimap* self = (CMinimap*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ShowSelected'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->ShowSelected = ((bool)  tolua_toboolean(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Transparent of class  CMinimap */
#ifndef TOLUA_DISABLE_tolua_get_CMinimap_Transparent
static int tolua_get_CMinimap_Transparent(lua_State* tolua_S)
{
  CMinimap* self = (CMinimap*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Transparent'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->Transparent);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Transparent of class  CMinimap */
#ifndef TOLUA_DISABLE_tolua_set_CMinimap_Transparent
static int tolua_set_CMinimap_Transparent(lua_State* tolua_S)
{
  CMinimap* self = (CMinimap*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Transparent'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Transparent = ((bool)  tolua_toboolean(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: X of class  CButtonPanel */
#ifndef TOLUA_DISABLE_tolua_get_CButtonPanel_X
static int tolua_get_CButtonPanel_X(lua_State* tolua_S)
{
  CButtonPanel* self = (CButtonPanel*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'X'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->X);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: X of class  CButtonPanel */
#ifndef TOLUA_DISABLE_tolua_set_CButtonPanel_X
static int tolua_set_CButtonPanel_X(lua_State* tolua_S)
{
  CButtonPanel* self = (CButtonPanel*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'X'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->X = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Y of class  CButtonPanel */
#ifndef TOLUA_DISABLE_tolua_get_CButtonPanel_Y
static int tolua_get_CButtonPanel_Y(lua_State* tolua_S)
{
  CButtonPanel* self = (CButtonPanel*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Y'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->Y);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Y of class  CButtonPanel */
#ifndef TOLUA_DISABLE_tolua_set_CButtonPanel_Y
static int tolua_set_CButtonPanel_Y(lua_State* tolua_S)
{
  CButtonPanel* self = (CButtonPanel*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Y'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Y = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: ShowCommandKey of class  CButtonPanel */
#ifndef TOLUA_DISABLE_tolua_get_CButtonPanel_ShowCommandKey
static int tolua_get_CButtonPanel_ShowCommandKey(lua_State* tolua_S)
{
  CButtonPanel* self = (CButtonPanel*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ShowCommandKey'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->ShowCommandKey);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: ShowCommandKey of class  CButtonPanel */
#ifndef TOLUA_DISABLE_tolua_set_CButtonPanel_ShowCommandKey
static int tolua_set_CButtonPanel_ShowCommandKey(lua_State* tolua_S)
{
  CButtonPanel* self = (CButtonPanel*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ShowCommandKey'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->ShowCommandKey = ((bool)  tolua_toboolean(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: X of class  CInfoPanel */
#ifndef TOLUA_DISABLE_tolua_get_CInfoPanel_X
static int tolua_get_CInfoPanel_X(lua_State* tolua_S)
{
  CInfoPanel* self = (CInfoPanel*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'X'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->X);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: X of class  CInfoPanel */
#ifndef TOLUA_DISABLE_tolua_set_CInfoPanel_X
static int tolua_set_CInfoPanel_X(lua_State* tolua_S)
{
  CInfoPanel* self = (CInfoPanel*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'X'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->X = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Y of class  CInfoPanel */
#ifndef TOLUA_DISABLE_tolua_get_CInfoPanel_Y
static int tolua_get_CInfoPanel_Y(lua_State* tolua_S)
{
  CInfoPanel* self = (CInfoPanel*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Y'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->Y);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Y of class  CInfoPanel */
#ifndef TOLUA_DISABLE_tolua_set_CInfoPanel_Y
static int tolua_set_CInfoPanel_Y(lua_State* tolua_S)
{
  CInfoPanel* self = (CInfoPanel*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Y'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Y = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: InfoPanel of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_get_CUserInterface_InfoPanel
static int tolua_get_CUserInterface_InfoPanel(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'InfoPanel'",NULL);
#endif
 tolua_pushusertype(tolua_S,(void*)&self->InfoPanel,"CInfoPanel");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: InfoPanel of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_set_CUserInterface_InfoPanel
static int tolua_set_CUserInterface_InfoPanel(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'InfoPanel'",NULL);
 if (!tolua_isusertype(tolua_S,2,"CInfoPanel",0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->InfoPanel = *((CInfoPanel*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: ButtonPanel of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_get_CUserInterface_ButtonPanel
static int tolua_get_CUserInterface_ButtonPanel(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ButtonPanel'",NULL);
#endif
 tolua_pushusertype(tolua_S,(void*)&self->ButtonPanel,"CButtonPanel");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: ButtonPanel of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_set_CUserInterface_ButtonPanel
static int tolua_set_CUserInterface_ButtonPanel(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ButtonPanel'",NULL);
 if (!tolua_isusertype(tolua_S,2,"CButtonPanel",0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->ButtonPanel = *((CButtonPanel*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Minimap of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_get_CUserInterface_Minimap
static int tolua_get_CUserInterface_Minimap(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Minimap'",NULL);
#endif
 tolua_pushusertype(tolua_S,(void*)&self->Minimap,"CMinimap");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Minimap of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_set_CUserInterface_Minimap
static int tolua_set_CUserInterface_Minimap(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Minimap'",NULL);
 if (!tolua_isusertype(tolua_S,2,"CMinimap",0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Minimap = *((CMinimap*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: UI */
#ifndef TOLUA_DISABLE_tolua_get_UI
static int tolua_get_UI(lua_State* tolua_S)
{
 tolua_pushusertype(tolua_S,(void*)&UI,"CUserInterface");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: UI */
#ifndef TOLUA_DISABLE_tolua_set_UI
static int tolua_set_UI(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!tolua_isusertype(tolua_S,2,"CUserInterface",0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  UI = *((CUserInterface*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* method: New of class  CIcon */
#ifndef TOLUA_DISABLE_tolua_stratagus_CIcon_New00
static int tolua_stratagus_CIcon_New00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"CIcon",0,&tolua_err) ||
 !tolua_isstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const char* ident = ((const char*)  tolua_tostring(tolua_S,2,0));
 {
  CIcon* tolua_ret = (CIcon*)  CIcon::New(ident);
 tolua_pushusertype(tolua_S,(void*)tolua_ret,"CIcon");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'New'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: Get of class  CIcon */
#ifndef TOLUA_DISABLE_tolua_stratagus_CIcon_Get00
static int tolua_stratagus_CIcon_Get00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"CIcon",0,&tolua_err) ||
 !tolua_isstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const char* ident = ((const char*)  tolua_tostring(tolua_S,2,0));
 {
  CIcon* tolua_ret = (CIcon*)  CIcon::Get(ident);
 tolua_pushusertype(tolua_S,(void*)tolua_ret,"CIcon");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Get'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Ident of class  CIcon */
#ifndef TOLUA_DISABLE_tolua_get_CIcon_Ident
static int tolua_get_CIcon_Ident(lua_State* tolua_S)
{
  CIcon* self = (CIcon*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Ident'",NULL);
#endif
 tolua_pushstring(tolua_S,(const char*)self->GetIdent());
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: G of class  CIcon */
#ifndef TOLUA_DISABLE_tolua_get_CIcon_G_ptr
static int tolua_get_CIcon_G_ptr(lua_State* tolua_S)
{
  CIcon* self = (CIcon*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'G'",NULL);
#endif
 tolua_pushusertype(tolua_S,(void*)self->G,"CGraphic");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: G of class  CIcon */
#ifndef TOLUA_DISABLE_tolua_set_CIcon_G_ptr
static int tolua_set_CIcon_G_ptr(lua_State* tolua_S)
{
  CIcon* self = (CIcon*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'G'",NULL);
 if (!tolua_isusertype(tolua_S,2,"CGraphic",0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->G = ((CGraphic*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Frame of class  CIcon */
#ifndef TOLUA_DISABLE_tolua_get_CIcon_Frame
static int tolua_get_CIcon_Frame(lua_State* tolua_S)
{
  CIcon* self = (CIcon*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Frame'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->Frame);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Frame of class  CIcon */
#ifndef TOLUA_DISABLE_tolua_set_CIcon_Frame
static int tolua_set_CIcon_Frame(lua_State* tolua_S)
{
  CIcon* self = (CIcon*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Frame'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Frame = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* function: GetMouseScroll */
#ifndef TOLUA_DISABLE_tolua_stratagus_GetMouseScroll00
static int tolua_stratagus_GetMouseScroll00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  bool tolua_ret = (bool)  GetMouseScroll();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetMouseScroll'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: SetMouseScroll */
#ifndef TOLUA_DISABLE_tolua_stratagus_SetMouseScroll00
static int tolua_stratagus_SetMouseScroll00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isboolean(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  bool enabled = ((bool)  tolua_toboolean(tolua_S,1,0));
 {
  SetMouseScroll(enabled);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetMouseScroll'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: GetMouseScrollSpeed */
#ifndef TOLUA_DISABLE_tolua_stratagus_GetMouseScrollSpeed00
static int tolua_stratagus_GetMouseScrollSpeed00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  int tolua_ret = (int)  GetMouseScrollSpeed();
 tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetMouseScrollSpeed'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: SetMouseScrollSpeed */
#ifndef TOLUA_DISABLE_tolua_stratagus_SetMouseScrollSpeed00
static int tolua_stratagus_SetMouseScrollSpeed00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int speed = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  SetMouseScrollSpeed(speed);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetMouseScrollSpeed'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: GetKeyScroll */
#ifndef TOLUA_DISABLE_tolua_stratagus_GetKeyScroll00
static int tolua_stratagus_GetKeyScroll00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  bool tolua_ret = (bool)  GetKeyScroll();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetKeyScroll'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: SetKeyScroll */
#ifndef TOLUA_DISABLE_tolua_stratagus_SetKeyScroll00
static int tolua_stratagus_SetKeyScroll00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isboolean(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  bool enabled = ((bool)  tolua_toboolean(tolua_S,1,0));
 {
  SetKeyScroll(enabled);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetKeyScroll'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: GetKeyScrollSpeed */
#ifndef TOLUA_DISABLE_tolua_stratagus_GetKeyScrollSpeed00
static int tolua_stratagus_GetKeyScrollSpeed00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  int tolua_ret = (int)  GetKeyScrollSpeed();
 tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetKeyScrollSpeed'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: SetKeyScrollSpeed */
#ifndef TOLUA_DISABLE_tolua_stratagus_SetKeyScrollSpeed00
static int tolua_stratagus_SetKeyScrollSpeed00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int speed = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  SetKeyScrollSpeed(speed);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetKeyScrollSpeed'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: GetGrabMouse */
#ifndef TOLUA_DISABLE_tolua_stratagus_GetGrabMouse00
static int tolua_stratagus_GetGrabMouse00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  bool tolua_ret = (bool)  GetGrabMouse();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetGrabMouse'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: SetGrabMouse */
#ifndef TOLUA_DISABLE_tolua_stratagus_SetGrabMouse00
static int tolua_stratagus_SetGrabMouse00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isboolean(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  bool enabled = ((bool)  tolua_toboolean(tolua_S,1,0));
 {
  SetGrabMouse(enabled);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetGrabMouse'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: GetLeaveStops */
#ifndef TOLUA_DISABLE_tolua_stratagus_GetLeaveStops00
static int tolua_stratagus_GetLeaveStops00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  bool tolua_ret = (bool)  GetLeaveStops();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetLeaveStops'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: SetLeaveStops */
#ifndef TOLUA_DISABLE_tolua_stratagus_SetLeaveStops00
static int tolua_stratagus_SetLeaveStops00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isboolean(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  bool enabled = ((bool)  tolua_toboolean(tolua_S,1,0));
 {
  SetLeaveStops(enabled);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetLeaveStops'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: GetDoubleClickDelay */
#ifndef TOLUA_DISABLE_tolua_stratagus_GetDoubleClickDelay00
static int tolua_stratagus_GetDoubleClickDelay00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  int tolua_ret = (int)  GetDoubleClickDelay();
 tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetDoubleClickDelay'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: SetDoubleClickDelay */
#ifndef TOLUA_DISABLE_tolua_stratagus_SetDoubleClickDelay00
static int tolua_stratagus_SetDoubleClickDelay00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int delay = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  SetDoubleClickDelay(delay);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetDoubleClickDelay'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: GetHoldClickDelay */
#ifndef TOLUA_DISABLE_tolua_stratagus_GetHoldClickDelay00
static int tolua_stratagus_GetHoldClickDelay00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  int tolua_ret = (int)  GetHoldClickDelay();
 tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetHoldClickDelay'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: SetHoldClickDelay */
#ifndef TOLUA_DISABLE_tolua_stratagus_SetHoldClickDelay00
static int tolua_stratagus_SetHoldClickDelay00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int delay = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  SetHoldClickDelay(delay);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetHoldClickDelay'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  Color */
#ifndef TOLUA_DISABLE_tolua_stratagus_Color_new00
static int tolua_stratagus_Color_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"Color",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,5,1,&tolua_err) ||
 !tolua_isnoobj(tolua_S,6,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int r = ((int)  tolua_tonumber(tolua_S,2,0));
  int g = ((int)  tolua_tonumber(tolua_S,3,0));
  int b = ((int)  tolua_tonumber(tolua_S,4,0));
  int a = ((int)  tolua_tonumber(tolua_S,5,255));
 {
  Color* tolua_ret = (Color*)  new Color(r,g,b,a);
 tolua_pushusertype(tolua_S,(void*)tolua_ret,"Color");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  Color */
#ifndef TOLUA_DISABLE_tolua_stratagus_Color_new00_local
static int tolua_stratagus_Color_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"Color",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,5,1,&tolua_err) ||
 !tolua_isnoobj(tolua_S,6,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int r = ((int)  tolua_tonumber(tolua_S,2,0));
  int g = ((int)  tolua_tonumber(tolua_S,3,0));
  int b = ((int)  tolua_tonumber(tolua_S,4,0));
  int a = ((int)  tolua_tonumber(tolua_S,5,255));
 {
  Color* tolua_ret = (Color*)  new Color(r,g,b,a);
 tolua_pushusertype_and_takeownership(tolua_S,(void *)tolua_ret,"Color");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* get function: r of class  Color */
#ifndef TOLUA_DISABLE_tolua_get_Color_r
static int tolua_get_Color_r(lua_State* tolua_S)
{
  Color* self = (Color*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'r'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->r);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: r of class  Color */
#ifndef TOLUA_DISABLE_tolua_set_Color_r
static int tolua_set_Color_r(lua_State* tolua_S)
{
  Color* self = (Color*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'r'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->r = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: g of class  Color */
#ifndef TOLUA_DISABLE_tolua_get_Color_g
static int tolua_get_Color_g(lua_State* tolua_S)
{
  Color* self = (Color*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'g'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->g);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: g of class  Color */
#ifndef TOLUA_DISABLE_tolua_set_Color_g
static int tolua_set_Color_g(lua_State* tolua_S)
{
  Color* self = (Color*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'g'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->g = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: b of class  Color */
#ifndef TOLUA_DISABLE_tolua_get_Color_b
static int tolua_get_Color_b(lua_State* tolua_S)
{
  Color* self = (Color*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'b'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->b);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: b of class  Color */
#ifndef TOLUA_DISABLE_tolua_set_Color_b
static int tolua_set_Color_b(lua_State* tolua_S)
{
  Color* self = (Color*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'b'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->b = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: a of class  Color */
#ifndef TOLUA_DISABLE_tolua_get_Color_a
static int tolua_get_Color_a(lua_State* tolua_S)
{
  Color* self = (Color*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'a'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->a);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: a of class  Color */
#ifndef TOLUA_DISABLE_tolua_set_Color_a
static int tolua_set_Color_a(lua_State* tolua_S)
{
  Color* self = (Color*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'a'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->a = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  LuaActionListener */
#ifndef TOLUA_DISABLE_tolua_stratagus_LuaActionListener_new00
static int tolua_stratagus_LuaActionListener_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"LuaActionListener",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  lua_State* lua =  tolua_S;
  lua_Object luaref = ((lua_Object)  tolua_tovalue(tolua_S,2,0));
 {
  LuaActionListener* tolua_ret = (LuaActionListener*)  new LuaActionListener(lua,luaref);
 tolua_pushusertype(tolua_S,(void*)tolua_ret,"LuaActionListener");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  LuaActionListener */
#ifndef TOLUA_DISABLE_tolua_stratagus_LuaActionListener_new00_local
static int tolua_stratagus_LuaActionListener_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"LuaActionListener",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  lua_State* lua =  tolua_S;
  lua_Object luaref = ((lua_Object)  tolua_tovalue(tolua_S,2,0));
 {
  LuaActionListener* tolua_ret = (LuaActionListener*)  new LuaActionListener(lua,luaref);
 tolua_pushusertype_and_takeownership(tolua_S,(void *)tolua_ret,"LuaActionListener");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setWidth of class  Widget */
#ifndef TOLUA_DISABLE_tolua_stratagus_Widget_setWidth00
static int tolua_stratagus_Widget_setWidth00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"Widget",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  Widget* self = (Widget*)  tolua_tousertype(tolua_S,1,0);
  int width = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setWidth'",NULL);
#endif
 {
  self->setWidth(width);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setWidth'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getWidth of class  Widget */
#ifndef TOLUA_DISABLE_tolua_stratagus_Widget_getWidth00
static int tolua_stratagus_Widget_getWidth00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"const Widget",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const Widget* self = (const Widget*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getWidth'",NULL);
#endif
 {
  int tolua_ret = (int)  self->getWidth();
 tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getWidth'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setHeight of class  Widget */
#ifndef TOLUA_DISABLE_tolua_stratagus_Widget_setHeight00
static int tolua_stratagus_Widget_setHeight00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"Widget",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  Widget* self = (Widget*)  tolua_tousertype(tolua_S,1,0);
  int height = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setHeight'",NULL);
#endif
 {
  self->setHeight(height);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setHeight'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getHeight of class  Widget */
#ifndef TOLUA_DISABLE_tolua_stratagus_Widget_getHeight00
static int tolua_stratagus_Widget_getHeight00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"const Widget",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const Widget* self = (const Widget*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getHeight'",NULL);
#endif
 {
  int tolua_ret = (int)  self->getHeight();
 tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getHeight'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setSize of class  Widget */
#ifndef TOLUA_DISABLE_tolua_stratagus_Widget_setSize00
static int tolua_stratagus_Widget_setSize00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"Widget",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  Widget* self = (Widget*)  tolua_tousertype(tolua_S,1,0);
  int width = ((int)  tolua_tonumber(tolua_S,2,0));
  int height = ((int)  tolua_tonumber(tolua_S,3,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setSize'",NULL);
#endif
 {
  self->setSize(width,height);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setSize'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setX of class  Widget */
#ifndef TOLUA_DISABLE_tolua_stratagus_Widget_setX00
static int tolua_stratagus_Widget_setX00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"Widget",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  Widget* self = (Widget*)  tolua_tousertype(tolua_S,1,0);
  int x = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setX'",NULL);
#endif
 {
  self->setX(x);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setX'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getX of class  Widget */
#ifndef TOLUA_DISABLE_tolua_stratagus_Widget_getX00
static int tolua_stratagus_Widget_getX00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"const Widget",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const Widget* self = (const Widget*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getX'",NULL);
#endif
 {
  int tolua_ret = (int)  self->getX();
 tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getX'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setY of class  Widget */
#ifndef TOLUA_DISABLE_tolua_stratagus_Widget_setY00
static int tolua_stratagus_Widget_setY00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"Widget",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  Widget* self = (Widget*)  tolua_tousertype(tolua_S,1,0);
  int y = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setY'",NULL);
#endif
 {
  self->setY(y);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setY'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getY of class  Widget */
#ifndef TOLUA_DISABLE_tolua_stratagus_Widget_getY00
static int tolua_stratagus_Widget_getY00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"const Widget",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const Widget* self = (const Widget*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getY'",NULL);
#endif
 {
  int tolua_ret = (int)  self->getY();
 tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getY'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setPosition of class  Widget */
#ifndef TOLUA_DISABLE_tolua_stratagus_Widget_setPosition00
static int tolua_stratagus_Widget_setPosition00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"Widget",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  Widget* self = (Widget*)  tolua_tousertype(tolua_S,1,0);
  int x = ((int)  tolua_tonumber(tolua_S,2,0));
  int y = ((int)  tolua_tonumber(tolua_S,3,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setPosition'",NULL);
#endif
 {
  self->setPosition(x,y);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setPosition'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setEnabled of class  Widget */
#ifndef TOLUA_DISABLE_tolua_stratagus_Widget_setEnabled00
static int tolua_stratagus_Widget_setEnabled00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"Widget",0,&tolua_err) ||
 !tolua_isboolean(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  Widget* self = (Widget*)  tolua_tousertype(tolua_S,1,0);
  bool enabled = ((bool)  tolua_toboolean(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setEnabled'",NULL);
#endif
 {
  self->setEnabled(enabled);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setEnabled'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: isEnabled of class  Widget */
#ifndef TOLUA_DISABLE_tolua_stratagus_Widget_isEnabled00
static int tolua_stratagus_Widget_isEnabled00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"const Widget",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const Widget* self = (const Widget*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'isEnabled'",NULL);
#endif
 {
  bool tolua_ret = (bool)  self->isEnabled();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'isEnabled'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setBaseColor of class  Widget */
#ifndef TOLUA_DISABLE_tolua_stratagus_Widget_setBaseColor00
static int tolua_stratagus_Widget_setBaseColor00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"Widget",0,&tolua_err) ||
 !tolua_isusertype(tolua_S,2,"const Color",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  Widget* self = (Widget*)  tolua_tousertype(tolua_S,1,0);
  const Color* color = ((const Color*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setBaseColor'",NULL);
#endif
 {
  self->setBaseColor(*color);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setBaseColor'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setForegroundColor of class  Widget */
#ifndef TOLUA_DISABLE_tolua_stratagus_Widget_setForegroundColor00
static int tolua_stratagus_Widget_setForegroundColor00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"Widget",0,&tolua_err) ||
 !tolua_isusertype(tolua_S,2,"const Color",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  Widget* self = (Widget*)  tolua_tousertype(tolua_S,1,0);
  const Color* color = ((const Color*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setForegroundColor'",NULL);
#endif
 {
  self->setForegroundColor(*color);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setForegroundColor'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setBackgroundColor of class  Widget */
#ifndef TOLUA_DISABLE_tolua_stratagus_Widget_setBackgroundColor00
static int tolua_stratagus_Widget_setBackgroundColor00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"Widget",0,&tolua_err) ||
 !tolua_isusertype(tolua_S,2,"const Color",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  Widget* self = (Widget*)  tolua_tousertype(tolua_S,1,0);
  const Color* color = ((const Color*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setBackgroundColor'",NULL);
#endif
 {
  self->setBackgroundColor(*color);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setBackgroundColor'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setGlobalFont of class  Widget */
#ifndef TOLUA_DISABLE_tolua_stratagus_Widget_setGlobalFont00
static int tolua_stratagus_Widget_setGlobalFont00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"Widget",0,&tolua_err) ||
 !tolua_isusertype(tolua_S,2,"CFont",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  CFont* font = ((CFont*)  tolua_tousertype(tolua_S,2,0));
 {
  Widget::setGlobalFont(font);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setGlobalFont'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setForegroundColor of class  Widget */
#ifndef TOLUA_DISABLE_tolua_stratagus_Widget_setForegroundColor01
static int tolua_stratagus_Widget_setForegroundColor01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"Widget",0,&tolua_err) ||
 !tolua_isusertype(tolua_S,2,"const Color",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
 {
  Widget* self = (Widget*)  tolua_tousertype(tolua_S,1,0);
  const Color* color = ((const Color*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setForegroundColor'",NULL);
#endif
 {
  self->setForegroundColor(*color);
 }
 }
 return 0;
tolua_lerror:
 return tolua_stratagus_Widget_setForegroundColor00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: setBackgroundColor of class  Widget */
#ifndef TOLUA_DISABLE_tolua_stratagus_Widget_setBackgroundColor01
static int tolua_stratagus_Widget_setBackgroundColor01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"Widget",0,&tolua_err) ||
 !tolua_isusertype(tolua_S,2,"const Color",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
 {
  Widget* self = (Widget*)  tolua_tousertype(tolua_S,1,0);
  const Color* color = ((const Color*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setBackgroundColor'",NULL);
#endif
 {
  self->setBackgroundColor(*color);
 }
 }
 return 0;
tolua_lerror:
 return tolua_stratagus_Widget_setBackgroundColor00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: setBaseColor of class  Widget */
#ifndef TOLUA_DISABLE_tolua_stratagus_Widget_setBaseColor01
static int tolua_stratagus_Widget_setBaseColor01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"Widget",0,&tolua_err) ||
 !tolua_isusertype(tolua_S,2,"const Color",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
 {
  Widget* self = (Widget*)  tolua_tousertype(tolua_S,1,0);
  const Color* color = ((const Color*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setBaseColor'",NULL);
#endif
 {
  self->setBaseColor(*color);
 }
 }
 return 0;
tolua_lerror:
 return tolua_stratagus_Widget_setBaseColor00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: setSize of class  Widget */
#ifndef TOLUA_DISABLE_tolua_stratagus_Widget_setSize01
static int tolua_stratagus_Widget_setSize01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"Widget",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
 {
  Widget* self = (Widget*)  tolua_tousertype(tolua_S,1,0);
  int width = ((int)  tolua_tonumber(tolua_S,2,0));
  int height = ((int)  tolua_tonumber(tolua_S,3,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setSize'",NULL);
#endif
 {
  self->setSize(width,height);
 }
 }
 return 0;
tolua_lerror:
 return tolua_stratagus_Widget_setSize00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: setBorderSize of class  Widget */
#ifndef TOLUA_DISABLE_tolua_stratagus_Widget_setBorderSize00
static int tolua_stratagus_Widget_setBorderSize00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"Widget",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  Widget* self = (Widget*)  tolua_tousertype(tolua_S,1,0);
  int width = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setBorderSize'",NULL);
#endif
 {
  self->setBorderSize(width);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setBorderSize'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setFont of class  Widget */
#ifndef TOLUA_DISABLE_tolua_stratagus_Widget_setFont00
static int tolua_stratagus_Widget_setFont00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"Widget",0,&tolua_err) ||
 !tolua_isusertype(tolua_S,2,"CFont",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  Widget* self = (Widget*)  tolua_tousertype(tolua_S,1,0);
  CFont* font = ((CFont*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setFont'",NULL);
#endif
 {
  self->setFont(font);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setFont'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: addActionListener of class  Widget */
#ifndef TOLUA_DISABLE_tolua_stratagus_Widget_addActionListener00
static int tolua_stratagus_Widget_addActionListener00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"Widget",0,&tolua_err) ||
 !tolua_isusertype(tolua_S,2,"LuaActionListener",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  Widget* self = (Widget*)  tolua_tousertype(tolua_S,1,0);
  LuaActionListener* actionListener = ((LuaActionListener*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'addActionListener'",NULL);
#endif
 {
  self->addActionListener(actionListener);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'addActionListener'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  ImageWidget */
#ifndef TOLUA_DISABLE_tolua_stratagus_ImageWidget_new00
static int tolua_stratagus_ImageWidget_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"ImageWidget",0,&tolua_err) ||
 !tolua_isusertype(tolua_S,2,"CGraphic",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  CGraphic* image = ((CGraphic*)  tolua_tousertype(tolua_S,2,0));
 {
  ImageWidget* tolua_ret = (ImageWidget*)  new ImageWidget(image);
 tolua_pushusertype(tolua_S,(void*)tolua_ret,"ImageWidget");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  ImageWidget */
#ifndef TOLUA_DISABLE_tolua_stratagus_ImageWidget_new00_local
static int tolua_stratagus_ImageWidget_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"ImageWidget",0,&tolua_err) ||
 !tolua_isusertype(tolua_S,2,"CGraphic",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  CGraphic* image = ((CGraphic*)  tolua_tousertype(tolua_S,2,0));
 {
  ImageWidget* tolua_ret = (ImageWidget*)  new ImageWidget(image);
 tolua_pushusertype_and_takeownership(tolua_S,(void *)tolua_ret,"ImageWidget");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  ButtonWidget */
#ifndef TOLUA_DISABLE_tolua_stratagus_ButtonWidget_new00
static int tolua_stratagus_ButtonWidget_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"ButtonWidget",0,&tolua_err) ||
 !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const std::string caption = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
 {
  ButtonWidget* tolua_ret = (ButtonWidget*)  new ButtonWidget(caption);
 tolua_pushusertype(tolua_S,(void*)tolua_ret,"ButtonWidget");
 tolua_pushcppstring(tolua_S,(const char*)caption);
 }
 }
 return 2;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  ButtonWidget */
#ifndef TOLUA_DISABLE_tolua_stratagus_ButtonWidget_new00_local
static int tolua_stratagus_ButtonWidget_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"ButtonWidget",0,&tolua_err) ||
 !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const std::string caption = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
 {
  ButtonWidget* tolua_ret = (ButtonWidget*)  new ButtonWidget(caption);
 tolua_pushusertype_and_takeownership(tolua_S,(void *)tolua_ret,"ButtonWidget");
 tolua_pushcppstring(tolua_S,(const char*)caption);
 }
 }
 return 2;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setCaption of class  ButtonWidget */
#ifndef TOLUA_DISABLE_tolua_stratagus_ButtonWidget_setCaption00
static int tolua_stratagus_ButtonWidget_setCaption00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"ButtonWidget",0,&tolua_err) ||
 !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  ButtonWidget* self = (ButtonWidget*)  tolua_tousertype(tolua_S,1,0);
  const std::string caption = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setCaption'",NULL);
#endif
 {
  self->setCaption(caption);
 tolua_pushcppstring(tolua_S,(const char*)caption);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setCaption'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getCaption of class  ButtonWidget */
#ifndef TOLUA_DISABLE_tolua_stratagus_ButtonWidget_getCaption00
static int tolua_stratagus_ButtonWidget_getCaption00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"const ButtonWidget",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const ButtonWidget* self = (const ButtonWidget*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getCaption'",NULL);
#endif
 {
  const std::string tolua_ret = (const std::string)  self->getCaption();
 tolua_pushcppstring(tolua_S,(const char*)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getCaption'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: adjustSize of class  ButtonWidget */
#ifndef TOLUA_DISABLE_tolua_stratagus_ButtonWidget_adjustSize00
static int tolua_stratagus_ButtonWidget_adjustSize00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"ButtonWidget",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  ButtonWidget* self = (ButtonWidget*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'adjustSize'",NULL);
#endif
 {
  self->adjustSize();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'adjustSize'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  ImageButton */
#ifndef TOLUA_DISABLE_tolua_stratagus_ImageButton_new00
static int tolua_stratagus_ImageButton_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"ImageButton",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  ImageButton* tolua_ret = (ImageButton*)  new ImageButton();
 tolua_pushusertype(tolua_S,(void*)tolua_ret,"ImageButton");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  ImageButton */
#ifndef TOLUA_DISABLE_tolua_stratagus_ImageButton_new00_local
static int tolua_stratagus_ImageButton_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"ImageButton",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  ImageButton* tolua_ret = (ImageButton*)  new ImageButton();
 tolua_pushusertype_and_takeownership(tolua_S,(void *)tolua_ret,"ImageButton");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  ImageButton */
#ifndef TOLUA_DISABLE_tolua_stratagus_ImageButton_new01
static int tolua_stratagus_ImageButton_new01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"ImageButton",0,&tolua_err) ||
 !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
 {
  const std::string caption = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
 {
  ImageButton* tolua_ret = (ImageButton*)  new ImageButton(caption);
 tolua_pushusertype(tolua_S,(void*)tolua_ret,"ImageButton");
 tolua_pushcppstring(tolua_S,(const char*)caption);
 }
 }
 return 2;
tolua_lerror:
 return tolua_stratagus_ImageButton_new00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  ImageButton */
#ifndef TOLUA_DISABLE_tolua_stratagus_ImageButton_new01_local
static int tolua_stratagus_ImageButton_new01_local(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"ImageButton",0,&tolua_err) ||
 !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
 {
  const std::string caption = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
 {
  ImageButton* tolua_ret = (ImageButton*)  new ImageButton(caption);
 tolua_pushusertype_and_takeownership(tolua_S,(void *)tolua_ret,"ImageButton");
 tolua_pushcppstring(tolua_S,(const char*)caption);
 }
 }
 return 2;
tolua_lerror:
 return tolua_stratagus_ImageButton_new00_local(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: setNormalImage of class  ImageButton */
#ifndef TOLUA_DISABLE_tolua_stratagus_ImageButton_setNormalImage00
static int tolua_stratagus_ImageButton_setNormalImage00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"ImageButton",0,&tolua_err) ||
 !tolua_isusertype(tolua_S,2,"CGraphic",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  ImageButton* self = (ImageButton*)  tolua_tousertype(tolua_S,1,0);
  CGraphic* image = ((CGraphic*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setNormalImage'",NULL);
#endif
 {
  self->setNormalImage(image);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setNormalImage'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setPressedImage of class  ImageButton */
#ifndef TOLUA_DISABLE_tolua_stratagus_ImageButton_setPressedImage00
static int tolua_stratagus_ImageButton_setPressedImage00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"ImageButton",0,&tolua_err) ||
 !tolua_isusertype(tolua_S,2,"CGraphic",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  ImageButton* self = (ImageButton*)  tolua_tousertype(tolua_S,1,0);
  CGraphic* image = ((CGraphic*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setPressedImage'",NULL);
#endif
 {
  self->setPressedImage(image);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setPressedImage'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setDisabledImage of class  ImageButton */
#ifndef TOLUA_DISABLE_tolua_stratagus_ImageButton_setDisabledImage00
static int tolua_stratagus_ImageButton_setDisabledImage00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"ImageButton",0,&tolua_err) ||
 !tolua_isusertype(tolua_S,2,"CGraphic",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  ImageButton* self = (ImageButton*)  tolua_tousertype(tolua_S,1,0);
  CGraphic* image = ((CGraphic*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setDisabledImage'",NULL);
#endif
 {
  self->setDisabledImage(image);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setDisabledImage'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setHotKey of class  ImageButton */
#ifndef TOLUA_DISABLE_tolua_stratagus_ImageButton_setHotKey00
static int tolua_stratagus_ImageButton_setHotKey00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"ImageButton",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  ImageButton* self = (ImageButton*)  tolua_tousertype(tolua_S,1,0);
  const int key = ((const int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setHotKey'",NULL);
#endif
 {
  self->setHotKey(key);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setHotKey'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setHotKey of class  ImageButton */
#ifndef TOLUA_DISABLE_tolua_stratagus_ImageButton_setHotKey01
static int tolua_stratagus_ImageButton_setHotKey01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"ImageButton",0,&tolua_err) ||
 !tolua_isstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
 {
  ImageButton* self = (ImageButton*)  tolua_tousertype(tolua_S,1,0);
  const char* key = ((const char*)  tolua_tostring(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setHotKey'",NULL);
#endif
 {
  self->setHotKey(key);
 }
 }
 return 0;
tolua_lerror:
 return tolua_stratagus_ImageButton_setHotKey00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  RadioButton */
#ifndef TOLUA_DISABLE_tolua_stratagus_RadioButton_new00
static int tolua_stratagus_RadioButton_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"RadioButton",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  RadioButton* tolua_ret = (RadioButton*)  new RadioButton();
 tolua_pushusertype(tolua_S,(void*)tolua_ret,"RadioButton");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  RadioButton */
#ifndef TOLUA_DISABLE_tolua_stratagus_RadioButton_new00_local
static int tolua_stratagus_RadioButton_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"RadioButton",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  RadioButton* tolua_ret = (RadioButton*)  new RadioButton();
 tolua_pushusertype_and_takeownership(tolua_S,(void *)tolua_ret,"RadioButton");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  RadioButton */
#ifndef TOLUA_DISABLE_tolua_stratagus_RadioButton_new01
static int tolua_stratagus_RadioButton_new01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"RadioButton",0,&tolua_err) ||
 !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
 !tolua_iscppstring(tolua_S,3,0,&tolua_err) ||
 !tolua_isboolean(tolua_S,4,1,&tolua_err) ||
 !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
 goto tolua_lerror;
 else
 {
  const std::string caption = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
  const std::string group = ((const std::string)  tolua_tocppstring(tolua_S,3,0));
  bool marked = ((bool)  tolua_toboolean(tolua_S,4,false));
 {
  RadioButton* tolua_ret = (RadioButton*)  new RadioButton(caption,group,marked);
 tolua_pushusertype(tolua_S,(void*)tolua_ret,"RadioButton");
 tolua_pushcppstring(tolua_S,(const char*)caption);
 tolua_pushcppstring(tolua_S,(const char*)group);
 }
 }
 return 3;
tolua_lerror:
 return tolua_stratagus_RadioButton_new00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  RadioButton */
#ifndef TOLUA_DISABLE_tolua_stratagus_RadioButton_new01_local
static int tolua_stratagus_RadioButton_new01_local(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"RadioButton",0,&tolua_err) ||
 !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
 !tolua_iscppstring(tolua_S,3,0,&tolua_err) ||
 !tolua_isboolean(tolua_S,4,1,&tolua_err) ||
 !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
 goto tolua_lerror;
 else
 {
  const std::string caption = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
  const std::string group = ((const std::string)  tolua_tocppstring(tolua_S,3,0));
  bool marked = ((bool)  tolua_toboolean(tolua_S,4,false));
 {
  RadioButton* tolua_ret = (RadioButton*)  new RadioButton(caption,group,marked);
 tolua_pushusertype_and_takeownership(tolua_S,(void *)tolua_ret,"RadioButton");
 tolua_pushcppstring(tolua_S,(const char*)caption);
 tolua_pushcppstring(tolua_S,(const char*)group);
 }
 }
 return 3;
tolua_lerror:
 return tolua_stratagus_RadioButton_new00_local(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: isMarked of class  RadioButton */
#ifndef TOLUA_DISABLE_tolua_stratagus_RadioButton_isMarked00
static int tolua_stratagus_RadioButton_isMarked00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"RadioButton",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  RadioButton* self = (RadioButton*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'isMarked'",NULL);
#endif
 {
  bool tolua_ret = (bool)  self->isMarked();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'isMarked'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setMarked of class  RadioButton */
#ifndef TOLUA_DISABLE_tolua_stratagus_RadioButton_setMarked00
static int tolua_stratagus_RadioButton_setMarked00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"RadioButton",0,&tolua_err) ||
 !tolua_isboolean(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  RadioButton* self = (RadioButton*)  tolua_tousertype(tolua_S,1,0);
  bool marked = ((bool)  tolua_toboolean(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setMarked'",NULL);
#endif
 {
  self->setMarked(marked);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setMarked'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getCaption of class  RadioButton */
#ifndef TOLUA_DISABLE_tolua_stratagus_RadioButton_getCaption00
static int tolua_stratagus_RadioButton_getCaption00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"const RadioButton",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const RadioButton* self = (const RadioButton*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getCaption'",NULL);
#endif
 {
  const std::string tolua_ret = (const std::string)  self->getCaption();
 tolua_pushcppstring(tolua_S,(const char*)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getCaption'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setCaption of class  RadioButton */
#ifndef TOLUA_DISABLE_tolua_stratagus_RadioButton_setCaption00
static int tolua_stratagus_RadioButton_setCaption00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"RadioButton",0,&tolua_err) ||
 !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  RadioButton* self = (RadioButton*)  tolua_tousertype(tolua_S,1,0);
  const std::string caption = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setCaption'",NULL);
#endif
 {
  self->setCaption(caption);
 tolua_pushcppstring(tolua_S,(const char*)caption);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setCaption'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setGroup of class  RadioButton */
#ifndef TOLUA_DISABLE_tolua_stratagus_RadioButton_setGroup00
static int tolua_stratagus_RadioButton_setGroup00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"RadioButton",0,&tolua_err) ||
 !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  RadioButton* self = (RadioButton*)  tolua_tousertype(tolua_S,1,0);
  const std::string group = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setGroup'",NULL);
#endif
 {
  self->setGroup(group);
 tolua_pushcppstring(tolua_S,(const char*)group);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setGroup'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getGroup of class  RadioButton */
#ifndef TOLUA_DISABLE_tolua_stratagus_RadioButton_getGroup00
static int tolua_stratagus_RadioButton_getGroup00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"const RadioButton",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const RadioButton* self = (const RadioButton*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getGroup'",NULL);
#endif
 {
  const std::string tolua_ret = (const std::string)  self->getGroup();
 tolua_pushcppstring(tolua_S,(const char*)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getGroup'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: adjustSize of class  RadioButton */
#ifndef TOLUA_DISABLE_tolua_stratagus_RadioButton_adjustSize00
static int tolua_stratagus_RadioButton_adjustSize00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"RadioButton",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  RadioButton* self = (RadioButton*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'adjustSize'",NULL);
#endif
 {
  self->adjustSize();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'adjustSize'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  ImageRadioButton */
#ifndef TOLUA_DISABLE_tolua_stratagus_ImageRadioButton_new00
static int tolua_stratagus_ImageRadioButton_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"ImageRadioButton",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  ImageRadioButton* tolua_ret = (ImageRadioButton*)  new ImageRadioButton();
 tolua_pushusertype(tolua_S,(void*)tolua_ret,"ImageRadioButton");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  ImageRadioButton */
#ifndef TOLUA_DISABLE_tolua_stratagus_ImageRadioButton_new00_local
static int tolua_stratagus_ImageRadioButton_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"ImageRadioButton",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  ImageRadioButton* tolua_ret = (ImageRadioButton*)  new ImageRadioButton();
 tolua_pushusertype_and_takeownership(tolua_S,(void *)tolua_ret,"ImageRadioButton");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  ImageRadioButton */
#ifndef TOLUA_DISABLE_tolua_stratagus_ImageRadioButton_new01
static int tolua_stratagus_ImageRadioButton_new01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"ImageRadioButton",0,&tolua_err) ||
 !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
 !tolua_iscppstring(tolua_S,3,0,&tolua_err) ||
 !tolua_isboolean(tolua_S,4,1,&tolua_err) ||
 !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
 goto tolua_lerror;
 else
 {
  const std::string caption = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
  const std::string group = ((const std::string)  tolua_tocppstring(tolua_S,3,0));
  bool marked = ((bool)  tolua_toboolean(tolua_S,4,false));
 {
  ImageRadioButton* tolua_ret = (ImageRadioButton*)  new ImageRadioButton(caption,group,marked);
 tolua_pushusertype(tolua_S,(void*)tolua_ret,"ImageRadioButton");
 tolua_pushcppstring(tolua_S,(const char*)caption);
 tolua_pushcppstring(tolua_S,(const char*)group);
 }
 }
 return 3;
tolua_lerror:
 return tolua_stratagus_ImageRadioButton_new00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  ImageRadioButton */
#ifndef TOLUA_DISABLE_tolua_stratagus_ImageRadioButton_new01_local
static int tolua_stratagus_ImageRadioButton_new01_local(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"ImageRadioButton",0,&tolua_err) ||
 !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
 !tolua_iscppstring(tolua_S,3,0,&tolua_err) ||
 !tolua_isboolean(tolua_S,4,1,&tolua_err) ||
 !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
 goto tolua_lerror;
 else
 {
  const std::string caption = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
  const std::string group = ((const std::string)  tolua_tocppstring(tolua_S,3,0));
  bool marked = ((bool)  tolua_toboolean(tolua_S,4,false));
 {
  ImageRadioButton* tolua_ret = (ImageRadioButton*)  new ImageRadioButton(caption,group,marked);
 tolua_pushusertype_and_takeownership(tolua_S,(void *)tolua_ret,"ImageRadioButton");
 tolua_pushcppstring(tolua_S,(const char*)caption);
 tolua_pushcppstring(tolua_S,(const char*)group);
 }
 }
 return 3;
tolua_lerror:
 return tolua_stratagus_ImageRadioButton_new00_local(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: setUncheckedNormalImage of class  ImageRadioButton */
#ifndef TOLUA_DISABLE_tolua_stratagus_ImageRadioButton_setUncheckedNormalImage00
static int tolua_stratagus_ImageRadioButton_setUncheckedNormalImage00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"ImageRadioButton",0,&tolua_err) ||
 !tolua_isusertype(tolua_S,2,"CGraphic",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  ImageRadioButton* self = (ImageRadioButton*)  tolua_tousertype(tolua_S,1,0);
  CGraphic* image = ((CGraphic*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setUncheckedNormalImage'",NULL);
#endif
 {
  self->setUncheckedNormalImage(image);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setUncheckedNormalImage'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setUncheckedPressedImage of class  ImageRadioButton */
#ifndef TOLUA_DISABLE_tolua_stratagus_ImageRadioButton_setUncheckedPressedImage00
static int tolua_stratagus_ImageRadioButton_setUncheckedPressedImage00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"ImageRadioButton",0,&tolua_err) ||
 !tolua_isusertype(tolua_S,2,"CGraphic",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  ImageRadioButton* self = (ImageRadioButton*)  tolua_tousertype(tolua_S,1,0);
  CGraphic* image = ((CGraphic*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setUncheckedPressedImage'",NULL);
#endif
 {
  self->setUncheckedPressedImage(image);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setUncheckedPressedImage'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setCheckedNormalImage of class  ImageRadioButton */
#ifndef TOLUA_DISABLE_tolua_stratagus_ImageRadioButton_setCheckedNormalImage00
static int tolua_stratagus_ImageRadioButton_setCheckedNormalImage00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"ImageRadioButton",0,&tolua_err) ||
 !tolua_isusertype(tolua_S,2,"CGraphic",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  ImageRadioButton* self = (ImageRadioButton*)  tolua_tousertype(tolua_S,1,0);
  CGraphic* image = ((CGraphic*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setCheckedNormalImage'",NULL);
#endif
 {
  self->setCheckedNormalImage(image);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setCheckedNormalImage'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setCheckedPressedImage of class  ImageRadioButton */
#ifndef TOLUA_DISABLE_tolua_stratagus_ImageRadioButton_setCheckedPressedImage00
static int tolua_stratagus_ImageRadioButton_setCheckedPressedImage00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"ImageRadioButton",0,&tolua_err) ||
 !tolua_isusertype(tolua_S,2,"CGraphic",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  ImageRadioButton* self = (ImageRadioButton*)  tolua_tousertype(tolua_S,1,0);
  CGraphic* image = ((CGraphic*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setCheckedPressedImage'",NULL);
#endif
 {
  self->setCheckedPressedImage(image);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setCheckedPressedImage'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  CheckBox */
#ifndef TOLUA_DISABLE_tolua_stratagus_CheckBox_new00
static int tolua_stratagus_CheckBox_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"CheckBox",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  CheckBox* tolua_ret = (CheckBox*)  new CheckBox();
 tolua_pushusertype(tolua_S,(void*)tolua_ret,"CheckBox");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  CheckBox */
#ifndef TOLUA_DISABLE_tolua_stratagus_CheckBox_new00_local
static int tolua_stratagus_CheckBox_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"CheckBox",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  CheckBox* tolua_ret = (CheckBox*)  new CheckBox();
 tolua_pushusertype_and_takeownership(tolua_S,(void *)tolua_ret,"CheckBox");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  CheckBox */
#ifndef TOLUA_DISABLE_tolua_stratagus_CheckBox_new01
static int tolua_stratagus_CheckBox_new01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"CheckBox",0,&tolua_err) ||
 !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isboolean(tolua_S,3,1,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
 {
  const std::string caption = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
  bool marked = ((bool)  tolua_toboolean(tolua_S,3,false));
 {
  CheckBox* tolua_ret = (CheckBox*)  new CheckBox(caption,marked);
 tolua_pushusertype(tolua_S,(void*)tolua_ret,"CheckBox");
 tolua_pushcppstring(tolua_S,(const char*)caption);
 }
 }
 return 2;
tolua_lerror:
 return tolua_stratagus_CheckBox_new00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  CheckBox */
#ifndef TOLUA_DISABLE_tolua_stratagus_CheckBox_new01_local
static int tolua_stratagus_CheckBox_new01_local(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"CheckBox",0,&tolua_err) ||
 !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isboolean(tolua_S,3,1,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
 {
  const std::string caption = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
  bool marked = ((bool)  tolua_toboolean(tolua_S,3,false));
 {
  CheckBox* tolua_ret = (CheckBox*)  new CheckBox(caption,marked);
 tolua_pushusertype_and_takeownership(tolua_S,(void *)tolua_ret,"CheckBox");
 tolua_pushcppstring(tolua_S,(const char*)caption);
 }
 }
 return 2;
tolua_lerror:
 return tolua_stratagus_CheckBox_new00_local(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: isMarked of class  CheckBox */
#ifndef TOLUA_DISABLE_tolua_stratagus_CheckBox_isMarked00
static int tolua_stratagus_CheckBox_isMarked00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"const CheckBox",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const CheckBox* self = (const CheckBox*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'isMarked'",NULL);
#endif
 {
  bool tolua_ret = (bool)  self->isMarked();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'isMarked'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setMarked of class  CheckBox */
#ifndef TOLUA_DISABLE_tolua_stratagus_CheckBox_setMarked00
static int tolua_stratagus_CheckBox_setMarked00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"CheckBox",0,&tolua_err) ||
 !tolua_isboolean(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  CheckBox* self = (CheckBox*)  tolua_tousertype(tolua_S,1,0);
  bool marked = ((bool)  tolua_toboolean(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setMarked'",NULL);
#endif
 {
  self->setMarked(marked);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setMarked'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getCaption of class  CheckBox */
#ifndef TOLUA_DISABLE_tolua_stratagus_CheckBox_getCaption00
static int tolua_stratagus_CheckBox_getCaption00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"const CheckBox",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const CheckBox* self = (const CheckBox*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getCaption'",NULL);
#endif
 {
  const std::string tolua_ret = (const std::string)  self->getCaption();
 tolua_pushcppstring(tolua_S,(const char*)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getCaption'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setCaption of class  CheckBox */
#ifndef TOLUA_DISABLE_tolua_stratagus_CheckBox_setCaption00
static int tolua_stratagus_CheckBox_setCaption00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"CheckBox",0,&tolua_err) ||
 !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  CheckBox* self = (CheckBox*)  tolua_tousertype(tolua_S,1,0);
  const std::string caption = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setCaption'",NULL);
#endif
 {
  self->setCaption(caption);
 tolua_pushcppstring(tolua_S,(const char*)caption);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setCaption'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: adjustSize of class  CheckBox */
#ifndef TOLUA_DISABLE_tolua_stratagus_CheckBox_adjustSize00
static int tolua_stratagus_CheckBox_adjustSize00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"CheckBox",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  CheckBox* self = (CheckBox*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'adjustSize'",NULL);
#endif
 {
  self->adjustSize();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'adjustSize'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  ImageCheckBox */
#ifndef TOLUA_DISABLE_tolua_stratagus_ImageCheckBox_new00
static int tolua_stratagus_ImageCheckBox_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"ImageCheckBox",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  ImageCheckBox* tolua_ret = (ImageCheckBox*)  new ImageCheckBox();
 tolua_pushusertype(tolua_S,(void*)tolua_ret,"ImageCheckBox");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  ImageCheckBox */
#ifndef TOLUA_DISABLE_tolua_stratagus_ImageCheckBox_new00_local
static int tolua_stratagus_ImageCheckBox_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"ImageCheckBox",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  ImageCheckBox* tolua_ret = (ImageCheckBox*)  new ImageCheckBox();
 tolua_pushusertype_and_takeownership(tolua_S,(void *)tolua_ret,"ImageCheckBox");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  ImageCheckBox */
#ifndef TOLUA_DISABLE_tolua_stratagus_ImageCheckBox_new01
static int tolua_stratagus_ImageCheckBox_new01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"ImageCheckBox",0,&tolua_err) ||
 !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isboolean(tolua_S,3,1,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
 {
  const std::string caption = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
  bool marked = ((bool)  tolua_toboolean(tolua_S,3,false));
 {
  ImageCheckBox* tolua_ret = (ImageCheckBox*)  new ImageCheckBox(caption,marked);
 tolua_pushusertype(tolua_S,(void*)tolua_ret,"ImageCheckBox");
 tolua_pushcppstring(tolua_S,(const char*)caption);
 }
 }
 return 2;
tolua_lerror:
 return tolua_stratagus_ImageCheckBox_new00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  ImageCheckBox */
#ifndef TOLUA_DISABLE_tolua_stratagus_ImageCheckBox_new01_local
static int tolua_stratagus_ImageCheckBox_new01_local(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"ImageCheckBox",0,&tolua_err) ||
 !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isboolean(tolua_S,3,1,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
 {
  const std::string caption = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
  bool marked = ((bool)  tolua_toboolean(tolua_S,3,false));
 {
  ImageCheckBox* tolua_ret = (ImageCheckBox*)  new ImageCheckBox(caption,marked);
 tolua_pushusertype_and_takeownership(tolua_S,(void *)tolua_ret,"ImageCheckBox");
 tolua_pushcppstring(tolua_S,(const char*)caption);
 }
 }
 return 2;
tolua_lerror:
 return tolua_stratagus_ImageCheckBox_new00_local(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: setUncheckedNormalImage of class  ImageCheckBox */
#ifndef TOLUA_DISABLE_tolua_stratagus_ImageCheckBox_setUncheckedNormalImage00
static int tolua_stratagus_ImageCheckBox_setUncheckedNormalImage00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"ImageCheckBox",0,&tolua_err) ||
 !tolua_isusertype(tolua_S,2,"CGraphic",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  ImageCheckBox* self = (ImageCheckBox*)  tolua_tousertype(tolua_S,1,0);
  CGraphic* image = ((CGraphic*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setUncheckedNormalImage'",NULL);
#endif
 {
  self->setUncheckedNormalImage(image);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setUncheckedNormalImage'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setUncheckedPressedImage of class  ImageCheckBox */
#ifndef TOLUA_DISABLE_tolua_stratagus_ImageCheckBox_setUncheckedPressedImage00
static int tolua_stratagus_ImageCheckBox_setUncheckedPressedImage00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"ImageCheckBox",0,&tolua_err) ||
 !tolua_isusertype(tolua_S,2,"CGraphic",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  ImageCheckBox* self = (ImageCheckBox*)  tolua_tousertype(tolua_S,1,0);
  CGraphic* image = ((CGraphic*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setUncheckedPressedImage'",NULL);
#endif
 {
  self->setUncheckedPressedImage(image);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setUncheckedPressedImage'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setCheckedNormalImage of class  ImageCheckBox */
#ifndef TOLUA_DISABLE_tolua_stratagus_ImageCheckBox_setCheckedNormalImage00
static int tolua_stratagus_ImageCheckBox_setCheckedNormalImage00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"ImageCheckBox",0,&tolua_err) ||
 !tolua_isusertype(tolua_S,2,"CGraphic",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  ImageCheckBox* self = (ImageCheckBox*)  tolua_tousertype(tolua_S,1,0);
  CGraphic* image = ((CGraphic*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setCheckedNormalImage'",NULL);
#endif
 {
  self->setCheckedNormalImage(image);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setCheckedNormalImage'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setCheckedPressedImage of class  ImageCheckBox */
#ifndef TOLUA_DISABLE_tolua_stratagus_ImageCheckBox_setCheckedPressedImage00
static int tolua_stratagus_ImageCheckBox_setCheckedPressedImage00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"ImageCheckBox",0,&tolua_err) ||
 !tolua_isusertype(tolua_S,2,"CGraphic",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  ImageCheckBox* self = (ImageCheckBox*)  tolua_tousertype(tolua_S,1,0);
  CGraphic* image = ((CGraphic*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setCheckedPressedImage'",NULL);
#endif
 {
  self->setCheckedPressedImage(image);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setCheckedPressedImage'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  Slider */
#ifndef TOLUA_DISABLE_tolua_stratagus_Slider_new00
static int tolua_stratagus_Slider_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"Slider",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,1,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  double scaleEnd = ((double)  tolua_tonumber(tolua_S,2,1.0));
 {
  Slider* tolua_ret = (Slider*)  new Slider(scaleEnd);
 tolua_pushusertype(tolua_S,(void*)tolua_ret,"Slider");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  Slider */
#ifndef TOLUA_DISABLE_tolua_stratagus_Slider_new00_local
static int tolua_stratagus_Slider_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"Slider",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,1,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  double scaleEnd = ((double)  tolua_tonumber(tolua_S,2,1.0));
 {
  Slider* tolua_ret = (Slider*)  new Slider(scaleEnd);
 tolua_pushusertype_and_takeownership(tolua_S,(void *)tolua_ret,"Slider");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  Slider */
#ifndef TOLUA_DISABLE_tolua_stratagus_Slider_new01
static int tolua_stratagus_Slider_new01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"Slider",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
 {
  double scaleStart = ((double)  tolua_tonumber(tolua_S,2,0));
  double scaleEnd = ((double)  tolua_tonumber(tolua_S,3,0));
 {
  Slider* tolua_ret = (Slider*)  new Slider(scaleStart,scaleEnd);
 tolua_pushusertype(tolua_S,(void*)tolua_ret,"Slider");
 }
 }
 return 1;
tolua_lerror:
 return tolua_stratagus_Slider_new00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  Slider */
#ifndef TOLUA_DISABLE_tolua_stratagus_Slider_new01_local
static int tolua_stratagus_Slider_new01_local(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"Slider",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
 {
  double scaleStart = ((double)  tolua_tonumber(tolua_S,2,0));
  double scaleEnd = ((double)  tolua_tonumber(tolua_S,3,0));
 {
  Slider* tolua_ret = (Slider*)  new Slider(scaleStart,scaleEnd);
 tolua_pushusertype_and_takeownership(tolua_S,(void *)tolua_ret,"Slider");
 }
 }
 return 1;
tolua_lerror:
 return tolua_stratagus_Slider_new00_local(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: setScale of class  Slider */
#ifndef TOLUA_DISABLE_tolua_stratagus_Slider_setScale00
static int tolua_stratagus_Slider_setScale00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"Slider",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  Slider* self = (Slider*)  tolua_tousertype(tolua_S,1,0);
  double scaleStart = ((double)  tolua_tonumber(tolua_S,2,0));
  double scaleEnd = ((double)  tolua_tonumber(tolua_S,3,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setScale'",NULL);
#endif
 {
  self->setScale(scaleStart,scaleEnd);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setScale'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getScaleStart of class  Slider */
#ifndef TOLUA_DISABLE_tolua_stratagus_Slider_getScaleStart00
static int tolua_stratagus_Slider_getScaleStart00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"const Slider",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const Slider* self = (const Slider*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getScaleStart'",NULL);
#endif
 {
  double tolua_ret = (double)  self->getScaleStart();
 tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getScaleStart'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setScaleStart of class  Slider */
#ifndef TOLUA_DISABLE_tolua_stratagus_Slider_setScaleStart00
static int tolua_stratagus_Slider_setScaleStart00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"Slider",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  Slider* self = (Slider*)  tolua_tousertype(tolua_S,1,0);
  double scaleStart = ((double)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setScaleStart'",NULL);
#endif
 {
  self->setScaleStart(scaleStart);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setScaleStart'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getScaleEnd of class  Slider */
#ifndef TOLUA_DISABLE_tolua_stratagus_Slider_getScaleEnd00
static int tolua_stratagus_Slider_getScaleEnd00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"const Slider",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const Slider* self = (const Slider*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getScaleEnd'",NULL);
#endif
 {
  double tolua_ret = (double)  self->getScaleEnd();
 tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getScaleEnd'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setScaleEnd of class  Slider */
#ifndef TOLUA_DISABLE_tolua_stratagus_Slider_setScaleEnd00
static int tolua_stratagus_Slider_setScaleEnd00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"Slider",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  Slider* self = (Slider*)  tolua_tousertype(tolua_S,1,0);
  double scaleEnd = ((double)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setScaleEnd'",NULL);
#endif
 {
  self->setScaleEnd(scaleEnd);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setScaleEnd'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getValue of class  Slider */
#ifndef TOLUA_DISABLE_tolua_stratagus_Slider_getValue00
static int tolua_stratagus_Slider_getValue00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"Slider",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  Slider* self = (Slider*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getValue'",NULL);
#endif
 {
  double tolua_ret = (double)  self->getValue();
 tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getValue'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setValue of class  Slider */
#ifndef TOLUA_DISABLE_tolua_stratagus_Slider_setValue00
static int tolua_stratagus_Slider_setValue00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"Slider",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  Slider* self = (Slider*)  tolua_tousertype(tolua_S,1,0);
  double value = ((double)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setValue'",NULL);
#endif
 {
  self->setValue(value);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setValue'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setMarkerLength of class  Slider */
#ifndef TOLUA_DISABLE_tolua_stratagus_Slider_setMarkerLength00
static int tolua_stratagus_Slider_setMarkerLength00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"Slider",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  Slider* self = (Slider*)  tolua_tousertype(tolua_S,1,0);
  int length = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setMarkerLength'",NULL);
#endif
 {
  self->setMarkerLength(length);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setMarkerLength'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getMarkerLength of class  Slider */
#ifndef TOLUA_DISABLE_tolua_stratagus_Slider_getMarkerLength00
static int tolua_stratagus_Slider_getMarkerLength00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"const Slider",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const Slider* self = (const Slider*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getMarkerLength'",NULL);
#endif
 {
  int tolua_ret = (int)  self->getMarkerLength();
 tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getMarkerLength'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setOrientation of class  Slider */
#ifndef TOLUA_DISABLE_tolua_stratagus_Slider_setOrientation00
static int tolua_stratagus_Slider_setOrientation00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"Slider",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  Slider* self = (Slider*)  tolua_tousertype(tolua_S,1,0);
  unsigned int orientation = ((unsigned int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setOrientation'",NULL);
#endif
 {
  self->setOrientation(orientation);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setOrientation'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getOrientation of class  Slider */
#ifndef TOLUA_DISABLE_tolua_stratagus_Slider_getOrientation00
static int tolua_stratagus_Slider_getOrientation00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"const Slider",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const Slider* self = (const Slider*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getOrientation'",NULL);
#endif
 {
  unsigned int tolua_ret = (unsigned int)  self->getOrientation();
 tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getOrientation'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setStepLength of class  Slider */
#ifndef TOLUA_DISABLE_tolua_stratagus_Slider_setStepLength00
static int tolua_stratagus_Slider_setStepLength00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"Slider",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  Slider* self = (Slider*)  tolua_tousertype(tolua_S,1,0);
  double length = ((double)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setStepLength'",NULL);
#endif
 {
  self->setStepLength(length);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setStepLength'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getStepLength of class  Slider */
#ifndef TOLUA_DISABLE_tolua_stratagus_Slider_getStepLength00
static int tolua_stratagus_Slider_getStepLength00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"const Slider",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const Slider* self = (const Slider*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getStepLength'",NULL);
#endif
 {
  double tolua_ret = (double)  self->getStepLength();
 tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getStepLength'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  ImageSlider */
#ifndef TOLUA_DISABLE_tolua_stratagus_ImageSlider_new00
static int tolua_stratagus_ImageSlider_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"ImageSlider",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,1,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  double scaleEnd = ((double)  tolua_tonumber(tolua_S,2,1.0));
 {
  ImageSlider* tolua_ret = (ImageSlider*)  new ImageSlider(scaleEnd);
 tolua_pushusertype(tolua_S,(void*)tolua_ret,"ImageSlider");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  ImageSlider */
#ifndef TOLUA_DISABLE_tolua_stratagus_ImageSlider_new00_local
static int tolua_stratagus_ImageSlider_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"ImageSlider",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,1,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  double scaleEnd = ((double)  tolua_tonumber(tolua_S,2,1.0));
 {
  ImageSlider* tolua_ret = (ImageSlider*)  new ImageSlider(scaleEnd);
 tolua_pushusertype_and_takeownership(tolua_S,(void *)tolua_ret,"ImageSlider");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  ImageSlider */
#ifndef TOLUA_DISABLE_tolua_stratagus_ImageSlider_new01
static int tolua_stratagus_ImageSlider_new01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"ImageSlider",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
 {
  double scaleStart = ((double)  tolua_tonumber(tolua_S,2,0));
  double scaleEnd = ((double)  tolua_tonumber(tolua_S,3,0));
 {
  ImageSlider* tolua_ret = (ImageSlider*)  new ImageSlider(scaleStart,scaleEnd);
 tolua_pushusertype(tolua_S,(void*)tolua_ret,"ImageSlider");
 }
 }
 return 1;
tolua_lerror:
 return tolua_stratagus_ImageSlider_new00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  ImageSlider */
#ifndef TOLUA_DISABLE_tolua_stratagus_ImageSlider_new01_local
static int tolua_stratagus_ImageSlider_new01_local(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"ImageSlider",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
 {
  double scaleStart = ((double)  tolua_tonumber(tolua_S,2,0));
  double scaleEnd = ((double)  tolua_tonumber(tolua_S,3,0));
 {
  ImageSlider* tolua_ret = (ImageSlider*)  new ImageSlider(scaleStart,scaleEnd);
 tolua_pushusertype_and_takeownership(tolua_S,(void *)tolua_ret,"ImageSlider");
 }
 }
 return 1;
tolua_lerror:
 return tolua_stratagus_ImageSlider_new00_local(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: setMarkerImage of class  ImageSlider */
#ifndef TOLUA_DISABLE_tolua_stratagus_ImageSlider_setMarkerImage00
static int tolua_stratagus_ImageSlider_setMarkerImage00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"ImageSlider",0,&tolua_err) ||
 !tolua_isusertype(tolua_S,2,"CGraphic",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  ImageSlider* self = (ImageSlider*)  tolua_tousertype(tolua_S,1,0);
  CGraphic* image = ((CGraphic*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setMarkerImage'",NULL);
#endif
 {
  self->setMarkerImage(image);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setMarkerImage'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setBackgroundImage of class  ImageSlider */
#ifndef TOLUA_DISABLE_tolua_stratagus_ImageSlider_setBackgroundImage00
static int tolua_stratagus_ImageSlider_setBackgroundImage00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"ImageSlider",0,&tolua_err) ||
 !tolua_isusertype(tolua_S,2,"CGraphic",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  ImageSlider* self = (ImageSlider*)  tolua_tousertype(tolua_S,1,0);
  CGraphic* image = ((CGraphic*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setBackgroundImage'",NULL);
#endif
 {
  self->setBackgroundImage(image);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setBackgroundImage'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  Label */
#ifndef TOLUA_DISABLE_tolua_stratagus_Label_new00
static int tolua_stratagus_Label_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"Label",0,&tolua_err) ||
 !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const std::string caption = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
 {
  Label* tolua_ret = (Label*)  new Label(caption);
 tolua_pushusertype(tolua_S,(void*)tolua_ret,"Label");
 tolua_pushcppstring(tolua_S,(const char*)caption);
 }
 }
 return 2;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  Label */
#ifndef TOLUA_DISABLE_tolua_stratagus_Label_new00_local
static int tolua_stratagus_Label_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"Label",0,&tolua_err) ||
 !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const std::string caption = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
 {
  Label* tolua_ret = (Label*)  new Label(caption);
 tolua_pushusertype_and_takeownership(tolua_S,(void *)tolua_ret,"Label");
 tolua_pushcppstring(tolua_S,(const char*)caption);
 }
 }
 return 2;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getCaption of class  Label */
#ifndef TOLUA_DISABLE_tolua_stratagus_Label_getCaption00
static int tolua_stratagus_Label_getCaption00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"const Label",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const Label* self = (const Label*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getCaption'",NULL);
#endif
 {
  const std::string tolua_ret = (const std::string)  self->getCaption();
 tolua_pushcppstring(tolua_S,(const char*)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getCaption'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setCaption of class  Label */
#ifndef TOLUA_DISABLE_tolua_stratagus_Label_setCaption00
static int tolua_stratagus_Label_setCaption00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"Label",0,&tolua_err) ||
 !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  Label* self = (Label*)  tolua_tousertype(tolua_S,1,0);
  const std::string caption = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setCaption'",NULL);
#endif
 {
  self->setCaption(caption);
 tolua_pushcppstring(tolua_S,(const char*)caption);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setCaption'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: adjustSize of class  Label */
#ifndef TOLUA_DISABLE_tolua_stratagus_Label_adjustSize00
static int tolua_stratagus_Label_adjustSize00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"Label",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  Label* self = (Label*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'adjustSize'",NULL);
#endif
 {
  self->adjustSize();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'adjustSize'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  TextField */
#ifndef TOLUA_DISABLE_tolua_stratagus_TextField_new00
static int tolua_stratagus_TextField_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"TextField",0,&tolua_err) ||
 !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const std::string text = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
 {
  TextField* tolua_ret = (TextField*)  new TextField(text);
 tolua_pushusertype(tolua_S,(void*)tolua_ret,"TextField");
 tolua_pushcppstring(tolua_S,(const char*)text);
 }
 }
 return 2;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  TextField */
#ifndef TOLUA_DISABLE_tolua_stratagus_TextField_new00_local
static int tolua_stratagus_TextField_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"TextField",0,&tolua_err) ||
 !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const std::string text = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
 {
  TextField* tolua_ret = (TextField*)  new TextField(text);
 tolua_pushusertype_and_takeownership(tolua_S,(void *)tolua_ret,"TextField");
 tolua_pushcppstring(tolua_S,(const char*)text);
 }
 }
 return 2;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  ListBoxWidget */
#ifndef TOLUA_DISABLE_tolua_stratagus_ListBoxWidget_new00
static int tolua_stratagus_ListBoxWidget_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"ListBoxWidget",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  unsigned int width = ((unsigned int)  tolua_tonumber(tolua_S,2,0));
  unsigned int height = ((unsigned int)  tolua_tonumber(tolua_S,3,0));
 {
  ListBoxWidget* tolua_ret = (ListBoxWidget*)  new ListBoxWidget(width,height);
 tolua_pushusertype(tolua_S,(void*)tolua_ret,"ListBoxWidget");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  ListBoxWidget */
#ifndef TOLUA_DISABLE_tolua_stratagus_ListBoxWidget_new00_local
static int tolua_stratagus_ListBoxWidget_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"ListBoxWidget",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  unsigned int width = ((unsigned int)  tolua_tonumber(tolua_S,2,0));
  unsigned int height = ((unsigned int)  tolua_tonumber(tolua_S,3,0));
 {
  ListBoxWidget* tolua_ret = (ListBoxWidget*)  new ListBoxWidget(width,height);
 tolua_pushusertype_and_takeownership(tolua_S,(void *)tolua_ret,"ListBoxWidget");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setList of class  ListBoxWidget */
#ifndef TOLUA_DISABLE_tolua_stratagus_ListBoxWidget_setList00
static int tolua_stratagus_ListBoxWidget_setList00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"ListBoxWidget",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  ListBoxWidget* self = (ListBoxWidget*)  tolua_tousertype(tolua_S,1,0);
  lua_State* lua =  tolua_S;
  lua_Object lo = ((lua_Object)  tolua_tovalue(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setList'",NULL);
#endif
 {
  self->setList(lua,&lo);
 tolua_pushvalue(tolua_S,(int)lo);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setList'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setSelected of class  ListBoxWidget */
#ifndef TOLUA_DISABLE_tolua_stratagus_ListBoxWidget_setSelected00
static int tolua_stratagus_ListBoxWidget_setSelected00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"ListBoxWidget",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  ListBoxWidget* self = (ListBoxWidget*)  tolua_tousertype(tolua_S,1,0);
  int selected = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setSelected'",NULL);
#endif
 {
  self->setSelected(selected);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setSelected'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getSelected of class  ListBoxWidget */
#ifndef TOLUA_DISABLE_tolua_stratagus_ListBoxWidget_getSelected00
static int tolua_stratagus_ListBoxWidget_getSelected00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"ListBoxWidget",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  ListBoxWidget* self = (ListBoxWidget*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getSelected'",NULL);
#endif
 {
  int tolua_ret = (int)  self->getSelected();
 tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getSelected'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  DropDownWidget */
#ifndef TOLUA_DISABLE_tolua_stratagus_DropDownWidget_new00
static int tolua_stratagus_DropDownWidget_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"DropDownWidget",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  DropDownWidget* tolua_ret = (DropDownWidget*)  new DropDownWidget();
 tolua_pushusertype(tolua_S,(void*)tolua_ret,"DropDownWidget");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  DropDownWidget */
#ifndef TOLUA_DISABLE_tolua_stratagus_DropDownWidget_new00_local
static int tolua_stratagus_DropDownWidget_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"DropDownWidget",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  DropDownWidget* tolua_ret = (DropDownWidget*)  new DropDownWidget();
 tolua_pushusertype_and_takeownership(tolua_S,(void *)tolua_ret,"DropDownWidget");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setList of class  DropDownWidget */
#ifndef TOLUA_DISABLE_tolua_stratagus_DropDownWidget_setList00
static int tolua_stratagus_DropDownWidget_setList00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"DropDownWidget",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  DropDownWidget* self = (DropDownWidget*)  tolua_tousertype(tolua_S,1,0);
  lua_State* lua =  tolua_S;
  lua_Object lo = ((lua_Object)  tolua_tovalue(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setList'",NULL);
#endif
 {
  self->setList(lua,&lo);
 tolua_pushvalue(tolua_S,(int)lo);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setList'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getSelected of class  DropDownWidget */
#ifndef TOLUA_DISABLE_tolua_stratagus_DropDownWidget_getSelected00
static int tolua_stratagus_DropDownWidget_getSelected00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"DropDownWidget",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  DropDownWidget* self = (DropDownWidget*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getSelected'",NULL);
#endif
 {
  int tolua_ret = (int)  self->getSelected();
 tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getSelected'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setSelected of class  DropDownWidget */
#ifndef TOLUA_DISABLE_tolua_stratagus_DropDownWidget_setSelected00
static int tolua_stratagus_DropDownWidget_setSelected00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"DropDownWidget",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  DropDownWidget* self = (DropDownWidget*)  tolua_tousertype(tolua_S,1,0);
  int selection = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setSelected'",NULL);
#endif
 {
  self->setSelected(selection);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setSelected'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  Windows */
#ifndef TOLUA_DISABLE_tolua_stratagus_Windows_new00
static int tolua_stratagus_Windows_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"Windows",0,&tolua_err) ||
 !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const std::string text = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
  int width = ((int)  tolua_tonumber(tolua_S,3,0));
  int height = ((int)  tolua_tonumber(tolua_S,4,0));
 {
  Windows* tolua_ret = (Windows*)  new Windows(text,width,height);
 tolua_pushusertype(tolua_S,(void*)tolua_ret,"Windows");
 tolua_pushcppstring(tolua_S,(const char*)text);
 }
 }
 return 2;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  Windows */
#ifndef TOLUA_DISABLE_tolua_stratagus_Windows_new00_local
static int tolua_stratagus_Windows_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"Windows",0,&tolua_err) ||
 !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const std::string text = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
  int width = ((int)  tolua_tonumber(tolua_S,3,0));
  int height = ((int)  tolua_tonumber(tolua_S,4,0));
 {
  Windows* tolua_ret = (Windows*)  new Windows(text,width,height);
 tolua_pushusertype_and_takeownership(tolua_S,(void *)tolua_ret,"Windows");
 tolua_pushcppstring(tolua_S,(const char*)text);
 }
 }
 return 2;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: add of class  Windows */
#ifndef TOLUA_DISABLE_tolua_stratagus_Windows_add00
static int tolua_stratagus_Windows_add00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"Windows",0,&tolua_err) ||
 !tolua_isusertype(tolua_S,2,"Widget",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  Windows* self = (Windows*)  tolua_tousertype(tolua_S,1,0);
  Widget* widget = ((Widget*)  tolua_tousertype(tolua_S,2,0));
  int x = ((int)  tolua_tonumber(tolua_S,3,0));
  int y = ((int)  tolua_tonumber(tolua_S,4,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'add'",NULL);
#endif
 {
  self->add(widget,x,y);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'add'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  ScrollingWidget */
#ifndef TOLUA_DISABLE_tolua_stratagus_ScrollingWidget_new00
static int tolua_stratagus_ScrollingWidget_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"ScrollingWidget",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int width = ((int)  tolua_tonumber(tolua_S,2,0));
  int height = ((int)  tolua_tonumber(tolua_S,3,0));
 {
  ScrollingWidget* tolua_ret = (ScrollingWidget*)  new ScrollingWidget(width,height);
 tolua_pushusertype(tolua_S,(void*)tolua_ret,"ScrollingWidget");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  ScrollingWidget */
#ifndef TOLUA_DISABLE_tolua_stratagus_ScrollingWidget_new00_local
static int tolua_stratagus_ScrollingWidget_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"ScrollingWidget",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int width = ((int)  tolua_tonumber(tolua_S,2,0));
  int height = ((int)  tolua_tonumber(tolua_S,3,0));
 {
  ScrollingWidget* tolua_ret = (ScrollingWidget*)  new ScrollingWidget(width,height);
 tolua_pushusertype_and_takeownership(tolua_S,(void *)tolua_ret,"ScrollingWidget");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: add of class  ScrollingWidget */
#ifndef TOLUA_DISABLE_tolua_stratagus_ScrollingWidget_add00
static int tolua_stratagus_ScrollingWidget_add00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"ScrollingWidget",0,&tolua_err) ||
 !tolua_isusertype(tolua_S,2,"Widget",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  ScrollingWidget* self = (ScrollingWidget*)  tolua_tousertype(tolua_S,1,0);
  Widget* widget = ((Widget*)  tolua_tousertype(tolua_S,2,0));
  int x = ((int)  tolua_tonumber(tolua_S,3,0));
  int y = ((int)  tolua_tonumber(tolua_S,4,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'add'",NULL);
#endif
 {
  self->add(widget,x,y);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'add'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: restart of class  ScrollingWidget */
#ifndef TOLUA_DISABLE_tolua_stratagus_ScrollingWidget_restart00
static int tolua_stratagus_ScrollingWidget_restart00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"ScrollingWidget",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  ScrollingWidget* self = (ScrollingWidget*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'restart'",NULL);
#endif
 {
  self->restart();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'restart'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  StatBoxWidget */
#ifndef TOLUA_DISABLE_tolua_stratagus_StatBoxWidget_new00
static int tolua_stratagus_StatBoxWidget_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"StatBoxWidget",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int width = ((int)  tolua_tonumber(tolua_S,2,0));
  int height = ((int)  tolua_tonumber(tolua_S,3,0));
 {
  StatBoxWidget* tolua_ret = (StatBoxWidget*)  new StatBoxWidget(width,height);
 tolua_pushusertype(tolua_S,(void*)tolua_ret,"StatBoxWidget");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  StatBoxWidget */
#ifndef TOLUA_DISABLE_tolua_stratagus_StatBoxWidget_new00_local
static int tolua_stratagus_StatBoxWidget_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"StatBoxWidget",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int width = ((int)  tolua_tonumber(tolua_S,2,0));
  int height = ((int)  tolua_tonumber(tolua_S,3,0));
 {
  StatBoxWidget* tolua_ret = (StatBoxWidget*)  new StatBoxWidget(width,height);
 tolua_pushusertype_and_takeownership(tolua_S,(void *)tolua_ret,"StatBoxWidget");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* get function: caption of class  StatBoxWidget */
#ifndef TOLUA_DISABLE_tolua_get_StatBoxWidget_caption
static int tolua_get_StatBoxWidget_caption(lua_State* tolua_S)
{
  StatBoxWidget* self = (StatBoxWidget*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'caption'",NULL);
#endif
 tolua_pushcppstring(tolua_S,(const char*)self->Getcaption());
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: caption of class  StatBoxWidget */
#ifndef TOLUA_DISABLE_tolua_set_StatBoxWidget_caption
static int tolua_set_StatBoxWidget_caption(lua_State* tolua_S)
{
  StatBoxWidget* self = (StatBoxWidget*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'caption'",NULL);
 if (!tolua_iscppstring(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Setcaption(((std::string)  tolua_tocppstring(tolua_S,2,0))
)
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: percent of class  StatBoxWidget */
#ifndef TOLUA_DISABLE_tolua_get_StatBoxWidget_unsigned_percent
static int tolua_get_StatBoxWidget_unsigned_percent(lua_State* tolua_S)
{
  StatBoxWidget* self = (StatBoxWidget*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'percent'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->Getpercent());
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: percent of class  StatBoxWidget */
#ifndef TOLUA_DISABLE_tolua_set_StatBoxWidget_unsigned_percent
static int tolua_set_StatBoxWidget_unsigned_percent(lua_State* tolua_S)
{
  StatBoxWidget* self = (StatBoxWidget*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'percent'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Setpercent((( unsigned int)  tolua_tonumber(tolua_S,2,0))
)
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* method: add of class  Container */
#ifndef TOLUA_DISABLE_tolua_stratagus_Container_add00
static int tolua_stratagus_Container_add00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"Container",0,&tolua_err) ||
 !tolua_isusertype(tolua_S,2,"Widget",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  Container* self = (Container*)  tolua_tousertype(tolua_S,1,0);
  Widget* widget = ((Widget*)  tolua_tousertype(tolua_S,2,0));
  int x = ((int)  tolua_tonumber(tolua_S,3,0));
  int y = ((int)  tolua_tonumber(tolua_S,4,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'add'",NULL);
#endif
 {
  self->add(widget,x,y);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'add'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: clear of class  Container */
#ifndef TOLUA_DISABLE_tolua_stratagus_Container_clear00
static int tolua_stratagus_Container_clear00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"Container",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  Container* self = (Container*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'clear'",NULL);
#endif
 {
  self->clear();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'clear'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  MenuScreen */
#ifndef TOLUA_DISABLE_tolua_stratagus_MenuScreen_new00
static int tolua_stratagus_MenuScreen_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"MenuScreen",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  MenuScreen* tolua_ret = (MenuScreen*)  new MenuScreen();
 tolua_pushusertype(tolua_S,(void*)tolua_ret,"MenuScreen");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  MenuScreen */
#ifndef TOLUA_DISABLE_tolua_stratagus_MenuScreen_new00_local
static int tolua_stratagus_MenuScreen_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"MenuScreen",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  MenuScreen* tolua_ret = (MenuScreen*)  new MenuScreen();
 tolua_pushusertype_and_takeownership(tolua_S,(void *)tolua_ret,"MenuScreen");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: run of class  MenuScreen */
#ifndef TOLUA_DISABLE_tolua_stratagus_MenuScreen_run00
static int tolua_stratagus_MenuScreen_run00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"MenuScreen",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  MenuScreen* self = (MenuScreen*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'run'",NULL);
#endif
 {
  int tolua_ret = (int)  self->run();
 tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'run'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: stop of class  MenuScreen */
#ifndef TOLUA_DISABLE_tolua_stratagus_MenuScreen_stop00
static int tolua_stratagus_MenuScreen_stop00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"MenuScreen",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,1,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  MenuScreen* self = (MenuScreen*)  tolua_tousertype(tolua_S,1,0);
  int result = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'stop'",NULL);
#endif
 {
  self->stop(result);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'stop'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: StartMap */
#ifndef TOLUA_DISABLE_tolua_stratagus_StartMap00
static int tolua_stratagus_StartMap00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isstring(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const char* str = ((const char*)  tolua_tostring(tolua_S,1,0));
 {
  StartMap(str);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'StartMap'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: StartEditor */
#ifndef TOLUA_DISABLE_tolua_stratagus_StartEditor00
static int tolua_stratagus_StartEditor00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isstring(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const char* str = ((const char*)  tolua_tostring(tolua_S,1,0));
 {
  StartEditor(str);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'StartEditor'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: StartReplay */
#ifndef TOLUA_DISABLE_tolua_stratagus_StartReplay00
static int tolua_stratagus_StartReplay00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isstring(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const char* str = ((const char*)  tolua_tostring(tolua_S,1,0));
 {
  StartReplay(str);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'StartReplay'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: StartSavedGame */
#ifndef TOLUA_DISABLE_tolua_stratagus_StartSavedGame00
static int tolua_stratagus_StartSavedGame00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isstring(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const char* str = ((const char*)  tolua_tostring(tolua_S,1,0));
 {
  StartSavedGame(str);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'StartSavedGame'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Width of class  CVideo */
#ifndef TOLUA_DISABLE_tolua_get_CVideo_Width
static int tolua_get_CVideo_Width(lua_State* tolua_S)
{
  CVideo* self = (CVideo*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Width'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->Width);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Width of class  CVideo */
#ifndef TOLUA_DISABLE_tolua_set_CVideo_Width
static int tolua_set_CVideo_Width(lua_State* tolua_S)
{
  CVideo* self = (CVideo*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Width'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Width = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Height of class  CVideo */
#ifndef TOLUA_DISABLE_tolua_get_CVideo_Height
static int tolua_get_CVideo_Height(lua_State* tolua_S)
{
  CVideo* self = (CVideo*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Height'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->Height);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Height of class  CVideo */
#ifndef TOLUA_DISABLE_tolua_set_CVideo_Height
static int tolua_set_CVideo_Height(lua_State* tolua_S)
{
  CVideo* self = (CVideo*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Height'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Height = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Depth of class  CVideo */
#ifndef TOLUA_DISABLE_tolua_get_CVideo_Depth
static int tolua_get_CVideo_Depth(lua_State* tolua_S)
{
  CVideo* self = (CVideo*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Depth'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->Depth);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Depth of class  CVideo */
#ifndef TOLUA_DISABLE_tolua_set_CVideo_Depth
static int tolua_set_CVideo_Depth(lua_State* tolua_S)
{
  CVideo* self = (CVideo*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Depth'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Depth = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: FullScreen of class  CVideo */
#ifndef TOLUA_DISABLE_tolua_get_CVideo_FullScreen
static int tolua_get_CVideo_FullScreen(lua_State* tolua_S)
{
  CVideo* self = (CVideo*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'FullScreen'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->FullScreen);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: FullScreen of class  CVideo */
#ifndef TOLUA_DISABLE_tolua_set_CVideo_FullScreen
static int tolua_set_CVideo_FullScreen(lua_State* tolua_S)
{
  CVideo* self = (CVideo*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'FullScreen'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->FullScreen = ((bool)  tolua_toboolean(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* method: ResizeScreen of class  CVideo */
#ifndef TOLUA_DISABLE_tolua_stratagus_CVideo_ResizeScreen00
static int tolua_stratagus_CVideo_ResizeScreen00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"CVideo",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  CVideo* self = (CVideo*)  tolua_tousertype(tolua_S,1,0);
  int width = ((int)  tolua_tonumber(tolua_S,2,0));
  int height = ((int)  tolua_tonumber(tolua_S,3,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'ResizeScreen'",NULL);
#endif
 {
  self->ResizeScreen(width,height);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'ResizeScreen'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Video */
#ifndef TOLUA_DISABLE_tolua_get_Video
static int tolua_get_Video(lua_State* tolua_S)
{
 tolua_pushusertype(tolua_S,(void*)&Video,"CVideo");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Video */
#ifndef TOLUA_DISABLE_tolua_set_Video
static int tolua_set_Video(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!tolua_isusertype(tolua_S,2,"CVideo",0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  Video = *((CVideo*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* function: ToggleFullScreen */
#ifndef TOLUA_DISABLE_tolua_stratagus_ToggleFullScreen00
static int tolua_stratagus_ToggleFullScreen00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  ToggleFullScreen();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'ToggleFullScreen'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: New of class  CGraphic */
#ifndef TOLUA_DISABLE_tolua_stratagus_CGraphic_New00
static int tolua_stratagus_CGraphic_New00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"CGraphic",0,&tolua_err) ||
 !tolua_isstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,1,&tolua_err) ||
 !tolua_isnumber(tolua_S,4,1,&tolua_err) ||
 !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const char* file = ((const char*)  tolua_tostring(tolua_S,2,0));
  int w = ((int)  tolua_tonumber(tolua_S,3,0));
  int h = ((int)  tolua_tonumber(tolua_S,4,0));
 {
  CGraphic* tolua_ret = (CGraphic*)  CGraphic::New(file,w,h);
 tolua_pushusertype(tolua_S,(void*)tolua_ret,"CGraphic");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'New'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: Free of class  CGraphic */
#ifndef TOLUA_DISABLE_tolua_stratagus_CGraphic_Free00
static int tolua_stratagus_CGraphic_Free00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"CGraphic",0,&tolua_err) ||
 !tolua_isusertype(tolua_S,2,"CGraphic",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  CGraphic* tolua_var_1 = ((CGraphic*)  tolua_tousertype(tolua_S,2,0));
 {
  CGraphic::Free(tolua_var_1);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Free'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: Load of class  CGraphic */
#ifndef TOLUA_DISABLE_tolua_stratagus_CGraphic_Load00
static int tolua_stratagus_CGraphic_Load00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"CGraphic",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  CGraphic* self = (CGraphic*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'Load'",NULL);
#endif
 {
  self->Load();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Load'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: Resize of class  CGraphic */
#ifndef TOLUA_DISABLE_tolua_stratagus_CGraphic_Resize00
static int tolua_stratagus_CGraphic_Resize00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"CGraphic",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  CGraphic* self = (CGraphic*)  tolua_tousertype(tolua_S,1,0);
  int w = ((int)  tolua_tonumber(tolua_S,2,0));
  int h = ((int)  tolua_tonumber(tolua_S,3,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'Resize'",NULL);
#endif
 {
  self->Resize(w,h);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Resize'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  CColor */
#ifndef TOLUA_DISABLE_tolua_stratagus_CColor_new00
static int tolua_stratagus_CColor_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"CColor",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,1,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,1,&tolua_err) ||
 !tolua_isnumber(tolua_S,4,1,&tolua_err) ||
 !tolua_isnumber(tolua_S,5,1,&tolua_err) ||
 !tolua_isnoobj(tolua_S,6,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  unsigned char r = ((unsigned char)  tolua_tonumber(tolua_S,2,0));
  unsigned char g = ((unsigned char)  tolua_tonumber(tolua_S,3,0));
  unsigned char b = ((unsigned char)  tolua_tonumber(tolua_S,4,0));
  unsigned char a = ((unsigned char)  tolua_tonumber(tolua_S,5,0));
 {
  CColor* tolua_ret = (CColor*)  new CColor(r,g,b,a);
 tolua_pushusertype(tolua_S,(void*)tolua_ret,"CColor");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  CColor */
#ifndef TOLUA_DISABLE_tolua_stratagus_CColor_new00_local
static int tolua_stratagus_CColor_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"CColor",0,&tolua_err) ||
 !tolua_isnumber(tolua_S,2,1,&tolua_err) ||
 !tolua_isnumber(tolua_S,3,1,&tolua_err) ||
 !tolua_isnumber(tolua_S,4,1,&tolua_err) ||
 !tolua_isnumber(tolua_S,5,1,&tolua_err) ||
 !tolua_isnoobj(tolua_S,6,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  unsigned char r = ((unsigned char)  tolua_tonumber(tolua_S,2,0));
  unsigned char g = ((unsigned char)  tolua_tonumber(tolua_S,3,0));
  unsigned char b = ((unsigned char)  tolua_tonumber(tolua_S,4,0));
  unsigned char a = ((unsigned char)  tolua_tonumber(tolua_S,5,0));
 {
  CColor* tolua_ret = (CColor*)  new CColor(r,g,b,a);
 tolua_pushusertype_and_takeownership(tolua_S,(void *)tolua_ret,"CColor");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'new'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* get function: R of class  CColor */
#ifndef TOLUA_DISABLE_tolua_get_CColor_unsigned_R
static int tolua_get_CColor_unsigned_R(lua_State* tolua_S)
{
  CColor* self = (CColor*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'R'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->R);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: R of class  CColor */
#ifndef TOLUA_DISABLE_tolua_set_CColor_unsigned_R
static int tolua_set_CColor_unsigned_R(lua_State* tolua_S)
{
  CColor* self = (CColor*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'R'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->R = ((unsigned char)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: G of class  CColor */
#ifndef TOLUA_DISABLE_tolua_get_CColor_unsigned_G
static int tolua_get_CColor_unsigned_G(lua_State* tolua_S)
{
  CColor* self = (CColor*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'G'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->G);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: G of class  CColor */
#ifndef TOLUA_DISABLE_tolua_set_CColor_unsigned_G
static int tolua_set_CColor_unsigned_G(lua_State* tolua_S)
{
  CColor* self = (CColor*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'G'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->G = ((unsigned char)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: B of class  CColor */
#ifndef TOLUA_DISABLE_tolua_get_CColor_unsigned_B
static int tolua_get_CColor_unsigned_B(lua_State* tolua_S)
{
  CColor* self = (CColor*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'B'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->B);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: B of class  CColor */
#ifndef TOLUA_DISABLE_tolua_set_CColor_unsigned_B
static int tolua_set_CColor_unsigned_B(lua_State* tolua_S)
{
  CColor* self = (CColor*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'B'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->B = ((unsigned char)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: A of class  CColor */
#ifndef TOLUA_DISABLE_tolua_get_CColor_unsigned_A
static int tolua_get_CColor_unsigned_A(lua_State* tolua_S)
{
  CColor* self = (CColor*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'A'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->A);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: A of class  CColor */
#ifndef TOLUA_DISABLE_tolua_set_CColor_unsigned_A
static int tolua_set_CColor_unsigned_A(lua_State* tolua_S)
{
  CColor* self = (CColor*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'A'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->A = ((unsigned char)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* method: New of class  CFont */
#ifndef TOLUA_DISABLE_tolua_stratagus_CFont_New00
static int tolua_stratagus_CFont_New00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"CFont",0,&tolua_err) ||
 !tolua_isstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isusertype(tolua_S,3,"CGraphic",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const char* ident = ((const char*)  tolua_tostring(tolua_S,2,0));
  CGraphic* g = ((CGraphic*)  tolua_tousertype(tolua_S,3,0));
 {
  CFont* tolua_ret = (CFont*)  CFont::New(ident,g);
 tolua_pushusertype(tolua_S,(void*)tolua_ret,"CFont");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'New'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: Get of class  CFont */
#ifndef TOLUA_DISABLE_tolua_stratagus_CFont_Get00
static int tolua_stratagus_CFont_Get00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"CFont",0,&tolua_err) ||
 !tolua_isstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const char* ident = ((const char*)  tolua_tostring(tolua_S,2,0));
 {
  CFont* tolua_ret = (CFont*)  CFont::Get(ident);
 tolua_pushusertype(tolua_S,(void*)tolua_ret,"CFont");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Get'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: Height of class  CFont */
#ifndef TOLUA_DISABLE_tolua_stratagus_CFont_Height00
static int tolua_stratagus_CFont_Height00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"CFont",0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  CFont* self = (CFont*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'Height'",NULL);
#endif
 {
  int tolua_ret = (int)  self->Height();
 tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Height'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: Width of class  CFont */
#ifndef TOLUA_DISABLE_tolua_stratagus_CFont_Width00
static int tolua_stratagus_CFont_Width00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertype(tolua_S,1,"CFont",0,&tolua_err) ||
 !tolua_isstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  CFont* self = (CFont*)  tolua_tousertype(tolua_S,1,0);
  const char* text = ((const char*)  tolua_tostring(tolua_S,2,0));
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in function 'Width'",NULL);
#endif
 {
  int tolua_ret = (int)  self->Width(text);
 tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Width'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: New of class  CFontColor */
#ifndef TOLUA_DISABLE_tolua_stratagus_CFontColor_New00
static int tolua_stratagus_CFontColor_New00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"CFontColor",0,&tolua_err) ||
 !tolua_isstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const char* ident = ((const char*)  tolua_tostring(tolua_S,2,0));
 {
  CFontColor* tolua_ret = (CFontColor*)  CFontColor::New(ident);
 tolua_pushusertype(tolua_S,(void*)tolua_ret,"CFontColor");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'New'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: Get of class  CFontColor */
#ifndef TOLUA_DISABLE_tolua_stratagus_CFontColor_Get00
static int tolua_stratagus_CFontColor_Get00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"CFontColor",0,&tolua_err) ||
 !tolua_isstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const char* ident = ((const char*)  tolua_tostring(tolua_S,2,0));
 {
  CFontColor* tolua_ret = (CFontColor*)  CFontColor::Get(ident);
 tolua_pushusertype(tolua_S,(void*)tolua_ret,"CFontColor");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Get'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Colors of class  CFontColor */
#ifndef TOLUA_DISABLE_tolua_get_stratagus_CFontColor_Colors
static int tolua_get_stratagus_CFontColor_Colors(lua_State* tolua_S)
{
 int tolua_index;
  CFontColor* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (CFontColor*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0);
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=MaxFontColors)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushusertype(tolua_S,(void*)&self->Colors[tolua_index],"CColor");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Colors of class  CFontColor */
#ifndef TOLUA_DISABLE_tolua_set_stratagus_CFontColor_Colors
static int tolua_set_stratagus_CFontColor_Colors(lua_State* tolua_S)
{
 int tolua_index;
  CFontColor* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (CFontColor*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0);
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=MaxFontColors)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
  self->Colors[tolua_index] = *((CColor*)  tolua_tousertype(tolua_S,3,0));
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* method: New of class  CUpgrade */
#ifndef TOLUA_DISABLE_tolua_stratagus_CUpgrade_New00
static int tolua_stratagus_CUpgrade_New00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"CUpgrade",0,&tolua_err) ||
 !tolua_isstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const char* ident = ((const char*)  tolua_tostring(tolua_S,2,0));
 {
  CUpgrade* tolua_ret = (CUpgrade*)  CUpgrade::New(ident);
 tolua_pushusertype(tolua_S,(void*)tolua_ret,"CUpgrade");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'New'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: Get of class  CUpgrade */
#ifndef TOLUA_DISABLE_tolua_stratagus_CUpgrade_Get00
static int tolua_stratagus_CUpgrade_Get00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isusertable(tolua_S,1,"CUpgrade",0,&tolua_err) ||
 !tolua_isstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const char* ident = ((const char*)  tolua_tostring(tolua_S,2,0));
 {
  CUpgrade* tolua_ret = (CUpgrade*)  CUpgrade::Get(ident);
 tolua_pushusertype(tolua_S,(void*)tolua_ret,"CUpgrade");
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Get'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Costs of class  CUpgrade */
#ifndef TOLUA_DISABLE_tolua_get_stratagus_CUpgrade_Costs
static int tolua_get_stratagus_CUpgrade_Costs(lua_State* tolua_S)
{
 int tolua_index;
  CUpgrade* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (CUpgrade*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0);
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=MaxCosts)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->Costs[tolua_index]);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Costs of class  CUpgrade */
#ifndef TOLUA_DISABLE_tolua_set_stratagus_CUpgrade_Costs
static int tolua_set_stratagus_CUpgrade_Costs(lua_State* tolua_S)
{
 int tolua_index;
  CUpgrade* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (CUpgrade*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0);
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=MaxCosts)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
  self->Costs[tolua_index] = ((int)  tolua_tonumber(tolua_S,3,0));
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Icon of class  CUpgrade */
#ifndef TOLUA_DISABLE_tolua_get_CUpgrade_Icon_ptr
static int tolua_get_CUpgrade_Icon_ptr(lua_State* tolua_S)
{
  CUpgrade* self = (CUpgrade*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Icon'",NULL);
#endif
 tolua_pushusertype(tolua_S,(void*)self->Icon,"CIcon");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Icon of class  CUpgrade */
#ifndef TOLUA_DISABLE_tolua_set_CUpgrade_Icon_ptr
static int tolua_set_CUpgrade_Icon_ptr(lua_State* tolua_S)
{
  CUpgrade* self = (CUpgrade*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Icon'",NULL);
 if (!tolua_isusertype(tolua_S,2,"CIcon",0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Icon = ((CIcon*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Name of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_get_CPlayer_Name
static int tolua_get_CPlayer_Name(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Name'",NULL);
#endif
 tolua_pushstring(tolua_S,(const char*)self->Name);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Name of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_set_CPlayer_Name
static int tolua_set_CPlayer_Name(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Name'",NULL);
 if (!tolua_isstring(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Name = ((char*)  tolua_tostring(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Resources of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_get_stratagus_CPlayer_Resources
static int tolua_get_stratagus_CPlayer_Resources(lua_State* tolua_S)
{
 int tolua_index;
  CPlayer* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (CPlayer*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0);
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=MaxCosts)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->Resources[tolua_index]);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Resources of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_set_stratagus_CPlayer_Resources
static int tolua_set_stratagus_CPlayer_Resources(lua_State* tolua_S)
{
 int tolua_index;
  CPlayer* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (CPlayer*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0);
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=MaxCosts)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
  self->Resources[tolua_index] = ((int)  tolua_tonumber(tolua_S,3,0));
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Incomes of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_get_stratagus_CPlayer_Incomes
static int tolua_get_stratagus_CPlayer_Incomes(lua_State* tolua_S)
{
 int tolua_index;
  CPlayer* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (CPlayer*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0);
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=MaxCosts)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->Incomes[tolua_index]);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Incomes of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_set_stratagus_CPlayer_Incomes
static int tolua_set_stratagus_CPlayer_Incomes(lua_State* tolua_S)
{
 int tolua_index;
  CPlayer* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (CPlayer*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0);
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=MaxCosts)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
  self->Incomes[tolua_index] = ((int)  tolua_tonumber(tolua_S,3,0));
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Revenue of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_get_stratagus_CPlayer_Revenue
static int tolua_get_stratagus_CPlayer_Revenue(lua_State* tolua_S)
{
 int tolua_index;
  CPlayer* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (CPlayer*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0);
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=MaxCosts)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->Revenue[tolua_index]);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Units of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_get_stratagus_CPlayer_Units
static int tolua_get_stratagus_CPlayer_Units(lua_State* tolua_S)
{
 int tolua_index;
  CPlayer* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (CPlayer*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0);
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=UnitMax)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushusertype(tolua_S,(void*)self->Units[tolua_index],"CUnit");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Units of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_set_stratagus_CPlayer_Units
static int tolua_set_stratagus_CPlayer_Units(lua_State* tolua_S)
{
 int tolua_index;
  CPlayer* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (CPlayer*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0);
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=UnitMax)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
  self->Units[tolua_index] = ((CUnit*)  tolua_tousertype(tolua_S,3,0));
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: TotalNumUnits of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_get_CPlayer_TotalNumUnits
static int tolua_get_CPlayer_TotalNumUnits(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'TotalNumUnits'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->TotalNumUnits);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: TotalNumUnits of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_set_CPlayer_TotalNumUnits
static int tolua_set_CPlayer_TotalNumUnits(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'TotalNumUnits'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->TotalNumUnits = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: NumBuildings of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_get_CPlayer_NumBuildings
static int tolua_get_CPlayer_NumBuildings(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'NumBuildings'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->NumBuildings);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: NumBuildings of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_set_CPlayer_NumBuildings
static int tolua_set_CPlayer_NumBuildings(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'NumBuildings'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->NumBuildings = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Supply of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_get_CPlayer_Supply
static int tolua_get_CPlayer_Supply(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Supply'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->Supply);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Supply of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_set_CPlayer_Supply
static int tolua_set_CPlayer_Supply(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Supply'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Supply = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Demand of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_get_CPlayer_Demand
static int tolua_get_CPlayer_Demand(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Demand'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->Demand);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Demand of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_set_CPlayer_Demand
static int tolua_set_CPlayer_Demand(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Demand'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Demand = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: UnitLimit of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_get_CPlayer_UnitLimit
static int tolua_get_CPlayer_UnitLimit(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'UnitLimit'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->UnitLimit);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: UnitLimit of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_set_CPlayer_UnitLimit
static int tolua_set_CPlayer_UnitLimit(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'UnitLimit'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->UnitLimit = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: BuildingLimit of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_get_CPlayer_BuildingLimit
static int tolua_get_CPlayer_BuildingLimit(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'BuildingLimit'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->BuildingLimit);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: BuildingLimit of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_set_CPlayer_BuildingLimit
static int tolua_set_CPlayer_BuildingLimit(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'BuildingLimit'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->BuildingLimit = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: TotalUnitLimit of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_get_CPlayer_TotalUnitLimit
static int tolua_get_CPlayer_TotalUnitLimit(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'TotalUnitLimit'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->TotalUnitLimit);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: TotalUnitLimit of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_set_CPlayer_TotalUnitLimit
static int tolua_set_CPlayer_TotalUnitLimit(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'TotalUnitLimit'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->TotalUnitLimit = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Score of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_get_CPlayer_Score
static int tolua_get_CPlayer_Score(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Score'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->Score);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Score of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_set_CPlayer_Score
static int tolua_set_CPlayer_Score(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Score'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Score = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: TotalUnits of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_get_CPlayer_TotalUnits
static int tolua_get_CPlayer_TotalUnits(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'TotalUnits'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->TotalUnits);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: TotalUnits of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_set_CPlayer_TotalUnits
static int tolua_set_CPlayer_TotalUnits(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'TotalUnits'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->TotalUnits = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: TotalBuildings of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_get_CPlayer_TotalBuildings
static int tolua_get_CPlayer_TotalBuildings(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'TotalBuildings'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->TotalBuildings);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: TotalBuildings of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_set_CPlayer_TotalBuildings
static int tolua_set_CPlayer_TotalBuildings(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'TotalBuildings'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->TotalBuildings = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: TotalResources of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_get_stratagus_CPlayer_TotalResources
static int tolua_get_stratagus_CPlayer_TotalResources(lua_State* tolua_S)
{
 int tolua_index;
  CPlayer* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (CPlayer*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0);
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=MaxCosts)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->TotalResources[tolua_index]);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: TotalResources of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_set_stratagus_CPlayer_TotalResources
static int tolua_set_stratagus_CPlayer_TotalResources(lua_State* tolua_S)
{
 int tolua_index;
  CPlayer* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (CPlayer*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0);
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=MaxCosts)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
  self->TotalResources[tolua_index] = ((int)  tolua_tonumber(tolua_S,3,0));
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: TotalRazings of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_get_CPlayer_TotalRazings
static int tolua_get_CPlayer_TotalRazings(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'TotalRazings'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->TotalRazings);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: TotalRazings of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_set_CPlayer_TotalRazings
static int tolua_set_CPlayer_TotalRazings(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'TotalRazings'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->TotalRazings = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: TotalKills of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_get_CPlayer_TotalKills
static int tolua_get_CPlayer_TotalKills(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'TotalKills'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->TotalKills);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: TotalKills of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_set_CPlayer_TotalKills
static int tolua_set_CPlayer_TotalKills(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'TotalKills'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->TotalKills = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Players */
#ifndef TOLUA_DISABLE_tolua_get_stratagus_Players
static int tolua_get_stratagus_Players(lua_State* tolua_S)
{
 int tolua_index;
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0);
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=PlayerMax)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushusertype(tolua_S,(void*)&Players[tolua_index],"CPlayer");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Players */
#ifndef TOLUA_DISABLE_tolua_set_stratagus_Players
static int tolua_set_stratagus_Players(lua_State* tolua_S)
{
 int tolua_index;
#ifndef TOLUA_RELEASE
 {
 tolua_Error tolua_err;
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0);
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=PlayerMax)
 tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
  Players[tolua_index] = *((CPlayer*)  tolua_tousertype(tolua_S,3,0));
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: ThisPlayer */
#ifndef TOLUA_DISABLE_tolua_get_ThisPlayer_ptr
static int tolua_get_ThisPlayer_ptr(lua_State* tolua_S)
{
 tolua_pushusertype(tolua_S,(void*)ThisPlayer,"CPlayer");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: ThisPlayer */
#ifndef TOLUA_DISABLE_tolua_set_ThisPlayer_ptr
static int tolua_set_ThisPlayer_ptr(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!tolua_isusertype(tolua_S,2,"CPlayer",0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  ThisPlayer = ((CPlayer*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Ident of class  CUnitType */
#ifndef TOLUA_DISABLE_tolua_get_CUnitType_Ident
static int tolua_get_CUnitType_Ident(lua_State* tolua_S)
{
  CUnitType* self = (CUnitType*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Ident'",NULL);
#endif
 tolua_pushstring(tolua_S,(const char*)self->Ident);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Ident of class  CUnitType */
#ifndef TOLUA_DISABLE_tolua_set_CUnitType_Ident
static int tolua_set_CUnitType_Ident(lua_State* tolua_S)
{
  CUnitType* self = (CUnitType*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Ident'",NULL);
 if (!tolua_isstring(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Ident = ((char*)  tolua_tostring(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Name of class  CUnitType */
#ifndef TOLUA_DISABLE_tolua_get_CUnitType_Name
static int tolua_get_CUnitType_Name(lua_State* tolua_S)
{
  CUnitType* self = (CUnitType*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Name'",NULL);
#endif
 tolua_pushstring(tolua_S,(const char*)self->Name);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Name of class  CUnitType */
#ifndef TOLUA_DISABLE_tolua_set_CUnitType_Name
static int tolua_set_CUnitType_Name(lua_State* tolua_S)
{
  CUnitType* self = (CUnitType*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Name'",NULL);
 if (!tolua_isstring(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Name = ((char*)  tolua_tostring(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: MinAttackRange of class  CUnitType */
#ifndef TOLUA_DISABLE_tolua_get_CUnitType_MinAttackRange
static int tolua_get_CUnitType_MinAttackRange(lua_State* tolua_S)
{
  CUnitType* self = (CUnitType*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MinAttackRange'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->MinAttackRange);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: MinAttackRange of class  CUnitType */
#ifndef TOLUA_DISABLE_tolua_set_CUnitType_MinAttackRange
static int tolua_set_CUnitType_MinAttackRange(lua_State* tolua_S)
{
  CUnitType* self = (CUnitType*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MinAttackRange'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->MinAttackRange = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: ClicksToExplode of class  CUnitType */
#ifndef TOLUA_DISABLE_tolua_get_CUnitType_ClicksToExplode
static int tolua_get_CUnitType_ClicksToExplode(lua_State* tolua_S)
{
  CUnitType* self = (CUnitType*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ClicksToExplode'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->ClicksToExplode);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: ClicksToExplode of class  CUnitType */
#ifndef TOLUA_DISABLE_tolua_set_CUnitType_ClicksToExplode
static int tolua_set_CUnitType_ClicksToExplode(lua_State* tolua_S)
{
  CUnitType* self = (CUnitType*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ClicksToExplode'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->ClicksToExplode = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Supply of class  CUnitType */
#ifndef TOLUA_DISABLE_tolua_get_CUnitType_Supply
static int tolua_get_CUnitType_Supply(lua_State* tolua_S)
{
  CUnitType* self = (CUnitType*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Supply'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->Supply);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Supply of class  CUnitType */
#ifndef TOLUA_DISABLE_tolua_set_CUnitType_Supply
static int tolua_set_CUnitType_Supply(lua_State* tolua_S)
{
  CUnitType* self = (CUnitType*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Supply'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Supply = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Demand of class  CUnitType */
#ifndef TOLUA_DISABLE_tolua_get_CUnitType_Demand
static int tolua_get_CUnitType_Demand(lua_State* tolua_S)
{
  CUnitType* self = (CUnitType*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Demand'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->Demand);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Demand of class  CUnitType */
#ifndef TOLUA_DISABLE_tolua_set_CUnitType_Demand
static int tolua_set_CUnitType_Demand(lua_State* tolua_S)
{
  CUnitType* self = (CUnitType*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Demand'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Demand = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: X of class  CUnit */
#ifndef TOLUA_DISABLE_tolua_get_CUnit_X
static int tolua_get_CUnit_X(lua_State* tolua_S)
{
  CUnit* self = (CUnit*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'X'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->X);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: X of class  CUnit */
#ifndef TOLUA_DISABLE_tolua_set_CUnit_X
static int tolua_set_CUnit_X(lua_State* tolua_S)
{
  CUnit* self = (CUnit*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'X'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->X = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Y of class  CUnit */
#ifndef TOLUA_DISABLE_tolua_get_CUnit_Y
static int tolua_get_CUnit_Y(lua_State* tolua_S)
{
  CUnit* self = (CUnit*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Y'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->Y);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Y of class  CUnit */
#ifndef TOLUA_DISABLE_tolua_set_CUnit_Y
static int tolua_set_CUnit_Y(lua_State* tolua_S)
{
  CUnit* self = (CUnit*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Y'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Y = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Type of class  CUnit */
#ifndef TOLUA_DISABLE_tolua_get_CUnit_Type_ptr
static int tolua_get_CUnit_Type_ptr(lua_State* tolua_S)
{
  CUnit* self = (CUnit*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Type'",NULL);
#endif
 tolua_pushusertype(tolua_S,(void*)self->Type,"CUnitType");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Type of class  CUnit */
#ifndef TOLUA_DISABLE_tolua_set_CUnit_Type_ptr
static int tolua_set_CUnit_Type_ptr(lua_State* tolua_S)
{
  CUnit* self = (CUnit*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Type'",NULL);
 if (!tolua_isusertype(tolua_S,2,"CUnitType",0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Type = ((CUnitType*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Player of class  CUnit */
#ifndef TOLUA_DISABLE_tolua_get_CUnit_Player_ptr
static int tolua_get_CUnit_Player_ptr(lua_State* tolua_S)
{
  CUnit* self = (CUnit*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Player'",NULL);
#endif
 tolua_pushusertype(tolua_S,(void*)self->Player,"CPlayer");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Player of class  CUnit */
#ifndef TOLUA_DISABLE_tolua_set_CUnit_Player_ptr
static int tolua_set_CUnit_Player_ptr(lua_State* tolua_S)
{
  CUnit* self = (CUnit*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Player'",NULL);
 if (!tolua_isusertype(tolua_S,2,"CPlayer",0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Player = ((CPlayer*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: ShowSightRange of class  CPreference */
#ifndef TOLUA_DISABLE_tolua_get_CPreference_ShowSightRange
static int tolua_get_CPreference_ShowSightRange(lua_State* tolua_S)
{
  CPreference* self = (CPreference*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ShowSightRange'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->ShowSightRange);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: ShowSightRange of class  CPreference */
#ifndef TOLUA_DISABLE_tolua_set_CPreference_ShowSightRange
static int tolua_set_CPreference_ShowSightRange(lua_State* tolua_S)
{
  CPreference* self = (CPreference*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ShowSightRange'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->ShowSightRange = ((bool)  tolua_toboolean(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: ShowReactionRange of class  CPreference */
#ifndef TOLUA_DISABLE_tolua_get_CPreference_ShowReactionRange
static int tolua_get_CPreference_ShowReactionRange(lua_State* tolua_S)
{
  CPreference* self = (CPreference*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ShowReactionRange'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->ShowReactionRange);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: ShowReactionRange of class  CPreference */
#ifndef TOLUA_DISABLE_tolua_set_CPreference_ShowReactionRange
static int tolua_set_CPreference_ShowReactionRange(lua_State* tolua_S)
{
  CPreference* self = (CPreference*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ShowReactionRange'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->ShowReactionRange = ((bool)  tolua_toboolean(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: ShowAttackRange of class  CPreference */
#ifndef TOLUA_DISABLE_tolua_get_CPreference_ShowAttackRange
static int tolua_get_CPreference_ShowAttackRange(lua_State* tolua_S)
{
  CPreference* self = (CPreference*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ShowAttackRange'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->ShowAttackRange);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: ShowAttackRange of class  CPreference */
#ifndef TOLUA_DISABLE_tolua_set_CPreference_ShowAttackRange
static int tolua_set_CPreference_ShowAttackRange(lua_State* tolua_S)
{
  CPreference* self = (CPreference*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ShowAttackRange'",NULL);
 if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->ShowAttackRange = ((bool)  tolua_toboolean(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: ShowOrders of class  CPreference */
#ifndef TOLUA_DISABLE_tolua_get_CPreference_unsigned_ShowOrders
static int tolua_get_CPreference_unsigned_ShowOrders(lua_State* tolua_S)
{
  CPreference* self = (CPreference*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ShowOrders'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->ShowOrders);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: ShowOrders of class  CPreference */
#ifndef TOLUA_DISABLE_tolua_set_CPreference_unsigned_ShowOrders
static int tolua_set_CPreference_unsigned_ShowOrders(lua_State* tolua_S)
{
  CPreference* self = (CPreference*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ShowOrders'",NULL);
 if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->ShowOrders = ((unsigned int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Preference */
#ifndef TOLUA_DISABLE_tolua_get_Preference
static int tolua_get_Preference(lua_State* tolua_S)
{
 tolua_pushusertype(tolua_S,(void*)&Preference,"CPreference");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Preference */
#ifndef TOLUA_DISABLE_tolua_set_Preference
static int tolua_set_Preference(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!tolua_isusertype(tolua_S,2,"CPreference",0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  Preference = *((CPreference*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* function: GetEffectsVolume */
#ifndef TOLUA_DISABLE_tolua_stratagus_GetEffectsVolume00
static int tolua_stratagus_GetEffectsVolume00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  int tolua_ret = (int)  GetEffectsVolume();
 tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetEffectsVolume'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: SetEffectsVolume */
#ifndef TOLUA_DISABLE_tolua_stratagus_SetEffectsVolume00
static int tolua_stratagus_SetEffectsVolume00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int volume = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  SetEffectsVolume(volume);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetEffectsVolume'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: GetMusicVolume */
#ifndef TOLUA_DISABLE_tolua_stratagus_GetMusicVolume00
static int tolua_stratagus_GetMusicVolume00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  int tolua_ret = (int)  GetMusicVolume();
 tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetMusicVolume'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: SetMusicVolume */
#ifndef TOLUA_DISABLE_tolua_stratagus_SetMusicVolume00
static int tolua_stratagus_SetMusicVolume00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  int volume = ((int)  tolua_tonumber(tolua_S,1,0));
 {
  SetMusicVolume(volume);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetMusicVolume'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: SetEffectsEnabled */
#ifndef TOLUA_DISABLE_tolua_stratagus_SetEffectsEnabled00
static int tolua_stratagus_SetEffectsEnabled00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isboolean(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  bool enabled = ((bool)  tolua_toboolean(tolua_S,1,0));
 {
  SetEffectsEnabled(enabled);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetEffectsEnabled'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: IsEffectsEnabled */
#ifndef TOLUA_DISABLE_tolua_stratagus_IsEffectsEnabled00
static int tolua_stratagus_IsEffectsEnabled00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  bool tolua_ret = (bool)  IsEffectsEnabled();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'IsEffectsEnabled'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: SetMusicEnabled */
#ifndef TOLUA_DISABLE_tolua_stratagus_SetMusicEnabled00
static int tolua_stratagus_SetMusicEnabled00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isboolean(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  bool enabled = ((bool)  tolua_toboolean(tolua_S,1,0));
 {
  SetMusicEnabled(enabled);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetMusicEnabled'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: IsMusicEnabled */
#ifndef TOLUA_DISABLE_tolua_stratagus_IsMusicEnabled00
static int tolua_stratagus_IsMusicEnabled00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  bool tolua_ret = (bool)  IsMusicEnabled();
 tolua_pushboolean(tolua_S,(bool)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'IsMusicEnabled'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: PlayMusic */
#ifndef TOLUA_DISABLE_tolua_stratagus_PlayMusic00
static int tolua_stratagus_PlayMusic00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isstring(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const char* name = ((const char*)  tolua_tostring(tolua_S,1,0));
 {
  int tolua_ret = (int)  PlayMusic(name);
 tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'PlayMusic'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: StopMusic */
#ifndef TOLUA_DISABLE_tolua_stratagus_StopMusic00
static int tolua_stratagus_StopMusic00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
 {
  StopMusic();
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'StopMusic'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: Translate */
#ifndef TOLUA_DISABLE_tolua_stratagus_Translate00
static int tolua_stratagus_Translate00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isstring(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const char* str = ((const char*)  tolua_tostring(tolua_S,1,0));
 {
  const char* tolua_ret = (const char*)  Translate(str);
 tolua_pushstring(tolua_S,(const char*)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Translate'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: AddTranslation */
#ifndef TOLUA_DISABLE_tolua_stratagus_AddTranslation00
static int tolua_stratagus_AddTranslation00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isstring(tolua_S,1,0,&tolua_err) ||
 !tolua_isstring(tolua_S,2,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const char* str1 = ((const char*)  tolua_tostring(tolua_S,1,0));
  const char* str2 = ((const char*)  tolua_tostring(tolua_S,2,0));
 {
  AddTranslation(str1,str2);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'AddTranslation'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: LoadPO */
#ifndef TOLUA_DISABLE_tolua_stratagus_LoadPO00
static int tolua_stratagus_LoadPO00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isstring(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const char* file = ((const char*)  tolua_tostring(tolua_S,1,0));
 {
  LoadPO(file);
 }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'LoadPO'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: Translate */
#ifndef TOLUA_DISABLE_tolua_stratagus__00
static int tolua_stratagus__00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
 !tolua_isstring(tolua_S,1,0,&tolua_err) ||
 !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
 goto tolua_lerror;
 else
#endif
 {
  const char* str = ((const char*)  tolua_tostring(tolua_S,1,0));
 {
  const char* tolua_ret = (const char*)  Translate(str);
 tolua_pushstring(tolua_S,(const char*)tolua_ret);
 }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function '_'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* Open function */
TOLUA_API int tolua_stratagus_open (lua_State* tolua_S)
{
 tolua_open(tolua_S);
 tolua_reg_types(tolua_S);
 tolua_module(tolua_S,NULL,1);
 tolua_beginmodule(tolua_S,NULL);

 { /* begin embedded lua code */
  int top;
  top = lua_gettop(tolua_S);
  static unsigned char B[] = {
  10,109,116, 32, 61, 32,123, 32, 95, 95,105,110,100,101,120,
   32, 61, 32,102,117,110, 99,116,105,111,110, 40,116, 44, 32,
  107,101,121, 41, 32,114,101,116,117,114,110, 32, 67, 73, 99,
  111,110, 58, 71,101,116, 40,107,101,121, 41, 32,101,110,100,
   32,125, 10, 73, 99,111,110,115, 32, 61, 32,123,125, 10,115,
  101,116,109,101,116, 97,116, 97, 98,108,101, 40, 73, 99,111,
  110,115, 44, 32,109,116, 41, 10,109,116, 32, 61, 32,123, 32,
   95, 95,105,110,100,101,120, 32, 61, 32,102,117,110, 99,116,
  105,111,110, 40,116, 44, 32,107,101,121, 41, 32,114,101,116,
  117,114,110, 32, 67, 85,112,103,114, 97,100,101, 58, 71,101,
  116, 40,107,101,121, 41, 32,101,110,100, 32,125, 10, 85,112,
  103,114, 97,100,101,115, 32, 61, 32,123,125, 10,115,101,116,
  109,101,116, 97,116, 97, 98,108,101, 40, 85,112,103,114, 97,
  100,101,115, 44, 32,109,116, 41, 10,109,116, 32, 61, 32,123,
   32, 95, 95,105,110,100,101,120, 32, 61, 32,102,117,110, 99,
  116,105,111,110, 40,116, 44, 32,107,101,121, 41, 32,114,101,
  116,117,114,110, 32, 67, 70,111,110,116, 58, 71,101,116, 40,
  107,101,121, 41, 32,101,110,100, 32,125, 10, 70,111,110,116,
  115, 32, 61, 32,123,125, 10,115,101,116,109,101,116, 97,116,
   97, 98,108,101, 40, 70,111,110,116,115, 44, 32,109,116, 41,
   10,109,116, 32, 61, 32,123, 32, 95, 95,105,110,100,101,120,
   32, 61, 32,102,117,110, 99,116,105,111,110, 40,116, 44, 32,
  107,101,121, 41, 32,114,101,116,117,114,110, 32, 67, 70,111,
  110,116, 67,111,108,111,114, 58, 71,101,116, 40,107,101,121,
   41, 32,101,110,100, 32,125, 10, 70,111,110,116, 67,111,108,
  111,114,115, 32, 61, 32,123,125, 10,115,101,116,109,101,116,
   97,116, 97, 98,108,101, 40, 70,111,110,116, 67,111,108,111,
  114,115, 44, 32,109,116, 41,32
  };
  lua_dobuffer(tolua_S,(char*)B,sizeof(B),"tolua: embedded Lua code 1");
  lua_settop(tolua_S, top);
 } /* end of embedded lua code */

 tolua_constant(tolua_S,"MaxCosts",MaxCosts);
 tolua_constant(tolua_S,"PlayerMax",PlayerMax);
 tolua_constant(tolua_S,"UnitMax",UnitMax);
 tolua_cclass(tolua_S,"CMinimap","CMinimap","",NULL);
 tolua_beginmodule(tolua_S,"CMinimap");
  tolua_variable(tolua_S,"X",tolua_get_CMinimap_X,tolua_set_CMinimap_X);
  tolua_variable(tolua_S,"Y",tolua_get_CMinimap_Y,tolua_set_CMinimap_Y);
  tolua_variable(tolua_S,"W",tolua_get_CMinimap_W,tolua_set_CMinimap_W);
  tolua_variable(tolua_S,"H",tolua_get_CMinimap_H,tolua_set_CMinimap_H);
  tolua_variable(tolua_S,"WithTerrain",tolua_get_CMinimap_WithTerrain,tolua_set_CMinimap_WithTerrain);
  tolua_variable(tolua_S,"ShowSelected",tolua_get_CMinimap_ShowSelected,tolua_set_CMinimap_ShowSelected);
  tolua_variable(tolua_S,"Transparent",tolua_get_CMinimap_Transparent,tolua_set_CMinimap_Transparent);
 tolua_endmodule(tolua_S);
 tolua_cclass(tolua_S,"CButtonPanel","CButtonPanel","",NULL);
 tolua_beginmodule(tolua_S,"CButtonPanel");
  tolua_variable(tolua_S,"X",tolua_get_CButtonPanel_X,tolua_set_CButtonPanel_X);
  tolua_variable(tolua_S,"Y",tolua_get_CButtonPanel_Y,tolua_set_CButtonPanel_Y);
  tolua_variable(tolua_S,"ShowCommandKey",tolua_get_CButtonPanel_ShowCommandKey,tolua_set_CButtonPanel_ShowCommandKey);
 tolua_endmodule(tolua_S);
 tolua_cclass(tolua_S,"CInfoPanel","CInfoPanel","",NULL);
 tolua_beginmodule(tolua_S,"CInfoPanel");
  tolua_variable(tolua_S,"X",tolua_get_CInfoPanel_X,tolua_set_CInfoPanel_X);
  tolua_variable(tolua_S,"Y",tolua_get_CInfoPanel_Y,tolua_set_CInfoPanel_Y);
 tolua_endmodule(tolua_S);
 tolua_cclass(tolua_S,"CUserInterface","CUserInterface","",NULL);
 tolua_beginmodule(tolua_S,"CUserInterface");
  tolua_variable(tolua_S,"InfoPanel",tolua_get_CUserInterface_InfoPanel,tolua_set_CUserInterface_InfoPanel);
  tolua_variable(tolua_S,"ButtonPanel",tolua_get_CUserInterface_ButtonPanel,tolua_set_CUserInterface_ButtonPanel);
  tolua_variable(tolua_S,"Minimap",tolua_get_CUserInterface_Minimap,tolua_set_CUserInterface_Minimap);
 tolua_endmodule(tolua_S);
 tolua_variable(tolua_S,"UI",tolua_get_UI,tolua_set_UI);
 tolua_cclass(tolua_S,"CIcon","CIcon","",NULL);
 tolua_beginmodule(tolua_S,"CIcon");
  tolua_function(tolua_S,"New",tolua_stratagus_CIcon_New00);
  tolua_function(tolua_S,"Get",tolua_stratagus_CIcon_Get00);
  tolua_variable(tolua_S,"Ident",tolua_get_CIcon_Ident,NULL);
  tolua_variable(tolua_S,"G",tolua_get_CIcon_G_ptr,tolua_set_CIcon_G_ptr);
  tolua_variable(tolua_S,"Frame",tolua_get_CIcon_Frame,tolua_set_CIcon_Frame);
 tolua_endmodule(tolua_S);
 tolua_function(tolua_S,"GetMouseScroll",tolua_stratagus_GetMouseScroll00);
 tolua_function(tolua_S,"SetMouseScroll",tolua_stratagus_SetMouseScroll00);
 tolua_function(tolua_S,"GetMouseScrollSpeed",tolua_stratagus_GetMouseScrollSpeed00);
 tolua_function(tolua_S,"SetMouseScrollSpeed",tolua_stratagus_SetMouseScrollSpeed00);
 tolua_function(tolua_S,"GetKeyScroll",tolua_stratagus_GetKeyScroll00);
 tolua_function(tolua_S,"SetKeyScroll",tolua_stratagus_SetKeyScroll00);
 tolua_function(tolua_S,"GetKeyScrollSpeed",tolua_stratagus_GetKeyScrollSpeed00);
 tolua_function(tolua_S,"SetKeyScrollSpeed",tolua_stratagus_SetKeyScrollSpeed00);
 tolua_function(tolua_S,"GetGrabMouse",tolua_stratagus_GetGrabMouse00);
 tolua_function(tolua_S,"SetGrabMouse",tolua_stratagus_SetGrabMouse00);
 tolua_function(tolua_S,"GetLeaveStops",tolua_stratagus_GetLeaveStops00);
 tolua_function(tolua_S,"SetLeaveStops",tolua_stratagus_SetLeaveStops00);
 tolua_function(tolua_S,"GetDoubleClickDelay",tolua_stratagus_GetDoubleClickDelay00);
 tolua_function(tolua_S,"SetDoubleClickDelay",tolua_stratagus_SetDoubleClickDelay00);
 tolua_function(tolua_S,"GetHoldClickDelay",tolua_stratagus_GetHoldClickDelay00);
 tolua_function(tolua_S,"SetHoldClickDelay",tolua_stratagus_SetHoldClickDelay00);
#ifdef __cplusplus
 tolua_cclass(tolua_S,"Color","Color","",tolua_collect_Color);
#else
 tolua_cclass(tolua_S,"Color","Color","",NULL);
#endif
 tolua_beginmodule(tolua_S,"Color");
  tolua_function(tolua_S,"new",tolua_stratagus_Color_new00);
  tolua_function(tolua_S,"new_local",tolua_stratagus_Color_new00_local);
  tolua_function(tolua_S,".call",tolua_stratagus_Color_new00_local);
  tolua_variable(tolua_S,"r",tolua_get_Color_r,tolua_set_Color_r);
  tolua_variable(tolua_S,"g",tolua_get_Color_g,tolua_set_Color_g);
  tolua_variable(tolua_S,"b",tolua_get_Color_b,tolua_set_Color_b);
  tolua_variable(tolua_S,"a",tolua_get_Color_a,tolua_set_Color_a);
 tolua_endmodule(tolua_S);
#ifdef __cplusplus
 tolua_cclass(tolua_S,"LuaActionListener","LuaActionListener","",tolua_collect_LuaActionListener);
#else
 tolua_cclass(tolua_S,"LuaActionListener","LuaActionListener","",NULL);
#endif
 tolua_beginmodule(tolua_S,"LuaActionListener");
  tolua_function(tolua_S,"new",tolua_stratagus_LuaActionListener_new00);
  tolua_function(tolua_S,"new_local",tolua_stratagus_LuaActionListener_new00_local);
  tolua_function(tolua_S,".call",tolua_stratagus_LuaActionListener_new00_local);
 tolua_endmodule(tolua_S);
 tolua_cclass(tolua_S,"Widget","Widget","",NULL);
 tolua_beginmodule(tolua_S,"Widget");
  tolua_function(tolua_S,"setWidth",tolua_stratagus_Widget_setWidth00);
  tolua_function(tolua_S,"getWidth",tolua_stratagus_Widget_getWidth00);
  tolua_function(tolua_S,"setHeight",tolua_stratagus_Widget_setHeight00);
  tolua_function(tolua_S,"getHeight",tolua_stratagus_Widget_getHeight00);
  tolua_function(tolua_S,"setSize",tolua_stratagus_Widget_setSize00);
  tolua_function(tolua_S,"setX",tolua_stratagus_Widget_setX00);
  tolua_function(tolua_S,"getX",tolua_stratagus_Widget_getX00);
  tolua_function(tolua_S,"setY",tolua_stratagus_Widget_setY00);
  tolua_function(tolua_S,"getY",tolua_stratagus_Widget_getY00);
  tolua_function(tolua_S,"setPosition",tolua_stratagus_Widget_setPosition00);
  tolua_function(tolua_S,"setEnabled",tolua_stratagus_Widget_setEnabled00);
  tolua_function(tolua_S,"isEnabled",tolua_stratagus_Widget_isEnabled00);
  tolua_function(tolua_S,"setBaseColor",tolua_stratagus_Widget_setBaseColor00);
  tolua_function(tolua_S,"setForegroundColor",tolua_stratagus_Widget_setForegroundColor00);
  tolua_function(tolua_S,"setBackgroundColor",tolua_stratagus_Widget_setBackgroundColor00);
  tolua_function(tolua_S,"setGlobalFont",tolua_stratagus_Widget_setGlobalFont00);
  tolua_function(tolua_S,"setForegroundColor",tolua_stratagus_Widget_setForegroundColor01);
  tolua_function(tolua_S,"setBackgroundColor",tolua_stratagus_Widget_setBackgroundColor01);
  tolua_function(tolua_S,"setBaseColor",tolua_stratagus_Widget_setBaseColor01);
  tolua_function(tolua_S,"setSize",tolua_stratagus_Widget_setSize01);
  tolua_function(tolua_S,"setBorderSize",tolua_stratagus_Widget_setBorderSize00);
  tolua_function(tolua_S,"setFont",tolua_stratagus_Widget_setFont00);
  tolua_function(tolua_S,"addActionListener",tolua_stratagus_Widget_addActionListener00);
 tolua_endmodule(tolua_S);

 { /* begin embedded lua code */
  int top;
  top = lua_gettop(tolua_S);
  static unsigned char B[] = {
  10, 87,105,100,103,101,116, 46,115,101,116, 65, 99,116,105,
  111,110, 67, 97,108,108, 98, 97, 99,107, 32, 61, 32,102,117,
  110, 99,116,105,111,110, 40,119, 44, 32,102, 41, 10,119, 46,
   97, 99,116,105,111,110, 99, 98, 32, 61, 32, 76,117, 97, 65,
   99,116,105,111,110, 76,105,115,116,101,110,101,114, 40,102,
   41, 10,119, 58, 97,100,100, 65, 99,116,105,111,110, 76,105,
  115,116,101,110,101,114, 40,119, 46, 97, 99,116,105,111,110,
   99, 98, 41, 10,101,110,100, 32,32
  };
  lua_dobuffer(tolua_S,(char*)B,sizeof(B),"tolua: embedded Lua code 2");
  lua_settop(tolua_S, top);
 } /* end of embedded lua code */

#ifdef __cplusplus
 tolua_cclass(tolua_S,"ImageWidget","ImageWidget","Widget",tolua_collect_ImageWidget);
#else
 tolua_cclass(tolua_S,"ImageWidget","ImageWidget","Widget",NULL);
#endif
 tolua_beginmodule(tolua_S,"ImageWidget");
  tolua_function(tolua_S,"new",tolua_stratagus_ImageWidget_new00);
  tolua_function(tolua_S,"new_local",tolua_stratagus_ImageWidget_new00_local);
  tolua_function(tolua_S,".call",tolua_stratagus_ImageWidget_new00_local);
 tolua_endmodule(tolua_S);
#ifdef __cplusplus
 tolua_cclass(tolua_S,"ButtonWidget","ButtonWidget","Widget",tolua_collect_ButtonWidget);
#else
 tolua_cclass(tolua_S,"ButtonWidget","ButtonWidget","Widget",NULL);
#endif
 tolua_beginmodule(tolua_S,"ButtonWidget");
  tolua_function(tolua_S,"new",tolua_stratagus_ButtonWidget_new00);
  tolua_function(tolua_S,"new_local",tolua_stratagus_ButtonWidget_new00_local);
  tolua_function(tolua_S,".call",tolua_stratagus_ButtonWidget_new00_local);
  tolua_function(tolua_S,"setCaption",tolua_stratagus_ButtonWidget_setCaption00);
  tolua_function(tolua_S,"getCaption",tolua_stratagus_ButtonWidget_getCaption00);
  tolua_function(tolua_S,"adjustSize",tolua_stratagus_ButtonWidget_adjustSize00);
 tolua_endmodule(tolua_S);
#ifdef __cplusplus
 tolua_cclass(tolua_S,"ImageButton","ImageButton","Widget",tolua_collect_ImageButton);
#else
 tolua_cclass(tolua_S,"ImageButton","ImageButton","Widget",NULL);
#endif
 tolua_beginmodule(tolua_S,"ImageButton");
  tolua_function(tolua_S,"new",tolua_stratagus_ImageButton_new00);
  tolua_function(tolua_S,"new_local",tolua_stratagus_ImageButton_new00_local);
  tolua_function(tolua_S,".call",tolua_stratagus_ImageButton_new00_local);
  tolua_function(tolua_S,"new",tolua_stratagus_ImageButton_new01);
  tolua_function(tolua_S,"new_local",tolua_stratagus_ImageButton_new01_local);
  tolua_function(tolua_S,".call",tolua_stratagus_ImageButton_new01_local);
  tolua_function(tolua_S,"setNormalImage",tolua_stratagus_ImageButton_setNormalImage00);
  tolua_function(tolua_S,"setPressedImage",tolua_stratagus_ImageButton_setPressedImage00);
  tolua_function(tolua_S,"setDisabledImage",tolua_stratagus_ImageButton_setDisabledImage00);
  tolua_function(tolua_S,"setHotKey",tolua_stratagus_ImageButton_setHotKey00);
  tolua_function(tolua_S,"setHotKey",tolua_stratagus_ImageButton_setHotKey01);
 tolua_endmodule(tolua_S);
#ifdef __cplusplus
 tolua_cclass(tolua_S,"RadioButton","RadioButton","Widget",tolua_collect_RadioButton);
#else
 tolua_cclass(tolua_S,"RadioButton","RadioButton","Widget",NULL);
#endif
 tolua_beginmodule(tolua_S,"RadioButton");
  tolua_function(tolua_S,"new",tolua_stratagus_RadioButton_new00);
  tolua_function(tolua_S,"new_local",tolua_stratagus_RadioButton_new00_local);
  tolua_function(tolua_S,".call",tolua_stratagus_RadioButton_new00_local);
  tolua_function(tolua_S,"new",tolua_stratagus_RadioButton_new01);
  tolua_function(tolua_S,"new_local",tolua_stratagus_RadioButton_new01_local);
  tolua_function(tolua_S,".call",tolua_stratagus_RadioButton_new01_local);
  tolua_function(tolua_S,"isMarked",tolua_stratagus_RadioButton_isMarked00);
  tolua_function(tolua_S,"setMarked",tolua_stratagus_RadioButton_setMarked00);
  tolua_function(tolua_S,"getCaption",tolua_stratagus_RadioButton_getCaption00);
  tolua_function(tolua_S,"setCaption",tolua_stratagus_RadioButton_setCaption00);
  tolua_function(tolua_S,"setGroup",tolua_stratagus_RadioButton_setGroup00);
  tolua_function(tolua_S,"getGroup",tolua_stratagus_RadioButton_getGroup00);
  tolua_function(tolua_S,"adjustSize",tolua_stratagus_RadioButton_adjustSize00);
 tolua_endmodule(tolua_S);
#ifdef __cplusplus
 tolua_cclass(tolua_S,"ImageRadioButton","ImageRadioButton","RadioButton",tolua_collect_ImageRadioButton);
#else
 tolua_cclass(tolua_S,"ImageRadioButton","ImageRadioButton","RadioButton",NULL);
#endif
 tolua_beginmodule(tolua_S,"ImageRadioButton");
  tolua_function(tolua_S,"new",tolua_stratagus_ImageRadioButton_new00);
  tolua_function(tolua_S,"new_local",tolua_stratagus_ImageRadioButton_new00_local);
  tolua_function(tolua_S,".call",tolua_stratagus_ImageRadioButton_new00_local);
  tolua_function(tolua_S,"new",tolua_stratagus_ImageRadioButton_new01);
  tolua_function(tolua_S,"new_local",tolua_stratagus_ImageRadioButton_new01_local);
  tolua_function(tolua_S,".call",tolua_stratagus_ImageRadioButton_new01_local);
  tolua_function(tolua_S,"setUncheckedNormalImage",tolua_stratagus_ImageRadioButton_setUncheckedNormalImage00);
  tolua_function(tolua_S,"setUncheckedPressedImage",tolua_stratagus_ImageRadioButton_setUncheckedPressedImage00);
  tolua_function(tolua_S,"setCheckedNormalImage",tolua_stratagus_ImageRadioButton_setCheckedNormalImage00);
  tolua_function(tolua_S,"setCheckedPressedImage",tolua_stratagus_ImageRadioButton_setCheckedPressedImage00);
 tolua_endmodule(tolua_S);
#ifdef __cplusplus
 tolua_cclass(tolua_S,"CheckBox","CheckBox","Widget",tolua_collect_CheckBox);
#else
 tolua_cclass(tolua_S,"CheckBox","CheckBox","Widget",NULL);
#endif
 tolua_beginmodule(tolua_S,"CheckBox");
  tolua_function(tolua_S,"new",tolua_stratagus_CheckBox_new00);
  tolua_function(tolua_S,"new_local",tolua_stratagus_CheckBox_new00_local);
  tolua_function(tolua_S,".call",tolua_stratagus_CheckBox_new00_local);
  tolua_function(tolua_S,"new",tolua_stratagus_CheckBox_new01);
  tolua_function(tolua_S,"new_local",tolua_stratagus_CheckBox_new01_local);
  tolua_function(tolua_S,".call",tolua_stratagus_CheckBox_new01_local);
  tolua_function(tolua_S,"isMarked",tolua_stratagus_CheckBox_isMarked00);
  tolua_function(tolua_S,"setMarked",tolua_stratagus_CheckBox_setMarked00);
  tolua_function(tolua_S,"getCaption",tolua_stratagus_CheckBox_getCaption00);
  tolua_function(tolua_S,"setCaption",tolua_stratagus_CheckBox_setCaption00);
  tolua_function(tolua_S,"adjustSize",tolua_stratagus_CheckBox_adjustSize00);
 tolua_endmodule(tolua_S);
#ifdef __cplusplus
 tolua_cclass(tolua_S,"ImageCheckBox","ImageCheckBox","CheckBox",tolua_collect_ImageCheckBox);
#else
 tolua_cclass(tolua_S,"ImageCheckBox","ImageCheckBox","CheckBox",NULL);
#endif
 tolua_beginmodule(tolua_S,"ImageCheckBox");
  tolua_function(tolua_S,"new",tolua_stratagus_ImageCheckBox_new00);
  tolua_function(tolua_S,"new_local",tolua_stratagus_ImageCheckBox_new00_local);
  tolua_function(tolua_S,".call",tolua_stratagus_ImageCheckBox_new00_local);
  tolua_function(tolua_S,"new",tolua_stratagus_ImageCheckBox_new01);
  tolua_function(tolua_S,"new_local",tolua_stratagus_ImageCheckBox_new01_local);
  tolua_function(tolua_S,".call",tolua_stratagus_ImageCheckBox_new01_local);
  tolua_function(tolua_S,"setUncheckedNormalImage",tolua_stratagus_ImageCheckBox_setUncheckedNormalImage00);
  tolua_function(tolua_S,"setUncheckedPressedImage",tolua_stratagus_ImageCheckBox_setUncheckedPressedImage00);
  tolua_function(tolua_S,"setCheckedNormalImage",tolua_stratagus_ImageCheckBox_setCheckedNormalImage00);
  tolua_function(tolua_S,"setCheckedPressedImage",tolua_stratagus_ImageCheckBox_setCheckedPressedImage00);
 tolua_endmodule(tolua_S);
#ifdef __cplusplus
 tolua_cclass(tolua_S,"Slider","Slider","Widget",tolua_collect_Slider);
#else
 tolua_cclass(tolua_S,"Slider","Slider","Widget",NULL);
#endif
 tolua_beginmodule(tolua_S,"Slider");
  tolua_function(tolua_S,"new",tolua_stratagus_Slider_new00);
  tolua_function(tolua_S,"new_local",tolua_stratagus_Slider_new00_local);
  tolua_function(tolua_S,".call",tolua_stratagus_Slider_new00_local);
  tolua_function(tolua_S,"new",tolua_stratagus_Slider_new01);
  tolua_function(tolua_S,"new_local",tolua_stratagus_Slider_new01_local);
  tolua_function(tolua_S,".call",tolua_stratagus_Slider_new01_local);
  tolua_function(tolua_S,"setScale",tolua_stratagus_Slider_setScale00);
  tolua_function(tolua_S,"getScaleStart",tolua_stratagus_Slider_getScaleStart00);
  tolua_function(tolua_S,"setScaleStart",tolua_stratagus_Slider_setScaleStart00);
  tolua_function(tolua_S,"getScaleEnd",tolua_stratagus_Slider_getScaleEnd00);
  tolua_function(tolua_S,"setScaleEnd",tolua_stratagus_Slider_setScaleEnd00);
  tolua_function(tolua_S,"getValue",tolua_stratagus_Slider_getValue00);
  tolua_function(tolua_S,"setValue",tolua_stratagus_Slider_setValue00);
  tolua_function(tolua_S,"setMarkerLength",tolua_stratagus_Slider_setMarkerLength00);
  tolua_function(tolua_S,"getMarkerLength",tolua_stratagus_Slider_getMarkerLength00);
  tolua_function(tolua_S,"setOrientation",tolua_stratagus_Slider_setOrientation00);
  tolua_function(tolua_S,"getOrientation",tolua_stratagus_Slider_getOrientation00);
  tolua_function(tolua_S,"setStepLength",tolua_stratagus_Slider_setStepLength00);
  tolua_function(tolua_S,"getStepLength",tolua_stratagus_Slider_getStepLength00);
  tolua_constant(tolua_S,"HORIZONTAL",Slider::HORIZONTAL);
  tolua_constant(tolua_S,"VERTICAL",Slider::VERTICAL);
 tolua_endmodule(tolua_S);
#ifdef __cplusplus
 tolua_cclass(tolua_S,"ImageSlider","ImageSlider","Slider",tolua_collect_ImageSlider);
#else
 tolua_cclass(tolua_S,"ImageSlider","ImageSlider","Slider",NULL);
#endif
 tolua_beginmodule(tolua_S,"ImageSlider");
  tolua_function(tolua_S,"new",tolua_stratagus_ImageSlider_new00);
  tolua_function(tolua_S,"new_local",tolua_stratagus_ImageSlider_new00_local);
  tolua_function(tolua_S,".call",tolua_stratagus_ImageSlider_new00_local);
  tolua_function(tolua_S,"new",tolua_stratagus_ImageSlider_new01);
  tolua_function(tolua_S,"new_local",tolua_stratagus_ImageSlider_new01_local);
  tolua_function(tolua_S,".call",tolua_stratagus_ImageSlider_new01_local);
  tolua_function(tolua_S,"setMarkerImage",tolua_stratagus_ImageSlider_setMarkerImage00);
  tolua_function(tolua_S,"setBackgroundImage",tolua_stratagus_ImageSlider_setBackgroundImage00);
 tolua_endmodule(tolua_S);
#ifdef __cplusplus
 tolua_cclass(tolua_S,"Label","Label","Widget",tolua_collect_Label);
#else
 tolua_cclass(tolua_S,"Label","Label","Widget",NULL);
#endif
 tolua_beginmodule(tolua_S,"Label");
  tolua_function(tolua_S,"new",tolua_stratagus_Label_new00);
  tolua_function(tolua_S,"new_local",tolua_stratagus_Label_new00_local);
  tolua_function(tolua_S,".call",tolua_stratagus_Label_new00_local);
  tolua_function(tolua_S,"getCaption",tolua_stratagus_Label_getCaption00);
  tolua_function(tolua_S,"setCaption",tolua_stratagus_Label_setCaption00);
  tolua_function(tolua_S,"adjustSize",tolua_stratagus_Label_adjustSize00);
 tolua_endmodule(tolua_S);
#ifdef __cplusplus
 tolua_cclass(tolua_S,"TextField","TextField","Widget",tolua_collect_TextField);
#else
 tolua_cclass(tolua_S,"TextField","TextField","Widget",NULL);
#endif
 tolua_beginmodule(tolua_S,"TextField");
  tolua_function(tolua_S,"new",tolua_stratagus_TextField_new00);
  tolua_function(tolua_S,"new_local",tolua_stratagus_TextField_new00_local);
  tolua_function(tolua_S,".call",tolua_stratagus_TextField_new00_local);
 tolua_endmodule(tolua_S);
#ifdef __cplusplus
 tolua_cclass(tolua_S,"ListBoxWidget","ListBoxWidget","Widget",tolua_collect_ListBoxWidget);
#else
 tolua_cclass(tolua_S,"ListBoxWidget","ListBoxWidget","Widget",NULL);
#endif
 tolua_beginmodule(tolua_S,"ListBoxWidget");
  tolua_function(tolua_S,"new",tolua_stratagus_ListBoxWidget_new00);
  tolua_function(tolua_S,"new_local",tolua_stratagus_ListBoxWidget_new00_local);
  tolua_function(tolua_S,".call",tolua_stratagus_ListBoxWidget_new00_local);
  tolua_function(tolua_S,"setList",tolua_stratagus_ListBoxWidget_setList00);
  tolua_function(tolua_S,"setSelected",tolua_stratagus_ListBoxWidget_setSelected00);
  tolua_function(tolua_S,"getSelected",tolua_stratagus_ListBoxWidget_getSelected00);
 tolua_endmodule(tolua_S);
#ifdef __cplusplus
 tolua_cclass(tolua_S,"DropDownWidget","DropDownWidget","Widget",tolua_collect_DropDownWidget);
#else
 tolua_cclass(tolua_S,"DropDownWidget","DropDownWidget","Widget",NULL);
#endif
 tolua_beginmodule(tolua_S,"DropDownWidget");
  tolua_function(tolua_S,"new",tolua_stratagus_DropDownWidget_new00);
  tolua_function(tolua_S,"new_local",tolua_stratagus_DropDownWidget_new00_local);
  tolua_function(tolua_S,".call",tolua_stratagus_DropDownWidget_new00_local);
  tolua_function(tolua_S,"setList",tolua_stratagus_DropDownWidget_setList00);
  tolua_function(tolua_S,"getSelected",tolua_stratagus_DropDownWidget_getSelected00);
  tolua_function(tolua_S,"setSelected",tolua_stratagus_DropDownWidget_setSelected00);
 tolua_endmodule(tolua_S);
#ifdef __cplusplus
 tolua_cclass(tolua_S,"Windows","Windows","Widget",tolua_collect_Windows);
#else
 tolua_cclass(tolua_S,"Windows","Windows","Widget",NULL);
#endif
 tolua_beginmodule(tolua_S,"Windows");
  tolua_function(tolua_S,"new",tolua_stratagus_Windows_new00);
  tolua_function(tolua_S,"new_local",tolua_stratagus_Windows_new00_local);
  tolua_function(tolua_S,".call",tolua_stratagus_Windows_new00_local);
  tolua_function(tolua_S,"add",tolua_stratagus_Windows_add00);
 tolua_endmodule(tolua_S);
#ifdef __cplusplus
 tolua_cclass(tolua_S,"ScrollingWidget","ScrollingWidget","Widget",tolua_collect_ScrollingWidget);
#else
 tolua_cclass(tolua_S,"ScrollingWidget","ScrollingWidget","Widget",NULL);
#endif
 tolua_beginmodule(tolua_S,"ScrollingWidget");
  tolua_function(tolua_S,"new",tolua_stratagus_ScrollingWidget_new00);
  tolua_function(tolua_S,"new_local",tolua_stratagus_ScrollingWidget_new00_local);
  tolua_function(tolua_S,".call",tolua_stratagus_ScrollingWidget_new00_local);
  tolua_function(tolua_S,"add",tolua_stratagus_ScrollingWidget_add00);
  tolua_function(tolua_S,"restart",tolua_stratagus_ScrollingWidget_restart00);
 tolua_endmodule(tolua_S);
#ifdef __cplusplus
 tolua_cclass(tolua_S,"StatBoxWidget","StatBoxWidget","Widget",tolua_collect_StatBoxWidget);
#else
 tolua_cclass(tolua_S,"StatBoxWidget","StatBoxWidget","Widget",NULL);
#endif
 tolua_beginmodule(tolua_S,"StatBoxWidget");
  tolua_function(tolua_S,"new",tolua_stratagus_StatBoxWidget_new00);
  tolua_function(tolua_S,"new_local",tolua_stratagus_StatBoxWidget_new00_local);
  tolua_function(tolua_S,".call",tolua_stratagus_StatBoxWidget_new00_local);
  tolua_variable(tolua_S,"caption",tolua_get_StatBoxWidget_caption,tolua_set_StatBoxWidget_caption);
  tolua_variable(tolua_S,"percent",tolua_get_StatBoxWidget_unsigned_percent,tolua_set_StatBoxWidget_unsigned_percent);
 tolua_endmodule(tolua_S);
 tolua_cclass(tolua_S,"Container","Container","",NULL);
 tolua_beginmodule(tolua_S,"Container");
  tolua_function(tolua_S,"add",tolua_stratagus_Container_add00);
  tolua_function(tolua_S,"clear",tolua_stratagus_Container_clear00);
 tolua_endmodule(tolua_S);
#ifdef __cplusplus
 tolua_cclass(tolua_S,"MenuScreen","MenuScreen","Container",tolua_collect_MenuScreen);
#else
 tolua_cclass(tolua_S,"MenuScreen","MenuScreen","Container",NULL);
#endif
 tolua_beginmodule(tolua_S,"MenuScreen");
  tolua_function(tolua_S,"new",tolua_stratagus_MenuScreen_new00);
  tolua_function(tolua_S,"new_local",tolua_stratagus_MenuScreen_new00_local);
  tolua_function(tolua_S,".call",tolua_stratagus_MenuScreen_new00_local);
  tolua_function(tolua_S,"run",tolua_stratagus_MenuScreen_run00);
  tolua_function(tolua_S,"stop",tolua_stratagus_MenuScreen_stop00);
 tolua_endmodule(tolua_S);
 tolua_function(tolua_S,"StartMap",tolua_stratagus_StartMap00);
 tolua_function(tolua_S,"StartEditor",tolua_stratagus_StartEditor00);
 tolua_function(tolua_S,"StartReplay",tolua_stratagus_StartReplay00);
 tolua_function(tolua_S,"StartSavedGame",tolua_stratagus_StartSavedGame00);
 tolua_cclass(tolua_S,"CVideo","CVideo","",NULL);
 tolua_beginmodule(tolua_S,"CVideo");
  tolua_variable(tolua_S,"Width",tolua_get_CVideo_Width,tolua_set_CVideo_Width);
  tolua_variable(tolua_S,"Height",tolua_get_CVideo_Height,tolua_set_CVideo_Height);
  tolua_variable(tolua_S,"Depth",tolua_get_CVideo_Depth,tolua_set_CVideo_Depth);
  tolua_variable(tolua_S,"FullScreen",tolua_get_CVideo_FullScreen,tolua_set_CVideo_FullScreen);
  tolua_function(tolua_S,"ResizeScreen",tolua_stratagus_CVideo_ResizeScreen00);
 tolua_endmodule(tolua_S);
 tolua_variable(tolua_S,"Video",tolua_get_Video,tolua_set_Video);
 tolua_function(tolua_S,"ToggleFullScreen",tolua_stratagus_ToggleFullScreen00);
 tolua_cclass(tolua_S,"CGraphic","CGraphic","",NULL);
 tolua_beginmodule(tolua_S,"CGraphic");
  tolua_function(tolua_S,"New",tolua_stratagus_CGraphic_New00);
  tolua_function(tolua_S,"Free",tolua_stratagus_CGraphic_Free00);
  tolua_function(tolua_S,"Load",tolua_stratagus_CGraphic_Load00);
  tolua_function(tolua_S,"Resize",tolua_stratagus_CGraphic_Resize00);
 tolua_endmodule(tolua_S);
#ifdef __cplusplus
 tolua_cclass(tolua_S,"CColor","CColor","",tolua_collect_CColor);
#else
 tolua_cclass(tolua_S,"CColor","CColor","",NULL);
#endif
 tolua_beginmodule(tolua_S,"CColor");
  tolua_function(tolua_S,"new",tolua_stratagus_CColor_new00);
  tolua_function(tolua_S,"new_local",tolua_stratagus_CColor_new00_local);
  tolua_function(tolua_S,".call",tolua_stratagus_CColor_new00_local);
  tolua_variable(tolua_S,"R",tolua_get_CColor_unsigned_R,tolua_set_CColor_unsigned_R);
  tolua_variable(tolua_S,"G",tolua_get_CColor_unsigned_G,tolua_set_CColor_unsigned_G);
  tolua_variable(tolua_S,"B",tolua_get_CColor_unsigned_B,tolua_set_CColor_unsigned_B);
  tolua_variable(tolua_S,"A",tolua_get_CColor_unsigned_A,tolua_set_CColor_unsigned_A);
 tolua_endmodule(tolua_S);
 tolua_cclass(tolua_S,"CFont","CFont","",NULL);
 tolua_beginmodule(tolua_S,"CFont");
  tolua_function(tolua_S,"New",tolua_stratagus_CFont_New00);
  tolua_function(tolua_S,"Get",tolua_stratagus_CFont_Get00);
  tolua_function(tolua_S,"Height",tolua_stratagus_CFont_Height00);
  tolua_function(tolua_S,"Width",tolua_stratagus_CFont_Width00);
 tolua_endmodule(tolua_S);
 tolua_constant(tolua_S,"MaxFontColors",MaxFontColors);
 tolua_cclass(tolua_S,"CFontColor","CFontColor","",NULL);
 tolua_beginmodule(tolua_S,"CFontColor");
  tolua_function(tolua_S,"New",tolua_stratagus_CFontColor_New00);
  tolua_function(tolua_S,"Get",tolua_stratagus_CFontColor_Get00);
  tolua_array(tolua_S,"Colors",tolua_get_stratagus_CFontColor_Colors,tolua_set_stratagus_CFontColor_Colors);
 tolua_endmodule(tolua_S);
 tolua_cclass(tolua_S,"CUpgrade","CUpgrade","",NULL);
 tolua_beginmodule(tolua_S,"CUpgrade");
  tolua_function(tolua_S,"New",tolua_stratagus_CUpgrade_New00);
  tolua_function(tolua_S,"Get",tolua_stratagus_CUpgrade_Get00);
  tolua_array(tolua_S,"Costs",tolua_get_stratagus_CUpgrade_Costs,tolua_set_stratagus_CUpgrade_Costs);
  tolua_variable(tolua_S,"Icon",tolua_get_CUpgrade_Icon_ptr,tolua_set_CUpgrade_Icon_ptr);
 tolua_endmodule(tolua_S);
#ifdef __cplusplus
 tolua_cclass(tolua_S,"CPlayer","CPlayer","",tolua_collect_CPlayer);
#else
 tolua_cclass(tolua_S,"CPlayer","CPlayer","",NULL);
#endif
 tolua_beginmodule(tolua_S,"CPlayer");
  tolua_variable(tolua_S,"Name",tolua_get_CPlayer_Name,tolua_set_CPlayer_Name);
  tolua_array(tolua_S,"Resources",tolua_get_stratagus_CPlayer_Resources,tolua_set_stratagus_CPlayer_Resources);
  tolua_array(tolua_S,"Incomes",tolua_get_stratagus_CPlayer_Incomes,tolua_set_stratagus_CPlayer_Incomes);
  tolua_array(tolua_S,"Revenue",tolua_get_stratagus_CPlayer_Revenue,NULL);
  tolua_array(tolua_S,"Units",tolua_get_stratagus_CPlayer_Units,tolua_set_stratagus_CPlayer_Units);
  tolua_variable(tolua_S,"TotalNumUnits",tolua_get_CPlayer_TotalNumUnits,tolua_set_CPlayer_TotalNumUnits);
  tolua_variable(tolua_S,"NumBuildings",tolua_get_CPlayer_NumBuildings,tolua_set_CPlayer_NumBuildings);
  tolua_variable(tolua_S,"Supply",tolua_get_CPlayer_Supply,tolua_set_CPlayer_Supply);
  tolua_variable(tolua_S,"Demand",tolua_get_CPlayer_Demand,tolua_set_CPlayer_Demand);
  tolua_variable(tolua_S,"UnitLimit",tolua_get_CPlayer_UnitLimit,tolua_set_CPlayer_UnitLimit);
  tolua_variable(tolua_S,"BuildingLimit",tolua_get_CPlayer_BuildingLimit,tolua_set_CPlayer_BuildingLimit);
  tolua_variable(tolua_S,"TotalUnitLimit",tolua_get_CPlayer_TotalUnitLimit,tolua_set_CPlayer_TotalUnitLimit);
  tolua_variable(tolua_S,"Score",tolua_get_CPlayer_Score,tolua_set_CPlayer_Score);
  tolua_variable(tolua_S,"TotalUnits",tolua_get_CPlayer_TotalUnits,tolua_set_CPlayer_TotalUnits);
  tolua_variable(tolua_S,"TotalBuildings",tolua_get_CPlayer_TotalBuildings,tolua_set_CPlayer_TotalBuildings);
  tolua_array(tolua_S,"TotalResources",tolua_get_stratagus_CPlayer_TotalResources,tolua_set_stratagus_CPlayer_TotalResources);
  tolua_variable(tolua_S,"TotalRazings",tolua_get_CPlayer_TotalRazings,tolua_set_CPlayer_TotalRazings);
  tolua_variable(tolua_S,"TotalKills",tolua_get_CPlayer_TotalKills,tolua_set_CPlayer_TotalKills);
 tolua_endmodule(tolua_S);
 tolua_array(tolua_S,"Players",tolua_get_stratagus_Players,tolua_set_stratagus_Players);
 tolua_variable(tolua_S,"ThisPlayer",tolua_get_ThisPlayer_ptr,tolua_set_ThisPlayer_ptr);
 tolua_cclass(tolua_S,"CUnitType","CUnitType","",NULL);
 tolua_beginmodule(tolua_S,"CUnitType");
  tolua_variable(tolua_S,"Ident",tolua_get_CUnitType_Ident,tolua_set_CUnitType_Ident);
  tolua_variable(tolua_S,"Name",tolua_get_CUnitType_Name,tolua_set_CUnitType_Name);
  tolua_variable(tolua_S,"MinAttackRange",tolua_get_CUnitType_MinAttackRange,tolua_set_CUnitType_MinAttackRange);
  tolua_variable(tolua_S,"ClicksToExplode",tolua_get_CUnitType_ClicksToExplode,tolua_set_CUnitType_ClicksToExplode);
  tolua_variable(tolua_S,"Supply",tolua_get_CUnitType_Supply,tolua_set_CUnitType_Supply);
  tolua_variable(tolua_S,"Demand",tolua_get_CUnitType_Demand,tolua_set_CUnitType_Demand);
 tolua_endmodule(tolua_S);
 tolua_cclass(tolua_S,"CUnit","CUnit","",NULL);
 tolua_beginmodule(tolua_S,"CUnit");
  tolua_variable(tolua_S,"X",tolua_get_CUnit_X,tolua_set_CUnit_X);
  tolua_variable(tolua_S,"Y",tolua_get_CUnit_Y,tolua_set_CUnit_Y);
  tolua_variable(tolua_S,"Type",tolua_get_CUnit_Type_ptr,tolua_set_CUnit_Type_ptr);
  tolua_variable(tolua_S,"Player",tolua_get_CUnit_Player_ptr,tolua_set_CUnit_Player_ptr);
 tolua_endmodule(tolua_S);
 tolua_cclass(tolua_S,"CPreference","CPreference","",NULL);
 tolua_beginmodule(tolua_S,"CPreference");
  tolua_variable(tolua_S,"ShowSightRange",tolua_get_CPreference_ShowSightRange,tolua_set_CPreference_ShowSightRange);
  tolua_variable(tolua_S,"ShowReactionRange",tolua_get_CPreference_ShowReactionRange,tolua_set_CPreference_ShowReactionRange);
  tolua_variable(tolua_S,"ShowAttackRange",tolua_get_CPreference_ShowAttackRange,tolua_set_CPreference_ShowAttackRange);
  tolua_variable(tolua_S,"ShowOrders",tolua_get_CPreference_unsigned_ShowOrders,tolua_set_CPreference_unsigned_ShowOrders);
 tolua_endmodule(tolua_S);
 tolua_variable(tolua_S,"Preference",tolua_get_Preference,tolua_set_Preference);
 tolua_function(tolua_S,"GetEffectsVolume",tolua_stratagus_GetEffectsVolume00);
 tolua_function(tolua_S,"SetEffectsVolume",tolua_stratagus_SetEffectsVolume00);
 tolua_function(tolua_S,"GetMusicVolume",tolua_stratagus_GetMusicVolume00);
 tolua_function(tolua_S,"SetMusicVolume",tolua_stratagus_SetMusicVolume00);
 tolua_function(tolua_S,"SetEffectsEnabled",tolua_stratagus_SetEffectsEnabled00);
 tolua_function(tolua_S,"IsEffectsEnabled",tolua_stratagus_IsEffectsEnabled00);
 tolua_function(tolua_S,"SetMusicEnabled",tolua_stratagus_SetMusicEnabled00);
 tolua_function(tolua_S,"IsMusicEnabled",tolua_stratagus_IsMusicEnabled00);
 tolua_function(tolua_S,"PlayMusic",tolua_stratagus_PlayMusic00);
 tolua_function(tolua_S,"StopMusic",tolua_stratagus_StopMusic00);
 tolua_function(tolua_S,"Translate",tolua_stratagus_Translate00);
 tolua_function(tolua_S,"AddTranslation",tolua_stratagus_AddTranslation00);
 tolua_function(tolua_S,"LoadPO",tolua_stratagus_LoadPO00);
 tolua_function(tolua_S,"_",tolua_stratagus__00);
 tolua_endmodule(tolua_S);
 return 1;
}
