/*
** Lua binding: stratagus
** Generated automatically by tolua++-1.0.6 on Mon Sep 19 11:53:26 2005.
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
#ifdef _MSC_VER
#pragma warning(disable:4800)
#endif

/* function to register type */
static void tolua_reg_types (lua_State* tolua_S)
{
 tolua_usertype(tolua_S,"CMinimap");
 tolua_usertype(tolua_S,"CVideo");
 tolua_usertype(tolua_S,"CUserInterface");
 tolua_usertype(tolua_S,"CButtonPanel");
 tolua_usertype(tolua_S,"CInfoPanel");
}

/* get function: X of class  CMinimap */
static int tolua_get_CMinimap_X(lua_State* tolua_S)
{
  CMinimap* self = (CMinimap*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'X'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->X);
 return 1;
}

/* set function: X of class  CMinimap */
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

/* get function: Y of class  CMinimap */
static int tolua_get_CMinimap_Y(lua_State* tolua_S)
{
  CMinimap* self = (CMinimap*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Y'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->Y);
 return 1;
}

/* set function: Y of class  CMinimap */
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

/* get function: W of class  CMinimap */
static int tolua_get_CMinimap_W(lua_State* tolua_S)
{
  CMinimap* self = (CMinimap*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'W'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->W);
 return 1;
}

/* set function: W of class  CMinimap */
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

/* get function: H of class  CMinimap */
static int tolua_get_CMinimap_H(lua_State* tolua_S)
{
  CMinimap* self = (CMinimap*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'H'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->H);
 return 1;
}

/* set function: H of class  CMinimap */
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

/* get function: WithTerrain of class  CMinimap */
static int tolua_get_CMinimap_WithTerrain(lua_State* tolua_S)
{
  CMinimap* self = (CMinimap*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'WithTerrain'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->WithTerrain);
 return 1;
}

/* set function: WithTerrain of class  CMinimap */
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

/* get function: ShowSelected of class  CMinimap */
static int tolua_get_CMinimap_ShowSelected(lua_State* tolua_S)
{
  CMinimap* self = (CMinimap*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ShowSelected'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->ShowSelected);
 return 1;
}

/* set function: ShowSelected of class  CMinimap */
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

/* get function: Transparent of class  CMinimap */
static int tolua_get_CMinimap_Transparent(lua_State* tolua_S)
{
  CMinimap* self = (CMinimap*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Transparent'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->Transparent);
 return 1;
}

/* set function: Transparent of class  CMinimap */
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

/* get function: X of class  CButtonPanel */
static int tolua_get_CButtonPanel_X(lua_State* tolua_S)
{
  CButtonPanel* self = (CButtonPanel*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'X'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->X);
 return 1;
}

/* set function: X of class  CButtonPanel */
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

/* get function: Y of class  CButtonPanel */
static int tolua_get_CButtonPanel_Y(lua_State* tolua_S)
{
  CButtonPanel* self = (CButtonPanel*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Y'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->Y);
 return 1;
}

/* set function: Y of class  CButtonPanel */
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

/* get function: ShowCommandKey of class  CButtonPanel */
static int tolua_get_CButtonPanel_ShowCommandKey(lua_State* tolua_S)
{
  CButtonPanel* self = (CButtonPanel*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ShowCommandKey'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->ShowCommandKey);
 return 1;
}

/* set function: ShowCommandKey of class  CButtonPanel */
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

/* get function: X of class  CInfoPanel */
static int tolua_get_CInfoPanel_X(lua_State* tolua_S)
{
  CInfoPanel* self = (CInfoPanel*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'X'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->X);
 return 1;
}

/* set function: X of class  CInfoPanel */
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

/* get function: Y of class  CInfoPanel */
static int tolua_get_CInfoPanel_Y(lua_State* tolua_S)
{
  CInfoPanel* self = (CInfoPanel*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Y'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->Y);
 return 1;
}

/* set function: Y of class  CInfoPanel */
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

/* get function: InfoPanel of class  CUserInterface */
static int tolua_get_CUserInterface_InfoPanel(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'InfoPanel'",NULL);
#endif
 tolua_pushusertype(tolua_S,(void*)&self->InfoPanel,"CInfoPanel");
 return 1;
}

/* set function: InfoPanel of class  CUserInterface */
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

/* get function: ButtonPanel of class  CUserInterface */
static int tolua_get_CUserInterface_ButtonPanel(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ButtonPanel'",NULL);
#endif
 tolua_pushusertype(tolua_S,(void*)&self->ButtonPanel,"CButtonPanel");
 return 1;
}

/* set function: ButtonPanel of class  CUserInterface */
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

/* get function: Minimap of class  CUserInterface */
static int tolua_get_CUserInterface_Minimap(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Minimap'",NULL);
#endif
 tolua_pushusertype(tolua_S,(void*)&self->Minimap,"CMinimap");
 return 1;
}

/* set function: Minimap of class  CUserInterface */
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

/* get function: UI */
static int tolua_get_UI(lua_State* tolua_S)
{
 tolua_pushusertype(tolua_S,(void*)&UI,"CUserInterface");
 return 1;
}

/* set function: UI */
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

/* get function: Width of class  CVideo */
static int tolua_get_CVideo_Width(lua_State* tolua_S)
{
  CVideo* self = (CVideo*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Width'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->Width);
 return 1;
}

/* set function: Width of class  CVideo */
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

/* get function: Height of class  CVideo */
static int tolua_get_CVideo_Height(lua_State* tolua_S)
{
  CVideo* self = (CVideo*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Height'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->Height);
 return 1;
}

/* set function: Height of class  CVideo */
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

/* get function: Depth of class  CVideo */
static int tolua_get_CVideo_Depth(lua_State* tolua_S)
{
  CVideo* self = (CVideo*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Depth'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->Depth);
 return 1;
}

/* set function: Depth of class  CVideo */
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

/* get function: FullScreen of class  CVideo */
static int tolua_get_CVideo_FullScreen(lua_State* tolua_S)
{
  CVideo* self = (CVideo*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'FullScreen'",NULL);
#endif
 tolua_pushboolean(tolua_S,(bool)self->FullScreen);
 return 1;
}

/* set function: FullScreen of class  CVideo */
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

/* Open function */
TOLUA_API int tolua_stratagus_open (lua_State* tolua_S)
{
 tolua_open(tolua_S);
 tolua_reg_types(tolua_S);
 tolua_module(tolua_S,NULL,1);
 tolua_beginmodule(tolua_S,NULL);
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
 tolua_cclass(tolua_S,"CVideo","CVideo","",NULL);
 tolua_beginmodule(tolua_S,"CVideo");
 tolua_variable(tolua_S,"Width",tolua_get_CVideo_Width,tolua_set_CVideo_Width);
 tolua_variable(tolua_S,"Height",tolua_get_CVideo_Height,tolua_set_CVideo_Height);
 tolua_variable(tolua_S,"Depth",tolua_get_CVideo_Depth,tolua_set_CVideo_Depth);
 tolua_variable(tolua_S,"FullScreen",tolua_get_CVideo_FullScreen,tolua_set_CVideo_FullScreen);
 tolua_endmodule(tolua_S);
 tolua_endmodule(tolua_S);
 return 1;
}
