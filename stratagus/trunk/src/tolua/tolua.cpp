/*
** Lua binding: stratagus
** Generated automatically by tolua++-1.0.6 on Fri Oct  7 18:21:06 2005.
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
#ifdef _MSC_VER
#pragma warning(disable:4800)
#endif

/* function to release collected object via destructor */
#ifdef __cplusplus

static int tolua_collect_CPlayer (lua_State* tolua_S)
{
 CPlayer* self = (CPlayer*) tolua_tousertype(tolua_S,1,0);
 delete self;
 return 0;
}
#endif


/* function to register type */
static void tolua_reg_types (lua_State* tolua_S)
{
 tolua_usertype(tolua_S,"CMinimap");
 tolua_usertype(tolua_S,"CVideo");
 tolua_usertype(tolua_S,"CGraphic");
 tolua_usertype(tolua_S,"CPlayer");
 tolua_usertype(tolua_S,"CUpgrade");
 tolua_usertype(tolua_S,"CUserInterface");
 tolua_usertype(tolua_S,"CButtonPanel");
 tolua_usertype(tolua_S,"CInfoPanel");
 tolua_usertype(tolua_S,"CIcon");
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

/* method: New of class  CIcon */
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

/* method: Get of class  CIcon */
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

/* get function: Ident of class  CIcon */
static int tolua_get_CIcon_Ident(lua_State* tolua_S)
{
  CIcon* self = (CIcon*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Ident'",NULL);
#endif
 tolua_pushstring(tolua_S,(const char*)self->GetIdent());
 return 1;
}

/* set function: Ident of class  CIcon */
static int tolua_set_CIcon_Ident(lua_State* tolua_S)
{
  CIcon* self = (CIcon*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Ident'",NULL);
 if (!tolua_isstring(tolua_S,2,0,&tolua_err))
 tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->SetIdent(((char*)  tolua_tostring(tolua_S,2,0))
)
;
 return 0;
}

/* get function: G of class  CIcon */
static int tolua_get_CIcon_G_ptr(lua_State* tolua_S)
{
  CIcon* self = (CIcon*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'G'",NULL);
#endif
 tolua_pushusertype(tolua_S,(void*)self->G,"CGraphic");
 return 1;
}

/* set function: G of class  CIcon */
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

/* get function: Frame of class  CIcon */
static int tolua_get_CIcon_Frame(lua_State* tolua_S)
{
  CIcon* self = (CIcon*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Frame'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->Frame);
 return 1;
}

/* set function: Frame of class  CIcon */
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

/* get function: Video */
static int tolua_get_Video(lua_State* tolua_S)
{
 tolua_pushusertype(tolua_S,(void*)&Video,"CVideo");
 return 1;
}

/* set function: Video */
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

/* method: New of class  CGraphic */
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

/* method: Free of class  CGraphic */
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

/* method: New of class  CUpgrade */
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

/* method: Get of class  CUpgrade */
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

/* get function: Costs of class  CUpgrade */
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

/* set function: Costs of class  CUpgrade */
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

/* get function: Icon of class  CUpgrade */
static int tolua_get_CUpgrade_Icon_ptr(lua_State* tolua_S)
{
  CUpgrade* self = (CUpgrade*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Icon'",NULL);
#endif
 tolua_pushusertype(tolua_S,(void*)self->Icon,"CIcon");
 return 1;
}

/* set function: Icon of class  CUpgrade */
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

/* get function: Name of class  CPlayer */
static int tolua_get_CPlayer_Name(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Name'",NULL);
#endif
 tolua_pushstring(tolua_S,(const char*)self->Name);
 return 1;
}

/* set function: Name of class  CPlayer */
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

/* get function: Resources of class  CPlayer */
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

/* set function: Resources of class  CPlayer */
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

/* get function: Incomes of class  CPlayer */
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

/* set function: Incomes of class  CPlayer */
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

/* get function: Revenue of class  CPlayer */
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

/* get function: TotalNumUnits of class  CPlayer */
static int tolua_get_CPlayer_TotalNumUnits(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'TotalNumUnits'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->TotalNumUnits);
 return 1;
}

/* set function: TotalNumUnits of class  CPlayer */
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

/* get function: NumBuildings of class  CPlayer */
static int tolua_get_CPlayer_NumBuildings(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'NumBuildings'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->NumBuildings);
 return 1;
}

/* set function: NumBuildings of class  CPlayer */
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

/* get function: Supply of class  CPlayer */
static int tolua_get_CPlayer_Supply(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Supply'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->Supply);
 return 1;
}

/* set function: Supply of class  CPlayer */
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

/* get function: Demand of class  CPlayer */
static int tolua_get_CPlayer_Demand(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Demand'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->Demand);
 return 1;
}

/* set function: Demand of class  CPlayer */
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

/* get function: UnitLimit of class  CPlayer */
static int tolua_get_CPlayer_UnitLimit(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'UnitLimit'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->UnitLimit);
 return 1;
}

/* set function: UnitLimit of class  CPlayer */
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

/* get function: BuildingLimit of class  CPlayer */
static int tolua_get_CPlayer_BuildingLimit(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'BuildingLimit'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->BuildingLimit);
 return 1;
}

/* set function: BuildingLimit of class  CPlayer */
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

/* get function: TotalUnitLimit of class  CPlayer */
static int tolua_get_CPlayer_TotalUnitLimit(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'TotalUnitLimit'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->TotalUnitLimit);
 return 1;
}

/* set function: TotalUnitLimit of class  CPlayer */
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

/* get function: Score of class  CPlayer */
static int tolua_get_CPlayer_Score(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Score'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->Score);
 return 1;
}

/* set function: Score of class  CPlayer */
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

/* get function: TotalUnits of class  CPlayer */
static int tolua_get_CPlayer_TotalUnits(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'TotalUnits'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->TotalUnits);
 return 1;
}

/* set function: TotalUnits of class  CPlayer */
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

/* get function: TotalBuildings of class  CPlayer */
static int tolua_get_CPlayer_TotalBuildings(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'TotalBuildings'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->TotalBuildings);
 return 1;
}

/* set function: TotalBuildings of class  CPlayer */
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

/* get function: TotalResources of class  CPlayer */
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

/* set function: TotalResources of class  CPlayer */
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

/* get function: TotalRazings of class  CPlayer */
static int tolua_get_CPlayer_TotalRazings(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'TotalRazings'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->TotalRazings);
 return 1;
}

/* set function: TotalRazings of class  CPlayer */
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

/* get function: TotalKills of class  CPlayer */
static int tolua_get_CPlayer_TotalKills(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
 if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'TotalKills'",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->TotalKills);
 return 1;
}

/* set function: TotalKills of class  CPlayer */
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

/* get function: Players */
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

/* set function: Players */
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
 100,101,115, 44, 32,109,116, 41,32
 };
 lua_dobuffer(tolua_S,(char*)B,sizeof(B),"tolua: embedded Lua code 1");
 lua_settop(tolua_S, top);
 } /* end of embedded lua code */

 tolua_constant(tolua_S,"MaxCosts",MaxCosts);
 tolua_constant(tolua_S,"PlayerMax",PlayerMax);
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
 tolua_variable(tolua_S,"Ident",tolua_get_CIcon_Ident,tolua_set_CIcon_Ident);
 tolua_variable(tolua_S,"G",tolua_get_CIcon_G_ptr,tolua_set_CIcon_G_ptr);
 tolua_variable(tolua_S,"Frame",tolua_get_CIcon_Frame,tolua_set_CIcon_Frame);
 tolua_endmodule(tolua_S);
 tolua_cclass(tolua_S,"CVideo","CVideo","",NULL);
 tolua_beginmodule(tolua_S,"CVideo");
 tolua_variable(tolua_S,"Width",tolua_get_CVideo_Width,tolua_set_CVideo_Width);
 tolua_variable(tolua_S,"Height",tolua_get_CVideo_Height,tolua_set_CVideo_Height);
 tolua_variable(tolua_S,"Depth",tolua_get_CVideo_Depth,tolua_set_CVideo_Depth);
 tolua_variable(tolua_S,"FullScreen",tolua_get_CVideo_FullScreen,tolua_set_CVideo_FullScreen);
 tolua_endmodule(tolua_S);
 tolua_variable(tolua_S,"Video",tolua_get_Video,tolua_set_Video);
 tolua_cclass(tolua_S,"CGraphic","CGraphic","",NULL);
 tolua_beginmodule(tolua_S,"CGraphic");
 tolua_function(tolua_S,"New",tolua_stratagus_CGraphic_New00);
 tolua_function(tolua_S,"Free",tolua_stratagus_CGraphic_Free00);
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
 tolua_endmodule(tolua_S);
 return 1;
}
