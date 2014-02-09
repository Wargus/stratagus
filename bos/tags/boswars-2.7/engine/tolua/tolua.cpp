/*
** Lua binding: stratagus
** Generated automatically by tolua++-1.0.93 on Wed Dec  7 01:31:03 2011.
*/

#ifndef __cplusplus
#include "stdlib.h"
#endif
#include "string.h"

#include "tolua++.h"

/* Exported function */
TOLUA_API int  tolua_stratagus_open (lua_State* tolua_S);

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
#include "netconnect.h"
#include "map.h"
#ifdef _MSC_VER
#pragma warning(disable:4800)
#endif
using std::string;
using std::vector;
using namespace gcn;
#include "network.h"
int GetNetworkState() {return (int)NetLocalState;}
extern string NetworkMapName;
void NetworkGamePrepareGameSettings(void);
extern void InitVideo();
#include "player.h"
#include "editor.h"
bool IsReplayGame();
void StartMap(const string &str, bool clean = true);
void StartReplay(const string &str, bool reveal = false);
void StartSavedGame(const string &str);
int SaveReplay(const std::string &filename);
#include "results.h"
void StopGame(GameResults result);
#include "settings.h"
extern int AlliedUnitRecyclingEfficiency[MaxCosts];
extern int EnemyUnitRecyclingEfficiency[MaxCosts];
#include "patch_type.h"
#include "patch.h"
#include "patch_manager.h"
#include "pathfinder.h"
int GetNumOpponents(int player);
int GetTimer();
void ActionVictory();
void ActionDefeat();
void ActionDraw();
void ActionSetTimer(int cycles, bool increasing);
void ActionStartTimer();
void ActionStopTimer();
void SetTrigger(int trigger);
#include "particle.h"
extern std::string CliMapName;

/* function to release collected object via destructor */
#ifdef __cplusplus

static int tolua_collect_MenuScreen (lua_State* tolua_S)
{
 MenuScreen* self = (MenuScreen*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_ImageRadioButton (lua_State* tolua_S)
{
 ImageRadioButton* self = (ImageRadioButton*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_vector_CUIButton_ (lua_State* tolua_S)
{
 vector<CUIButton>* self = (vector<CUIButton>*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_CResourceInfo (lua_State* tolua_S)
{
 CResourceInfo* self = (CResourceInfo*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_CChunkParticle (lua_State* tolua_S)
{
 CChunkParticle* self = (CChunkParticle*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_vector_CFiller_ (lua_State* tolua_S)
{
 vector<CFiller>* self = (vector<CFiller>*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_TextField (lua_State* tolua_S)
{
 TextField* self = (TextField*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_CUIButton (lua_State* tolua_S)
{
 CUIButton* self = (CUIButton*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_MultiLineLabel (lua_State* tolua_S)
{
 MultiLineLabel* self = (MultiLineLabel*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_CheckBox (lua_State* tolua_S)
{
 CheckBox* self = (CheckBox*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_ImageWidget (lua_State* tolua_S)
{
 ImageWidget* self = (ImageWidget*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_CPlayer (lua_State* tolua_S)
{
 CPlayer* self = (CPlayer*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_CColor (lua_State* tolua_S)
{
 CColor* self = (CColor*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_ImageSlider (lua_State* tolua_S)
{
 ImageSlider* self = (ImageSlider*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_ListBoxWidget (lua_State* tolua_S)
{
 ListBoxWidget* self = (ListBoxWidget*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_Color (lua_State* tolua_S)
{
 Color* self = (Color*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_Window (lua_State* tolua_S)
{
 Window* self = (Window*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_DropDownWidget (lua_State* tolua_S)
{
 DropDownWidget* self = (DropDownWidget*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_CFiller (lua_State* tolua_S)
{
 CFiller* self = (CFiller*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_vector_string_ (lua_State* tolua_S)
{
 vector<string>* self = (vector<string>*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_ScrollArea (lua_State* tolua_S)
{
 ScrollArea* self = (ScrollArea*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_Windows (lua_State* tolua_S)
{
 Windows* self = (Windows*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_Container (lua_State* tolua_S)
{
 Container* self = (Container*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_ButtonWidget (lua_State* tolua_S)
{
 ButtonWidget* self = (ButtonWidget*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_CParticleManager (lua_State* tolua_S)
{
 CParticleManager* self = (CParticleManager*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_ImageCheckBox (lua_State* tolua_S)
{
 ImageCheckBox* self = (ImageCheckBox*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_ScrollingWidget (lua_State* tolua_S)
{
 ScrollingWidget* self = (ScrollingWidget*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_CNetworkHost (lua_State* tolua_S)
{
 CNetworkHost* self = (CNetworkHost*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_StatBoxWidget (lua_State* tolua_S)
{
 StatBoxWidget* self = (StatBoxWidget*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_SettingsPresets (lua_State* tolua_S)
{
 SettingsPresets* self = (SettingsPresets*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_CPosition (lua_State* tolua_S)
{
 CPosition* self = (CPosition*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_GraphicAnimation (lua_State* tolua_S)
{
 GraphicAnimation* self = (GraphicAnimation*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_Label (lua_State* tolua_S)
{
 Label* self = (Label*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_RadioButton (lua_State* tolua_S)
{
 RadioButton* self = (RadioButton*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_LuaActionListener (lua_State* tolua_S)
{
 LuaActionListener* self = (LuaActionListener*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_StaticParticle (lua_State* tolua_S)
{
 StaticParticle* self = (StaticParticle*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_ImageButton (lua_State* tolua_S)
{
 ImageButton* self = (ImageButton*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}

static int tolua_collect_Slider (lua_State* tolua_S)
{
 Slider* self = (Slider*) tolua_tousertype(tolua_S,1,0);
	Mtolua_delete(self);
	return 0;
}
#endif


/* function to register type */
static void tolua_reg_types (lua_State* tolua_S)
{
 tolua_usertype(tolua_S,"CPlayerColorGraphic");
 tolua_usertype(tolua_S,"MenuScreen");
 tolua_usertype(tolua_S,"BasicContainer");
 tolua_usertype(tolua_S,"vector<CUIButton>");
 tolua_usertype(tolua_S,"CFontColor");
 tolua_usertype(tolua_S,"CMap");
 tolua_usertype(tolua_S,"CChunkParticle");
 tolua_usertype(tolua_S,"CIcon");
 tolua_usertype(tolua_S,"CUIButton");
 tolua_usertype(tolua_S,"CheckBox");
 tolua_usertype(tolua_S,"ImageButton");
 tolua_usertype(tolua_S,"CPlayer");
 tolua_usertype(tolua_S,"CUnit");
 tolua_usertype(tolua_S,"CParticle");
 tolua_usertype(tolua_S,"Graphics");
 tolua_usertype(tolua_S,"CColor");
 tolua_usertype(tolua_S,"CButtonPanel");
 tolua_usertype(tolua_S,"CPatch");
 tolua_usertype(tolua_S,"ImageSlider");
 tolua_usertype(tolua_S,"ListBoxWidget");
 tolua_usertype(tolua_S,"Color");
 tolua_usertype(tolua_S,"ButtonStyle");
 tolua_usertype(tolua_S,"DropDownWidget");
 tolua_usertype(tolua_S,"CPieMenu");
 tolua_usertype(tolua_S,"ScrollArea");
 tolua_usertype(tolua_S,"Button");
 tolua_usertype(tolua_S,"CInfoPanel");
 tolua_usertype(tolua_S,"Animation");
 tolua_usertype(tolua_S,"ScrollingWidget");
 tolua_usertype(tolua_S,"CGraphic");
 tolua_usertype(tolua_S,"StatBoxWidget");
 tolua_usertype(tolua_S,"CUITimer");
 tolua_usertype(tolua_S,"Widget");
 tolua_usertype(tolua_S,"Label");
 tolua_usertype(tolua_S,"Settings");
 tolua_usertype(tolua_S,"CResourceInfo");
 tolua_usertype(tolua_S,"vector<CFiller>");
 tolua_usertype(tolua_S,"CUnitType");
 tolua_usertype(tolua_S,"CMapInfo");
 tolua_usertype(tolua_S,"TextField");
 tolua_usertype(tolua_S,"CServerSetup");
 tolua_usertype(tolua_S,"CMapArea");
 tolua_usertype(tolua_S,"ImageWidget");
 tolua_usertype(tolua_S,"SettingsPresets");
 tolua_usertype(tolua_S,"gcn::Graphics");
 tolua_usertype(tolua_S,"CStatusLine");
 tolua_usertype(tolua_S,"CMinimap");
 tolua_usertype(tolua_S,"CPatchManager");
 tolua_usertype(tolua_S,"Window");
 tolua_usertype(tolua_S,"CParticleManager");
 tolua_usertype(tolua_S,"CPreference");
 tolua_usertype(tolua_S,"CFiller");
 tolua_usertype(tolua_S,"CUserInterface");
 tolua_usertype(tolua_S,"vector<string>");
 tolua_usertype(tolua_S,"Windows");
 tolua_usertype(tolua_S,"MultiLineLabel");
 tolua_usertype(tolua_S,"CSmokeParticle");
 tolua_usertype(tolua_S,"CPatchType");
 tolua_usertype(tolua_S,"Container");
 tolua_usertype(tolua_S,"ButtonWidget");
 tolua_usertype(tolua_S,"CFont");
 tolua_usertype(tolua_S,"CVideo");
 tolua_usertype(tolua_S,"ImageCheckBox");
 tolua_usertype(tolua_S,"LuaActionListener");
 tolua_usertype(tolua_S,"ImageRadioButton");
 tolua_usertype(tolua_S,"CEditor");
 tolua_usertype(tolua_S,"ListBox");
 tolua_usertype(tolua_S,"CPosition");
 tolua_usertype(tolua_S,"GraphicAnimation");
 tolua_usertype(tolua_S,"CNetworkHost");
 tolua_usertype(tolua_S,"RadioButton");
 tolua_usertype(tolua_S,"CViewport");
 tolua_usertype(tolua_S,"StaticParticle");
 tolua_usertype(tolua_S,"DropDown");
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
   LuaActionListener* tolua_ret = (LuaActionListener*)  Mtolua_new((LuaActionListener)(lua,luaref));
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
   LuaActionListener* tolua_ret = (LuaActionListener*)  Mtolua_new((LuaActionListener)(lua,luaref));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"LuaActionListener");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
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

/* method: new of class  CUIButton */
#ifndef TOLUA_DISABLE_tolua_stratagus_CUIButton_new00
static int tolua_stratagus_CUIButton_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"CUIButton",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   CUIButton* tolua_ret = (CUIButton*)  Mtolua_new((CUIButton)());
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"CUIButton");
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

/* method: new_local of class  CUIButton */
#ifndef TOLUA_DISABLE_tolua_stratagus_CUIButton_new00_local
static int tolua_stratagus_CUIButton_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"CUIButton",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   CUIButton* tolua_ret = (CUIButton*)  Mtolua_new((CUIButton)());
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"CUIButton");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
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

/* method: delete of class  CUIButton */
#ifndef TOLUA_DISABLE_tolua_stratagus_CUIButton_delete00
static int tolua_stratagus_CUIButton_delete00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"CUIButton",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CUIButton* self = (CUIButton*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'delete'", NULL);
#endif
  Mtolua_delete(self);
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'delete'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* get function: X of class  CUIButton */
#ifndef TOLUA_DISABLE_tolua_get_CUIButton_X
static int tolua_get_CUIButton_X(lua_State* tolua_S)
{
  CUIButton* self = (CUIButton*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'X'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->X);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: X of class  CUIButton */
#ifndef TOLUA_DISABLE_tolua_set_CUIButton_X
static int tolua_set_CUIButton_X(lua_State* tolua_S)
{
  CUIButton* self = (CUIButton*)  tolua_tousertype(tolua_S,1,0);
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

/* get function: Y of class  CUIButton */
#ifndef TOLUA_DISABLE_tolua_get_CUIButton_Y
static int tolua_get_CUIButton_Y(lua_State* tolua_S)
{
  CUIButton* self = (CUIButton*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Y'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->Y);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Y of class  CUIButton */
#ifndef TOLUA_DISABLE_tolua_set_CUIButton_Y
static int tolua_set_CUIButton_Y(lua_State* tolua_S)
{
  CUIButton* self = (CUIButton*)  tolua_tousertype(tolua_S,1,0);
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

/* get function: Text of class  CUIButton */
#ifndef TOLUA_DISABLE_tolua_get_CUIButton_Text
static int tolua_get_CUIButton_Text(lua_State* tolua_S)
{
  CUIButton* self = (CUIButton*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Text'",NULL);
#endif
  tolua_pushcppstring(tolua_S,(const char*)self->Text);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Text of class  CUIButton */
#ifndef TOLUA_DISABLE_tolua_set_CUIButton_Text
static int tolua_set_CUIButton_Text(lua_State* tolua_S)
{
  CUIButton* self = (CUIButton*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Text'",NULL);
  if (!tolua_iscppstring(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Text = ((string)  tolua_tocppstring(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Style of class  CUIButton */
#ifndef TOLUA_DISABLE_tolua_get_CUIButton_Style_ptr
static int tolua_get_CUIButton_Style_ptr(lua_State* tolua_S)
{
  CUIButton* self = (CUIButton*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Style'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)self->Style,"ButtonStyle");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Style of class  CUIButton */
#ifndef TOLUA_DISABLE_tolua_set_CUIButton_Style_ptr
static int tolua_set_CUIButton_Style_ptr(lua_State* tolua_S)
{
  CUIButton* self = (CUIButton*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Style'",NULL);
  if (!tolua_isusertype(tolua_S,2,"ButtonStyle",0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Style = ((ButtonStyle*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Callback of class  CUIButton */
#ifndef TOLUA_DISABLE_tolua_get_CUIButton_Callback_ptr
static int tolua_get_CUIButton_Callback_ptr(lua_State* tolua_S)
{
  CUIButton* self = (CUIButton*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Callback'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)self->Callback,"LuaActionListener");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Callback of class  CUIButton */
#ifndef TOLUA_DISABLE_tolua_set_CUIButton_Callback_ptr
static int tolua_set_CUIButton_Callback_ptr(lua_State* tolua_S)
{
  CUIButton* self = (CUIButton*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Callback'",NULL);
  if (!tolua_isusertype(tolua_S,2,"LuaActionListener",0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Callback = ((LuaActionListener*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: X of class  CMapArea */
#ifndef TOLUA_DISABLE_tolua_get_CMapArea_X
static int tolua_get_CMapArea_X(lua_State* tolua_S)
{
  CMapArea* self = (CMapArea*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'X'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->X);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: X of class  CMapArea */
#ifndef TOLUA_DISABLE_tolua_set_CMapArea_X
static int tolua_set_CMapArea_X(lua_State* tolua_S)
{
  CMapArea* self = (CMapArea*)  tolua_tousertype(tolua_S,1,0);
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

/* get function: Y of class  CMapArea */
#ifndef TOLUA_DISABLE_tolua_get_CMapArea_Y
static int tolua_get_CMapArea_Y(lua_State* tolua_S)
{
  CMapArea* self = (CMapArea*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Y'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->Y);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Y of class  CMapArea */
#ifndef TOLUA_DISABLE_tolua_set_CMapArea_Y
static int tolua_set_CMapArea_Y(lua_State* tolua_S)
{
  CMapArea* self = (CMapArea*)  tolua_tousertype(tolua_S,1,0);
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

/* get function: EndX of class  CMapArea */
#ifndef TOLUA_DISABLE_tolua_get_CMapArea_EndX
static int tolua_get_CMapArea_EndX(lua_State* tolua_S)
{
  CMapArea* self = (CMapArea*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'EndX'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->EndX);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: EndX of class  CMapArea */
#ifndef TOLUA_DISABLE_tolua_set_CMapArea_EndX
static int tolua_set_CMapArea_EndX(lua_State* tolua_S)
{
  CMapArea* self = (CMapArea*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'EndX'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->EndX = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: EndY of class  CMapArea */
#ifndef TOLUA_DISABLE_tolua_get_CMapArea_EndY
static int tolua_get_CMapArea_EndY(lua_State* tolua_S)
{
  CMapArea* self = (CMapArea*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'EndY'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->EndY);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: EndY of class  CMapArea */
#ifndef TOLUA_DISABLE_tolua_set_CMapArea_EndY
static int tolua_set_CMapArea_EndY(lua_State* tolua_S)
{
  CMapArea* self = (CMapArea*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'EndY'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->EndY = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: ScrollPaddingLeft of class  CMapArea */
#ifndef TOLUA_DISABLE_tolua_get_CMapArea_ScrollPaddingLeft
static int tolua_get_CMapArea_ScrollPaddingLeft(lua_State* tolua_S)
{
  CMapArea* self = (CMapArea*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ScrollPaddingLeft'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->ScrollPaddingLeft);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: ScrollPaddingLeft of class  CMapArea */
#ifndef TOLUA_DISABLE_tolua_set_CMapArea_ScrollPaddingLeft
static int tolua_set_CMapArea_ScrollPaddingLeft(lua_State* tolua_S)
{
  CMapArea* self = (CMapArea*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ScrollPaddingLeft'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->ScrollPaddingLeft = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: ScrollPaddingRight of class  CMapArea */
#ifndef TOLUA_DISABLE_tolua_get_CMapArea_ScrollPaddingRight
static int tolua_get_CMapArea_ScrollPaddingRight(lua_State* tolua_S)
{
  CMapArea* self = (CMapArea*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ScrollPaddingRight'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->ScrollPaddingRight);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: ScrollPaddingRight of class  CMapArea */
#ifndef TOLUA_DISABLE_tolua_set_CMapArea_ScrollPaddingRight
static int tolua_set_CMapArea_ScrollPaddingRight(lua_State* tolua_S)
{
  CMapArea* self = (CMapArea*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ScrollPaddingRight'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->ScrollPaddingRight = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: ScrollPaddingTop of class  CMapArea */
#ifndef TOLUA_DISABLE_tolua_get_CMapArea_ScrollPaddingTop
static int tolua_get_CMapArea_ScrollPaddingTop(lua_State* tolua_S)
{
  CMapArea* self = (CMapArea*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ScrollPaddingTop'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->ScrollPaddingTop);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: ScrollPaddingTop of class  CMapArea */
#ifndef TOLUA_DISABLE_tolua_set_CMapArea_ScrollPaddingTop
static int tolua_set_CMapArea_ScrollPaddingTop(lua_State* tolua_S)
{
  CMapArea* self = (CMapArea*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ScrollPaddingTop'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->ScrollPaddingTop = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: ScrollPaddingBottom of class  CMapArea */
#ifndef TOLUA_DISABLE_tolua_get_CMapArea_ScrollPaddingBottom
static int tolua_get_CMapArea_ScrollPaddingBottom(lua_State* tolua_S)
{
  CMapArea* self = (CMapArea*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ScrollPaddingBottom'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->ScrollPaddingBottom);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: ScrollPaddingBottom of class  CMapArea */
#ifndef TOLUA_DISABLE_tolua_set_CMapArea_ScrollPaddingBottom
static int tolua_set_CMapArea_ScrollPaddingBottom(lua_State* tolua_S)
{
  CMapArea* self = (CMapArea*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ScrollPaddingBottom'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->ScrollPaddingBottom = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* method: Viewport2MapX of class  CViewport */
#ifndef TOLUA_DISABLE_tolua_stratagus_CViewport_Viewport2MapX00
static int tolua_stratagus_CViewport_Viewport2MapX00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"CViewport",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CViewport* self = (CViewport*)  tolua_tousertype(tolua_S,1,0);
  int x = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'Viewport2MapX'", NULL);
#endif
  {
   int tolua_ret = (int)  self->Viewport2MapX(x);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Viewport2MapX'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: Viewport2MapY of class  CViewport */
#ifndef TOLUA_DISABLE_tolua_stratagus_CViewport_Viewport2MapY00
static int tolua_stratagus_CViewport_Viewport2MapY00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"CViewport",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CViewport* self = (CViewport*)  tolua_tousertype(tolua_S,1,0);
  int y = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'Viewport2MapY'", NULL);
#endif
  {
   int tolua_ret = (int)  self->Viewport2MapY(y);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Viewport2MapY'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  CFiller */
#ifndef TOLUA_DISABLE_tolua_stratagus_CFiller_new00
static int tolua_stratagus_CFiller_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"CFiller",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   CFiller* tolua_ret = (CFiller*)  Mtolua_new((CFiller)());
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"CFiller");
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

/* method: new_local of class  CFiller */
#ifndef TOLUA_DISABLE_tolua_stratagus_CFiller_new00_local
static int tolua_stratagus_CFiller_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"CFiller",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   CFiller* tolua_ret = (CFiller*)  Mtolua_new((CFiller)());
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"CFiller");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
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

/* get function: G of class  CFiller */
#ifndef TOLUA_DISABLE_tolua_get_CFiller_G_ptr
static int tolua_get_CFiller_G_ptr(lua_State* tolua_S)
{
  CFiller* self = (CFiller*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'G'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)self->G,"CGraphic");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: G of class  CFiller */
#ifndef TOLUA_DISABLE_tolua_set_CFiller_G_ptr
static int tolua_set_CFiller_G_ptr(lua_State* tolua_S)
{
  CFiller* self = (CFiller*)  tolua_tousertype(tolua_S,1,0);
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

/* get function: X of class  CFiller */
#ifndef TOLUA_DISABLE_tolua_get_CFiller_X
static int tolua_get_CFiller_X(lua_State* tolua_S)
{
  CFiller* self = (CFiller*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'X'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->X);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: X of class  CFiller */
#ifndef TOLUA_DISABLE_tolua_set_CFiller_X
static int tolua_set_CFiller_X(lua_State* tolua_S)
{
  CFiller* self = (CFiller*)  tolua_tousertype(tolua_S,1,0);
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

/* get function: Y of class  CFiller */
#ifndef TOLUA_DISABLE_tolua_get_CFiller_Y
static int tolua_get_CFiller_Y(lua_State* tolua_S)
{
  CFiller* self = (CFiller*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Y'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->Y);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Y of class  CFiller */
#ifndef TOLUA_DISABLE_tolua_set_CFiller_Y
static int tolua_set_CFiller_Y(lua_State* tolua_S)
{
  CFiller* self = (CFiller*)  tolua_tousertype(tolua_S,1,0);
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

/* method: new of class  vector<CFiller> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_CFiller__new00
static int tolua_stratagus_vector_CFiller__new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"vector<CFiller>",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   vector<CFiller>* tolua_ret = (vector<CFiller>*)  Mtolua_new((vector<CFiller>)());
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"vector<CFiller>");
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

/* method: new_local of class  vector<CFiller> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_CFiller__new00_local
static int tolua_stratagus_vector_CFiller__new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"vector<CFiller>",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   vector<CFiller>* tolua_ret = (vector<CFiller>*)  Mtolua_new((vector<CFiller>)());
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"vector<CFiller>");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
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

/* method: delete of class  vector<CFiller> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_CFiller__delete00
static int tolua_stratagus_vector_CFiller__delete00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"vector<CFiller>",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  vector<CFiller>* self = (vector<CFiller>*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'delete'", NULL);
#endif
  Mtolua_delete(self);
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'delete'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: operator[] of class  vector<CFiller> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_CFiller___geti00
static int tolua_stratagus_vector_CFiller___geti00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const vector<CFiller>",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const vector<CFiller>* self = (const vector<CFiller>*)  tolua_tousertype(tolua_S,1,0);
  int index = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'operator[]'", NULL);
#endif
  {
   const CFiller tolua_ret = (const CFiller)  self->operator[](index);
   {
#ifdef __cplusplus
    void* tolua_obj = Mtolua_new((CFiller)(tolua_ret));
     tolua_pushusertype(tolua_S,tolua_obj,"const CFiller");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
#else
    void* tolua_obj = tolua_copy(tolua_S,(void*)&tolua_ret,sizeof(const CFiller));
     tolua_pushusertype(tolua_S,tolua_obj,"const CFiller");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
#endif
   }
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function '.geti'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: operator&[] of class  vector<CFiller> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_CFiller___seti00
static int tolua_stratagus_vector_CFiller___seti00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"vector<CFiller>",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,3,&tolua_err) || !tolua_isusertype(tolua_S,3,"CFiller",0,&tolua_err)) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  vector<CFiller>* self = (vector<CFiller>*)  tolua_tousertype(tolua_S,1,0);
  int index = ((int)  tolua_tonumber(tolua_S,2,0));
  CFiller tolua_value = *((CFiller*)  tolua_tousertype(tolua_S,3,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'operator&[]'", NULL);
#endif
  self->operator[](index) =  tolua_value;
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function '.seti'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: operator[] of class  vector<CFiller> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_CFiller___geti01
static int tolua_stratagus_vector_CFiller___geti01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"vector<CFiller>",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  vector<CFiller>* self = (vector<CFiller>*)  tolua_tousertype(tolua_S,1,0);
  int index = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'operator[]'", NULL);
#endif
  {
   CFiller tolua_ret = (CFiller)  self->operator[](index);
   {
#ifdef __cplusplus
    void* tolua_obj = Mtolua_new((CFiller)(tolua_ret));
     tolua_pushusertype(tolua_S,tolua_obj,"CFiller");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
#else
    void* tolua_obj = tolua_copy(tolua_S,(void*)&tolua_ret,sizeof(CFiller));
     tolua_pushusertype(tolua_S,tolua_obj,"CFiller");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
#endif
   }
  }
 }
 return 1;
tolua_lerror:
 return tolua_stratagus_vector_CFiller___geti00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: at of class  vector<CFiller> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_CFiller__at00
static int tolua_stratagus_vector_CFiller__at00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const vector<CFiller>",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const vector<CFiller>* self = (const vector<CFiller>*)  tolua_tousertype(tolua_S,1,0);
  int index = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'at'", NULL);
#endif
  {
   const CFiller& tolua_ret = (const CFiller&)  self->at(index);
    tolua_pushusertype(tolua_S,(void*)&tolua_ret,"const CFiller");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'at'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: at of class  vector<CFiller> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_CFiller__at01
static int tolua_stratagus_vector_CFiller__at01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"vector<CFiller>",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  vector<CFiller>* self = (vector<CFiller>*)  tolua_tousertype(tolua_S,1,0);
  int index = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'at'", NULL);
#endif
  {
   CFiller& tolua_ret = (CFiller&)  self->at(index);
    tolua_pushusertype(tolua_S,(void*)&tolua_ret,"CFiller");
  }
 }
 return 1;
tolua_lerror:
 return tolua_stratagus_vector_CFiller__at00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: front of class  vector<CFiller> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_CFiller__front00
static int tolua_stratagus_vector_CFiller__front00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const vector<CFiller>",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const vector<CFiller>* self = (const vector<CFiller>*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'front'", NULL);
#endif
  {
   const CFiller& tolua_ret = (const CFiller&)  self->front();
    tolua_pushusertype(tolua_S,(void*)&tolua_ret,"const CFiller");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'front'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: front of class  vector<CFiller> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_CFiller__front01
static int tolua_stratagus_vector_CFiller__front01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"vector<CFiller>",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  vector<CFiller>* self = (vector<CFiller>*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'front'", NULL);
#endif
  {
   CFiller& tolua_ret = (CFiller&)  self->front();
    tolua_pushusertype(tolua_S,(void*)&tolua_ret,"CFiller");
  }
 }
 return 1;
tolua_lerror:
 return tolua_stratagus_vector_CFiller__front00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: back of class  vector<CFiller> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_CFiller__back00
static int tolua_stratagus_vector_CFiller__back00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const vector<CFiller>",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const vector<CFiller>* self = (const vector<CFiller>*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'back'", NULL);
#endif
  {
   const CFiller& tolua_ret = (const CFiller&)  self->back();
    tolua_pushusertype(tolua_S,(void*)&tolua_ret,"const CFiller");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'back'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: back of class  vector<CFiller> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_CFiller__back01
static int tolua_stratagus_vector_CFiller__back01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"vector<CFiller>",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  vector<CFiller>* self = (vector<CFiller>*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'back'", NULL);
#endif
  {
   CFiller& tolua_ret = (CFiller&)  self->back();
    tolua_pushusertype(tolua_S,(void*)&tolua_ret,"CFiller");
  }
 }
 return 1;
tolua_lerror:
 return tolua_stratagus_vector_CFiller__back00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: push_back of class  vector<CFiller> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_CFiller__push_back00
static int tolua_stratagus_vector_CFiller__push_back00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"vector<CFiller>",0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"CFiller",0,&tolua_err)) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  vector<CFiller>* self = (vector<CFiller>*)  tolua_tousertype(tolua_S,1,0);
  CFiller val = *((CFiller*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'push_back'", NULL);
#endif
  {
   self->push_back(val);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'push_back'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: pop_back of class  vector<CFiller> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_CFiller__pop_back00
static int tolua_stratagus_vector_CFiller__pop_back00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"vector<CFiller>",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  vector<CFiller>* self = (vector<CFiller>*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'pop_back'", NULL);
#endif
  {
   self->pop_back();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'pop_back'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: assign of class  vector<CFiller> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_CFiller__assign00
static int tolua_stratagus_vector_CFiller__assign00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"vector<CFiller>",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,3,&tolua_err) || !tolua_isusertype(tolua_S,3,"const CFiller",0,&tolua_err)) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  vector<CFiller>* self = (vector<CFiller>*)  tolua_tousertype(tolua_S,1,0);
  int num = ((int)  tolua_tonumber(tolua_S,2,0));
  const CFiller* val = ((const CFiller*)  tolua_tousertype(tolua_S,3,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'assign'", NULL);
#endif
  {
   self->assign(num,*val);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'assign'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: clear of class  vector<CFiller> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_CFiller__clear00
static int tolua_stratagus_vector_CFiller__clear00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"vector<CFiller>",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  vector<CFiller>* self = (vector<CFiller>*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'clear'", NULL);
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

/* method: empty of class  vector<CFiller> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_CFiller__empty00
static int tolua_stratagus_vector_CFiller__empty00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const vector<CFiller>",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const vector<CFiller>* self = (const vector<CFiller>*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'empty'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->empty();
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'empty'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: size of class  vector<CFiller> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_CFiller__size00
static int tolua_stratagus_vector_CFiller__size00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const vector<CFiller>",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const vector<CFiller>* self = (const vector<CFiller>*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'size'", NULL);
#endif
  {
   int tolua_ret = (int)  self->size();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'size'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  vector<CUIButton> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_CUIButton__new00
static int tolua_stratagus_vector_CUIButton__new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"vector<CUIButton>",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   vector<CUIButton>* tolua_ret = (vector<CUIButton>*)  Mtolua_new((vector<CUIButton>)());
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"vector<CUIButton>");
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

/* method: new_local of class  vector<CUIButton> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_CUIButton__new00_local
static int tolua_stratagus_vector_CUIButton__new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"vector<CUIButton>",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   vector<CUIButton>* tolua_ret = (vector<CUIButton>*)  Mtolua_new((vector<CUIButton>)());
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"vector<CUIButton>");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
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

/* method: delete of class  vector<CUIButton> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_CUIButton__delete00
static int tolua_stratagus_vector_CUIButton__delete00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"vector<CUIButton>",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  vector<CUIButton>* self = (vector<CUIButton>*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'delete'", NULL);
#endif
  Mtolua_delete(self);
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'delete'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: operator[] of class  vector<CUIButton> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_CUIButton___geti00
static int tolua_stratagus_vector_CUIButton___geti00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const vector<CUIButton>",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const vector<CUIButton>* self = (const vector<CUIButton>*)  tolua_tousertype(tolua_S,1,0);
  int index = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'operator[]'", NULL);
#endif
  {
   const CUIButton tolua_ret = (const CUIButton)  self->operator[](index);
   {
#ifdef __cplusplus
    void* tolua_obj = Mtolua_new((CUIButton)(tolua_ret));
     tolua_pushusertype(tolua_S,tolua_obj,"const CUIButton");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
#else
    void* tolua_obj = tolua_copy(tolua_S,(void*)&tolua_ret,sizeof(const CUIButton));
     tolua_pushusertype(tolua_S,tolua_obj,"const CUIButton");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
#endif
   }
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function '.geti'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: operator&[] of class  vector<CUIButton> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_CUIButton___seti00
static int tolua_stratagus_vector_CUIButton___seti00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"vector<CUIButton>",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,3,&tolua_err) || !tolua_isusertype(tolua_S,3,"CUIButton",0,&tolua_err)) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  vector<CUIButton>* self = (vector<CUIButton>*)  tolua_tousertype(tolua_S,1,0);
  int index = ((int)  tolua_tonumber(tolua_S,2,0));
  CUIButton tolua_value = *((CUIButton*)  tolua_tousertype(tolua_S,3,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'operator&[]'", NULL);
#endif
  self->operator[](index) =  tolua_value;
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function '.seti'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: operator[] of class  vector<CUIButton> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_CUIButton___geti01
static int tolua_stratagus_vector_CUIButton___geti01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"vector<CUIButton>",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  vector<CUIButton>* self = (vector<CUIButton>*)  tolua_tousertype(tolua_S,1,0);
  int index = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'operator[]'", NULL);
#endif
  {
   CUIButton tolua_ret = (CUIButton)  self->operator[](index);
   {
#ifdef __cplusplus
    void* tolua_obj = Mtolua_new((CUIButton)(tolua_ret));
     tolua_pushusertype(tolua_S,tolua_obj,"CUIButton");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
#else
    void* tolua_obj = tolua_copy(tolua_S,(void*)&tolua_ret,sizeof(CUIButton));
     tolua_pushusertype(tolua_S,tolua_obj,"CUIButton");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
#endif
   }
  }
 }
 return 1;
tolua_lerror:
 return tolua_stratagus_vector_CUIButton___geti00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: at of class  vector<CUIButton> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_CUIButton__at00
static int tolua_stratagus_vector_CUIButton__at00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const vector<CUIButton>",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const vector<CUIButton>* self = (const vector<CUIButton>*)  tolua_tousertype(tolua_S,1,0);
  int index = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'at'", NULL);
#endif
  {
   const CUIButton& tolua_ret = (const CUIButton&)  self->at(index);
    tolua_pushusertype(tolua_S,(void*)&tolua_ret,"const CUIButton");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'at'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: at of class  vector<CUIButton> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_CUIButton__at01
static int tolua_stratagus_vector_CUIButton__at01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"vector<CUIButton>",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  vector<CUIButton>* self = (vector<CUIButton>*)  tolua_tousertype(tolua_S,1,0);
  int index = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'at'", NULL);
#endif
  {
   CUIButton& tolua_ret = (CUIButton&)  self->at(index);
    tolua_pushusertype(tolua_S,(void*)&tolua_ret,"CUIButton");
  }
 }
 return 1;
tolua_lerror:
 return tolua_stratagus_vector_CUIButton__at00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: front of class  vector<CUIButton> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_CUIButton__front00
static int tolua_stratagus_vector_CUIButton__front00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const vector<CUIButton>",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const vector<CUIButton>* self = (const vector<CUIButton>*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'front'", NULL);
#endif
  {
   const CUIButton& tolua_ret = (const CUIButton&)  self->front();
    tolua_pushusertype(tolua_S,(void*)&tolua_ret,"const CUIButton");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'front'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: front of class  vector<CUIButton> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_CUIButton__front01
static int tolua_stratagus_vector_CUIButton__front01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"vector<CUIButton>",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  vector<CUIButton>* self = (vector<CUIButton>*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'front'", NULL);
#endif
  {
   CUIButton& tolua_ret = (CUIButton&)  self->front();
    tolua_pushusertype(tolua_S,(void*)&tolua_ret,"CUIButton");
  }
 }
 return 1;
tolua_lerror:
 return tolua_stratagus_vector_CUIButton__front00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: back of class  vector<CUIButton> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_CUIButton__back00
static int tolua_stratagus_vector_CUIButton__back00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const vector<CUIButton>",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const vector<CUIButton>* self = (const vector<CUIButton>*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'back'", NULL);
#endif
  {
   const CUIButton& tolua_ret = (const CUIButton&)  self->back();
    tolua_pushusertype(tolua_S,(void*)&tolua_ret,"const CUIButton");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'back'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: back of class  vector<CUIButton> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_CUIButton__back01
static int tolua_stratagus_vector_CUIButton__back01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"vector<CUIButton>",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  vector<CUIButton>* self = (vector<CUIButton>*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'back'", NULL);
#endif
  {
   CUIButton& tolua_ret = (CUIButton&)  self->back();
    tolua_pushusertype(tolua_S,(void*)&tolua_ret,"CUIButton");
  }
 }
 return 1;
tolua_lerror:
 return tolua_stratagus_vector_CUIButton__back00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: push_back of class  vector<CUIButton> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_CUIButton__push_back00
static int tolua_stratagus_vector_CUIButton__push_back00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"vector<CUIButton>",0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"CUIButton",0,&tolua_err)) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  vector<CUIButton>* self = (vector<CUIButton>*)  tolua_tousertype(tolua_S,1,0);
  CUIButton val = *((CUIButton*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'push_back'", NULL);
#endif
  {
   self->push_back(val);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'push_back'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: pop_back of class  vector<CUIButton> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_CUIButton__pop_back00
static int tolua_stratagus_vector_CUIButton__pop_back00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"vector<CUIButton>",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  vector<CUIButton>* self = (vector<CUIButton>*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'pop_back'", NULL);
#endif
  {
   self->pop_back();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'pop_back'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: assign of class  vector<CUIButton> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_CUIButton__assign00
static int tolua_stratagus_vector_CUIButton__assign00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"vector<CUIButton>",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,3,&tolua_err) || !tolua_isusertype(tolua_S,3,"const CUIButton",0,&tolua_err)) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  vector<CUIButton>* self = (vector<CUIButton>*)  tolua_tousertype(tolua_S,1,0);
  int num = ((int)  tolua_tonumber(tolua_S,2,0));
  const CUIButton* val = ((const CUIButton*)  tolua_tousertype(tolua_S,3,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'assign'", NULL);
#endif
  {
   self->assign(num,*val);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'assign'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: clear of class  vector<CUIButton> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_CUIButton__clear00
static int tolua_stratagus_vector_CUIButton__clear00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"vector<CUIButton>",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  vector<CUIButton>* self = (vector<CUIButton>*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'clear'", NULL);
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

/* method: empty of class  vector<CUIButton> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_CUIButton__empty00
static int tolua_stratagus_vector_CUIButton__empty00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const vector<CUIButton>",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const vector<CUIButton>* self = (const vector<CUIButton>*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'empty'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->empty();
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'empty'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: size of class  vector<CUIButton> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_CUIButton__size00
static int tolua_stratagus_vector_CUIButton__size00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const vector<CUIButton>",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const vector<CUIButton>* self = (const vector<CUIButton>*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'size'", NULL);
#endif
  {
   int tolua_ret = (int)  self->size();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'size'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  vector<string> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_string__new00
static int tolua_stratagus_vector_string__new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"vector<string>",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   vector<string>* tolua_ret = (vector<string>*)  Mtolua_new((vector<string>)());
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"vector<string>");
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

/* method: new_local of class  vector<string> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_string__new00_local
static int tolua_stratagus_vector_string__new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"vector<string>",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   vector<string>* tolua_ret = (vector<string>*)  Mtolua_new((vector<string>)());
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"vector<string>");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
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

/* method: delete of class  vector<string> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_string__delete00
static int tolua_stratagus_vector_string__delete00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"vector<string>",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  vector<string>* self = (vector<string>*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'delete'", NULL);
#endif
  Mtolua_delete(self);
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'delete'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: operator[] of class  vector<string> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_string___geti00
static int tolua_stratagus_vector_string___geti00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const vector<string>",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const vector<string>* self = (const vector<string>*)  tolua_tousertype(tolua_S,1,0);
  int index = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'operator[]'", NULL);
#endif
  {
   const string tolua_ret = (const string)  self->operator[](index);
   tolua_pushcppstring(tolua_S,(const char*)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function '.geti'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: operator&[] of class  vector<string> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_string___seti00
static int tolua_stratagus_vector_string___seti00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"vector<string>",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_iscppstring(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  vector<string>* self = (vector<string>*)  tolua_tousertype(tolua_S,1,0);
  int index = ((int)  tolua_tonumber(tolua_S,2,0));
  string tolua_value = ((string)  tolua_tocppstring(tolua_S,3,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'operator&[]'", NULL);
#endif
  self->operator[](index) =  tolua_value;
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function '.seti'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: operator[] of class  vector<string> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_string___geti01
static int tolua_stratagus_vector_string___geti01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"vector<string>",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  vector<string>* self = (vector<string>*)  tolua_tousertype(tolua_S,1,0);
  int index = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'operator[]'", NULL);
#endif
  {
   string tolua_ret = (string)  self->operator[](index);
   tolua_pushcppstring(tolua_S,(const char*)tolua_ret);
  }
 }
 return 1;
tolua_lerror:
 return tolua_stratagus_vector_string___geti00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: at of class  vector<string> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_string__at00
static int tolua_stratagus_vector_string__at00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const vector<string>",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const vector<string>* self = (const vector<string>*)  tolua_tousertype(tolua_S,1,0);
  int index = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'at'", NULL);
#endif
  {
   const string tolua_ret = (const string)  self->at(index);
   tolua_pushcppstring(tolua_S,(const char*)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'at'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: at of class  vector<string> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_string__at01
static int tolua_stratagus_vector_string__at01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"vector<string>",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  vector<string>* self = (vector<string>*)  tolua_tousertype(tolua_S,1,0);
  int index = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'at'", NULL);
#endif
  {
   string tolua_ret = (string)  self->at(index);
   tolua_pushcppstring(tolua_S,(const char*)tolua_ret);
  }
 }
 return 1;
tolua_lerror:
 return tolua_stratagus_vector_string__at00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: front of class  vector<string> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_string__front00
static int tolua_stratagus_vector_string__front00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const vector<string>",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const vector<string>* self = (const vector<string>*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'front'", NULL);
#endif
  {
   const string tolua_ret = (const string)  self->front();
   tolua_pushcppstring(tolua_S,(const char*)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'front'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: front of class  vector<string> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_string__front01
static int tolua_stratagus_vector_string__front01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"vector<string>",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  vector<string>* self = (vector<string>*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'front'", NULL);
#endif
  {
   string tolua_ret = (string)  self->front();
   tolua_pushcppstring(tolua_S,(const char*)tolua_ret);
  }
 }
 return 1;
tolua_lerror:
 return tolua_stratagus_vector_string__front00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: back of class  vector<string> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_string__back00
static int tolua_stratagus_vector_string__back00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const vector<string>",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const vector<string>* self = (const vector<string>*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'back'", NULL);
#endif
  {
   const string tolua_ret = (const string)  self->back();
   tolua_pushcppstring(tolua_S,(const char*)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'back'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: back of class  vector<string> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_string__back01
static int tolua_stratagus_vector_string__back01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"vector<string>",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  vector<string>* self = (vector<string>*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'back'", NULL);
#endif
  {
   string tolua_ret = (string)  self->back();
   tolua_pushcppstring(tolua_S,(const char*)tolua_ret);
  }
 }
 return 1;
tolua_lerror:
 return tolua_stratagus_vector_string__back00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: push_back of class  vector<string> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_string__push_back00
static int tolua_stratagus_vector_string__push_back00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"vector<string>",0,&tolua_err) ||
     !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  vector<string>* self = (vector<string>*)  tolua_tousertype(tolua_S,1,0);
  string val = ((string)  tolua_tocppstring(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'push_back'", NULL);
#endif
  {
   self->push_back(val);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'push_back'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: pop_back of class  vector<string> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_string__pop_back00
static int tolua_stratagus_vector_string__pop_back00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"vector<string>",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  vector<string>* self = (vector<string>*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'pop_back'", NULL);
#endif
  {
   self->pop_back();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'pop_back'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: assign of class  vector<string> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_string__assign00
static int tolua_stratagus_vector_string__assign00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"vector<string>",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_iscppstring(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  vector<string>* self = (vector<string>*)  tolua_tousertype(tolua_S,1,0);
  int num = ((int)  tolua_tonumber(tolua_S,2,0));
  const string val = ((const string)  tolua_tocppstring(tolua_S,3,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'assign'", NULL);
#endif
  {
   self->assign(num,val);
   tolua_pushcppstring(tolua_S,(const char*)val);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'assign'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: clear of class  vector<string> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_string__clear00
static int tolua_stratagus_vector_string__clear00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"vector<string>",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  vector<string>* self = (vector<string>*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'clear'", NULL);
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

/* method: empty of class  vector<string> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_string__empty00
static int tolua_stratagus_vector_string__empty00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const vector<string>",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const vector<string>* self = (const vector<string>*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'empty'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->empty();
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'empty'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: size of class  vector<string> */
#ifndef TOLUA_DISABLE_tolua_stratagus_vector_string__size00
static int tolua_stratagus_vector_string__size00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const vector<string>",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const vector<string>* self = (const vector<string>*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'size'", NULL);
#endif
  {
   int tolua_ret = (int)  self->size();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'size'.",&tolua_err);
 return 0;
#endif
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

/* get function: Buttons of class  CButtonPanel */
#ifndef TOLUA_DISABLE_tolua_get_CButtonPanel_Buttons
static int tolua_get_CButtonPanel_Buttons(lua_State* tolua_S)
{
  CButtonPanel* self = (CButtonPanel*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Buttons'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)&self->Buttons,"vector<CUIButton>");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Buttons of class  CButtonPanel */
#ifndef TOLUA_DISABLE_tolua_set_CButtonPanel_Buttons
static int tolua_set_CButtonPanel_Buttons(lua_State* tolua_S)
{
  CButtonPanel* self = (CButtonPanel*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Buttons'",NULL);
  if ((tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"vector<CUIButton>",0,&tolua_err)))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Buttons = *((vector<CUIButton>*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: AutoCastBorderColorRGB of class  CButtonPanel */
#ifndef TOLUA_DISABLE_tolua_get_CButtonPanel_AutoCastBorderColorRGB
static int tolua_get_CButtonPanel_AutoCastBorderColorRGB(lua_State* tolua_S)
{
  CButtonPanel* self = (CButtonPanel*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'AutoCastBorderColorRGB'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)&self->AutoCastBorderColorRGB,"CColor");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: AutoCastBorderColorRGB of class  CButtonPanel */
#ifndef TOLUA_DISABLE_tolua_set_CButtonPanel_AutoCastBorderColorRGB
static int tolua_set_CButtonPanel_AutoCastBorderColorRGB(lua_State* tolua_S)
{
  CButtonPanel* self = (CButtonPanel*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'AutoCastBorderColorRGB'",NULL);
  if ((tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"CColor",0,&tolua_err)))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->AutoCastBorderColorRGB = *((CColor*)  tolua_tousertype(tolua_S,2,0))
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

/* get function: G of class  CPieMenu */
#ifndef TOLUA_DISABLE_tolua_get_CPieMenu_G_ptr
static int tolua_get_CPieMenu_G_ptr(lua_State* tolua_S)
{
  CPieMenu* self = (CPieMenu*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'G'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)self->G,"CGraphic");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: G of class  CPieMenu */
#ifndef TOLUA_DISABLE_tolua_set_CPieMenu_G_ptr
static int tolua_set_CPieMenu_G_ptr(lua_State* tolua_S)
{
  CPieMenu* self = (CPieMenu*)  tolua_tousertype(tolua_S,1,0);
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

/* get function: MouseButton of class  CPieMenu */
#ifndef TOLUA_DISABLE_tolua_get_CPieMenu_MouseButton
static int tolua_get_CPieMenu_MouseButton(lua_State* tolua_S)
{
  CPieMenu* self = (CPieMenu*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MouseButton'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->MouseButton);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: MouseButton of class  CPieMenu */
#ifndef TOLUA_DISABLE_tolua_set_CPieMenu_MouseButton
static int tolua_set_CPieMenu_MouseButton(lua_State* tolua_S)
{
  CPieMenu* self = (CPieMenu*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MouseButton'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->MouseButton = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: X of class  CPieMenu */
#ifndef TOLUA_DISABLE_tolua_get_stratagus_CPieMenu_X
static int tolua_get_stratagus_CPieMenu_X(lua_State* tolua_S)
{
 int tolua_index;
  CPieMenu* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (CPieMenu*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
  tolua_Error tolua_err;
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0);
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=8)
  tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->X[tolua_index]);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: X of class  CPieMenu */
#ifndef TOLUA_DISABLE_tolua_set_stratagus_CPieMenu_X
static int tolua_set_stratagus_CPieMenu_X(lua_State* tolua_S)
{
 int tolua_index;
  CPieMenu* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (CPieMenu*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
  tolua_Error tolua_err;
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0);
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=8)
  tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
  self->X[tolua_index] = ((int)  tolua_tonumber(tolua_S,3,0));
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Y of class  CPieMenu */
#ifndef TOLUA_DISABLE_tolua_get_stratagus_CPieMenu_Y
static int tolua_get_stratagus_CPieMenu_Y(lua_State* tolua_S)
{
 int tolua_index;
  CPieMenu* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (CPieMenu*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
  tolua_Error tolua_err;
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0);
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=8)
  tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->Y[tolua_index]);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Y of class  CPieMenu */
#ifndef TOLUA_DISABLE_tolua_set_stratagus_CPieMenu_Y
static int tolua_set_stratagus_CPieMenu_Y(lua_State* tolua_S)
{
 int tolua_index;
  CPieMenu* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (CPieMenu*)  lua_touserdata(tolua_S,-1);
#ifndef TOLUA_RELEASE
 {
  tolua_Error tolua_err;
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in array indexing.",&tolua_err);
 }
#endif
 tolua_index = (int)tolua_tonumber(tolua_S,2,0);
#ifndef TOLUA_RELEASE
 if (tolua_index<0 || tolua_index>=8)
  tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
  self->Y[tolua_index] = ((int)  tolua_tonumber(tolua_S,3,0));
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* method: SetRadius of class  CPieMenu */
#ifndef TOLUA_DISABLE_tolua_stratagus_CPieMenu_SetRadius00
static int tolua_stratagus_CPieMenu_SetRadius00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"CPieMenu",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CPieMenu* self = (CPieMenu*)  tolua_tousertype(tolua_S,1,0);
  int radius = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SetRadius'", NULL);
#endif
  {
   self->SetRadius(radius);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetRadius'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* get function: G of class  CResourceInfo */
#ifndef TOLUA_DISABLE_tolua_get_CResourceInfo_G_ptr
static int tolua_get_CResourceInfo_G_ptr(lua_State* tolua_S)
{
  CResourceInfo* self = (CResourceInfo*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'G'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)self->G,"CGraphic");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: G of class  CResourceInfo */
#ifndef TOLUA_DISABLE_tolua_set_CResourceInfo_G_ptr
static int tolua_set_CResourceInfo_G_ptr(lua_State* tolua_S)
{
  CResourceInfo* self = (CResourceInfo*)  tolua_tousertype(tolua_S,1,0);
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

/* get function: IconFrame of class  CResourceInfo */
#ifndef TOLUA_DISABLE_tolua_get_CResourceInfo_IconFrame
static int tolua_get_CResourceInfo_IconFrame(lua_State* tolua_S)
{
  CResourceInfo* self = (CResourceInfo*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'IconFrame'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->IconFrame);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: IconFrame of class  CResourceInfo */
#ifndef TOLUA_DISABLE_tolua_set_CResourceInfo_IconFrame
static int tolua_set_CResourceInfo_IconFrame(lua_State* tolua_S)
{
  CResourceInfo* self = (CResourceInfo*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'IconFrame'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->IconFrame = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: IconX of class  CResourceInfo */
#ifndef TOLUA_DISABLE_tolua_get_CResourceInfo_IconX
static int tolua_get_CResourceInfo_IconX(lua_State* tolua_S)
{
  CResourceInfo* self = (CResourceInfo*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'IconX'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->IconX);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: IconX of class  CResourceInfo */
#ifndef TOLUA_DISABLE_tolua_set_CResourceInfo_IconX
static int tolua_set_CResourceInfo_IconX(lua_State* tolua_S)
{
  CResourceInfo* self = (CResourceInfo*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'IconX'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->IconX = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: IconY of class  CResourceInfo */
#ifndef TOLUA_DISABLE_tolua_get_CResourceInfo_IconY
static int tolua_get_CResourceInfo_IconY(lua_State* tolua_S)
{
  CResourceInfo* self = (CResourceInfo*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'IconY'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->IconY);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: IconY of class  CResourceInfo */
#ifndef TOLUA_DISABLE_tolua_set_CResourceInfo_IconY
static int tolua_set_CResourceInfo_IconY(lua_State* tolua_S)
{
  CResourceInfo* self = (CResourceInfo*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'IconY'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->IconY = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: TextX of class  CResourceInfo */
#ifndef TOLUA_DISABLE_tolua_get_CResourceInfo_TextX
static int tolua_get_CResourceInfo_TextX(lua_State* tolua_S)
{
  CResourceInfo* self = (CResourceInfo*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'TextX'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->TextX);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: TextX of class  CResourceInfo */
#ifndef TOLUA_DISABLE_tolua_set_CResourceInfo_TextX
static int tolua_set_CResourceInfo_TextX(lua_State* tolua_S)
{
  CResourceInfo* self = (CResourceInfo*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'TextX'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->TextX = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: TextY of class  CResourceInfo */
#ifndef TOLUA_DISABLE_tolua_get_CResourceInfo_TextY
static int tolua_get_CResourceInfo_TextY(lua_State* tolua_S)
{
  CResourceInfo* self = (CResourceInfo*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'TextY'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->TextY);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: TextY of class  CResourceInfo */
#ifndef TOLUA_DISABLE_tolua_set_CResourceInfo_TextY
static int tolua_set_CResourceInfo_TextY(lua_State* tolua_S)
{
  CResourceInfo* self = (CResourceInfo*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'TextY'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->TextY = ((int)  tolua_tonumber(tolua_S,2,0))
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

/* method: Set of class  CStatusLine */
#ifndef TOLUA_DISABLE_tolua_stratagus_CStatusLine_Set00
static int tolua_stratagus_CStatusLine_Set00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"CStatusLine",0,&tolua_err) ||
     !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CStatusLine* self = (CStatusLine*)  tolua_tousertype(tolua_S,1,0);
  const std::string status = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'Set'", NULL);
#endif
  {
   self->Set(status);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Set'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: Get of class  CStatusLine */
#ifndef TOLUA_DISABLE_tolua_stratagus_CStatusLine_Get00
static int tolua_stratagus_CStatusLine_Get00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"CStatusLine",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CStatusLine* self = (CStatusLine*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'Get'", NULL);
#endif
  {
   const std::string tolua_ret = (const std::string)  self->Get();
   tolua_pushcppstring(tolua_S,(const char*)tolua_ret);
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

/* method: Clear of class  CStatusLine */
#ifndef TOLUA_DISABLE_tolua_stratagus_CStatusLine_Clear00
static int tolua_stratagus_CStatusLine_Clear00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"CStatusLine",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CStatusLine* self = (CStatusLine*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'Clear'", NULL);
#endif
  {
   self->Clear();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Clear'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Width of class  CStatusLine */
#ifndef TOLUA_DISABLE_tolua_get_CStatusLine_Width
static int tolua_get_CStatusLine_Width(lua_State* tolua_S)
{
  CStatusLine* self = (CStatusLine*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Width'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->Width);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Width of class  CStatusLine */
#ifndef TOLUA_DISABLE_tolua_set_CStatusLine_Width
static int tolua_set_CStatusLine_Width(lua_State* tolua_S)
{
  CStatusLine* self = (CStatusLine*)  tolua_tousertype(tolua_S,1,0);
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

/* get function: TextX of class  CStatusLine */
#ifndef TOLUA_DISABLE_tolua_get_CStatusLine_TextX
static int tolua_get_CStatusLine_TextX(lua_State* tolua_S)
{
  CStatusLine* self = (CStatusLine*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'TextX'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->TextX);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: TextX of class  CStatusLine */
#ifndef TOLUA_DISABLE_tolua_set_CStatusLine_TextX
static int tolua_set_CStatusLine_TextX(lua_State* tolua_S)
{
  CStatusLine* self = (CStatusLine*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'TextX'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->TextX = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: TextY of class  CStatusLine */
#ifndef TOLUA_DISABLE_tolua_get_CStatusLine_TextY
static int tolua_get_CStatusLine_TextY(lua_State* tolua_S)
{
  CStatusLine* self = (CStatusLine*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'TextY'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->TextY);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: TextY of class  CStatusLine */
#ifndef TOLUA_DISABLE_tolua_set_CStatusLine_TextY
static int tolua_set_CStatusLine_TextY(lua_State* tolua_S)
{
  CStatusLine* self = (CStatusLine*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'TextY'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->TextY = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Font of class  CStatusLine */
#ifndef TOLUA_DISABLE_tolua_get_CStatusLine_Font_ptr
static int tolua_get_CStatusLine_Font_ptr(lua_State* tolua_S)
{
  CStatusLine* self = (CStatusLine*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Font'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)self->Font,"CFont");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Font of class  CStatusLine */
#ifndef TOLUA_DISABLE_tolua_set_CStatusLine_Font_ptr
static int tolua_set_CStatusLine_Font_ptr(lua_State* tolua_S)
{
  CStatusLine* self = (CStatusLine*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Font'",NULL);
  if (!tolua_isusertype(tolua_S,2,"CFont",0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Font = ((CFont*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: X of class  CUITimer */
#ifndef TOLUA_DISABLE_tolua_get_CUITimer_X
static int tolua_get_CUITimer_X(lua_State* tolua_S)
{
  CUITimer* self = (CUITimer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'X'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->X);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: X of class  CUITimer */
#ifndef TOLUA_DISABLE_tolua_set_CUITimer_X
static int tolua_set_CUITimer_X(lua_State* tolua_S)
{
  CUITimer* self = (CUITimer*)  tolua_tousertype(tolua_S,1,0);
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

/* get function: Y of class  CUITimer */
#ifndef TOLUA_DISABLE_tolua_get_CUITimer_Y
static int tolua_get_CUITimer_Y(lua_State* tolua_S)
{
  CUITimer* self = (CUITimer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Y'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->Y);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Y of class  CUITimer */
#ifndef TOLUA_DISABLE_tolua_set_CUITimer_Y
static int tolua_set_CUITimer_Y(lua_State* tolua_S)
{
  CUITimer* self = (CUITimer*)  tolua_tousertype(tolua_S,1,0);
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

/* get function: Font of class  CUITimer */
#ifndef TOLUA_DISABLE_tolua_get_CUITimer_Font_ptr
static int tolua_get_CUITimer_Font_ptr(lua_State* tolua_S)
{
  CUITimer* self = (CUITimer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Font'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)self->Font,"CFont");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Font of class  CUITimer */
#ifndef TOLUA_DISABLE_tolua_set_CUITimer_Font_ptr
static int tolua_set_CUITimer_Font_ptr(lua_State* tolua_S)
{
  CUITimer* self = (CUITimer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Font'",NULL);
  if (!tolua_isusertype(tolua_S,2,"CFont",0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Font = ((CFont*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: NormalFontColor of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_get_CUserInterface_NormalFontColor
static int tolua_get_CUserInterface_NormalFontColor(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'NormalFontColor'",NULL);
#endif
  tolua_pushcppstring(tolua_S,(const char*)self->NormalFontColor);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: NormalFontColor of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_set_CUserInterface_NormalFontColor
static int tolua_set_CUserInterface_NormalFontColor(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'NormalFontColor'",NULL);
  if (!tolua_iscppstring(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->NormalFontColor = ((string)  tolua_tocppstring(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: ReverseFontColor of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_get_CUserInterface_ReverseFontColor
static int tolua_get_CUserInterface_ReverseFontColor(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ReverseFontColor'",NULL);
#endif
  tolua_pushcppstring(tolua_S,(const char*)self->ReverseFontColor);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: ReverseFontColor of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_set_CUserInterface_ReverseFontColor
static int tolua_set_CUserInterface_ReverseFontColor(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ReverseFontColor'",NULL);
  if (!tolua_iscppstring(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->ReverseFontColor = ((string)  tolua_tocppstring(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Fillers of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_get_CUserInterface_Fillers
static int tolua_get_CUserInterface_Fillers(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Fillers'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)&self->Fillers,"vector<CFiller>");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Fillers of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_set_CUserInterface_Fillers
static int tolua_set_CUserInterface_Fillers(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Fillers'",NULL);
  if ((tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"vector<CFiller>",0,&tolua_err)))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Fillers = *((vector<CFiller>*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Resources of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_get_stratagus_CUserInterface_Resources
static int tolua_get_stratagus_CUserInterface_Resources(lua_State* tolua_S)
{
 int tolua_index;
  CUserInterface* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (CUserInterface*)  lua_touserdata(tolua_S,-1);
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
  tolua_pushusertype(tolua_S,(void*)&self->Resources[tolua_index],"CResourceInfo");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Resources of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_set_stratagus_CUserInterface_Resources
static int tolua_set_stratagus_CUserInterface_Resources(lua_State* tolua_S)
{
 int tolua_index;
  CUserInterface* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (CUserInterface*)  lua_touserdata(tolua_S,-1);
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
  self->Resources[tolua_index] = *((CResourceInfo*)  tolua_tousertype(tolua_S,3,0));
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
  if ((tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"CInfoPanel",0,&tolua_err)))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->InfoPanel = *((CInfoPanel*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: SingleSelectedButton of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_get_CUserInterface_SingleSelectedButton_ptr
static int tolua_get_CUserInterface_SingleSelectedButton_ptr(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'SingleSelectedButton'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)self->SingleSelectedButton,"CUIButton");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: SingleSelectedButton of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_set_CUserInterface_SingleSelectedButton_ptr
static int tolua_set_CUserInterface_SingleSelectedButton_ptr(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'SingleSelectedButton'",NULL);
  if (!tolua_isusertype(tolua_S,2,"CUIButton",0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->SingleSelectedButton = ((CUIButton*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: SelectedButtons of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_get_CUserInterface_SelectedButtons
static int tolua_get_CUserInterface_SelectedButtons(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'SelectedButtons'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)&self->SelectedButtons,"vector<CUIButton>");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: SelectedButtons of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_set_CUserInterface_SelectedButtons
static int tolua_set_CUserInterface_SelectedButtons(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'SelectedButtons'",NULL);
  if ((tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"vector<CUIButton>",0,&tolua_err)))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->SelectedButtons = *((vector<CUIButton>*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: MaxSelectedFont of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_get_CUserInterface_MaxSelectedFont_ptr
static int tolua_get_CUserInterface_MaxSelectedFont_ptr(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MaxSelectedFont'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)self->MaxSelectedFont,"CFont");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: MaxSelectedFont of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_set_CUserInterface_MaxSelectedFont_ptr
static int tolua_set_CUserInterface_MaxSelectedFont_ptr(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MaxSelectedFont'",NULL);
  if (!tolua_isusertype(tolua_S,2,"CFont",0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->MaxSelectedFont = ((CFont*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: MaxSelectedTextX of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_get_CUserInterface_MaxSelectedTextX
static int tolua_get_CUserInterface_MaxSelectedTextX(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MaxSelectedTextX'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->MaxSelectedTextX);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: MaxSelectedTextX of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_set_CUserInterface_MaxSelectedTextX
static int tolua_set_CUserInterface_MaxSelectedTextX(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MaxSelectedTextX'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->MaxSelectedTextX = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: MaxSelectedTextY of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_get_CUserInterface_MaxSelectedTextY
static int tolua_get_CUserInterface_MaxSelectedTextY(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MaxSelectedTextY'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->MaxSelectedTextY);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: MaxSelectedTextY of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_set_CUserInterface_MaxSelectedTextY
static int tolua_set_CUserInterface_MaxSelectedTextY(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MaxSelectedTextY'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->MaxSelectedTextY = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: SingleTrainingButton of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_get_CUserInterface_SingleTrainingButton_ptr
static int tolua_get_CUserInterface_SingleTrainingButton_ptr(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'SingleTrainingButton'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)self->SingleTrainingButton,"CUIButton");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: SingleTrainingButton of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_set_CUserInterface_SingleTrainingButton_ptr
static int tolua_set_CUserInterface_SingleTrainingButton_ptr(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'SingleTrainingButton'",NULL);
  if (!tolua_isusertype(tolua_S,2,"CUIButton",0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->SingleTrainingButton = ((CUIButton*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: TrainingButtons of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_get_CUserInterface_TrainingButtons
static int tolua_get_CUserInterface_TrainingButtons(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'TrainingButtons'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)&self->TrainingButtons,"vector<CUIButton>");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: TrainingButtons of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_set_CUserInterface_TrainingButtons
static int tolua_set_CUserInterface_TrainingButtons(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'TrainingButtons'",NULL);
  if ((tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"vector<CUIButton>",0,&tolua_err)))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->TrainingButtons = *((vector<CUIButton>*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: TransportingButtons of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_get_CUserInterface_TransportingButtons
static int tolua_get_CUserInterface_TransportingButtons(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'TransportingButtons'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)&self->TransportingButtons,"vector<CUIButton>");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: TransportingButtons of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_set_CUserInterface_TransportingButtons
static int tolua_set_CUserInterface_TransportingButtons(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'TransportingButtons'",NULL);
  if ((tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"vector<CUIButton>",0,&tolua_err)))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->TransportingButtons = *((vector<CUIButton>*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: CompletedBarColorRGB of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_get_CUserInterface_CompletedBarColorRGB
static int tolua_get_CUserInterface_CompletedBarColorRGB(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'CompletedBarColorRGB'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)&self->CompletedBarColorRGB,"CColor");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: CompletedBarColorRGB of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_set_CUserInterface_CompletedBarColorRGB
static int tolua_set_CUserInterface_CompletedBarColorRGB(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'CompletedBarColorRGB'",NULL);
  if ((tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"CColor",0,&tolua_err)))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->CompletedBarColorRGB = *((CColor*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: CompletedBarShadow of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_get_CUserInterface_CompletedBarShadow
static int tolua_get_CUserInterface_CompletedBarShadow(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'CompletedBarShadow'",NULL);
#endif
  tolua_pushboolean(tolua_S,(bool)self->CompletedBarShadow);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: CompletedBarShadow of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_set_CUserInterface_CompletedBarShadow
static int tolua_set_CUserInterface_CompletedBarShadow(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'CompletedBarShadow'",NULL);
  if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->CompletedBarShadow = ((bool)  tolua_toboolean(tolua_S,2,0))
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
  if ((tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"CButtonPanel",0,&tolua_err)))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->ButtonPanel = *((CButtonPanel*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: PieMenu of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_get_CUserInterface_PieMenu
static int tolua_get_CUserInterface_PieMenu(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'PieMenu'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)&self->PieMenu,"CPieMenu");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: PieMenu of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_set_CUserInterface_PieMenu
static int tolua_set_CUserInterface_PieMenu(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'PieMenu'",NULL);
  if ((tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"CPieMenu",0,&tolua_err)))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->PieMenu = *((CPieMenu*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: MouseViewport of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_get_CUserInterface_MouseViewport_ptr
static int tolua_get_CUserInterface_MouseViewport_ptr(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MouseViewport'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)self->MouseViewport,"CViewport");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: MouseViewport of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_set_CUserInterface_MouseViewport_ptr
static int tolua_set_CUserInterface_MouseViewport_ptr(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MouseViewport'",NULL);
  if (!tolua_isusertype(tolua_S,2,"CViewport",0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->MouseViewport = ((CViewport*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: MapArea of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_get_CUserInterface_MapArea
static int tolua_get_CUserInterface_MapArea(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MapArea'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)&self->MapArea,"CMapArea");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: MapArea of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_set_CUserInterface_MapArea
static int tolua_set_CUserInterface_MapArea(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MapArea'",NULL);
  if ((tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"CMapArea",0,&tolua_err)))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->MapArea = *((CMapArea*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: MessageFont of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_get_CUserInterface_MessageFont_ptr
static int tolua_get_CUserInterface_MessageFont_ptr(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MessageFont'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)self->MessageFont,"CFont");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: MessageFont of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_set_CUserInterface_MessageFont_ptr
static int tolua_set_CUserInterface_MessageFont_ptr(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MessageFont'",NULL);
  if (!tolua_isusertype(tolua_S,2,"CFont",0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->MessageFont = ((CFont*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: MessageScrollSpeed of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_get_CUserInterface_MessageScrollSpeed
static int tolua_get_CUserInterface_MessageScrollSpeed(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MessageScrollSpeed'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->MessageScrollSpeed);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: MessageScrollSpeed of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_set_CUserInterface_MessageScrollSpeed
static int tolua_set_CUserInterface_MessageScrollSpeed(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MessageScrollSpeed'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->MessageScrollSpeed = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: MenuButton of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_get_CUserInterface_MenuButton
static int tolua_get_CUserInterface_MenuButton(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MenuButton'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)&self->MenuButton,"CUIButton");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: MenuButton of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_set_CUserInterface_MenuButton
static int tolua_set_CUserInterface_MenuButton(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MenuButton'",NULL);
  if ((tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"CUIButton",0,&tolua_err)))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->MenuButton = *((CUIButton*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: NetworkMenuButton of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_get_CUserInterface_NetworkMenuButton
static int tolua_get_CUserInterface_NetworkMenuButton(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'NetworkMenuButton'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)&self->NetworkMenuButton,"CUIButton");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: NetworkMenuButton of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_set_CUserInterface_NetworkMenuButton
static int tolua_set_CUserInterface_NetworkMenuButton(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'NetworkMenuButton'",NULL);
  if ((tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"CUIButton",0,&tolua_err)))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->NetworkMenuButton = *((CUIButton*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: NetworkDiplomacyButton of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_get_CUserInterface_NetworkDiplomacyButton
static int tolua_get_CUserInterface_NetworkDiplomacyButton(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'NetworkDiplomacyButton'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)&self->NetworkDiplomacyButton,"CUIButton");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: NetworkDiplomacyButton of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_set_CUserInterface_NetworkDiplomacyButton
static int tolua_set_CUserInterface_NetworkDiplomacyButton(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'NetworkDiplomacyButton'",NULL);
  if ((tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"CUIButton",0,&tolua_err)))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->NetworkDiplomacyButton = *((CUIButton*)  tolua_tousertype(tolua_S,2,0))
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
  if ((tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"CMinimap",0,&tolua_err)))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Minimap = *((CMinimap*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: StatusLine of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_get_CUserInterface_StatusLine
static int tolua_get_CUserInterface_StatusLine(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'StatusLine'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)&self->StatusLine,"CStatusLine");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: StatusLine of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_set_CUserInterface_StatusLine
static int tolua_set_CUserInterface_StatusLine(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'StatusLine'",NULL);
  if ((tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"CStatusLine",0,&tolua_err)))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->StatusLine = *((CStatusLine*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Timer of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_get_CUserInterface_Timer
static int tolua_get_CUserInterface_Timer(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Timer'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)&self->Timer,"CUITimer");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Timer of class  CUserInterface */
#ifndef TOLUA_DISABLE_tolua_set_CUserInterface_Timer
static int tolua_set_CUserInterface_Timer(lua_State* tolua_S)
{
  CUserInterface* self = (CUserInterface*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Timer'",NULL);
  if ((tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"CUITimer",0,&tolua_err)))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Timer = *((CUITimer*)  tolua_tousertype(tolua_S,2,0))
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
  if ((tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"CUserInterface",0,&tolua_err)))
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
     !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const std::string ident = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
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
     !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const std::string ident = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
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
  tolua_pushcppstring(tolua_S,(const char*)self->GetIdent());
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

/* function: FindButtonStyle */
#ifndef TOLUA_DISABLE_tolua_stratagus_FindButtonStyle00
static int tolua_stratagus_FindButtonStyle00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_iscppstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const std::string style = ((const std::string)  tolua_tocppstring(tolua_S,1,0));
  {
   ButtonStyle* tolua_ret = (ButtonStyle*)  FindButtonStyle(style);
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"ButtonStyle");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'FindButtonStyle'.",&tolua_err);
 return 0;
#endif
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

/* function: SetSavedMapPosition */
#ifndef TOLUA_DISABLE_tolua_stratagus_SetSavedMapPosition00
static int tolua_stratagus_SetSavedMapPosition00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  int index = ((int)  tolua_tonumber(tolua_S,1,0));
  int x = ((int)  tolua_tonumber(tolua_S,2,0));
  int y = ((int)  tolua_tonumber(tolua_S,3,0));
  {
   SetSavedMapPosition(index,x,y);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetSavedMapPosition'.",&tolua_err);
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

/* get function: CursorX */
#ifndef TOLUA_DISABLE_tolua_get_CursorX
static int tolua_get_CursorX(lua_State* tolua_S)
{
  tolua_pushnumber(tolua_S,(lua_Number)CursorX);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: CursorX */
#ifndef TOLUA_DISABLE_tolua_set_CursorX
static int tolua_set_CursorX(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  CursorX = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: CursorY */
#ifndef TOLUA_DISABLE_tolua_get_CursorY
static int tolua_get_CursorY(lua_State* tolua_S)
{
  tolua_pushnumber(tolua_S,(lua_Number)CursorY);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: CursorY */
#ifndef TOLUA_DISABLE_tolua_set_CursorY
static int tolua_set_CursorY(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  CursorY = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
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
   Color* tolua_ret = (Color*)  Mtolua_new((Color)(r,g,b,a));
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
   Color* tolua_ret = (Color*)  Mtolua_new((Color)(r,g,b,a));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"Color");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setWidth'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getWidth'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setHeight'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getHeight'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setSize'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setX'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getX'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setY'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getY'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setPosition'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setBorderSize'", NULL);
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

/* method: getBorderSize of class  Widget */
#ifndef TOLUA_DISABLE_tolua_stratagus_Widget_getBorderSize00
static int tolua_stratagus_Widget_getBorderSize00(lua_State* tolua_S)
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getBorderSize'", NULL);
#endif
  {
   unsigned int tolua_ret = (unsigned int)  self->getBorderSize();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getBorderSize'.",&tolua_err);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setEnabled'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'isEnabled'", NULL);
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

/* method: setVisible of class  Widget */
#ifndef TOLUA_DISABLE_tolua_stratagus_Widget_setVisible00
static int tolua_stratagus_Widget_setVisible00(lua_State* tolua_S)
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
  bool visible = ((bool)  tolua_toboolean(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setVisible'", NULL);
#endif
  {
   self->setVisible(visible);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setVisible'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: isVisible of class  Widget */
#ifndef TOLUA_DISABLE_tolua_stratagus_Widget_isVisible00
static int tolua_stratagus_Widget_isVisible00(lua_State* tolua_S)
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'isVisible'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->isVisible();
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'isVisible'.",&tolua_err);
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
     (tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"const Color",0,&tolua_err)) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Widget* self = (Widget*)  tolua_tousertype(tolua_S,1,0);
  const Color color = *((const Color*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setBaseColor'", NULL);
#endif
  {
   self->setBaseColor(color);
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

/* method: getBaseColor of class  Widget */
#ifndef TOLUA_DISABLE_tolua_stratagus_Widget_getBaseColor00
static int tolua_stratagus_Widget_getBaseColor00(lua_State* tolua_S)
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getBaseColor'", NULL);
#endif
  {
   const Color& tolua_ret = (const Color&)  self->getBaseColor();
    tolua_pushusertype(tolua_S,(void*)&tolua_ret,"const Color");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getBaseColor'.",&tolua_err);
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
     (tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"const Color",0,&tolua_err)) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Widget* self = (Widget*)  tolua_tousertype(tolua_S,1,0);
  const Color color = *((const Color*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setForegroundColor'", NULL);
#endif
  {
   self->setForegroundColor(color);
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

/* method: getForegroundColor of class  Widget */
#ifndef TOLUA_DISABLE_tolua_stratagus_Widget_getForegroundColor00
static int tolua_stratagus_Widget_getForegroundColor00(lua_State* tolua_S)
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getForegroundColor'", NULL);
#endif
  {
   const Color& tolua_ret = (const Color&)  self->getForegroundColor();
    tolua_pushusertype(tolua_S,(void*)&tolua_ret,"const Color");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getForegroundColor'.",&tolua_err);
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
     (tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"const Color",0,&tolua_err)) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Widget* self = (Widget*)  tolua_tousertype(tolua_S,1,0);
  const Color color = *((const Color*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setBackgroundColor'", NULL);
#endif
  {
   self->setBackgroundColor(color);
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

/* method: getBackgroundColor of class  Widget */
#ifndef TOLUA_DISABLE_tolua_stratagus_Widget_getBackgroundColor00
static int tolua_stratagus_Widget_getBackgroundColor00(lua_State* tolua_S)
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getBackgroundColor'", NULL);
#endif
  {
   const Color& tolua_ret = (const Color&)  self->getBackgroundColor();
    tolua_pushusertype(tolua_S,(void*)&tolua_ret,"const Color");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getBackgroundColor'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setDisabledColor of class  Widget */
#ifndef TOLUA_DISABLE_tolua_stratagus_Widget_setDisabledColor00
static int tolua_stratagus_Widget_setDisabledColor00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Widget",0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"const Color",0,&tolua_err)) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Widget* self = (Widget*)  tolua_tousertype(tolua_S,1,0);
  const Color color = *((const Color*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setDisabledColor'", NULL);
#endif
  {
   self->setDisabledColor(color);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setDisabledColor'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getDisabledColor of class  Widget */
#ifndef TOLUA_DISABLE_tolua_stratagus_Widget_getDisabledColor00
static int tolua_stratagus_Widget_getDisabledColor00(lua_State* tolua_S)
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getDisabledColor'", NULL);
#endif
  {
   const Color& tolua_ret = (const Color&)  self->getDisabledColor();
    tolua_pushusertype(tolua_S,(void*)&tolua_ret,"const Color");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getDisabledColor'.",&tolua_err);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setFont'", NULL);
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

/* method: getHotKey of class  Widget */
#ifndef TOLUA_DISABLE_tolua_stratagus_Widget_getHotKey00
static int tolua_stratagus_Widget_getHotKey00(lua_State* tolua_S)
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getHotKey'", NULL);
#endif
  {
   int tolua_ret = (int)  self->getHotKey();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getHotKey'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setHotKey of class  Widget */
#ifndef TOLUA_DISABLE_tolua_stratagus_Widget_setHotKey00
static int tolua_stratagus_Widget_setHotKey00(lua_State* tolua_S)
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
  const int key = ((const int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setHotKey'", NULL);
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

/* method: setHotKey of class  Widget */
#ifndef TOLUA_DISABLE_tolua_stratagus_Widget_setHotKey01
static int tolua_stratagus_Widget_setHotKey01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Widget",0,&tolua_err) ||
     !tolua_isstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  Widget* self = (Widget*)  tolua_tousertype(tolua_S,1,0);
  const char* key = ((const char*)  tolua_tostring(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setHotKey'", NULL);
#endif
  {
   self->setHotKey(key);
  }
 }
 return 0;
tolua_lerror:
 return tolua_stratagus_Widget_setHotKey00(tolua_S);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'addActionListener'", NULL);
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

/* method: requestFocus of class  Widget */
#ifndef TOLUA_DISABLE_tolua_stratagus_Widget_requestFocus00
static int tolua_stratagus_Widget_requestFocus00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Widget",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Widget* self = (Widget*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'requestFocus'", NULL);
#endif
  {
   self->requestFocus();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'requestFocus'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  ScrollArea */
#ifndef TOLUA_DISABLE_tolua_stratagus_ScrollArea_new00
static int tolua_stratagus_ScrollArea_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"ScrollArea",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   ScrollArea* tolua_ret = (ScrollArea*)  Mtolua_new((ScrollArea)());
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"ScrollArea");
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

/* method: new_local of class  ScrollArea */
#ifndef TOLUA_DISABLE_tolua_stratagus_ScrollArea_new00_local
static int tolua_stratagus_ScrollArea_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"ScrollArea",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   ScrollArea* tolua_ret = (ScrollArea*)  Mtolua_new((ScrollArea)());
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"ScrollArea");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
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

/* method: setContent of class  ScrollArea */
#ifndef TOLUA_DISABLE_tolua_stratagus_ScrollArea_setContent00
static int tolua_stratagus_ScrollArea_setContent00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"ScrollArea",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"Widget",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  ScrollArea* self = (ScrollArea*)  tolua_tousertype(tolua_S,1,0);
  Widget* widget = ((Widget*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setContent'", NULL);
#endif
  {
   self->setContent(widget);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setContent'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getContent of class  ScrollArea */
#ifndef TOLUA_DISABLE_tolua_stratagus_ScrollArea_getContent00
static int tolua_stratagus_ScrollArea_getContent00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"ScrollArea",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  ScrollArea* self = (ScrollArea*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getContent'", NULL);
#endif
  {
   Widget* tolua_ret = (Widget*)  self->getContent();
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"Widget");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getContent'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setScrollbarWidth of class  ScrollArea */
#ifndef TOLUA_DISABLE_tolua_stratagus_ScrollArea_setScrollbarWidth00
static int tolua_stratagus_ScrollArea_setScrollbarWidth00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"ScrollArea",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  ScrollArea* self = (ScrollArea*)  tolua_tousertype(tolua_S,1,0);
  int width = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setScrollbarWidth'", NULL);
#endif
  {
   self->setScrollbarWidth(width);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setScrollbarWidth'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getScrollbarWidth of class  ScrollArea */
#ifndef TOLUA_DISABLE_tolua_stratagus_ScrollArea_getScrollbarWidth00
static int tolua_stratagus_ScrollArea_getScrollbarWidth00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"ScrollArea",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  ScrollArea* self = (ScrollArea*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getScrollbarWidth'", NULL);
#endif
  {
   int tolua_ret = (int)  self->getScrollbarWidth();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getScrollbarWidth'.",&tolua_err);
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
   ImageWidget* tolua_ret = (ImageWidget*)  Mtolua_new((ImageWidget)(image));
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
   ImageWidget* tolua_ret = (ImageWidget*)  Mtolua_new((ImageWidget)(image));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"ImageWidget");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
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
   ButtonWidget* tolua_ret = (ButtonWidget*)  Mtolua_new((ButtonWidget)(caption));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"ButtonWidget");
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
   ButtonWidget* tolua_ret = (ButtonWidget*)  Mtolua_new((ButtonWidget)(caption));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"ButtonWidget");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setCaption'", NULL);
#endif
  {
   self->setCaption(caption);
  }
 }
 return 0;
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getCaption'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'adjustSize'", NULL);
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
   ImageButton* tolua_ret = (ImageButton*)  Mtolua_new((ImageButton)());
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
   ImageButton* tolua_ret = (ImageButton*)  Mtolua_new((ImageButton)());
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"ImageButton");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
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
   ImageButton* tolua_ret = (ImageButton*)  Mtolua_new((ImageButton)(caption));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"ImageButton");
  }
 }
 return 1;
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
   ImageButton* tolua_ret = (ImageButton*)  Mtolua_new((ImageButton)(caption));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"ImageButton");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
  }
 }
 return 1;
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setNormalImage'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setPressedImage'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setDisabledImage'", NULL);
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
   RadioButton* tolua_ret = (RadioButton*)  Mtolua_new((RadioButton)());
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
   RadioButton* tolua_ret = (RadioButton*)  Mtolua_new((RadioButton)());
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"RadioButton");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
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
   RadioButton* tolua_ret = (RadioButton*)  Mtolua_new((RadioButton)(caption,group,marked));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"RadioButton");
  }
 }
 return 1;
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
   RadioButton* tolua_ret = (RadioButton*)  Mtolua_new((RadioButton)(caption,group,marked));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"RadioButton");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
  }
 }
 return 1;
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'isMarked'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setMarked'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getCaption'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setCaption'", NULL);
#endif
  {
   self->setCaption(caption);
  }
 }
 return 0;
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setGroup'", NULL);
#endif
  {
   self->setGroup(group);
  }
 }
 return 0;
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getGroup'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'adjustSize'", NULL);
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
   ImageRadioButton* tolua_ret = (ImageRadioButton*)  Mtolua_new((ImageRadioButton)());
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
   ImageRadioButton* tolua_ret = (ImageRadioButton*)  Mtolua_new((ImageRadioButton)());
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"ImageRadioButton");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
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
   ImageRadioButton* tolua_ret = (ImageRadioButton*)  Mtolua_new((ImageRadioButton)(caption,group,marked));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"ImageRadioButton");
  }
 }
 return 1;
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
   ImageRadioButton* tolua_ret = (ImageRadioButton*)  Mtolua_new((ImageRadioButton)(caption,group,marked));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"ImageRadioButton");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
  }
 }
 return 1;
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setUncheckedNormalImage'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setUncheckedPressedImage'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setCheckedNormalImage'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setCheckedPressedImage'", NULL);
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
   CheckBox* tolua_ret = (CheckBox*)  Mtolua_new((CheckBox)());
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
   CheckBox* tolua_ret = (CheckBox*)  Mtolua_new((CheckBox)());
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"CheckBox");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
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
   CheckBox* tolua_ret = (CheckBox*)  Mtolua_new((CheckBox)(caption,marked));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"CheckBox");
  }
 }
 return 1;
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
   CheckBox* tolua_ret = (CheckBox*)  Mtolua_new((CheckBox)(caption,marked));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"CheckBox");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
  }
 }
 return 1;
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'isMarked'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setMarked'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getCaption'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setCaption'", NULL);
#endif
  {
   self->setCaption(caption);
  }
 }
 return 0;
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'adjustSize'", NULL);
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
   ImageCheckBox* tolua_ret = (ImageCheckBox*)  Mtolua_new((ImageCheckBox)());
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
   ImageCheckBox* tolua_ret = (ImageCheckBox*)  Mtolua_new((ImageCheckBox)());
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"ImageCheckBox");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
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
   ImageCheckBox* tolua_ret = (ImageCheckBox*)  Mtolua_new((ImageCheckBox)(caption,marked));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"ImageCheckBox");
  }
 }
 return 1;
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
   ImageCheckBox* tolua_ret = (ImageCheckBox*)  Mtolua_new((ImageCheckBox)(caption,marked));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"ImageCheckBox");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
  }
 }
 return 1;
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setUncheckedNormalImage'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setUncheckedPressedImage'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setCheckedNormalImage'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setCheckedPressedImage'", NULL);
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
   Slider* tolua_ret = (Slider*)  Mtolua_new((Slider)(scaleEnd));
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
   Slider* tolua_ret = (Slider*)  Mtolua_new((Slider)(scaleEnd));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"Slider");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
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
   Slider* tolua_ret = (Slider*)  Mtolua_new((Slider)(scaleStart,scaleEnd));
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
   Slider* tolua_ret = (Slider*)  Mtolua_new((Slider)(scaleStart,scaleEnd));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"Slider");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setScale'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getScaleStart'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setScaleStart'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getScaleEnd'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setScaleEnd'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getValue'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setValue'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setMarkerLength'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getMarkerLength'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setOrientation'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getOrientation'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setStepLength'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getStepLength'", NULL);
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
   ImageSlider* tolua_ret = (ImageSlider*)  Mtolua_new((ImageSlider)(scaleEnd));
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
   ImageSlider* tolua_ret = (ImageSlider*)  Mtolua_new((ImageSlider)(scaleEnd));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"ImageSlider");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
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
   ImageSlider* tolua_ret = (ImageSlider*)  Mtolua_new((ImageSlider)(scaleStart,scaleEnd));
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
   ImageSlider* tolua_ret = (ImageSlider*)  Mtolua_new((ImageSlider)(scaleStart,scaleEnd));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"ImageSlider");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setMarkerImage'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setBackgroundImage'", NULL);
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
   Label* tolua_ret = (Label*)  Mtolua_new((Label)(caption));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"Label");
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
   Label* tolua_ret = (Label*)  Mtolua_new((Label)(caption));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"Label");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getCaption'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setCaption'", NULL);
#endif
  {
   self->setCaption(caption);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setCaption'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setAlignment of class  Label */
#ifndef TOLUA_DISABLE_tolua_stratagus_Label_setAlignment00
static int tolua_stratagus_Label_setAlignment00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Label",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Label* self = (Label*)  tolua_tousertype(tolua_S,1,0);
  unsigned int alignment = ((unsigned int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setAlignment'", NULL);
#endif
  {
   self->setAlignment(alignment);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setAlignment'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getAlignment of class  Label */
#ifndef TOLUA_DISABLE_tolua_stratagus_Label_getAlignment00
static int tolua_stratagus_Label_getAlignment00(lua_State* tolua_S)
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getAlignment'", NULL);
#endif
  {
   unsigned int tolua_ret = (unsigned int)  self->getAlignment();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getAlignment'.",&tolua_err);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'adjustSize'", NULL);
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

/* method: new of class  MultiLineLabel */
#ifndef TOLUA_DISABLE_tolua_stratagus_MultiLineLabel_new00
static int tolua_stratagus_MultiLineLabel_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"MultiLineLabel",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   MultiLineLabel* tolua_ret = (MultiLineLabel*)  Mtolua_new((MultiLineLabel)());
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"MultiLineLabel");
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

/* method: new_local of class  MultiLineLabel */
#ifndef TOLUA_DISABLE_tolua_stratagus_MultiLineLabel_new00_local
static int tolua_stratagus_MultiLineLabel_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"MultiLineLabel",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   MultiLineLabel* tolua_ret = (MultiLineLabel*)  Mtolua_new((MultiLineLabel)());
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"MultiLineLabel");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
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

/* method: new of class  MultiLineLabel */
#ifndef TOLUA_DISABLE_tolua_stratagus_MultiLineLabel_new01
static int tolua_stratagus_MultiLineLabel_new01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"MultiLineLabel",0,&tolua_err) ||
     !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  const std::string caption = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
  {
   MultiLineLabel* tolua_ret = (MultiLineLabel*)  Mtolua_new((MultiLineLabel)(caption));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"MultiLineLabel");
  }
 }
 return 1;
tolua_lerror:
 return tolua_stratagus_MultiLineLabel_new00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  MultiLineLabel */
#ifndef TOLUA_DISABLE_tolua_stratagus_MultiLineLabel_new01_local
static int tolua_stratagus_MultiLineLabel_new01_local(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"MultiLineLabel",0,&tolua_err) ||
     !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  const std::string caption = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
  {
   MultiLineLabel* tolua_ret = (MultiLineLabel*)  Mtolua_new((MultiLineLabel)(caption));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"MultiLineLabel");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
  }
 }
 return 1;
tolua_lerror:
 return tolua_stratagus_MultiLineLabel_new00_local(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: setCaption of class  MultiLineLabel */
#ifndef TOLUA_DISABLE_tolua_stratagus_MultiLineLabel_setCaption00
static int tolua_stratagus_MultiLineLabel_setCaption00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"MultiLineLabel",0,&tolua_err) ||
     !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  MultiLineLabel* self = (MultiLineLabel*)  tolua_tousertype(tolua_S,1,0);
  const std::string caption = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setCaption'", NULL);
#endif
  {
   self->setCaption(caption);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setCaption'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getCaption of class  MultiLineLabel */
#ifndef TOLUA_DISABLE_tolua_stratagus_MultiLineLabel_getCaption00
static int tolua_stratagus_MultiLineLabel_getCaption00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const MultiLineLabel",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const MultiLineLabel* self = (const MultiLineLabel*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getCaption'", NULL);
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

/* method: setAlignment of class  MultiLineLabel */
#ifndef TOLUA_DISABLE_tolua_stratagus_MultiLineLabel_setAlignment00
static int tolua_stratagus_MultiLineLabel_setAlignment00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"MultiLineLabel",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  MultiLineLabel* self = (MultiLineLabel*)  tolua_tousertype(tolua_S,1,0);
  unsigned int alignment = ((unsigned int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setAlignment'", NULL);
#endif
  {
   self->setAlignment(alignment);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setAlignment'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getAlignment of class  MultiLineLabel */
#ifndef TOLUA_DISABLE_tolua_stratagus_MultiLineLabel_getAlignment00
static int tolua_stratagus_MultiLineLabel_getAlignment00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"MultiLineLabel",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  MultiLineLabel* self = (MultiLineLabel*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getAlignment'", NULL);
#endif
  {
   unsigned int tolua_ret = (unsigned int)  self->getAlignment();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getAlignment'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setVerticalAlignment of class  MultiLineLabel */
#ifndef TOLUA_DISABLE_tolua_stratagus_MultiLineLabel_setVerticalAlignment00
static int tolua_stratagus_MultiLineLabel_setVerticalAlignment00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"MultiLineLabel",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  MultiLineLabel* self = (MultiLineLabel*)  tolua_tousertype(tolua_S,1,0);
  unsigned int alignment = ((unsigned int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setVerticalAlignment'", NULL);
#endif
  {
   self->setVerticalAlignment(alignment);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setVerticalAlignment'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getVerticalAlignment of class  MultiLineLabel */
#ifndef TOLUA_DISABLE_tolua_stratagus_MultiLineLabel_getVerticalAlignment00
static int tolua_stratagus_MultiLineLabel_getVerticalAlignment00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"MultiLineLabel",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  MultiLineLabel* self = (MultiLineLabel*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getVerticalAlignment'", NULL);
#endif
  {
   unsigned int tolua_ret = (unsigned int)  self->getVerticalAlignment();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getVerticalAlignment'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setLineWidth of class  MultiLineLabel */
#ifndef TOLUA_DISABLE_tolua_stratagus_MultiLineLabel_setLineWidth00
static int tolua_stratagus_MultiLineLabel_setLineWidth00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"MultiLineLabel",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  MultiLineLabel* self = (MultiLineLabel*)  tolua_tousertype(tolua_S,1,0);
  int width = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setLineWidth'", NULL);
#endif
  {
   self->setLineWidth(width);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setLineWidth'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getLineWidth of class  MultiLineLabel */
#ifndef TOLUA_DISABLE_tolua_stratagus_MultiLineLabel_getLineWidth00
static int tolua_stratagus_MultiLineLabel_getLineWidth00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"MultiLineLabel",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  MultiLineLabel* self = (MultiLineLabel*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getLineWidth'", NULL);
#endif
  {
   int tolua_ret = (int)  self->getLineWidth();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getLineWidth'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: adjustSize of class  MultiLineLabel */
#ifndef TOLUA_DISABLE_tolua_stratagus_MultiLineLabel_adjustSize00
static int tolua_stratagus_MultiLineLabel_adjustSize00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"MultiLineLabel",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  MultiLineLabel* self = (MultiLineLabel*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'adjustSize'", NULL);
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

/* method: draw of class  MultiLineLabel */
#ifndef TOLUA_DISABLE_tolua_stratagus_MultiLineLabel_draw00
static int tolua_stratagus_MultiLineLabel_draw00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"MultiLineLabel",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"gcn::Graphics",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  MultiLineLabel* self = (MultiLineLabel*)  tolua_tousertype(tolua_S,1,0);
  gcn::Graphics* graphics = ((gcn::Graphics*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'draw'", NULL);
#endif
  {
   self->draw(graphics);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'draw'.",&tolua_err);
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
   TextField* tolua_ret = (TextField*)  Mtolua_new((TextField)(text));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"TextField");
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
   TextField* tolua_ret = (TextField*)  Mtolua_new((TextField)(text));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"TextField");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
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

/* method: setText of class  TextField */
#ifndef TOLUA_DISABLE_tolua_stratagus_TextField_setText00
static int tolua_stratagus_TextField_setText00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"TextField",0,&tolua_err) ||
     !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  TextField* self = (TextField*)  tolua_tousertype(tolua_S,1,0);
  const std::string text = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setText'", NULL);
#endif
  {
   self->setText(text);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setText'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getText of class  TextField */
#ifndef TOLUA_DISABLE_tolua_stratagus_TextField_getText00
static int tolua_stratagus_TextField_getText00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"TextField",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  TextField* self = (TextField*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getText'", NULL);
#endif
  {
   std::string tolua_ret = (std::string)  self->getText();
   tolua_pushcppstring(tolua_S,(const char*)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getText'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setMaxLengthBytes of class  TextField */
#ifndef TOLUA_DISABLE_tolua_stratagus_TextField_setMaxLengthBytes00
static int tolua_stratagus_TextField_setMaxLengthBytes00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"TextField",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  TextField* self = (TextField*)  tolua_tousertype(tolua_S,1,0);
  int maxLengthBytes = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setMaxLengthBytes'", NULL);
#endif
  {
   self->setMaxLengthBytes(maxLengthBytes);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setMaxLengthBytes'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getMaxLengthBytes of class  TextField */
#ifndef TOLUA_DISABLE_tolua_stratagus_TextField_getMaxLengthBytes00
static int tolua_stratagus_TextField_getMaxLengthBytes00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const TextField",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const TextField* self = (const TextField*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getMaxLengthBytes'", NULL);
#endif
  {
   int tolua_ret = (int)  self->getMaxLengthBytes();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getMaxLengthBytes'.",&tolua_err);
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
   ListBoxWidget* tolua_ret = (ListBoxWidget*)  Mtolua_new((ListBoxWidget)(width,height));
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
   ListBoxWidget* tolua_ret = (ListBoxWidget*)  Mtolua_new((ListBoxWidget)(width,height));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"ListBoxWidget");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setList'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setSelected'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getSelected'", NULL);
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

/* method: new of class  Window */
#ifndef TOLUA_DISABLE_tolua_stratagus_Window_new00
static int tolua_stratagus_Window_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"Window",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   Window* tolua_ret = (Window*)  Mtolua_new((Window)());
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"Window");
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

/* method: new_local of class  Window */
#ifndef TOLUA_DISABLE_tolua_stratagus_Window_new00_local
static int tolua_stratagus_Window_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"Window",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   Window* tolua_ret = (Window*)  Mtolua_new((Window)());
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"Window");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
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

/* method: new of class  Window */
#ifndef TOLUA_DISABLE_tolua_stratagus_Window_new01
static int tolua_stratagus_Window_new01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"Window",0,&tolua_err) ||
     !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  const std::string caption = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
  {
   Window* tolua_ret = (Window*)  Mtolua_new((Window)(caption));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"Window");
  }
 }
 return 1;
tolua_lerror:
 return tolua_stratagus_Window_new00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  Window */
#ifndef TOLUA_DISABLE_tolua_stratagus_Window_new01_local
static int tolua_stratagus_Window_new01_local(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"Window",0,&tolua_err) ||
     !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  const std::string caption = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
  {
   Window* tolua_ret = (Window*)  Mtolua_new((Window)(caption));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"Window");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
  }
 }
 return 1;
tolua_lerror:
 return tolua_stratagus_Window_new00_local(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  Window */
#ifndef TOLUA_DISABLE_tolua_stratagus_Window_new02
static int tolua_stratagus_Window_new02(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"Window",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"Widget",0,&tolua_err) ||
     !tolua_iscppstring(tolua_S,3,1,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  Widget* content = ((Widget*)  tolua_tousertype(tolua_S,2,0));
  const std::string caption = ((const std::string)  tolua_tocppstring(tolua_S,3,""));
  {
   Window* tolua_ret = (Window*)  Mtolua_new((Window)(content,caption));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"Window");
  }
 }
 return 1;
tolua_lerror:
 return tolua_stratagus_Window_new01(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: new_local of class  Window */
#ifndef TOLUA_DISABLE_tolua_stratagus_Window_new02_local
static int tolua_stratagus_Window_new02_local(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"Window",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"Widget",0,&tolua_err) ||
     !tolua_iscppstring(tolua_S,3,1,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  Widget* content = ((Widget*)  tolua_tousertype(tolua_S,2,0));
  const std::string caption = ((const std::string)  tolua_tocppstring(tolua_S,3,""));
  {
   Window* tolua_ret = (Window*)  Mtolua_new((Window)(content,caption));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"Window");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
  }
 }
 return 1;
tolua_lerror:
 return tolua_stratagus_Window_new01_local(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: setCaption of class  Window */
#ifndef TOLUA_DISABLE_tolua_stratagus_Window_setCaption00
static int tolua_stratagus_Window_setCaption00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Window",0,&tolua_err) ||
     !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Window* self = (Window*)  tolua_tousertype(tolua_S,1,0);
  const std::string caption = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setCaption'", NULL);
#endif
  {
   self->setCaption(caption);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setCaption'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getCaption of class  Window */
#ifndef TOLUA_DISABLE_tolua_stratagus_Window_getCaption00
static int tolua_stratagus_Window_getCaption00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const Window",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const Window* self = (const Window*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getCaption'", NULL);
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

/* method: setAlignment of class  Window */
#ifndef TOLUA_DISABLE_tolua_stratagus_Window_setAlignment00
static int tolua_stratagus_Window_setAlignment00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Window",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Window* self = (Window*)  tolua_tousertype(tolua_S,1,0);
  unsigned int alignment = ((unsigned int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setAlignment'", NULL);
#endif
  {
   self->setAlignment(alignment);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setAlignment'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getAlignment of class  Window */
#ifndef TOLUA_DISABLE_tolua_stratagus_Window_getAlignment00
static int tolua_stratagus_Window_getAlignment00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const Window",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const Window* self = (const Window*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getAlignment'", NULL);
#endif
  {
   unsigned int tolua_ret = (unsigned int)  self->getAlignment();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getAlignment'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setContent of class  Window */
#ifndef TOLUA_DISABLE_tolua_stratagus_Window_setContent00
static int tolua_stratagus_Window_setContent00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Window",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"Widget",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Window* self = (Window*)  tolua_tousertype(tolua_S,1,0);
  Widget* widget = ((Widget*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setContent'", NULL);
#endif
  {
   self->setContent(widget);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setContent'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getContent of class  Window */
#ifndef TOLUA_DISABLE_tolua_stratagus_Window_getContent00
static int tolua_stratagus_Window_getContent00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const Window",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const Window* self = (const Window*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getContent'", NULL);
#endif
  {
   Widget* tolua_ret = (Widget*)  self->getContent();
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"Widget");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getContent'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setPadding of class  Window */
#ifndef TOLUA_DISABLE_tolua_stratagus_Window_setPadding00
static int tolua_stratagus_Window_setPadding00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Window",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Window* self = (Window*)  tolua_tousertype(tolua_S,1,0);
  unsigned int padding = ((unsigned int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setPadding'", NULL);
#endif
  {
   self->setPadding(padding);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setPadding'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getPadding of class  Window */
#ifndef TOLUA_DISABLE_tolua_stratagus_Window_getPadding00
static int tolua_stratagus_Window_getPadding00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const Window",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const Window* self = (const Window*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getPadding'", NULL);
#endif
  {
   unsigned int tolua_ret = (unsigned int)  self->getPadding();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getPadding'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setTitleBarHeight of class  Window */
#ifndef TOLUA_DISABLE_tolua_stratagus_Window_setTitleBarHeight00
static int tolua_stratagus_Window_setTitleBarHeight00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Window",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Window* self = (Window*)  tolua_tousertype(tolua_S,1,0);
  unsigned int height = ((unsigned int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setTitleBarHeight'", NULL);
#endif
  {
   self->setTitleBarHeight(height);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setTitleBarHeight'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getTitleBarHeight of class  Window */
#ifndef TOLUA_DISABLE_tolua_stratagus_Window_getTitleBarHeight00
static int tolua_stratagus_Window_getTitleBarHeight00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Window",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Window* self = (Window*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getTitleBarHeight'", NULL);
#endif
  {
   unsigned int tolua_ret = (unsigned int)  self->getTitleBarHeight();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getTitleBarHeight'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setMovable of class  Window */
#ifndef TOLUA_DISABLE_tolua_stratagus_Window_setMovable00
static int tolua_stratagus_Window_setMovable00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Window",0,&tolua_err) ||
     !tolua_isboolean(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Window* self = (Window*)  tolua_tousertype(tolua_S,1,0);
  bool movable = ((bool)  tolua_toboolean(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setMovable'", NULL);
#endif
  {
   self->setMovable(movable);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setMovable'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: isMovable of class  Window */
#ifndef TOLUA_DISABLE_tolua_stratagus_Window_isMovable00
static int tolua_stratagus_Window_isMovable00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const Window",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const Window* self = (const Window*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'isMovable'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->isMovable();
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'isMovable'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: resizeToContent of class  Window */
#ifndef TOLUA_DISABLE_tolua_stratagus_Window_resizeToContent00
static int tolua_stratagus_Window_resizeToContent00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Window",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Window* self = (Window*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'resizeToContent'", NULL);
#endif
  {
   self->resizeToContent();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'resizeToContent'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setOpaque of class  Window */
#ifndef TOLUA_DISABLE_tolua_stratagus_Window_setOpaque00
static int tolua_stratagus_Window_setOpaque00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Window",0,&tolua_err) ||
     !tolua_isboolean(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Window* self = (Window*)  tolua_tousertype(tolua_S,1,0);
  bool opaque = ((bool)  tolua_toboolean(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setOpaque'", NULL);
#endif
  {
   self->setOpaque(opaque);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setOpaque'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: isOpaque of class  Window */
#ifndef TOLUA_DISABLE_tolua_stratagus_Window_isOpaque00
static int tolua_stratagus_Window_isOpaque00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Window",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Window* self = (Window*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'isOpaque'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->isOpaque();
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'isOpaque'.",&tolua_err);
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
   Windows* tolua_ret = (Windows*)  Mtolua_new((Windows)(text,width,height));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"Windows");
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
   Windows* tolua_ret = (Windows*)  Mtolua_new((Windows)(text,width,height));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"Windows");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'add'", NULL);
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
   ScrollingWidget* tolua_ret = (ScrollingWidget*)  Mtolua_new((ScrollingWidget)(width,height));
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
   ScrollingWidget* tolua_ret = (ScrollingWidget*)  Mtolua_new((ScrollingWidget)(width,height));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"ScrollingWidget");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'add'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'restart'", NULL);
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

/* method: setSpeed of class  ScrollingWidget */
#ifndef TOLUA_DISABLE_tolua_stratagus_ScrollingWidget_setSpeed00
static int tolua_stratagus_ScrollingWidget_setSpeed00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"ScrollingWidget",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  ScrollingWidget* self = (ScrollingWidget*)  tolua_tousertype(tolua_S,1,0);
  float speed = ((float)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setSpeed'", NULL);
#endif
  {
   self->setSpeed(speed);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setSpeed'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getSpeed of class  ScrollingWidget */
#ifndef TOLUA_DISABLE_tolua_stratagus_ScrollingWidget_getSpeed00
static int tolua_stratagus_ScrollingWidget_getSpeed00(lua_State* tolua_S)
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getSpeed'", NULL);
#endif
  {
   float tolua_ret = (float)  self->getSpeed();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getSpeed'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getSelected of class  DropDown */
#ifndef TOLUA_DISABLE_tolua_stratagus_DropDown_getSelected00
static int tolua_stratagus_DropDown_getSelected00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"DropDown",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  DropDown* self = (DropDown*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getSelected'", NULL);
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

/* method: setSelected of class  DropDown */
#ifndef TOLUA_DISABLE_tolua_stratagus_DropDown_setSelected00
static int tolua_stratagus_DropDown_setSelected00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"DropDown",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  DropDown* self = (DropDown*)  tolua_tousertype(tolua_S,1,0);
  int selected = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setSelected'", NULL);
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

/* method: setScrollArea of class  DropDown */
#ifndef TOLUA_DISABLE_tolua_stratagus_DropDown_setScrollArea00
static int tolua_stratagus_DropDown_setScrollArea00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"DropDown",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"ScrollArea",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  DropDown* self = (DropDown*)  tolua_tousertype(tolua_S,1,0);
  ScrollArea* scrollArea = ((ScrollArea*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setScrollArea'", NULL);
#endif
  {
   self->setScrollArea(scrollArea);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setScrollArea'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getScrollArea of class  DropDown */
#ifndef TOLUA_DISABLE_tolua_stratagus_DropDown_getScrollArea00
static int tolua_stratagus_DropDown_getScrollArea00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"DropDown",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  DropDown* self = (DropDown*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getScrollArea'", NULL);
#endif
  {
   ScrollArea* tolua_ret = (ScrollArea*)  self->getScrollArea();
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"ScrollArea");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getScrollArea'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setListBox of class  DropDown */
#ifndef TOLUA_DISABLE_tolua_stratagus_DropDown_setListBox00
static int tolua_stratagus_DropDown_setListBox00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"DropDown",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"ListBox",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  DropDown* self = (DropDown*)  tolua_tousertype(tolua_S,1,0);
  ListBox* listBox = ((ListBox*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setListBox'", NULL);
#endif
  {
   self->setListBox(listBox);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setListBox'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getListBox of class  DropDown */
#ifndef TOLUA_DISABLE_tolua_stratagus_DropDown_getListBox00
static int tolua_stratagus_DropDown_getListBox00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"DropDown",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  DropDown* self = (DropDown*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getListBox'", NULL);
#endif
  {
   ListBox* tolua_ret = (ListBox*)  self->getListBox();
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"ListBox");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getListBox'.",&tolua_err);
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
   DropDownWidget* tolua_ret = (DropDownWidget*)  Mtolua_new((DropDownWidget)());
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
   DropDownWidget* tolua_ret = (DropDownWidget*)  Mtolua_new((DropDownWidget)());
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"DropDownWidget");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setList'", NULL);
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

/* method: getListBox of class  DropDownWidget */
#ifndef TOLUA_DISABLE_tolua_stratagus_DropDownWidget_getListBox00
static int tolua_stratagus_DropDownWidget_getListBox00(lua_State* tolua_S)
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getListBox'", NULL);
#endif
  {
   ListBox* tolua_ret = (ListBox*)  self->getListBox();
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"ListBox");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getListBox'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setSize of class  DropDownWidget */
#ifndef TOLUA_DISABLE_tolua_stratagus_DropDownWidget_setSize00
static int tolua_stratagus_DropDownWidget_setSize00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"DropDownWidget",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  DropDownWidget* self = (DropDownWidget*)  tolua_tousertype(tolua_S,1,0);
  int width = ((int)  tolua_tonumber(tolua_S,2,0));
  int height = ((int)  tolua_tonumber(tolua_S,3,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setSize'", NULL);
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
   StatBoxWidget* tolua_ret = (StatBoxWidget*)  Mtolua_new((StatBoxWidget)(width,height));
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
   StatBoxWidget* tolua_ret = (StatBoxWidget*)  Mtolua_new((StatBoxWidget)(width,height));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"StatBoxWidget");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
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

/* method: setCaption of class  StatBoxWidget */
#ifndef TOLUA_DISABLE_tolua_stratagus_StatBoxWidget_setCaption00
static int tolua_stratagus_StatBoxWidget_setCaption00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"StatBoxWidget",0,&tolua_err) ||
     !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  StatBoxWidget* self = (StatBoxWidget*)  tolua_tousertype(tolua_S,1,0);
  const std::string s = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setCaption'", NULL);
#endif
  {
   self->setCaption(s);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setCaption'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getCaption of class  StatBoxWidget */
#ifndef TOLUA_DISABLE_tolua_stratagus_StatBoxWidget_getCaption00
static int tolua_stratagus_StatBoxWidget_getCaption00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const StatBoxWidget",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const StatBoxWidget* self = (const StatBoxWidget*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getCaption'", NULL);
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

/* method: setPercent of class  StatBoxWidget */
#ifndef TOLUA_DISABLE_tolua_stratagus_StatBoxWidget_setPercent00
static int tolua_stratagus_StatBoxWidget_setPercent00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"StatBoxWidget",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  StatBoxWidget* self = (StatBoxWidget*)  tolua_tousertype(tolua_S,1,0);
  const int percent = ((const int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setPercent'", NULL);
#endif
  {
   self->setPercent(percent);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setPercent'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getPercent of class  StatBoxWidget */
#ifndef TOLUA_DISABLE_tolua_stratagus_StatBoxWidget_getPercent00
static int tolua_stratagus_StatBoxWidget_getPercent00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const StatBoxWidget",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const StatBoxWidget* self = (const StatBoxWidget*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getPercent'", NULL);
#endif
  {
   int tolua_ret = (int)  self->getPercent();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getPercent'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  Container */
#ifndef TOLUA_DISABLE_tolua_stratagus_Container_new00
static int tolua_stratagus_Container_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"Container",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   Container* tolua_ret = (Container*)  Mtolua_new((Container)());
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"Container");
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

/* method: new_local of class  Container */
#ifndef TOLUA_DISABLE_tolua_stratagus_Container_new00_local
static int tolua_stratagus_Container_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"Container",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   Container* tolua_ret = (Container*)  Mtolua_new((Container)());
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"Container");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
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

/* method: setOpaque of class  Container */
#ifndef TOLUA_DISABLE_tolua_stratagus_Container_setOpaque00
static int tolua_stratagus_Container_setOpaque00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Container",0,&tolua_err) ||
     !tolua_isboolean(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Container* self = (Container*)  tolua_tousertype(tolua_S,1,0);
  bool opaque = ((bool)  tolua_toboolean(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setOpaque'", NULL);
#endif
  {
   self->setOpaque(opaque);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setOpaque'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: isOpaque of class  Container */
#ifndef TOLUA_DISABLE_tolua_stratagus_Container_isOpaque00
static int tolua_stratagus_Container_isOpaque00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const Container",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const Container* self = (const Container*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'isOpaque'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->isOpaque();
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'isOpaque'.",&tolua_err);
 return 0;
#endif
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'add'", NULL);
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

/* method: remove of class  Container */
#ifndef TOLUA_DISABLE_tolua_stratagus_Container_remove00
static int tolua_stratagus_Container_remove00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"Container",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"Widget",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  Container* self = (Container*)  tolua_tousertype(tolua_S,1,0);
  Widget* widget = ((Widget*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'remove'", NULL);
#endif
  {
   self->remove(widget);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'remove'.",&tolua_err);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'clear'", NULL);
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
#ifndef TOLUA_DISABLE_tolua_stratagus_CMenuScreen_new00
static int tolua_stratagus_CMenuScreen_new00(lua_State* tolua_S)
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
   MenuScreen* tolua_ret = (MenuScreen*)  Mtolua_new((MenuScreen)());
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
#ifndef TOLUA_DISABLE_tolua_stratagus_CMenuScreen_new00_local
static int tolua_stratagus_CMenuScreen_new00_local(lua_State* tolua_S)
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
   MenuScreen* tolua_ret = (MenuScreen*)  Mtolua_new((MenuScreen)());
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"MenuScreen");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
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
#ifndef TOLUA_DISABLE_tolua_stratagus_CMenuScreen_run00
static int tolua_stratagus_CMenuScreen_run00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"MenuScreen",0,&tolua_err) ||
     !tolua_isboolean(tolua_S,2,1,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  MenuScreen* self = (MenuScreen*)  tolua_tousertype(tolua_S,1,0);
  bool loop = ((bool)  tolua_toboolean(tolua_S,2,true));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'run'", NULL);
#endif
  {
   int tolua_ret = (int)  self->run(loop);
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
#ifndef TOLUA_DISABLE_tolua_stratagus_CMenuScreen_stop00
static int tolua_stratagus_CMenuScreen_stop00(lua_State* tolua_S)
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'stop'", NULL);
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

/* method: stopAll of class  MenuScreen */
#ifndef TOLUA_DISABLE_tolua_stratagus_CMenuScreen_stopAll00
static int tolua_stratagus_CMenuScreen_stopAll00(lua_State* tolua_S)
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'stopAll'", NULL);
#endif
  {
   self->stopAll(result);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'stopAll'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: addLogicCallback of class  MenuScreen */
#ifndef TOLUA_DISABLE_tolua_stratagus_CMenuScreen_addLogicCallback00
static int tolua_stratagus_CMenuScreen_addLogicCallback00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"MenuScreen",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"LuaActionListener",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  MenuScreen* self = (MenuScreen*)  tolua_tousertype(tolua_S,1,0);
  LuaActionListener* actionListener = ((LuaActionListener*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'addLogicCallback'", NULL);
#endif
  {
   self->addLogicCallback(actionListener);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'addLogicCallback'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setDrawMenusUnder of class  MenuScreen */
#ifndef TOLUA_DISABLE_tolua_stratagus_CMenuScreen_setDrawMenusUnder00
static int tolua_stratagus_CMenuScreen_setDrawMenusUnder00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"MenuScreen",0,&tolua_err) ||
     !tolua_isboolean(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  MenuScreen* self = (MenuScreen*)  tolua_tousertype(tolua_S,1,0);
  bool drawunder = ((bool)  tolua_toboolean(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setDrawMenusUnder'", NULL);
#endif
  {
   self->setDrawMenusUnder(drawunder);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setDrawMenusUnder'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getDrawMenusUnder of class  MenuScreen */
#ifndef TOLUA_DISABLE_tolua_stratagus_CMenuScreen_getDrawMenusUnder00
static int tolua_stratagus_CMenuScreen_getDrawMenusUnder00(lua_State* tolua_S)
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getDrawMenusUnder'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->getDrawMenusUnder();
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getDrawMenusUnder'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: InitNetwork1 */
#ifndef TOLUA_DISABLE_tolua_stratagus_InitNetwork100
static int tolua_stratagus_InitNetwork100(lua_State* tolua_S)
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
   InitNetwork1();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'InitNetwork1'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: ExitNetwork1 */
#ifndef TOLUA_DISABLE_tolua_stratagus_ExitNetwork100
static int tolua_stratagus_ExitNetwork100(lua_State* tolua_S)
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
   ExitNetwork1();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'ExitNetwork1'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: IsNetworkGame */
#ifndef TOLUA_DISABLE_tolua_stratagus_IsNetworkGame00
static int tolua_stratagus_IsNetworkGame00(lua_State* tolua_S)
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
   bool tolua_ret = (bool)  IsNetworkGame();
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'IsNetworkGame'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: NetworkSetupServerAddress */
#ifndef TOLUA_DISABLE_tolua_stratagus_NetworkSetupServerAddress00
static int tolua_stratagus_NetworkSetupServerAddress00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_iscppstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const std::string serveraddr = ((const std::string)  tolua_tocppstring(tolua_S,1,0));
  {
   int tolua_ret = (int)  NetworkSetupServerAddress(serveraddr);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'NetworkSetupServerAddress'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: NetworkInitClientConnect */
#ifndef TOLUA_DISABLE_tolua_stratagus_NetworkInitClientConnect00
static int tolua_stratagus_NetworkInitClientConnect00(lua_State* tolua_S)
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
   NetworkInitClientConnect();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'NetworkInitClientConnect'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: NetworkInitServerConnect */
#ifndef TOLUA_DISABLE_tolua_stratagus_NetworkInitServerConnect00
static int tolua_stratagus_NetworkInitServerConnect00(lua_State* tolua_S)
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
  int openslots = ((int)  tolua_tonumber(tolua_S,1,0));
  {
   NetworkInitServerConnect(openslots);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'NetworkInitServerConnect'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: NetworkServerStartGame */
#ifndef TOLUA_DISABLE_tolua_stratagus_NetworkServerStartGame00
static int tolua_stratagus_NetworkServerStartGame00(lua_State* tolua_S)
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
   NetworkServerStartGame();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'NetworkServerStartGame'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: NetworkProcessClientRequest */
#ifndef TOLUA_DISABLE_tolua_stratagus_NetworkProcessClientRequest00
static int tolua_stratagus_NetworkProcessClientRequest00(lua_State* tolua_S)
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
   NetworkProcessClientRequest();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'NetworkProcessClientRequest'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: GetNetworkState */
#ifndef TOLUA_DISABLE_tolua_stratagus_GetNetworkState00
static int tolua_stratagus_GetNetworkState00(lua_State* tolua_S)
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
   int tolua_ret = (int)  GetNetworkState();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetNetworkState'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: NetworkServerResyncClients */
#ifndef TOLUA_DISABLE_tolua_stratagus_NetworkServerResyncClients00
static int tolua_stratagus_NetworkServerResyncClients00(lua_State* tolua_S)
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
   NetworkServerResyncClients();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'NetworkServerResyncClients'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: NetworkDetachFromServer */
#ifndef TOLUA_DISABLE_tolua_stratagus_NetworkDetachFromServer00
static int tolua_stratagus_NetworkDetachFromServer00(lua_State* tolua_S)
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
   NetworkDetachFromServer();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'NetworkDetachFromServer'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* get function: ResourcesOption of class  CServerSetup */
#ifndef TOLUA_DISABLE_tolua_get_CServerSetup_unsigned_ResourcesOption
static int tolua_get_CServerSetup_unsigned_ResourcesOption(lua_State* tolua_S)
{
  CServerSetup* self = (CServerSetup*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ResourcesOption'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->ResourcesOption);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: ResourcesOption of class  CServerSetup */
#ifndef TOLUA_DISABLE_tolua_set_CServerSetup_unsigned_ResourcesOption
static int tolua_set_CServerSetup_unsigned_ResourcesOption(lua_State* tolua_S)
{
  CServerSetup* self = (CServerSetup*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ResourcesOption'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->ResourcesOption = ((unsigned char)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: UnitsOption of class  CServerSetup */
#ifndef TOLUA_DISABLE_tolua_get_CServerSetup_unsigned_UnitsOption
static int tolua_get_CServerSetup_unsigned_UnitsOption(lua_State* tolua_S)
{
  CServerSetup* self = (CServerSetup*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'UnitsOption'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->UnitsOption);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: UnitsOption of class  CServerSetup */
#ifndef TOLUA_DISABLE_tolua_set_CServerSetup_unsigned_UnitsOption
static int tolua_set_CServerSetup_unsigned_UnitsOption(lua_State* tolua_S)
{
  CServerSetup* self = (CServerSetup*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'UnitsOption'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->UnitsOption = ((unsigned char)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: FogOfWar of class  CServerSetup */
#ifndef TOLUA_DISABLE_tolua_get_CServerSetup_unsigned_FogOfWar
static int tolua_get_CServerSetup_unsigned_FogOfWar(lua_State* tolua_S)
{
  CServerSetup* self = (CServerSetup*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'FogOfWar'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->FogOfWar);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: FogOfWar of class  CServerSetup */
#ifndef TOLUA_DISABLE_tolua_set_CServerSetup_unsigned_FogOfWar
static int tolua_set_CServerSetup_unsigned_FogOfWar(lua_State* tolua_S)
{
  CServerSetup* self = (CServerSetup*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'FogOfWar'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->FogOfWar = ((unsigned char)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: RevealMap of class  CServerSetup */
#ifndef TOLUA_DISABLE_tolua_get_CServerSetup_unsigned_RevealMap
static int tolua_get_CServerSetup_unsigned_RevealMap(lua_State* tolua_S)
{
  CServerSetup* self = (CServerSetup*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'RevealMap'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->RevealMap);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: RevealMap of class  CServerSetup */
#ifndef TOLUA_DISABLE_tolua_set_CServerSetup_unsigned_RevealMap
static int tolua_set_CServerSetup_unsigned_RevealMap(lua_State* tolua_S)
{
  CServerSetup* self = (CServerSetup*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'RevealMap'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->RevealMap = ((unsigned char)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: GameTypeOption of class  CServerSetup */
#ifndef TOLUA_DISABLE_tolua_get_CServerSetup_unsigned_GameTypeOption
static int tolua_get_CServerSetup_unsigned_GameTypeOption(lua_State* tolua_S)
{
  CServerSetup* self = (CServerSetup*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'GameTypeOption'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->GameTypeOption);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: GameTypeOption of class  CServerSetup */
#ifndef TOLUA_DISABLE_tolua_set_CServerSetup_unsigned_GameTypeOption
static int tolua_set_CServerSetup_unsigned_GameTypeOption(lua_State* tolua_S)
{
  CServerSetup* self = (CServerSetup*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'GameTypeOption'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->GameTypeOption = ((unsigned char)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Difficulty of class  CServerSetup */
#ifndef TOLUA_DISABLE_tolua_get_CServerSetup_unsigned_Difficulty
static int tolua_get_CServerSetup_unsigned_Difficulty(lua_State* tolua_S)
{
  CServerSetup* self = (CServerSetup*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Difficulty'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->Difficulty);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Difficulty of class  CServerSetup */
#ifndef TOLUA_DISABLE_tolua_set_CServerSetup_unsigned_Difficulty
static int tolua_set_CServerSetup_unsigned_Difficulty(lua_State* tolua_S)
{
  CServerSetup* self = (CServerSetup*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Difficulty'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Difficulty = ((unsigned char)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: MapRichness of class  CServerSetup */
#ifndef TOLUA_DISABLE_tolua_get_CServerSetup_unsigned_MapRichness
static int tolua_get_CServerSetup_unsigned_MapRichness(lua_State* tolua_S)
{
  CServerSetup* self = (CServerSetup*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MapRichness'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->MapRichness);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: MapRichness of class  CServerSetup */
#ifndef TOLUA_DISABLE_tolua_set_CServerSetup_unsigned_MapRichness
static int tolua_set_CServerSetup_unsigned_MapRichness(lua_State* tolua_S)
{
  CServerSetup* self = (CServerSetup*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MapRichness'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->MapRichness = ((unsigned char)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: CompOpt of class  CServerSetup */
#ifndef TOLUA_DISABLE_tolua_get_stratagus_CServerSetup_CompOpt
static int tolua_get_stratagus_CServerSetup_CompOpt(lua_State* tolua_S)
{
 int tolua_index;
  CServerSetup* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (CServerSetup*)  lua_touserdata(tolua_S,-1);
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
 tolua_pushnumber(tolua_S,(lua_Number)self->CompOpt[tolua_index]);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: CompOpt of class  CServerSetup */
#ifndef TOLUA_DISABLE_tolua_set_stratagus_CServerSetup_CompOpt
static int tolua_set_stratagus_CServerSetup_CompOpt(lua_State* tolua_S)
{
 int tolua_index;
  CServerSetup* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (CServerSetup*)  lua_touserdata(tolua_S,-1);
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
  self->CompOpt[tolua_index] = ((unsigned)  tolua_tonumber(tolua_S,3,0));
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Ready of class  CServerSetup */
#ifndef TOLUA_DISABLE_tolua_get_stratagus_CServerSetup_Ready
static int tolua_get_stratagus_CServerSetup_Ready(lua_State* tolua_S)
{
 int tolua_index;
  CServerSetup* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (CServerSetup*)  lua_touserdata(tolua_S,-1);
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
 tolua_pushnumber(tolua_S,(lua_Number)self->Ready[tolua_index]);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Ready of class  CServerSetup */
#ifndef TOLUA_DISABLE_tolua_set_stratagus_CServerSetup_Ready
static int tolua_set_stratagus_CServerSetup_Ready(lua_State* tolua_S)
{
 int tolua_index;
  CServerSetup* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (CServerSetup*)  lua_touserdata(tolua_S,-1);
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
  self->Ready[tolua_index] = ((unsigned)  tolua_tonumber(tolua_S,3,0));
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: LastFrame of class  CServerSetup */
#ifndef TOLUA_DISABLE_tolua_get_stratagus_CServerSetup_LastFrame
static int tolua_get_stratagus_CServerSetup_LastFrame(lua_State* tolua_S)
{
 int tolua_index;
  CServerSetup* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (CServerSetup*)  lua_touserdata(tolua_S,-1);
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
 tolua_pushnumber(tolua_S,(lua_Number)self->LastFrame[tolua_index]);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: LastFrame of class  CServerSetup */
#ifndef TOLUA_DISABLE_tolua_set_stratagus_CServerSetup_LastFrame
static int tolua_set_stratagus_CServerSetup_LastFrame(lua_State* tolua_S)
{
 int tolua_index;
  CServerSetup* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (CServerSetup*)  lua_touserdata(tolua_S,-1);
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
  self->LastFrame[tolua_index] = ((unsigned long)  tolua_tonumber(tolua_S,3,0));
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: LocalSetupState */
#ifndef TOLUA_DISABLE_tolua_get_LocalSetupState
static int tolua_get_LocalSetupState(lua_State* tolua_S)
{
   tolua_pushusertype(tolua_S,(void*)&LocalSetupState,"CServerSetup");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: LocalSetupState */
#ifndef TOLUA_DISABLE_tolua_set_LocalSetupState
static int tolua_set_LocalSetupState(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if ((tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"CServerSetup",0,&tolua_err)))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  LocalSetupState = *((CServerSetup*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: ServerSetupState */
#ifndef TOLUA_DISABLE_tolua_get_ServerSetupState
static int tolua_get_ServerSetupState(lua_State* tolua_S)
{
   tolua_pushusertype(tolua_S,(void*)&ServerSetupState,"CServerSetup");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: ServerSetupState */
#ifndef TOLUA_DISABLE_tolua_set_ServerSetupState
static int tolua_set_ServerSetupState(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if ((tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"CServerSetup",0,&tolua_err)))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  ServerSetupState = *((CServerSetup*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: NetLocalHostsSlot */
#ifndef TOLUA_DISABLE_tolua_get_NetLocalHostsSlot
static int tolua_get_NetLocalHostsSlot(lua_State* tolua_S)
{
  tolua_pushnumber(tolua_S,(lua_Number)NetLocalHostsSlot);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: NetLocalHostsSlot */
#ifndef TOLUA_DISABLE_tolua_set_NetLocalHostsSlot
static int tolua_set_NetLocalHostsSlot(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  NetLocalHostsSlot = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: NetPlayerNameSize */
#ifndef TOLUA_DISABLE_tolua_get_NetPlayerNameSize
static int tolua_get_NetPlayerNameSize(lua_State* tolua_S)
{
  tolua_pushnumber(tolua_S,(lua_Number)NetPlayerNameSize);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Host of class  CNetworkHost */
#ifndef TOLUA_DISABLE_tolua_get_CNetworkHost_unsigned_Host
static int tolua_get_CNetworkHost_unsigned_Host(lua_State* tolua_S)
{
  CNetworkHost* self = (CNetworkHost*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Host'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->Host);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Host of class  CNetworkHost */
#ifndef TOLUA_DISABLE_tolua_set_CNetworkHost_unsigned_Host
static int tolua_set_CNetworkHost_unsigned_Host(lua_State* tolua_S)
{
  CNetworkHost* self = (CNetworkHost*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Host'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Host = ((unsigned long)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Port of class  CNetworkHost */
#ifndef TOLUA_DISABLE_tolua_get_CNetworkHost_unsigned_Port
static int tolua_get_CNetworkHost_unsigned_Port(lua_State* tolua_S)
{
  CNetworkHost* self = (CNetworkHost*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Port'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->Port);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Port of class  CNetworkHost */
#ifndef TOLUA_DISABLE_tolua_set_CNetworkHost_unsigned_Port
static int tolua_set_CNetworkHost_unsigned_Port(lua_State* tolua_S)
{
  CNetworkHost* self = (CNetworkHost*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Port'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Port = ((unsigned short)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: PlyNr of class  CNetworkHost */
#ifndef TOLUA_DISABLE_tolua_get_CNetworkHost_unsigned_PlyNr
static int tolua_get_CNetworkHost_unsigned_PlyNr(lua_State* tolua_S)
{
  CNetworkHost* self = (CNetworkHost*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'PlyNr'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->PlyNr);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: PlyNr of class  CNetworkHost */
#ifndef TOLUA_DISABLE_tolua_set_CNetworkHost_unsigned_PlyNr
static int tolua_set_CNetworkHost_unsigned_PlyNr(lua_State* tolua_S)
{
  CNetworkHost* self = (CNetworkHost*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'PlyNr'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->PlyNr = ((unsigned short)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: PlyName of class  CNetworkHost */
#ifndef TOLUA_DISABLE_tolua_get_CNetworkHost_PlyName
static int tolua_get_CNetworkHost_PlyName(lua_State* tolua_S)
{
  CNetworkHost* self = (CNetworkHost*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'PlyName'",NULL);
#endif
  tolua_pushstring(tolua_S,(const char*)self->PlyName);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: PlyName of class  CNetworkHost */
#ifndef TOLUA_DISABLE_tolua_set_CNetworkHost_PlyName
static int tolua_set_CNetworkHost_PlyName(lua_State* tolua_S)
{
  CNetworkHost* self = (CNetworkHost*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'PlyName'",NULL);
  if (!tolua_istable(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
 strncpy((char*)
self->PlyName,(const char*)tolua_tostring(tolua_S,2,0),NetPlayerNameSize-1);
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Hosts */
#ifndef TOLUA_DISABLE_tolua_get_stratagus_Hosts
static int tolua_get_stratagus_Hosts(lua_State* tolua_S)
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
  tolua_pushusertype(tolua_S,(void*)&Hosts[tolua_index],"CNetworkHost");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Hosts */
#ifndef TOLUA_DISABLE_tolua_set_stratagus_Hosts
static int tolua_set_stratagus_Hosts(lua_State* tolua_S)
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
  Hosts[tolua_index] = *((CNetworkHost*)  tolua_tousertype(tolua_S,3,0));
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: NetworkMapName */
#ifndef TOLUA_DISABLE_tolua_get_NetworkMapName
static int tolua_get_NetworkMapName(lua_State* tolua_S)
{
  tolua_pushcppstring(tolua_S,(const char*)NetworkMapName);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: NetworkMapName */
#ifndef TOLUA_DISABLE_tolua_set_NetworkMapName
static int tolua_set_NetworkMapName(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_iscppstring(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  NetworkMapName = ((string)  tolua_tocppstring(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* function: NetworkGamePrepareGameSettings */
#ifndef TOLUA_DISABLE_tolua_stratagus_NetworkGamePrepareGameSettings00
static int tolua_stratagus_NetworkGamePrepareGameSettings00(lua_State* tolua_S)
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
   NetworkGamePrepareGameSettings();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'NetworkGamePrepareGameSettings'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: InitVideo */
#ifndef TOLUA_DISABLE_tolua_stratagus_InitVideo00
static int tolua_stratagus_InitVideo00(lua_State* tolua_S)
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
   InitVideo();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'InitVideo'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* get function: UseOpenGL */
#ifndef TOLUA_DISABLE_tolua_get_UseOpenGL
static int tolua_get_UseOpenGL(lua_State* tolua_S)
{
  tolua_pushboolean(tolua_S,(bool)UseOpenGL);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: UseOpenGL */
#ifndef TOLUA_DISABLE_tolua_set_UseOpenGL
static int tolua_set_UseOpenGL(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  UseOpenGL = ((bool)  tolua_toboolean(tolua_S,2,0))
;
 return 0;
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'ResizeScreen'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->ResizeScreen(width,height);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
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
  if ((tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"CVideo",0,&tolua_err)))
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
     !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,1,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,1,&tolua_err) ||
     !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const std::string file = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'Load'", NULL);
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'Resize'", NULL);
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

/* method: New of class  CPlayerColorGraphic */
#ifndef TOLUA_DISABLE_tolua_stratagus_CPlayerColorGraphic_New00
static int tolua_stratagus_CPlayerColorGraphic_New00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"CPlayerColorGraphic",0,&tolua_err) ||
     !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,1,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,1,&tolua_err) ||
     !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const std::string file = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
  int w = ((int)  tolua_tonumber(tolua_S,3,0));
  int h = ((int)  tolua_tonumber(tolua_S,4,0));
  {
   CPlayerColorGraphic* tolua_ret = (CPlayerColorGraphic*)  CPlayerColorGraphic::New(file,w,h);
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"CPlayerColorGraphic");
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
   CColor* tolua_ret = (CColor*)  Mtolua_new((CColor)(r,g,b,a));
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
   CColor* tolua_ret = (CColor*)  Mtolua_new((CColor)(r,g,b,a));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"CColor");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
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

/* get function: UseGLTextureCompression */
#ifndef TOLUA_DISABLE_tolua_get_UseGLTextureCompression
static int tolua_get_UseGLTextureCompression(lua_State* tolua_S)
{
  tolua_pushboolean(tolua_S,(bool)UseGLTextureCompression);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: UseGLTextureCompression */
#ifndef TOLUA_DISABLE_tolua_set_UseGLTextureCompression
static int tolua_set_UseGLTextureCompression(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  UseGLTextureCompression = ((bool)  tolua_toboolean(tolua_S,2,0))
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
     !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isusertype(tolua_S,3,"CGraphic",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const std::string ident = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
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
     !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const std::string ident = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'Height'", NULL);
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
     !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CFont* self = (CFont*)  tolua_tousertype(tolua_S,1,0);
  const std::string text = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'Width'", NULL);
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

/* method: PlainText of class  CFont */
#ifndef TOLUA_DISABLE_tolua_stratagus_CFont_PlainText00
static int tolua_stratagus_CFont_PlainText00(lua_State* tolua_S)
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
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'PlainText'", NULL);
#endif
  {
   CFont* tolua_ret = (CFont*)  self->PlainText();
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"CFont");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'PlainText'.",&tolua_err);
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
     !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const std::string ident = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
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
     !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const std::string ident = ((const std::string)  tolua_tocppstring(tolua_S,2,0));
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

/* get function: Index of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_get_CPlayer_Index
static int tolua_get_CPlayer_Index(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Index'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->Index);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Index of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_set_CPlayer_Index
static int tolua_set_CPlayer_Index(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Index'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Index = ((int)  tolua_tonumber(tolua_S,2,0))
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
  tolua_pushcppstring(tolua_S,(const char*)self->Name);
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
  if (!tolua_iscppstring(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Name = ((string)  tolua_tocppstring(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Type of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_get_CPlayer_Type
static int tolua_get_CPlayer_Type(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Type'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->Type);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Type of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_set_CPlayer_Type
static int tolua_set_CPlayer_Type(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Type'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Type = ((PlayerTypes) (int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: AiName of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_get_CPlayer_AiName
static int tolua_get_CPlayer_AiName(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'AiName'",NULL);
#endif
  tolua_pushcppstring(tolua_S,(const char*)self->AiName);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: AiName of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_set_CPlayer_AiName
static int tolua_set_CPlayer_AiName(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'AiName'",NULL);
  if (!tolua_iscppstring(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->AiName = ((string)  tolua_tocppstring(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: StartX of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_get_CPlayer_StartX
static int tolua_get_CPlayer_StartX(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'StartX'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->StartX);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: StartX of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_set_CPlayer_StartX
static int tolua_set_CPlayer_StartX(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'StartX'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->StartX = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: StartY of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_get_CPlayer_StartY
static int tolua_get_CPlayer_StartY(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'StartY'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->StartY);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: StartY of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_set_CPlayer_StartY
static int tolua_set_CPlayer_StartY(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'StartY'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->StartY = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* method: SetStartView of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_stratagus_CPlayer_SetStartView00
static int tolua_stratagus_CPlayer_SetStartView00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"CPlayer",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
  int x = ((int)  tolua_tonumber(tolua_S,2,0));
  int y = ((int)  tolua_tonumber(tolua_S,3,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'SetStartView'", NULL);
#endif
  {
   self->SetStartView(x,y);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetStartView'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* get function: EnergyProductionRate of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_get_CPlayer_EnergyProductionRate
static int tolua_get_CPlayer_EnergyProductionRate(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'EnergyProductionRate'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->GetEnergyProductionRate());
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: EnergyProductionRate of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_set_CPlayer_EnergyProductionRate
static int tolua_set_CPlayer_EnergyProductionRate(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'EnergyProductionRate'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->SetEnergyProductionRate(((int)  tolua_tonumber(tolua_S,2,0))
)
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: MagmaProductionRate of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_get_CPlayer_MagmaProductionRate
static int tolua_get_CPlayer_MagmaProductionRate(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MagmaProductionRate'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->GetMagmaProductionRate());
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: MagmaProductionRate of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_set_CPlayer_MagmaProductionRate
static int tolua_set_CPlayer_MagmaProductionRate(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MagmaProductionRate'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->SetMagmaProductionRate(((int)  tolua_tonumber(tolua_S,2,0))
)
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: EnergyStored of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_get_CPlayer_EnergyStored
static int tolua_get_CPlayer_EnergyStored(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'EnergyStored'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->GetEnergyStored());
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: EnergyStored of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_set_CPlayer_EnergyStored
static int tolua_set_CPlayer_EnergyStored(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'EnergyStored'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->SetEnergyStored(((int)  tolua_tonumber(tolua_S,2,0))
)
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: MagmaStored of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_get_CPlayer_MagmaStored
static int tolua_get_CPlayer_MagmaStored(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MagmaStored'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->GetMagmaStored());
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: MagmaStored of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_set_CPlayer_MagmaStored
static int tolua_set_CPlayer_MagmaStored(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MagmaStored'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->SetMagmaStored(((int)  tolua_tonumber(tolua_S,2,0))
)
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: EnergyStorageCapacity of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_get_CPlayer_EnergyStorageCapacity
static int tolua_get_CPlayer_EnergyStorageCapacity(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'EnergyStorageCapacity'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->GetEnergyStorageCapacity());
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: EnergyStorageCapacity of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_set_CPlayer_EnergyStorageCapacity
static int tolua_set_CPlayer_EnergyStorageCapacity(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'EnergyStorageCapacity'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->SetEnergyStorageCapacity(((int)  tolua_tonumber(tolua_S,2,0))
)
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: MagmaStorageCapacity of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_get_CPlayer_MagmaStorageCapacity
static int tolua_get_CPlayer_MagmaStorageCapacity(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MagmaStorageCapacity'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->GetMagmaStorageCapacity());
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: MagmaStorageCapacity of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_set_CPlayer_MagmaStorageCapacity
static int tolua_set_CPlayer_MagmaStorageCapacity(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MagmaStorageCapacity'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->SetMagmaStorageCapacity(((int)  tolua_tonumber(tolua_S,2,0))
)
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: UnitTypesCount of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_get_stratagus_CPlayer_UnitTypesCount
static int tolua_get_stratagus_CPlayer_UnitTypesCount(lua_State* tolua_S)
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
 if (tolua_index<0 || tolua_index>=UnitTypeMax)
  tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)self->UnitTypesCount[tolua_index]);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: AiEnabled of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_get_CPlayer_AiEnabled
static int tolua_get_CPlayer_AiEnabled(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'AiEnabled'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->AiEnabled);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: AiEnabled of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_set_CPlayer_AiEnabled
static int tolua_set_CPlayer_AiEnabled(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'AiEnabled'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->AiEnabled = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
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

/* get function: TotalEnergy of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_get_CPlayer_TotalEnergy
static int tolua_get_CPlayer_TotalEnergy(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'TotalEnergy'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->GetTotalEnergy());
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: TotalEnergy of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_set_CPlayer_TotalEnergy
static int tolua_set_CPlayer_TotalEnergy(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'TotalEnergy'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->SetTotalEnergy(((int)  tolua_tonumber(tolua_S,2,0))
)
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: TotalMagma of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_get_CPlayer_TotalMagma
static int tolua_get_CPlayer_TotalMagma(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'TotalMagma'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->GetTotalMagma());
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: TotalMagma of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_set_CPlayer_TotalMagma
static int tolua_set_CPlayer_TotalMagma(lua_State* tolua_S)
{
  CPlayer* self = (CPlayer*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'TotalMagma'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->SetTotalMagma(((int)  tolua_tonumber(tolua_S,2,0))
)
;
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

/* method: IsEnemy of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_stratagus_CPlayer_IsEnemy00
static int tolua_stratagus_CPlayer_IsEnemy00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const CPlayer",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"const CPlayer",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const CPlayer* self = (const CPlayer*)  tolua_tousertype(tolua_S,1,0);
  const CPlayer* x = ((const CPlayer*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'IsEnemy'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->IsEnemy(x);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'IsEnemy'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: IsEnemy of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_stratagus_CPlayer_IsEnemy01
static int tolua_stratagus_CPlayer_IsEnemy01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const CPlayer",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"const CUnit",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  const CPlayer* self = (const CPlayer*)  tolua_tousertype(tolua_S,1,0);
  const CUnit* x = ((const CUnit*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'IsEnemy'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->IsEnemy(x);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
tolua_lerror:
 return tolua_stratagus_CPlayer_IsEnemy00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: IsAllied of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_stratagus_CPlayer_IsAllied00
static int tolua_stratagus_CPlayer_IsAllied00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const CPlayer",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"const CPlayer",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const CPlayer* self = (const CPlayer*)  tolua_tousertype(tolua_S,1,0);
  const CPlayer* x = ((const CPlayer*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'IsAllied'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->IsAllied(x);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'IsAllied'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: IsAllied of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_stratagus_CPlayer_IsAllied01
static int tolua_stratagus_CPlayer_IsAllied01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const CPlayer",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"const CUnit",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  const CPlayer* self = (const CPlayer*)  tolua_tousertype(tolua_S,1,0);
  const CUnit* x = ((const CUnit*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'IsAllied'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->IsAllied(x);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
tolua_lerror:
 return tolua_stratagus_CPlayer_IsAllied00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: IsSharedVision of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_stratagus_CPlayer_IsSharedVision00
static int tolua_stratagus_CPlayer_IsSharedVision00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const CPlayer",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"const CPlayer",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const CPlayer* self = (const CPlayer*)  tolua_tousertype(tolua_S,1,0);
  const CPlayer* x = ((const CPlayer*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'IsSharedVision'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->IsSharedVision(x);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'IsSharedVision'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: IsSharedVision of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_stratagus_CPlayer_IsSharedVision01
static int tolua_stratagus_CPlayer_IsSharedVision01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const CPlayer",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"const CUnit",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  const CPlayer* self = (const CPlayer*)  tolua_tousertype(tolua_S,1,0);
  const CUnit* x = ((const CUnit*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'IsSharedVision'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->IsSharedVision(x);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
tolua_lerror:
 return tolua_stratagus_CPlayer_IsSharedVision00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: IsBothSharedVision of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_stratagus_CPlayer_IsBothSharedVision00
static int tolua_stratagus_CPlayer_IsBothSharedVision00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const CPlayer",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"const CPlayer",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const CPlayer* self = (const CPlayer*)  tolua_tousertype(tolua_S,1,0);
  const CPlayer* x = ((const CPlayer*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'IsBothSharedVision'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->IsBothSharedVision(x);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'IsBothSharedVision'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: IsBothSharedVision of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_stratagus_CPlayer_IsBothSharedVision01
static int tolua_stratagus_CPlayer_IsBothSharedVision01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const CPlayer",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"const CUnit",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  const CPlayer* self = (const CPlayer*)  tolua_tousertype(tolua_S,1,0);
  const CUnit* x = ((const CUnit*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'IsBothSharedVision'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->IsBothSharedVision(x);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
tolua_lerror:
 return tolua_stratagus_CPlayer_IsBothSharedVision00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* method: IsTeamed of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_stratagus_CPlayer_IsTeamed00
static int tolua_stratagus_CPlayer_IsTeamed00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const CPlayer",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"const CPlayer",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const CPlayer* self = (const CPlayer*)  tolua_tousertype(tolua_S,1,0);
  const CPlayer* x = ((const CPlayer*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'IsTeamed'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->IsTeamed(x);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'IsTeamed'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: IsTeamed of class  CPlayer */
#ifndef TOLUA_DISABLE_tolua_stratagus_CPlayer_IsTeamed01
static int tolua_stratagus_CPlayer_IsTeamed01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const CPlayer",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"const CUnit",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  const CPlayer* self = (const CPlayer*)  tolua_tousertype(tolua_S,1,0);
  const CUnit* x = ((const CUnit*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'IsTeamed'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->IsTeamed(x);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
tolua_lerror:
 return tolua_stratagus_CPlayer_IsTeamed00(tolua_S);
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

/* function: CclChangeUnitsOwner */
#ifndef TOLUA_DISABLE_tolua_stratagus_ChangeUnitsOwner00
static int tolua_stratagus_ChangeUnitsOwner00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_istable(tolua_S,1,0,&tolua_err) ||
     !tolua_istable(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,6,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  int topLeft[2];
  int bottomRight[2];
  int oldPlayer = ((int)  tolua_tonumber(tolua_S,3,0));
  int newPlayer = ((int)  tolua_tonumber(tolua_S,4,0));
  lua_Object unitType = ((lua_Object)  tolua_tovalue(tolua_S,5,0));
  lua_State* l =  tolua_S;
  {
#ifndef TOLUA_RELEASE
   if (!tolua_isnumberarray(tolua_S,1,2,0,&tolua_err))
    goto tolua_lerror;
   else
#endif
   {
    int i;
    for(i=0; i<2;i++)
    topLeft[i] = ((int)  tolua_tofieldnumber(tolua_S,1,i+1,0));
   }
  }
  {
#ifndef TOLUA_RELEASE
   if (!tolua_isnumberarray(tolua_S,2,2,0,&tolua_err))
    goto tolua_lerror;
   else
#endif
   {
    int i;
    for(i=0; i<2;i++)
    bottomRight[i] = ((int)  tolua_tofieldnumber(tolua_S,2,i+1,0));
   }
  }
  {
   CclChangeUnitsOwner(topLeft,bottomRight,oldPlayer,newPlayer,unitType,l);
  }
  {
   int i;
   for(i=0; i<2;i++)
    tolua_pushfieldnumber(tolua_S,1,i+1,(lua_Number) topLeft[i]);
  }
  {
   int i;
   for(i=0; i<2;i++)
    tolua_pushfieldnumber(tolua_S,2,i+1,(lua_Number) bottomRight[i]);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'ChangeUnitsOwner'.",&tolua_err);
 return 0;
#endif
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
  tolua_pushcppstring(tolua_S,(const char*)self->Ident);
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
  if (!tolua_iscppstring(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Ident = ((std::string)  tolua_tocppstring(tolua_S,2,0))
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
  tolua_pushcppstring(tolua_S,(const char*)self->Name);
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
  if (!tolua_iscppstring(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Name = ((std::string)  tolua_tocppstring(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Slot of class  CUnitType */
#ifndef TOLUA_DISABLE_tolua_get_CUnitType_Slot
static int tolua_get_CUnitType_Slot(lua_State* tolua_S)
{
  CUnitType* self = (CUnitType*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Slot'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->Slot);
 return 1;
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

/* get function: MaxEnergyUtilizationRate of class  CUnitType */
#ifndef TOLUA_DISABLE_tolua_get_CUnitType_MaxEnergyUtilizationRate
static int tolua_get_CUnitType_MaxEnergyUtilizationRate(lua_State* tolua_S)
{
  CUnitType* self = (CUnitType*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MaxEnergyUtilizationRate'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->GetMaxEnergyUtilizationRate());
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: MaxEnergyUtilizationRate of class  CUnitType */
#ifndef TOLUA_DISABLE_tolua_set_CUnitType_MaxEnergyUtilizationRate
static int tolua_set_CUnitType_MaxEnergyUtilizationRate(lua_State* tolua_S)
{
  CUnitType* self = (CUnitType*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MaxEnergyUtilizationRate'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->SetMaxEnergyUtilizationRate(((int)  tolua_tonumber(tolua_S,2,0))
)
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: MaxMagmaUtilizationRate of class  CUnitType */
#ifndef TOLUA_DISABLE_tolua_get_CUnitType_MaxMagmaUtilizationRate
static int tolua_get_CUnitType_MaxMagmaUtilizationRate(lua_State* tolua_S)
{
  CUnitType* self = (CUnitType*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MaxMagmaUtilizationRate'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->GetMaxMagmaUtilizationRate());
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: MaxMagmaUtilizationRate of class  CUnitType */
#ifndef TOLUA_DISABLE_tolua_set_CUnitType_MaxMagmaUtilizationRate
static int tolua_set_CUnitType_MaxMagmaUtilizationRate(lua_State* tolua_S)
{
  CUnitType* self = (CUnitType*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MaxMagmaUtilizationRate'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->SetMaxMagmaUtilizationRate(((int)  tolua_tonumber(tolua_S,2,0))
)
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: EnergyProductionRate of class  CUnitType */
#ifndef TOLUA_DISABLE_tolua_get_CUnitType_EnergyProductionRate
static int tolua_get_CUnitType_EnergyProductionRate(lua_State* tolua_S)
{
  CUnitType* self = (CUnitType*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'EnergyProductionRate'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->GetEnergyProductionRate());
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: EnergyProductionRate of class  CUnitType */
#ifndef TOLUA_DISABLE_tolua_set_CUnitType_EnergyProductionRate
static int tolua_set_CUnitType_EnergyProductionRate(lua_State* tolua_S)
{
  CUnitType* self = (CUnitType*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'EnergyProductionRate'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->SetEnergyProductionRate(((int)  tolua_tonumber(tolua_S,2,0))
)
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: MagmaProductionRate of class  CUnitType */
#ifndef TOLUA_DISABLE_tolua_get_CUnitType_MagmaProductionRate
static int tolua_get_CUnitType_MagmaProductionRate(lua_State* tolua_S)
{
  CUnitType* self = (CUnitType*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MagmaProductionRate'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->GetMagmaProductionRate());
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: MagmaProductionRate of class  CUnitType */
#ifndef TOLUA_DISABLE_tolua_set_CUnitType_MagmaProductionRate
static int tolua_set_CUnitType_MagmaProductionRate(lua_State* tolua_S)
{
  CUnitType* self = (CUnitType*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MagmaProductionRate'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->SetMagmaProductionRate(((int)  tolua_tonumber(tolua_S,2,0))
)
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: EnergyValue of class  CUnitType */
#ifndef TOLUA_DISABLE_tolua_get_CUnitType_EnergyValue
static int tolua_get_CUnitType_EnergyValue(lua_State* tolua_S)
{
  CUnitType* self = (CUnitType*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'EnergyValue'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->GetEnergyValue());
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: EnergyValue of class  CUnitType */
#ifndef TOLUA_DISABLE_tolua_set_CUnitType_EnergyValue
static int tolua_set_CUnitType_EnergyValue(lua_State* tolua_S)
{
  CUnitType* self = (CUnitType*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'EnergyValue'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->SetEnergyValue(((int)  tolua_tonumber(tolua_S,2,0))
)
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: MagmaValue of class  CUnitType */
#ifndef TOLUA_DISABLE_tolua_get_CUnitType_MagmaValue
static int tolua_get_CUnitType_MagmaValue(lua_State* tolua_S)
{
  CUnitType* self = (CUnitType*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MagmaValue'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->GetMagmaValue());
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: MagmaValue of class  CUnitType */
#ifndef TOLUA_DISABLE_tolua_set_CUnitType_MagmaValue
static int tolua_set_CUnitType_MagmaValue(lua_State* tolua_S)
{
  CUnitType* self = (CUnitType*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MagmaValue'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->SetMagmaValue(((int)  tolua_tonumber(tolua_S,2,0))
)
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: EnergyStorageCapacity of class  CUnitType */
#ifndef TOLUA_DISABLE_tolua_get_CUnitType_EnergyStorageCapacity
static int tolua_get_CUnitType_EnergyStorageCapacity(lua_State* tolua_S)
{
  CUnitType* self = (CUnitType*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'EnergyStorageCapacity'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->GetEnergyStorageCapacity());
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: EnergyStorageCapacity of class  CUnitType */
#ifndef TOLUA_DISABLE_tolua_set_CUnitType_EnergyStorageCapacity
static int tolua_set_CUnitType_EnergyStorageCapacity(lua_State* tolua_S)
{
  CUnitType* self = (CUnitType*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'EnergyStorageCapacity'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->SetEnergyStorageCapacity(((int)  tolua_tonumber(tolua_S,2,0))
)
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: MagmaStorageCapacity of class  CUnitType */
#ifndef TOLUA_DISABLE_tolua_get_CUnitType_MagmaStorageCapacity
static int tolua_get_CUnitType_MagmaStorageCapacity(lua_State* tolua_S)
{
  CUnitType* self = (CUnitType*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MagmaStorageCapacity'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->GetMagmaStorageCapacity());
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: MagmaStorageCapacity of class  CUnitType */
#ifndef TOLUA_DISABLE_tolua_set_CUnitType_MagmaStorageCapacity
static int tolua_set_CUnitType_MagmaStorageCapacity(lua_State* tolua_S)
{
  CUnitType* self = (CUnitType*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MagmaStorageCapacity'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->SetMagmaStorageCapacity(((int)  tolua_tonumber(tolua_S,2,0))
)
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* function: UnitTypeByIdent */
#ifndef TOLUA_DISABLE_tolua_stratagus_UnitTypeByIdent00
static int tolua_stratagus_UnitTypeByIdent00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_iscppstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const std::string ident = ((const std::string)  tolua_tocppstring(tolua_S,1,0));
  {
   CUnitType* tolua_ret = (CUnitType*)  UnitTypeByIdent(ident);
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"CUnitType");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'UnitTypeByIdent'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Slot of class  CUnit */
#ifndef TOLUA_DISABLE_tolua_get_CUnit_Slot
static int tolua_get_CUnit_Slot(lua_State* tolua_S)
{
  CUnit* self = (CUnit*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Slot'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->Slot);
 return 1;
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
  if ((tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"CPreference",0,&tolua_err)))
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

/* function: PlayFile */
#ifndef TOLUA_DISABLE_tolua_stratagus_PlayFile00
static int tolua_stratagus_PlayFile00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_iscppstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"LuaActionListener",1,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const std::string name = ((const std::string)  tolua_tocppstring(tolua_S,1,0));
  LuaActionListener* listener = ((LuaActionListener*)  tolua_tousertype(tolua_S,2,NULL));
  {
   int tolua_ret = (int)  PlayFile(name,listener);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'PlayFile'.",&tolua_err);
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
     !tolua_iscppstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const std::string name = ((const std::string)  tolua_tocppstring(tolua_S,1,0));
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

/* function: SetChannelVolume */
#ifndef TOLUA_DISABLE_tolua_stratagus_SetChannelVolume00
static int tolua_stratagus_SetChannelVolume00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  int channel = ((int)  tolua_tonumber(tolua_S,1,0));
  int volume = ((int)  tolua_tonumber(tolua_S,2,0));
  {
   int tolua_ret = (int)  SetChannelVolume(channel,volume);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetChannelVolume'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: SetChannelStereo */
#ifndef TOLUA_DISABLE_tolua_stratagus_SetChannelStereo00
static int tolua_stratagus_SetChannelStereo00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  int channel = ((int)  tolua_tonumber(tolua_S,1,0));
  int stereo = ((int)  tolua_tonumber(tolua_S,2,0));
  {
   int tolua_ret = (int)  SetChannelStereo(channel,stereo);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetChannelStereo'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: StopChannel */
#ifndef TOLUA_DISABLE_tolua_stratagus_StopChannel00
static int tolua_stratagus_StopChannel00(lua_State* tolua_S)
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
  int channel = ((int)  tolua_tonumber(tolua_S,1,0));
  {
   StopChannel(channel);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'StopChannel'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: StopAllChannels */
#ifndef TOLUA_DISABLE_tolua_stratagus_StopAllChannels00
static int tolua_stratagus_StopAllChannels00(lua_State* tolua_S)
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
   StopAllChannels();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'StopAllChannels'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* get function: UnitTypes of class  CEditor */
#ifndef TOLUA_DISABLE_tolua_get_CEditor_UnitTypes
static int tolua_get_CEditor_UnitTypes(lua_State* tolua_S)
{
  CEditor* self = (CEditor*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'UnitTypes'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)&self->UnitTypes,"vector<string>");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: UnitTypes of class  CEditor */
#ifndef TOLUA_DISABLE_tolua_set_CEditor_UnitTypes
static int tolua_set_CEditor_UnitTypes(lua_State* tolua_S)
{
  CEditor* self = (CEditor*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'UnitTypes'",NULL);
  if ((tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"vector<string>",0,&tolua_err)))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->UnitTypes = *((vector<string>*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: StartUnit of class  CEditor */
#ifndef TOLUA_DISABLE_tolua_get_CEditor_StartUnit_ptr
static int tolua_get_CEditor_StartUnit_ptr(lua_State* tolua_S)
{
  CEditor* self = (CEditor*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'StartUnit'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)self->StartUnit,"const CUnitType");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: ShowTerrainFlags of class  CEditor */
#ifndef TOLUA_DISABLE_tolua_get_CEditor_ShowTerrainFlags
static int tolua_get_CEditor_ShowTerrainFlags(lua_State* tolua_S)
{
  CEditor* self = (CEditor*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ShowTerrainFlags'",NULL);
#endif
  tolua_pushboolean(tolua_S,(bool)self->ShowTerrainFlags);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: ShowTerrainFlags of class  CEditor */
#ifndef TOLUA_DISABLE_tolua_set_CEditor_ShowTerrainFlags
static int tolua_set_CEditor_ShowTerrainFlags(lua_State* tolua_S)
{
  CEditor* self = (CEditor*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'ShowTerrainFlags'",NULL);
  if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->ShowTerrainFlags = ((bool)  tolua_toboolean(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Running of class  CEditor */
#ifndef TOLUA_DISABLE_tolua_get_CEditor_Running
static int tolua_get_CEditor_Running(lua_State* tolua_S)
{
  CEditor* self = (CEditor*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Running'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->Running);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Running of class  CEditor */
#ifndef TOLUA_DISABLE_tolua_set_CEditor_Running
static int tolua_set_CEditor_Running(lua_State* tolua_S)
{
  CEditor* self = (CEditor*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Running'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Running = ((EditorRunningType) (int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* method: TileSelectedPatch of class  CEditor */
#ifndef TOLUA_DISABLE_tolua_stratagus_CEditor_TileSelectedPatch00
static int tolua_stratagus_CEditor_TileSelectedPatch00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"CEditor",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CEditor* self = (CEditor*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'TileSelectedPatch'", NULL);
#endif
  {
   self->TileSelectedPatch();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'TileSelectedPatch'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: SetEditorSelectIcon */
#ifndef TOLUA_DISABLE_tolua_stratagus_SetEditorSelectIcon00
static int tolua_stratagus_SetEditorSelectIcon00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_iscppstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  std::string icon = ((std::string)  tolua_tocppstring(tolua_S,1,0));
  {
   SetEditorSelectIcon(icon);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetEditorSelectIcon'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: SetEditorUnitsIcon */
#ifndef TOLUA_DISABLE_tolua_stratagus_SetEditorUnitsIcon00
static int tolua_stratagus_SetEditorUnitsIcon00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_iscppstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  std::string icon = ((std::string)  tolua_tocppstring(tolua_S,1,0));
  {
   SetEditorUnitsIcon(icon);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetEditorUnitsIcon'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: SetEditorPatchIcon */
#ifndef TOLUA_DISABLE_tolua_stratagus_SetEditorPatchIcon00
static int tolua_stratagus_SetEditorPatchIcon00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_iscppstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  std::string icon = ((std::string)  tolua_tocppstring(tolua_S,1,0));
  {
   SetEditorPatchIcon(icon);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetEditorPatchIcon'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: SetEditorStartUnit */
#ifndef TOLUA_DISABLE_tolua_stratagus_SetEditorStartUnit00
static int tolua_stratagus_SetEditorStartUnit00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_iscppstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  std::string name = ((std::string)  tolua_tocppstring(tolua_S,1,0));
  {
   SetEditorStartUnit(name);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetEditorStartUnit'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Editor */
#ifndef TOLUA_DISABLE_tolua_get_Editor
static int tolua_get_Editor(lua_State* tolua_S)
{
   tolua_pushusertype(tolua_S,(void*)&Editor,"CEditor");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Editor */
#ifndef TOLUA_DISABLE_tolua_set_Editor
static int tolua_set_Editor(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if ((tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"CEditor",0,&tolua_err)))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  Editor = *((CEditor*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* function: StartEditor */
#ifndef TOLUA_DISABLE_tolua_stratagus_StartEditor00
static int tolua_stratagus_StartEditor00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_iscppstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  std::string filename = ((std::string)  tolua_tocppstring(tolua_S,1,0));
  {
   StartEditor(filename);
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

/* function: EditorSaveMap */
#ifndef TOLUA_DISABLE_tolua_stratagus_EditorSaveMap00
static int tolua_stratagus_EditorSaveMap00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_iscppstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  std::string file = ((std::string)  tolua_tocppstring(tolua_S,1,0));
  {
   int tolua_ret = (int)  EditorSaveMap(file);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'EditorSaveMap'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: StartPatchEditor */
#ifndef TOLUA_DISABLE_tolua_stratagus_StartPatchEditor00
static int tolua_stratagus_StartPatchEditor00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_iscppstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  std::string patchName = ((std::string)  tolua_tocppstring(tolua_S,1,0));
  {
   StartPatchEditor(patchName);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'StartPatchEditor'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: IsReplayGame */
#ifndef TOLUA_DISABLE_tolua_stratagus_IsReplayGame00
static int tolua_stratagus_IsReplayGame00(lua_State* tolua_S)
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
   bool tolua_ret = (bool)  IsReplayGame();
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'IsReplayGame'.",&tolua_err);
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
     !tolua_iscppstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isboolean(tolua_S,2,1,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const string str = ((const string)  tolua_tocppstring(tolua_S,1,0));
  bool clean = ((bool)  tolua_toboolean(tolua_S,2,true));
  {
   StartMap(str,clean);
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

/* function: StartReplay */
#ifndef TOLUA_DISABLE_tolua_stratagus_StartReplay00
static int tolua_stratagus_StartReplay00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_iscppstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isboolean(tolua_S,2,1,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const string str = ((const string)  tolua_tocppstring(tolua_S,1,0));
  bool reveal = ((bool)  tolua_toboolean(tolua_S,2,false));
  {
   StartReplay(str,reveal);
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
     !tolua_iscppstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const string str = ((const string)  tolua_tocppstring(tolua_S,1,0));
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

/* function: SaveReplay */
#ifndef TOLUA_DISABLE_tolua_stratagus_SaveReplay00
static int tolua_stratagus_SaveReplay00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_iscppstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const std::string filename = ((const std::string)  tolua_tocppstring(tolua_S,1,0));
  {
   int tolua_ret = (int)  SaveReplay(filename);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SaveReplay'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* get function: GameResult */
#ifndef TOLUA_DISABLE_tolua_get_GameResult
static int tolua_get_GameResult(lua_State* tolua_S)
{
  tolua_pushnumber(tolua_S,(lua_Number)GameResult);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: GameResult */
#ifndef TOLUA_DISABLE_tolua_set_GameResult
static int tolua_set_GameResult(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  GameResult = ((GameResults) (int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* function: StopGame */
#ifndef TOLUA_DISABLE_tolua_stratagus_StopGame00
static int tolua_stratagus_StopGame00(lua_State* tolua_S)
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
  GameResults result = ((GameResults) (int)  tolua_tonumber(tolua_S,1,0));
  {
   StopGame(result);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'StopGame'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* get function: GameRunning */
#ifndef TOLUA_DISABLE_tolua_get_GameRunning
static int tolua_get_GameRunning(lua_State* tolua_S)
{
  tolua_pushboolean(tolua_S,(bool)GameRunning);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: GameRunning */
#ifndef TOLUA_DISABLE_tolua_set_GameRunning
static int tolua_set_GameRunning(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  GameRunning = ((bool)  tolua_toboolean(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* function: SetGamePaused */
#ifndef TOLUA_DISABLE_tolua_stratagus_SetGamePaused00
static int tolua_stratagus_SetGamePaused00(lua_State* tolua_S)
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
  bool paused = ((bool)  tolua_toboolean(tolua_S,1,0));
  {
   SetGamePaused(paused);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetGamePaused'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: GetGamePaused */
#ifndef TOLUA_DISABLE_tolua_stratagus_GetGamePaused00
static int tolua_stratagus_GetGamePaused00(lua_State* tolua_S)
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
   bool tolua_ret = (bool)  GetGamePaused();
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetGamePaused'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* get function: GamePaused */
#ifndef TOLUA_DISABLE_tolua_get_GamePaused
static int tolua_get_GamePaused(lua_State* tolua_S)
{
  tolua_pushboolean(tolua_S,(bool)GetGamePaused());
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: GamePaused */
#ifndef TOLUA_DISABLE_tolua_set_GamePaused
static int tolua_set_GamePaused(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  SetGamePaused(((bool)  tolua_toboolean(tolua_S,2,0))
)
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* function: SetGameSpeed */
#ifndef TOLUA_DISABLE_tolua_stratagus_SetGameSpeed00
static int tolua_stratagus_SetGameSpeed00(lua_State* tolua_S)
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
   SetGameSpeed(speed);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetGameSpeed'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: GetGameSpeed */
#ifndef TOLUA_DISABLE_tolua_stratagus_GetGameSpeed00
static int tolua_stratagus_GetGameSpeed00(lua_State* tolua_S)
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
   int tolua_ret = (int)  GetGameSpeed();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetGameSpeed'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* get function: GameSpeed */
#ifndef TOLUA_DISABLE_tolua_get_GameSpeed
static int tolua_get_GameSpeed(lua_State* tolua_S)
{
  tolua_pushnumber(tolua_S,(lua_Number)GetGameSpeed());
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: GameSpeed */
#ifndef TOLUA_DISABLE_tolua_set_GameSpeed
static int tolua_set_GameSpeed(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  SetGameSpeed(((int)  tolua_tonumber(tolua_S,2,0))
)
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: GameObserve */
#ifndef TOLUA_DISABLE_tolua_get_GameObserve
static int tolua_get_GameObserve(lua_State* tolua_S)
{
  tolua_pushboolean(tolua_S,(bool)GameObserve);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: GameObserve */
#ifndef TOLUA_DISABLE_tolua_set_GameObserve
static int tolua_set_GameObserve(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  GameObserve = ((bool)  tolua_toboolean(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: GameCycle */
#ifndef TOLUA_DISABLE_tolua_get_unsigned_GameCycle
static int tolua_get_unsigned_GameCycle(lua_State* tolua_S)
{
  tolua_pushnumber(tolua_S,(lua_Number)GameCycle);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: GameCycle */
#ifndef TOLUA_DISABLE_tolua_set_unsigned_GameCycle
static int tolua_set_unsigned_GameCycle(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  GameCycle = ((unsigned long)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Team of class  SettingsPresets */
#ifndef TOLUA_DISABLE_tolua_get_SettingsPresets_Team
static int tolua_get_SettingsPresets_Team(lua_State* tolua_S)
{
  SettingsPresets* self = (SettingsPresets*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Team'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->Team);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Team of class  SettingsPresets */
#ifndef TOLUA_DISABLE_tolua_set_SettingsPresets_Team
static int tolua_set_SettingsPresets_Team(lua_State* tolua_S)
{
  SettingsPresets* self = (SettingsPresets*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Team'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Team = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Type of class  SettingsPresets */
#ifndef TOLUA_DISABLE_tolua_get_SettingsPresets_Type
static int tolua_get_SettingsPresets_Type(lua_State* tolua_S)
{
  SettingsPresets* self = (SettingsPresets*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Type'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->Type);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Type of class  SettingsPresets */
#ifndef TOLUA_DISABLE_tolua_set_SettingsPresets_Type
static int tolua_set_SettingsPresets_Type(lua_State* tolua_S)
{
  SettingsPresets* self = (SettingsPresets*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Type'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Type = ((PlayerTypes) (int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: NetGameType of class  Settings */
#ifndef TOLUA_DISABLE_tolua_get_Settings_NetGameType
static int tolua_get_Settings_NetGameType(lua_State* tolua_S)
{
  Settings* self = (Settings*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'NetGameType'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->NetGameType);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: NetGameType of class  Settings */
#ifndef TOLUA_DISABLE_tolua_set_Settings_NetGameType
static int tolua_set_Settings_NetGameType(lua_State* tolua_S)
{
  Settings* self = (Settings*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'NetGameType'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->NetGameType = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Presets of class  Settings */
#ifndef TOLUA_DISABLE_tolua_get_stratagus_Settings_Presets
static int tolua_get_stratagus_Settings_Presets(lua_State* tolua_S)
{
 int tolua_index;
  Settings* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (Settings*)  lua_touserdata(tolua_S,-1);
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
  tolua_pushusertype(tolua_S,(void*)&self->Presets[tolua_index],"SettingsPresets");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Presets of class  Settings */
#ifndef TOLUA_DISABLE_tolua_set_stratagus_Settings_Presets
static int tolua_set_stratagus_Settings_Presets(lua_State* tolua_S)
{
 int tolua_index;
  Settings* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (Settings*)  lua_touserdata(tolua_S,-1);
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
  self->Presets[tolua_index] = *((SettingsPresets*)  tolua_tousertype(tolua_S,3,0));
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Resources of class  Settings */
#ifndef TOLUA_DISABLE_tolua_get_Settings_Resources
static int tolua_get_Settings_Resources(lua_State* tolua_S)
{
  Settings* self = (Settings*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Resources'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->Resources);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Resources of class  Settings */
#ifndef TOLUA_DISABLE_tolua_set_Settings_Resources
static int tolua_set_Settings_Resources(lua_State* tolua_S)
{
  Settings* self = (Settings*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Resources'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Resources = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: NumUnits of class  Settings */
#ifndef TOLUA_DISABLE_tolua_get_Settings_NumUnits
static int tolua_get_Settings_NumUnits(lua_State* tolua_S)
{
  Settings* self = (Settings*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'NumUnits'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->NumUnits);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: NumUnits of class  Settings */
#ifndef TOLUA_DISABLE_tolua_set_Settings_NumUnits
static int tolua_set_Settings_NumUnits(lua_State* tolua_S)
{
  Settings* self = (Settings*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'NumUnits'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->NumUnits = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Opponents of class  Settings */
#ifndef TOLUA_DISABLE_tolua_get_Settings_Opponents
static int tolua_get_Settings_Opponents(lua_State* tolua_S)
{
  Settings* self = (Settings*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Opponents'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->Opponents);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Opponents of class  Settings */
#ifndef TOLUA_DISABLE_tolua_set_Settings_Opponents
static int tolua_set_Settings_Opponents(lua_State* tolua_S)
{
  Settings* self = (Settings*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Opponents'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Opponents = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Difficulty of class  Settings */
#ifndef TOLUA_DISABLE_tolua_get_Settings_Difficulty
static int tolua_get_Settings_Difficulty(lua_State* tolua_S)
{
  Settings* self = (Settings*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Difficulty'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->Difficulty);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Difficulty of class  Settings */
#ifndef TOLUA_DISABLE_tolua_set_Settings_Difficulty
static int tolua_set_Settings_Difficulty(lua_State* tolua_S)
{
  Settings* self = (Settings*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Difficulty'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Difficulty = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: GameType of class  Settings */
#ifndef TOLUA_DISABLE_tolua_get_Settings_GameType
static int tolua_get_Settings_GameType(lua_State* tolua_S)
{
  Settings* self = (Settings*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'GameType'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->GameType);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: GameType of class  Settings */
#ifndef TOLUA_DISABLE_tolua_set_Settings_GameType
static int tolua_set_Settings_GameType(lua_State* tolua_S)
{
  Settings* self = (Settings*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'GameType'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->GameType = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: NoFogOfWar of class  Settings */
#ifndef TOLUA_DISABLE_tolua_get_Settings_NoFogOfWar
static int tolua_get_Settings_NoFogOfWar(lua_State* tolua_S)
{
  Settings* self = (Settings*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'NoFogOfWar'",NULL);
#endif
  tolua_pushboolean(tolua_S,(bool)self->NoFogOfWar);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: NoFogOfWar of class  Settings */
#ifndef TOLUA_DISABLE_tolua_set_Settings_NoFogOfWar
static int tolua_set_Settings_NoFogOfWar(lua_State* tolua_S)
{
  Settings* self = (Settings*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'NoFogOfWar'",NULL);
  if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->NoFogOfWar = ((bool)  tolua_toboolean(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: RevealMap of class  Settings */
#ifndef TOLUA_DISABLE_tolua_get_Settings_RevealMap
static int tolua_get_Settings_RevealMap(lua_State* tolua_S)
{
  Settings* self = (Settings*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'RevealMap'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->RevealMap);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: RevealMap of class  Settings */
#ifndef TOLUA_DISABLE_tolua_set_Settings_RevealMap
static int tolua_set_Settings_RevealMap(lua_State* tolua_S)
{
  Settings* self = (Settings*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'RevealMap'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->RevealMap = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: MapRichness of class  Settings */
#ifndef TOLUA_DISABLE_tolua_get_Settings_MapRichness
static int tolua_get_Settings_MapRichness(lua_State* tolua_S)
{
  Settings* self = (Settings*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MapRichness'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->MapRichness);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: MapRichness of class  Settings */
#ifndef TOLUA_DISABLE_tolua_set_Settings_MapRichness
static int tolua_set_Settings_MapRichness(lua_State* tolua_S)
{
  Settings* self = (Settings*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MapRichness'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->MapRichness = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: GameSettings */
#ifndef TOLUA_DISABLE_tolua_get_GameSettings
static int tolua_get_GameSettings(lua_State* tolua_S)
{
   tolua_pushusertype(tolua_S,(void*)&GameSettings,"Settings");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: GameSettings */
#ifndef TOLUA_DISABLE_tolua_set_GameSettings
static int tolua_set_GameSettings(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if ((tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"Settings",0,&tolua_err)))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  GameSettings = *((Settings*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: AlliedUnitRecyclingEfficiency */
#ifndef TOLUA_DISABLE_tolua_get_stratagus_AlliedUnitRecyclingEfficiency
static int tolua_get_stratagus_AlliedUnitRecyclingEfficiency(lua_State* tolua_S)
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
 if (tolua_index<0 || tolua_index>=MaxCosts)
  tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)AlliedUnitRecyclingEfficiency[tolua_index]);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: AlliedUnitRecyclingEfficiency */
#ifndef TOLUA_DISABLE_tolua_set_stratagus_AlliedUnitRecyclingEfficiency
static int tolua_set_stratagus_AlliedUnitRecyclingEfficiency(lua_State* tolua_S)
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
 if (tolua_index<0 || tolua_index>=MaxCosts)
  tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
  AlliedUnitRecyclingEfficiency[tolua_index] = ((int)  tolua_tonumber(tolua_S,3,0));
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: EnemyUnitRecyclingEfficiency */
#ifndef TOLUA_DISABLE_tolua_get_stratagus_EnemyUnitRecyclingEfficiency
static int tolua_get_stratagus_EnemyUnitRecyclingEfficiency(lua_State* tolua_S)
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
 if (tolua_index<0 || tolua_index>=MaxCosts)
  tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
 tolua_pushnumber(tolua_S,(lua_Number)EnemyUnitRecyclingEfficiency[tolua_index]);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: EnemyUnitRecyclingEfficiency */
#ifndef TOLUA_DISABLE_tolua_set_stratagus_EnemyUnitRecyclingEfficiency
static int tolua_set_stratagus_EnemyUnitRecyclingEfficiency(lua_State* tolua_S)
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
 if (tolua_index<0 || tolua_index>=MaxCosts)
  tolua_error(tolua_S,"array indexing out of range.",NULL);
#endif
  EnemyUnitRecyclingEfficiency[tolua_index] = ((int)  tolua_tonumber(tolua_S,3,0));
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Description of class  CMapInfo */
#ifndef TOLUA_DISABLE_tolua_get_CMapInfo_Description
static int tolua_get_CMapInfo_Description(lua_State* tolua_S)
{
  CMapInfo* self = (CMapInfo*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Description'",NULL);
#endif
  tolua_pushcppstring(tolua_S,(const char*)self->Description);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Description of class  CMapInfo */
#ifndef TOLUA_DISABLE_tolua_set_CMapInfo_Description
static int tolua_set_CMapInfo_Description(lua_State* tolua_S)
{
  CMapInfo* self = (CMapInfo*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Description'",NULL);
  if (!tolua_iscppstring(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Description = ((string)  tolua_tocppstring(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: MapWidth of class  CMapInfo */
#ifndef TOLUA_DISABLE_tolua_get_CMapInfo_MapWidth
static int tolua_get_CMapInfo_MapWidth(lua_State* tolua_S)
{
  CMapInfo* self = (CMapInfo*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MapWidth'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->MapWidth);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: MapWidth of class  CMapInfo */
#ifndef TOLUA_DISABLE_tolua_set_CMapInfo_MapWidth
static int tolua_set_CMapInfo_MapWidth(lua_State* tolua_S)
{
  CMapInfo* self = (CMapInfo*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MapWidth'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->MapWidth = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: MapHeight of class  CMapInfo */
#ifndef TOLUA_DISABLE_tolua_get_CMapInfo_MapHeight
static int tolua_get_CMapInfo_MapHeight(lua_State* tolua_S)
{
  CMapInfo* self = (CMapInfo*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MapHeight'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->MapHeight);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: MapHeight of class  CMapInfo */
#ifndef TOLUA_DISABLE_tolua_set_CMapInfo_MapHeight
static int tolua_set_CMapInfo_MapHeight(lua_State* tolua_S)
{
  CMapInfo* self = (CMapInfo*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'MapHeight'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->MapHeight = ((int)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: PlayerType of class  CMapInfo */
#ifndef TOLUA_DISABLE_tolua_get_stratagus_CMapInfo_PlayerType
static int tolua_get_stratagus_CMapInfo_PlayerType(lua_State* tolua_S)
{
 int tolua_index;
  CMapInfo* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (CMapInfo*)  lua_touserdata(tolua_S,-1);
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
 tolua_pushnumber(tolua_S,(lua_Number)self->PlayerType[tolua_index]);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: PlayerType of class  CMapInfo */
#ifndef TOLUA_DISABLE_tolua_set_stratagus_CMapInfo_PlayerType
static int tolua_set_stratagus_CMapInfo_PlayerType(lua_State* tolua_S)
{
 int tolua_index;
  CMapInfo* self;
 lua_pushstring(tolua_S,".self");
 lua_rawget(tolua_S,1);
 self = (CMapInfo*)  lua_touserdata(tolua_S,-1);
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
  self->PlayerType[tolua_index] = ((int)  tolua_tonumber(tolua_S,3,0));
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Info of class  CMap */
#ifndef TOLUA_DISABLE_tolua_get_CMap_Info
static int tolua_get_CMap_Info(lua_State* tolua_S)
{
  CMap* self = (CMap*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Info'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)&self->Info,"CMapInfo");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Info of class  CMap */
#ifndef TOLUA_DISABLE_tolua_set_CMap_Info
static int tolua_set_CMap_Info(lua_State* tolua_S)
{
  CMap* self = (CMap*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'Info'",NULL);
  if ((tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"CMapInfo",0,&tolua_err)))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->Info = *((CMapInfo*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: PatchManager of class  CMap */
#ifndef TOLUA_DISABLE_tolua_get_CMap_PatchManager
static int tolua_get_CMap_PatchManager(lua_State* tolua_S)
{
  CMap* self = (CMap*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'PatchManager'",NULL);
#endif
   tolua_pushusertype(tolua_S,(void*)&self->PatchManager,"CPatchManager");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: PatchManager of class  CMap */
#ifndef TOLUA_DISABLE_tolua_set_CMap_PatchManager
static int tolua_set_CMap_PatchManager(lua_State* tolua_S)
{
  CMap* self = (CMap*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'PatchManager'",NULL);
  if ((tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"CPatchManager",0,&tolua_err)))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->PatchManager = *((CPatchManager*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: Map */
#ifndef TOLUA_DISABLE_tolua_get_Map
static int tolua_get_Map(lua_State* tolua_S)
{
   tolua_pushusertype(tolua_S,(void*)&Map,"CMap");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: Map */
#ifndef TOLUA_DISABLE_tolua_set_Map
static int tolua_set_Map(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if ((tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"CMap",0,&tolua_err)))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  Map = *((CMap*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* method: getGraphic of class  CPatchType */
#ifndef TOLUA_DISABLE_tolua_stratagus_CPatchType_getGraphic00
static int tolua_stratagus_CPatchType_getGraphic00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const CPatchType",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const CPatchType* self = (const CPatchType*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getGraphic'", NULL);
#endif
  {
   const CGraphic* tolua_ret = (const CGraphic*)  self->getGraphic();
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"const CGraphic");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getGraphic'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getTileWidth of class  CPatchType */
#ifndef TOLUA_DISABLE_tolua_stratagus_CPatchType_getTileWidth00
static int tolua_stratagus_CPatchType_getTileWidth00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const CPatchType",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const CPatchType* self = (const CPatchType*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getTileWidth'", NULL);
#endif
  {
   int tolua_ret = (int)  self->getTileWidth();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getTileWidth'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getTileHeight of class  CPatchType */
#ifndef TOLUA_DISABLE_tolua_stratagus_CPatchType_getTileHeight00
static int tolua_stratagus_CPatchType_getTileHeight00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const CPatchType",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const CPatchType* self = (const CPatchType*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getTileHeight'", NULL);
#endif
  {
   int tolua_ret = (int)  self->getTileHeight();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getTileHeight'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getFlag of class  CPatchType */
#ifndef TOLUA_DISABLE_tolua_stratagus_CPatchType_getFlag00
static int tolua_stratagus_CPatchType_getFlag00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"CPatchType",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CPatchType* self = (CPatchType*)  tolua_tousertype(tolua_S,1,0);
  int x = ((int)  tolua_tonumber(tolua_S,2,0));
  int y = ((int)  tolua_tonumber(tolua_S,3,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getFlag'", NULL);
#endif
  {
   unsigned short tolua_ret = (unsigned short)  self->getFlag(x,y);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getFlag'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getType of class  CPatch */
#ifndef TOLUA_DISABLE_tolua_stratagus_CPatch_getType00
static int tolua_stratagus_CPatch_getType00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"CPatch",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CPatch* self = (CPatch*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getType'", NULL);
#endif
  {
   CPatchType* tolua_ret = (CPatchType*)  self->getType();
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"CPatchType");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getType'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setPos of class  CPatch */
#ifndef TOLUA_DISABLE_tolua_stratagus_CPatch_setPos00
static int tolua_stratagus_CPatch_setPos00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"CPatch",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CPatch* self = (CPatch*)  tolua_tousertype(tolua_S,1,0);
  int x = ((int)  tolua_tonumber(tolua_S,2,0));
  int y = ((int)  tolua_tonumber(tolua_S,3,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setPos'", NULL);
#endif
  {
   self->setPos(x,y);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'setPos'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: setX of class  CPatch */
#ifndef TOLUA_DISABLE_tolua_stratagus_CPatch_setX00
static int tolua_stratagus_CPatch_setX00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"CPatch",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CPatch* self = (CPatch*)  tolua_tousertype(tolua_S,1,0);
  int x = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setX'", NULL);
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

/* method: getX of class  CPatch */
#ifndef TOLUA_DISABLE_tolua_stratagus_CPatch_getX00
static int tolua_stratagus_CPatch_getX00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const CPatch",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const CPatch* self = (const CPatch*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getX'", NULL);
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

/* method: setY of class  CPatch */
#ifndef TOLUA_DISABLE_tolua_stratagus_CPatch_setY00
static int tolua_stratagus_CPatch_setY00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"CPatch",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CPatch* self = (CPatch*)  tolua_tousertype(tolua_S,1,0);
  int y = ((int)  tolua_tonumber(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'setY'", NULL);
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

/* method: getY of class  CPatch */
#ifndef TOLUA_DISABLE_tolua_stratagus_CPatch_getY00
static int tolua_stratagus_CPatch_getY00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const CPatch",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const CPatch* self = (const CPatch*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getY'", NULL);
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

/* method: add of class  CPatchManager */
#ifndef TOLUA_DISABLE_tolua_stratagus_CPatchManager_add00
static int tolua_stratagus_CPatchManager_add00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"CPatchManager",0,&tolua_err) ||
     !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CPatchManager* self = (CPatchManager*)  tolua_tousertype(tolua_S,1,0);
  std::string typeName = ((std::string)  tolua_tocppstring(tolua_S,2,0));
  int x = ((int)  tolua_tonumber(tolua_S,3,0));
  int y = ((int)  tolua_tonumber(tolua_S,4,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'add'", NULL);
#endif
  {
   CPatch* tolua_ret = (CPatch*)  self->add(typeName,x,y);
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"CPatch");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'add'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: moveToTop of class  CPatchManager */
#ifndef TOLUA_DISABLE_tolua_stratagus_CPatchManager_moveToTop00
static int tolua_stratagus_CPatchManager_moveToTop00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"CPatchManager",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"CPatch",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CPatchManager* self = (CPatchManager*)  tolua_tousertype(tolua_S,1,0);
  CPatch* patch = ((CPatch*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'moveToTop'", NULL);
#endif
  {
   self->moveToTop(patch);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'moveToTop'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: moveToBottom of class  CPatchManager */
#ifndef TOLUA_DISABLE_tolua_stratagus_CPatchManager_moveToBottom00
static int tolua_stratagus_CPatchManager_moveToBottom00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"CPatchManager",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"CPatch",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CPatchManager* self = (CPatchManager*)  tolua_tousertype(tolua_S,1,0);
  CPatch* patch = ((CPatch*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'moveToBottom'", NULL);
#endif
  {
   self->moveToBottom(patch);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'moveToBottom'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getPatch of class  CPatchManager */
#ifndef TOLUA_DISABLE_tolua_stratagus_CPatchManager_getPatch00
static int tolua_stratagus_CPatchManager_getPatch00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const CPatchManager",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const CPatchManager* self = (const CPatchManager*)  tolua_tousertype(tolua_S,1,0);
  int x = ((int)  tolua_tonumber(tolua_S,2,0));
  int y = ((int)  tolua_tonumber(tolua_S,3,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getPatch'", NULL);
#endif
  {
   CPatch* tolua_ret = (CPatch*)  self->getPatch(x,y);
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"CPatch");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getPatch'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getPatchTypeNames of class  CPatchManager */
#ifndef TOLUA_DISABLE_tolua_stratagus_CPatchManager_getPatchTypeNames00
static int tolua_stratagus_CPatchManager_getPatchTypeNames00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const CPatchManager",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const CPatchManager* self = (const CPatchManager*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getPatchTypeNames'", NULL);
#endif
  {
   vector<string> tolua_ret = (vector<string>)  self->getPatchTypeNames();
   {
#ifdef __cplusplus
    void* tolua_obj = Mtolua_new((vector<string>)(tolua_ret));
     tolua_pushusertype(tolua_S,tolua_obj,"vector<string>");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
#else
    void* tolua_obj = tolua_copy(tolua_S,(void*)&tolua_ret,sizeof(vector<string>));
     tolua_pushusertype(tolua_S,tolua_obj,"vector<string>");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
#endif
   }
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getPatchTypeNames'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: getPatchTypeNamesUsingGraphic of class  CPatchManager */
#ifndef TOLUA_DISABLE_tolua_stratagus_CPatchManager_getPatchTypeNamesUsingGraphic00
static int tolua_stratagus_CPatchManager_getPatchTypeNamesUsingGraphic00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const CPatchManager",0,&tolua_err) ||
     !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const CPatchManager* self = (const CPatchManager*)  tolua_tousertype(tolua_S,1,0);
  std::string graphicFile = ((std::string)  tolua_tocppstring(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'getPatchTypeNamesUsingGraphic'", NULL);
#endif
  {
   vector<string> tolua_ret = (vector<string>)  self->getPatchTypeNamesUsingGraphic(graphicFile);
   {
#ifdef __cplusplus
    void* tolua_obj = Mtolua_new((vector<string>)(tolua_ret));
     tolua_pushusertype(tolua_S,tolua_obj,"vector<string>");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
#else
    void* tolua_obj = tolua_copy(tolua_S,(void*)&tolua_ret,sizeof(vector<string>));
     tolua_pushusertype(tolua_S,tolua_obj,"vector<string>");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
#endif
   }
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'getPatchTypeNamesUsingGraphic'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: newPatchType of class  CPatchManager */
#ifndef TOLUA_DISABLE_tolua_stratagus_CPatchManager_newPatchType00
static int tolua_stratagus_CPatchManager_newPatchType00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"CPatchManager",0,&tolua_err) ||
     !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
     !tolua_iscppstring(tolua_S,3,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,5,0,&tolua_err) ||
     !tolua_istable(tolua_S,6,0,&tolua_err) ||
     !tolua_iscppstring(tolua_S,7,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,8,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CPatchManager* self = (CPatchManager*)  tolua_tousertype(tolua_S,1,0);
  std::string name = ((std::string)  tolua_tocppstring(tolua_S,2,0));
  std::string file = ((std::string)  tolua_tocppstring(tolua_S,3,0));
  int width = ((int)  tolua_tonumber(tolua_S,4,0));
  int height = ((int)  tolua_tonumber(tolua_S,5,0));
#ifdef __cplusplus
  int* flags = Mtolua_new_dim(int, width*height);
#else
  int* flags = (int*) malloc((width*height)*sizeof(int));
#endif
  std::string theme = ((std::string)  tolua_tocppstring(tolua_S,7,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'newPatchType'", NULL);
#endif
  {
#ifndef TOLUA_RELEASE
   if (!tolua_isnumberarray(tolua_S,6,width*height,0,&tolua_err))
    goto tolua_lerror;
   else
#endif
   {
    int i;
    for(i=0; i<width*height;i++)
    flags[i] = ((int)  tolua_tofieldnumber(tolua_S,6,i+1,0));
   }
  }
  {
   CPatchType* tolua_ret = (CPatchType*)  self->newPatchType(name,file,width,height,flags,theme);
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"CPatchType");
  }
  {
   int i;
   for(i=0; i<width*height;i++)
    tolua_pushfieldnumber(tolua_S,6,i+1,(lua_Number) flags[i]);
  }
#ifdef __cplusplus
  Mtolua_delete_dim(flags);
#else
  free(flags);
#endif
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'newPatchType'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: computePatchSize of class  CPatchManager */
#ifndef TOLUA_DISABLE_tolua_stratagus_CPatchManager_computePatchSize00
static int tolua_stratagus_CPatchManager_computePatchSize00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"const CPatchManager",0,&tolua_err) ||
     !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,1,&tolua_err) ||
     !tolua_isnumber(tolua_S,4,1,&tolua_err) ||
     !tolua_isnoobj(tolua_S,5,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const CPatchManager* self = (const CPatchManager*)  tolua_tousertype(tolua_S,1,0);
  std::string graphicFile = ((std::string)  tolua_tocppstring(tolua_S,2,0));
  int width = ((int)  tolua_tonumber(tolua_S,3,0));
  int height = ((int)  tolua_tonumber(tolua_S,4,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'computePatchSize'", NULL);
#endif
  {
   bool tolua_ret = (bool)  self->computePatchSize(graphicFile,&width,&height);
   tolua_pushboolean(tolua_S,(bool)tolua_ret);
   tolua_pushnumber(tolua_S,(lua_Number)width);
   tolua_pushnumber(tolua_S,(lua_Number)height);
  }
 }
 return 3;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'computePatchSize'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* get function: AStarFixedUnitCrossingCost */
#ifndef TOLUA_DISABLE_tolua_get_AStarFixedUnitCrossingCost
static int tolua_get_AStarFixedUnitCrossingCost(lua_State* tolua_S)
{
  tolua_pushnumber(tolua_S,(lua_Number)GetAStarFixedUnitCrossingCost());
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: AStarFixedUnitCrossingCost */
#ifndef TOLUA_DISABLE_tolua_set_AStarFixedUnitCrossingCost
static int tolua_set_AStarFixedUnitCrossingCost(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  SetAStarFixedUnitCrossingCost(((int)  tolua_tonumber(tolua_S,2,0))
)
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: AStarMovingUnitCrossingCost */
#ifndef TOLUA_DISABLE_tolua_get_AStarMovingUnitCrossingCost
static int tolua_get_AStarMovingUnitCrossingCost(lua_State* tolua_S)
{
  tolua_pushnumber(tolua_S,(lua_Number)GetAStarMovingUnitCrossingCost());
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: AStarMovingUnitCrossingCost */
#ifndef TOLUA_DISABLE_tolua_set_AStarMovingUnitCrossingCost
static int tolua_set_AStarMovingUnitCrossingCost(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  SetAStarMovingUnitCrossingCost(((int)  tolua_tonumber(tolua_S,2,0))
)
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: AStarKnowUnseenTerrain */
#ifndef TOLUA_DISABLE_tolua_get_AStarKnowUnseenTerrain
static int tolua_get_AStarKnowUnseenTerrain(lua_State* tolua_S)
{
  tolua_pushboolean(tolua_S,(bool)AStarKnowUnseenTerrain);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: AStarKnowUnseenTerrain */
#ifndef TOLUA_DISABLE_tolua_set_AStarKnowUnseenTerrain
static int tolua_set_AStarKnowUnseenTerrain(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_isboolean(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  AStarKnowUnseenTerrain = ((bool)  tolua_toboolean(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: AStarUnknownTerrainCost */
#ifndef TOLUA_DISABLE_tolua_get_AStarUnknownTerrainCost
static int tolua_get_AStarUnknownTerrainCost(lua_State* tolua_S)
{
  tolua_pushnumber(tolua_S,(lua_Number)GetAStarUnknownTerrainCost());
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: AStarUnknownTerrainCost */
#ifndef TOLUA_DISABLE_tolua_set_AStarUnknownTerrainCost
static int tolua_set_AStarUnknownTerrainCost(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  SetAStarUnknownTerrainCost(((int)  tolua_tonumber(tolua_S,2,0))
)
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* function: GetNumOpponents */
#ifndef TOLUA_DISABLE_tolua_stratagus_GetNumOpponents00
static int tolua_stratagus_GetNumOpponents00(lua_State* tolua_S)
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
  int player = ((int)  tolua_tonumber(tolua_S,1,0));
  {
   int tolua_ret = (int)  GetNumOpponents(player);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetNumOpponents'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: GetTimer */
#ifndef TOLUA_DISABLE_tolua_stratagus_GetTimer00
static int tolua_stratagus_GetTimer00(lua_State* tolua_S)
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
   int tolua_ret = (int)  GetTimer();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'GetTimer'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: ActionVictory */
#ifndef TOLUA_DISABLE_tolua_stratagus_ActionVictory00
static int tolua_stratagus_ActionVictory00(lua_State* tolua_S)
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
   ActionVictory();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'ActionVictory'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: ActionDefeat */
#ifndef TOLUA_DISABLE_tolua_stratagus_ActionDefeat00
static int tolua_stratagus_ActionDefeat00(lua_State* tolua_S)
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
   ActionDefeat();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'ActionDefeat'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: ActionDraw */
#ifndef TOLUA_DISABLE_tolua_stratagus_ActionDraw00
static int tolua_stratagus_ActionDraw00(lua_State* tolua_S)
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
   ActionDraw();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'ActionDraw'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: ActionSetTimer */
#ifndef TOLUA_DISABLE_tolua_stratagus_ActionSetTimer00
static int tolua_stratagus_ActionSetTimer00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
     !tolua_isboolean(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  int cycles = ((int)  tolua_tonumber(tolua_S,1,0));
  bool increasing = ((bool)  tolua_toboolean(tolua_S,2,0));
  {
   ActionSetTimer(cycles,increasing);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'ActionSetTimer'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: ActionStartTimer */
#ifndef TOLUA_DISABLE_tolua_stratagus_ActionStartTimer00
static int tolua_stratagus_ActionStartTimer00(lua_State* tolua_S)
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
   ActionStartTimer();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'ActionStartTimer'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: ActionStopTimer */
#ifndef TOLUA_DISABLE_tolua_stratagus_ActionStopTimer00
static int tolua_stratagus_ActionStopTimer00(lua_State* tolua_S)
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
   ActionStopTimer();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'ActionStopTimer'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: SetTrigger */
#ifndef TOLUA_DISABLE_tolua_stratagus_SetTrigger00
static int tolua_stratagus_SetTrigger00(lua_State* tolua_S)
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
  int trigger = ((int)  tolua_tonumber(tolua_S,1,0));
  {
   SetTrigger(trigger);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetTrigger'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  CPosition */
#ifndef TOLUA_DISABLE_tolua_stratagus_CPosition_new00
static int tolua_stratagus_CPosition_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"CPosition",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  int x = ((int)  tolua_tonumber(tolua_S,2,0));
  int y = ((int)  tolua_tonumber(tolua_S,3,0));
  {
   CPosition* tolua_ret = (CPosition*)  Mtolua_new((CPosition)(x,y));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"CPosition");
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

/* method: new_local of class  CPosition */
#ifndef TOLUA_DISABLE_tolua_stratagus_CPosition_new00_local
static int tolua_stratagus_CPosition_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"CPosition",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,2,0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  int x = ((int)  tolua_tonumber(tolua_S,2,0));
  int y = ((int)  tolua_tonumber(tolua_S,3,0));
  {
   CPosition* tolua_ret = (CPosition*)  Mtolua_new((CPosition)(x,y));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"CPosition");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
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

/* get function: x of class  CPosition */
#ifndef TOLUA_DISABLE_tolua_get_CPosition_x
static int tolua_get_CPosition_x(lua_State* tolua_S)
{
  CPosition* self = (CPosition*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'x'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->x);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: x of class  CPosition */
#ifndef TOLUA_DISABLE_tolua_set_CPosition_x
static int tolua_set_CPosition_x(lua_State* tolua_S)
{
  CPosition* self = (CPosition*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'x'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->x = ((float)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: y of class  CPosition */
#ifndef TOLUA_DISABLE_tolua_get_CPosition_y
static int tolua_get_CPosition_y(lua_State* tolua_S)
{
  CPosition* self = (CPosition*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'y'",NULL);
#endif
  tolua_pushnumber(tolua_S,(lua_Number)self->y);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: y of class  CPosition */
#ifndef TOLUA_DISABLE_tolua_set_CPosition_y
static int tolua_set_CPosition_y(lua_State* tolua_S)
{
  CPosition* self = (CPosition*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!self) tolua_error(tolua_S,"invalid 'self' in accessing variable 'y'",NULL);
  if (!tolua_isnumber(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  self->y = ((float)  tolua_tonumber(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  GraphicAnimation */
#ifndef TOLUA_DISABLE_tolua_stratagus_GraphicAnimation_new00
static int tolua_stratagus_GraphicAnimation_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"GraphicAnimation",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"CGraphic",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CGraphic* g = ((CGraphic*)  tolua_tousertype(tolua_S,2,0));
  int ticksPerFrame = ((int)  tolua_tonumber(tolua_S,3,0));
  {
   GraphicAnimation* tolua_ret = (GraphicAnimation*)  Mtolua_new((GraphicAnimation)(g,ticksPerFrame));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"GraphicAnimation");
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

/* method: new_local of class  GraphicAnimation */
#ifndef TOLUA_DISABLE_tolua_stratagus_GraphicAnimation_new00_local
static int tolua_stratagus_GraphicAnimation_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"GraphicAnimation",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"CGraphic",0,&tolua_err) ||
     !tolua_isnumber(tolua_S,3,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CGraphic* g = ((CGraphic*)  tolua_tousertype(tolua_S,2,0));
  int ticksPerFrame = ((int)  tolua_tonumber(tolua_S,3,0));
  {
   GraphicAnimation* tolua_ret = (GraphicAnimation*)  Mtolua_new((GraphicAnimation)(g,ticksPerFrame));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"GraphicAnimation");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
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

/* method: clone of class  GraphicAnimation */
#ifndef TOLUA_DISABLE_tolua_stratagus_GraphicAnimation_clone00
static int tolua_stratagus_GraphicAnimation_clone00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"GraphicAnimation",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  GraphicAnimation* self = (GraphicAnimation*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'clone'", NULL);
#endif
  {
   Animation* tolua_ret = (Animation*)  self->clone();
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"Animation");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'clone'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: clone of class  CParticle */
#ifndef TOLUA_DISABLE_tolua_stratagus_CParticle_clone00
static int tolua_stratagus_CParticle_clone00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"CParticle",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CParticle* self = (CParticle*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'clone'", NULL);
#endif
  {
   CParticle* tolua_ret = (CParticle*)  self->clone();
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"CParticle");
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'clone'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: new of class  StaticParticle */
#ifndef TOLUA_DISABLE_tolua_stratagus_StaticParticle_new00
static int tolua_stratagus_StaticParticle_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"StaticParticle",0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"CPosition",0,&tolua_err)) ||
     !tolua_isusertype(tolua_S,3,"Animation",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CPosition position = *((CPosition*)  tolua_tousertype(tolua_S,2,0));
  Animation* animation = ((Animation*)  tolua_tousertype(tolua_S,3,0));
  {
   StaticParticle* tolua_ret = (StaticParticle*)  Mtolua_new((StaticParticle)(position,animation));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"StaticParticle");
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

/* method: new_local of class  StaticParticle */
#ifndef TOLUA_DISABLE_tolua_stratagus_StaticParticle_new00_local
static int tolua_stratagus_StaticParticle_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"StaticParticle",0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"CPosition",0,&tolua_err)) ||
     !tolua_isusertype(tolua_S,3,"Animation",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CPosition position = *((CPosition*)  tolua_tousertype(tolua_S,2,0));
  Animation* animation = ((Animation*)  tolua_tousertype(tolua_S,3,0));
  {
   StaticParticle* tolua_ret = (StaticParticle*)  Mtolua_new((StaticParticle)(position,animation));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"StaticParticle");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
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

/* method: new of class  CChunkParticle */
#ifndef TOLUA_DISABLE_tolua_stratagus_CChunkParticle_new00
static int tolua_stratagus_CChunkParticle_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"CChunkParticle",0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"CPosition",0,&tolua_err)) ||
     !tolua_isusertype(tolua_S,3,"Animation",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CPosition position = *((CPosition*)  tolua_tousertype(tolua_S,2,0));
  Animation* smokeAnimation = ((Animation*)  tolua_tousertype(tolua_S,3,0));
  {
   CChunkParticle* tolua_ret = (CChunkParticle*)  Mtolua_new((CChunkParticle)(position,smokeAnimation));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"CChunkParticle");
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

/* method: new_local of class  CChunkParticle */
#ifndef TOLUA_DISABLE_tolua_stratagus_CChunkParticle_new00_local
static int tolua_stratagus_CChunkParticle_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"CChunkParticle",0,&tolua_err) ||
     (tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"CPosition",0,&tolua_err)) ||
     !tolua_isusertype(tolua_S,3,"Animation",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,4,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CPosition position = *((CPosition*)  tolua_tousertype(tolua_S,2,0));
  Animation* smokeAnimation = ((Animation*)  tolua_tousertype(tolua_S,3,0));
  {
   CChunkParticle* tolua_ret = (CChunkParticle*)  Mtolua_new((CChunkParticle)(position,smokeAnimation));
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"CChunkParticle");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
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

/* method: new of class  CParticleManager */
#ifndef TOLUA_DISABLE_tolua_stratagus_CParticleManager_new00
static int tolua_stratagus_CParticleManager_new00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"CParticleManager",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   CParticleManager* tolua_ret = (CParticleManager*)  Mtolua_new((CParticleManager)());
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"CParticleManager");
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

/* method: new_local of class  CParticleManager */
#ifndef TOLUA_DISABLE_tolua_stratagus_CParticleManager_new00_local
static int tolua_stratagus_CParticleManager_new00_local(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertable(tolua_S,1,"CParticleManager",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   CParticleManager* tolua_ret = (CParticleManager*)  Mtolua_new((CParticleManager)());
    tolua_pushusertype(tolua_S,(void*)tolua_ret,"CParticleManager");
    tolua_register_gc(tolua_S,lua_gettop(tolua_S));
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

/* method: delete of class  CParticleManager */
#ifndef TOLUA_DISABLE_tolua_stratagus_CParticleManager_delete00
static int tolua_stratagus_CParticleManager_delete00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"CParticleManager",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CParticleManager* self = (CParticleManager*)  tolua_tousertype(tolua_S,1,0);
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'delete'", NULL);
#endif
  Mtolua_delete(self);
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'delete'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* method: add of class  CParticleManager */
#ifndef TOLUA_DISABLE_tolua_stratagus_CParticleManager_add00
static int tolua_stratagus_CParticleManager_add00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isusertype(tolua_S,1,"CParticleManager",0,&tolua_err) ||
     !tolua_isusertype(tolua_S,2,"CParticle",0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  CParticleManager* self = (CParticleManager*)  tolua_tousertype(tolua_S,1,0);
  CParticle* particle = ((CParticle*)  tolua_tousertype(tolua_S,2,0));
#ifndef TOLUA_RELEASE
  if (!self) tolua_error(tolua_S,"invalid 'self' in function 'add'", NULL);
#endif
  {
   self->add(particle);
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

/* get function: ParticleManager */
#ifndef TOLUA_DISABLE_tolua_get_ParticleManager
static int tolua_get_ParticleManager(lua_State* tolua_S)
{
   tolua_pushusertype(tolua_S,(void*)&ParticleManager,"CParticleManager");
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: ParticleManager */
#ifndef TOLUA_DISABLE_tolua_set_ParticleManager
static int tolua_set_ParticleManager(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if ((tolua_isvaluenil(tolua_S,2,&tolua_err) || !tolua_isusertype(tolua_S,2,"CParticleManager",0,&tolua_err)))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  ParticleManager = *((CParticleManager*)  tolua_tousertype(tolua_S,2,0))
;
 return 0;
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
     !tolua_iscppstring(tolua_S,1,0,&tolua_err) ||
     !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const string str1 = ((const string)  tolua_tocppstring(tolua_S,1,0));
  const string str2 = ((const string)  tolua_tocppstring(tolua_S,2,0));
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
     !tolua_iscppstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const string file = ((const string)  tolua_tocppstring(tolua_S,1,0));
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

/* function: SetTranslationsFiles */
#ifndef TOLUA_DISABLE_tolua_stratagus_SetTranslationsFiles00
static int tolua_stratagus_SetTranslationsFiles00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_iscppstring(tolua_S,1,0,&tolua_err) ||
     !tolua_iscppstring(tolua_S,2,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,3,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const string stratagusfile = ((const string)  tolua_tocppstring(tolua_S,1,0));
  const string gamefile = ((const string)  tolua_tocppstring(tolua_S,2,0));
  {
   SetTranslationsFiles(stratagusfile,gamefile);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SetTranslationsFiles'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* get function: StratagusTranslation */
#ifndef TOLUA_DISABLE_tolua_get_StratagusTranslation
static int tolua_get_StratagusTranslation(lua_State* tolua_S)
{
  tolua_pushcppstring(tolua_S,(const char*)StratagusTranslation);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: StratagusTranslation */
#ifndef TOLUA_DISABLE_tolua_set_StratagusTranslation
static int tolua_set_StratagusTranslation(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_iscppstring(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  StratagusTranslation = ((std::string)  tolua_tocppstring(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* get function: GameTranslation */
#ifndef TOLUA_DISABLE_tolua_get_GameTranslation
static int tolua_get_GameTranslation(lua_State* tolua_S)
{
  tolua_pushcppstring(tolua_S,(const char*)GameTranslation);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: GameTranslation */
#ifndef TOLUA_DISABLE_tolua_set_GameTranslation
static int tolua_set_GameTranslation(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_iscppstring(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  GameTranslation = ((std::string)  tolua_tocppstring(tolua_S,2,0))
;
 return 0;
}
#endif //#ifndef TOLUA_DISABLE

/* function: SaveGame */
#ifndef TOLUA_DISABLE_tolua_stratagus_SaveGame00
static int tolua_stratagus_SaveGame00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_iscppstring(tolua_S,1,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  const std::string filename = ((const std::string)  tolua_tocppstring(tolua_S,1,0));
  {
   SaveGame(filename);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SaveGame'.",&tolua_err);
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

/* function: SyncRand */
#ifndef TOLUA_DISABLE_tolua_stratagus_SyncRand00
static int tolua_stratagus_SyncRand00(lua_State* tolua_S)
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
   int tolua_ret = (int)  SyncRand();
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'SyncRand'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: SyncRand */
#ifndef TOLUA_DISABLE_tolua_stratagus_SyncRand01
static int tolua_stratagus_SyncRand01(lua_State* tolua_S)
{
 tolua_Error tolua_err;
 if (
     !tolua_isnumber(tolua_S,1,0,&tolua_err) ||
     !tolua_isnoobj(tolua_S,2,&tolua_err)
 )
  goto tolua_lerror;
 else
 {
  int max = ((int)  tolua_tonumber(tolua_S,1,0));
  {
   int tolua_ret = (int)  SyncRand(max);
   tolua_pushnumber(tolua_S,(lua_Number)tolua_ret);
  }
 }
 return 1;
tolua_lerror:
 return tolua_stratagus_SyncRand00(tolua_S);
}
#endif //#ifndef TOLUA_DISABLE

/* function: Exit */
#ifndef TOLUA_DISABLE_tolua_stratagus_Exit00
static int tolua_stratagus_Exit00(lua_State* tolua_S)
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
  int err = ((int)  tolua_tonumber(tolua_S,1,0));
  {
   Exit(err);
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Exit'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* get function: CliMapName */
#ifndef TOLUA_DISABLE_tolua_get_CliMapName
static int tolua_get_CliMapName(lua_State* tolua_S)
{
  tolua_pushcppstring(tolua_S,(const char*)CliMapName);
 return 1;
}
#endif //#ifndef TOLUA_DISABLE

/* set function: CliMapName */
#ifndef TOLUA_DISABLE_tolua_set_CliMapName
static int tolua_set_CliMapName(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
  tolua_Error tolua_err;
  if (!tolua_iscppstring(tolua_S,2,0,&tolua_err))
   tolua_error(tolua_S,"#vinvalid type in variable assignment.",&tolua_err);
#endif
  CliMapName = ((std::string)  tolua_tocppstring(tolua_S,2,0))
;
 return 0;
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
   int top = lua_gettop(tolua_S);
   static const unsigned char B[] = {
    10,109,116, 32, 61, 32,123, 32, 95, 95,105,110,100,101,120,
     32, 61, 32,102,117,110, 99,116,105,111,110, 40,116, 44, 32,
    107,101,121, 41, 32,114,101,116,117,114,110, 32, 67, 73, 99,
    111,110, 58, 71,101,116, 40,107,101,121, 41, 32,101,110,100,
     32,125, 10, 73, 99,111,110,115, 32, 61, 32,123,125, 10,115,
    101,116,109,101,116, 97,116, 97, 98,108,101, 40, 73, 99,111,
    110,115, 44, 32,109,116, 41, 10,109,116, 32, 61, 32,123, 32,
     95, 95,105,110,100,101,120, 32, 61, 32,102,117,110, 99,116,
    105,111,110, 40,116, 44, 32,107,101,121, 41, 32,114,101,116,
    117,114,110, 32, 67, 70,111,110,116, 58, 71,101,116, 40,107,
    101,121, 41, 32,101,110,100, 32,125, 10, 70,111,110,116,115,
     32, 61, 32,123,125, 10,115,101,116,109,101,116, 97,116, 97,
     98,108,101, 40, 70,111,110,116,115, 44, 32,109,116, 41, 10,
    109,116, 32, 61, 32,123, 32, 95, 95,105,110,100,101,120, 32,
     61, 32,102,117,110, 99,116,105,111,110, 40,116, 44, 32,107,
    101,121, 41, 32,114,101,116,117,114,110, 32, 67, 70,111,110,
    116, 67,111,108,111,114, 58, 71,101,116, 40,107,101,121, 41,
     32,101,110,100, 32,125, 10, 70,111,110,116, 67,111,108,111,
    114,115, 32, 61, 32,123,125, 10,115,101,116,109,101,116, 97,
    116, 97, 98,108,101, 40, 70,111,110,116, 67,111,108,111,114,
    115, 44, 32,109,116, 41, 10,102,117,110, 99,116,105,111,110,
     32, 71, 97,109,101, 83,116, 97,114,116,105,110,103, 40, 41,
     10,101,110,100, 45, 45,32
   };
   tolua_dobuffer(tolua_S,(char*)B,sizeof(B),"tolua: embedded Lua code 1");
   lua_settop(tolua_S, top);
  } /* end of embedded lua code */

  tolua_constant(tolua_S,"MaxCosts",MaxCosts);
  tolua_constant(tolua_S,"EnergyCost",EnergyCost);
  tolua_constant(tolua_S,"MagmaCost",MagmaCost);
  tolua_constant(tolua_S,"PlayerMax",PlayerMax);
  tolua_constant(tolua_S,"PlayerNumNeutral",PlayerNumNeutral);
  tolua_constant(tolua_S,"UnitMax",UnitMax);
  tolua_constant(tolua_S,"NoButton",NoButton);
  tolua_constant(tolua_S,"LeftButton",LeftButton);
  tolua_constant(tolua_S,"MiddleButton",MiddleButton);
  tolua_constant(tolua_S,"RightButton",RightButton);
  tolua_constant(tolua_S,"UpButton",UpButton);
  tolua_constant(tolua_S,"DownButton",DownButton);
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
  #ifdef __cplusplus
  tolua_cclass(tolua_S,"CUIButton","CUIButton","",tolua_collect_CUIButton);
  #else
  tolua_cclass(tolua_S,"CUIButton","CUIButton","",NULL);
  #endif
  tolua_beginmodule(tolua_S,"CUIButton");
   tolua_function(tolua_S,"new",tolua_stratagus_CUIButton_new00);
   tolua_function(tolua_S,"new_local",tolua_stratagus_CUIButton_new00_local);
   tolua_function(tolua_S,".call",tolua_stratagus_CUIButton_new00_local);
   tolua_function(tolua_S,"delete",tolua_stratagus_CUIButton_delete00);
   tolua_variable(tolua_S,"X",tolua_get_CUIButton_X,tolua_set_CUIButton_X);
   tolua_variable(tolua_S,"Y",tolua_get_CUIButton_Y,tolua_set_CUIButton_Y);
   tolua_variable(tolua_S,"Text",tolua_get_CUIButton_Text,tolua_set_CUIButton_Text);
   tolua_variable(tolua_S,"Style",tolua_get_CUIButton_Style_ptr,tolua_set_CUIButton_Style_ptr);
   tolua_variable(tolua_S,"Callback",tolua_get_CUIButton_Callback_ptr,tolua_set_CUIButton_Callback_ptr);
  tolua_endmodule(tolua_S);

  { /* begin embedded lua code */
   int top = lua_gettop(tolua_S);
   static const unsigned char B[] = {
    10, 95,117,105, 66,117,116,116,111,110, 67, 97,108,108, 98,
     97, 99,107,115, 32, 61, 32,123,125, 10, 67, 85, 73, 66,117,
    116,116,111,110, 46, 83,101,116, 67, 97,108,108, 98, 97, 99,
    107, 32, 61, 32,102,117,110, 99,116,105,111,110, 40,119, 44,
     32,102, 41, 10,119, 46, 67, 97,108,108, 98, 97, 99,107, 32,
     61, 32, 76,117, 97, 65, 99,116,105,111,110, 76,105,115,116,
    101,110,101,114, 40,102, 41, 10,116, 97, 98,108,101, 46,105,
    110,115,101,114,116, 40, 95,117,105, 66,117,116,116,111,110,
     67, 97,108,108, 98, 97, 99,107,115, 44, 32,119, 46, 67, 97,
    108,108, 98, 97, 99,107, 41, 10,101,110,100, 45, 45, 45, 45,
     45, 45, 45, 45, 45, 45, 45, 45, 45, 45,32
   };
   tolua_dobuffer(tolua_S,(char*)B,sizeof(B),"tolua: embedded Lua code 2");
   lua_settop(tolua_S, top);
  } /* end of embedded lua code */

  tolua_cclass(tolua_S,"CMapArea","CMapArea","",NULL);
  tolua_beginmodule(tolua_S,"CMapArea");
   tolua_variable(tolua_S,"X",tolua_get_CMapArea_X,tolua_set_CMapArea_X);
   tolua_variable(tolua_S,"Y",tolua_get_CMapArea_Y,tolua_set_CMapArea_Y);
   tolua_variable(tolua_S,"EndX",tolua_get_CMapArea_EndX,tolua_set_CMapArea_EndX);
   tolua_variable(tolua_S,"EndY",tolua_get_CMapArea_EndY,tolua_set_CMapArea_EndY);
   tolua_variable(tolua_S,"ScrollPaddingLeft",tolua_get_CMapArea_ScrollPaddingLeft,tolua_set_CMapArea_ScrollPaddingLeft);
   tolua_variable(tolua_S,"ScrollPaddingRight",tolua_get_CMapArea_ScrollPaddingRight,tolua_set_CMapArea_ScrollPaddingRight);
   tolua_variable(tolua_S,"ScrollPaddingTop",tolua_get_CMapArea_ScrollPaddingTop,tolua_set_CMapArea_ScrollPaddingTop);
   tolua_variable(tolua_S,"ScrollPaddingBottom",tolua_get_CMapArea_ScrollPaddingBottom,tolua_set_CMapArea_ScrollPaddingBottom);
  tolua_endmodule(tolua_S);
  tolua_cclass(tolua_S,"CViewport","CViewport","",NULL);
  tolua_beginmodule(tolua_S,"CViewport");
   tolua_function(tolua_S,"Viewport2MapX",tolua_stratagus_CViewport_Viewport2MapX00);
   tolua_function(tolua_S,"Viewport2MapY",tolua_stratagus_CViewport_Viewport2MapY00);
  tolua_endmodule(tolua_S);
  #ifdef __cplusplus
  tolua_cclass(tolua_S,"CFiller","CFiller","",tolua_collect_CFiller);
  #else
  tolua_cclass(tolua_S,"CFiller","CFiller","",NULL);
  #endif
  tolua_beginmodule(tolua_S,"CFiller");
   tolua_function(tolua_S,"new",tolua_stratagus_CFiller_new00);
   tolua_function(tolua_S,"new_local",tolua_stratagus_CFiller_new00_local);
   tolua_function(tolua_S,".call",tolua_stratagus_CFiller_new00_local);
   tolua_variable(tolua_S,"G",tolua_get_CFiller_G_ptr,tolua_set_CFiller_G_ptr);
   tolua_variable(tolua_S,"X",tolua_get_CFiller_X,tolua_set_CFiller_X);
   tolua_variable(tolua_S,"Y",tolua_get_CFiller_Y,tolua_set_CFiller_Y);
  tolua_endmodule(tolua_S);
  #ifdef __cplusplus
  tolua_cclass(tolua_S,"vector_CFiller_","vector<CFiller>","",tolua_collect_vector_CFiller_);
  #else
  tolua_cclass(tolua_S,"vector_CFiller_","vector<CFiller>","",NULL);
  #endif
  tolua_beginmodule(tolua_S,"vector_CFiller_");
   tolua_function(tolua_S,"new",tolua_stratagus_vector_CFiller__new00);
   tolua_function(tolua_S,"new_local",tolua_stratagus_vector_CFiller__new00_local);
   tolua_function(tolua_S,".call",tolua_stratagus_vector_CFiller__new00_local);
   tolua_function(tolua_S,"delete",tolua_stratagus_vector_CFiller__delete00);
   tolua_function(tolua_S,".geti",tolua_stratagus_vector_CFiller___geti00);
   tolua_function(tolua_S,".seti",tolua_stratagus_vector_CFiller___seti00);
   tolua_function(tolua_S,".geti",tolua_stratagus_vector_CFiller___geti01);
   tolua_function(tolua_S,"at",tolua_stratagus_vector_CFiller__at00);
   tolua_function(tolua_S,"at",tolua_stratagus_vector_CFiller__at01);
   tolua_function(tolua_S,"front",tolua_stratagus_vector_CFiller__front00);
   tolua_function(tolua_S,"front",tolua_stratagus_vector_CFiller__front01);
   tolua_function(tolua_S,"back",tolua_stratagus_vector_CFiller__back00);
   tolua_function(tolua_S,"back",tolua_stratagus_vector_CFiller__back01);
   tolua_function(tolua_S,"push_back",tolua_stratagus_vector_CFiller__push_back00);
   tolua_function(tolua_S,"pop_back",tolua_stratagus_vector_CFiller__pop_back00);
   tolua_function(tolua_S,"assign",tolua_stratagus_vector_CFiller__assign00);
   tolua_function(tolua_S,"clear",tolua_stratagus_vector_CFiller__clear00);
   tolua_function(tolua_S,"empty",tolua_stratagus_vector_CFiller__empty00);
   tolua_function(tolua_S,"size",tolua_stratagus_vector_CFiller__size00);
  tolua_endmodule(tolua_S);
  #ifdef __cplusplus
  tolua_cclass(tolua_S,"vector_CUIButton_","vector<CUIButton>","",tolua_collect_vector_CUIButton_);
  #else
  tolua_cclass(tolua_S,"vector_CUIButton_","vector<CUIButton>","",NULL);
  #endif
  tolua_beginmodule(tolua_S,"vector_CUIButton_");
   tolua_function(tolua_S,"new",tolua_stratagus_vector_CUIButton__new00);
   tolua_function(tolua_S,"new_local",tolua_stratagus_vector_CUIButton__new00_local);
   tolua_function(tolua_S,".call",tolua_stratagus_vector_CUIButton__new00_local);
   tolua_function(tolua_S,"delete",tolua_stratagus_vector_CUIButton__delete00);
   tolua_function(tolua_S,".geti",tolua_stratagus_vector_CUIButton___geti00);
   tolua_function(tolua_S,".seti",tolua_stratagus_vector_CUIButton___seti00);
   tolua_function(tolua_S,".geti",tolua_stratagus_vector_CUIButton___geti01);
   tolua_function(tolua_S,"at",tolua_stratagus_vector_CUIButton__at00);
   tolua_function(tolua_S,"at",tolua_stratagus_vector_CUIButton__at01);
   tolua_function(tolua_S,"front",tolua_stratagus_vector_CUIButton__front00);
   tolua_function(tolua_S,"front",tolua_stratagus_vector_CUIButton__front01);
   tolua_function(tolua_S,"back",tolua_stratagus_vector_CUIButton__back00);
   tolua_function(tolua_S,"back",tolua_stratagus_vector_CUIButton__back01);
   tolua_function(tolua_S,"push_back",tolua_stratagus_vector_CUIButton__push_back00);
   tolua_function(tolua_S,"pop_back",tolua_stratagus_vector_CUIButton__pop_back00);
   tolua_function(tolua_S,"assign",tolua_stratagus_vector_CUIButton__assign00);
   tolua_function(tolua_S,"clear",tolua_stratagus_vector_CUIButton__clear00);
   tolua_function(tolua_S,"empty",tolua_stratagus_vector_CUIButton__empty00);
   tolua_function(tolua_S,"size",tolua_stratagus_vector_CUIButton__size00);
  tolua_endmodule(tolua_S);
  #ifdef __cplusplus
  tolua_cclass(tolua_S,"vector_string_","vector<string>","",tolua_collect_vector_string_);
  #else
  tolua_cclass(tolua_S,"vector_string_","vector<string>","",NULL);
  #endif
  tolua_beginmodule(tolua_S,"vector_string_");
   tolua_function(tolua_S,"new",tolua_stratagus_vector_string__new00);
   tolua_function(tolua_S,"new_local",tolua_stratagus_vector_string__new00_local);
   tolua_function(tolua_S,".call",tolua_stratagus_vector_string__new00_local);
   tolua_function(tolua_S,"delete",tolua_stratagus_vector_string__delete00);
   tolua_function(tolua_S,".geti",tolua_stratagus_vector_string___geti00);
   tolua_function(tolua_S,".seti",tolua_stratagus_vector_string___seti00);
   tolua_function(tolua_S,".geti",tolua_stratagus_vector_string___geti01);
   tolua_function(tolua_S,"at",tolua_stratagus_vector_string__at00);
   tolua_function(tolua_S,"at",tolua_stratagus_vector_string__at01);
   tolua_function(tolua_S,"front",tolua_stratagus_vector_string__front00);
   tolua_function(tolua_S,"front",tolua_stratagus_vector_string__front01);
   tolua_function(tolua_S,"back",tolua_stratagus_vector_string__back00);
   tolua_function(tolua_S,"back",tolua_stratagus_vector_string__back01);
   tolua_function(tolua_S,"push_back",tolua_stratagus_vector_string__push_back00);
   tolua_function(tolua_S,"pop_back",tolua_stratagus_vector_string__pop_back00);
   tolua_function(tolua_S,"assign",tolua_stratagus_vector_string__assign00);
   tolua_function(tolua_S,"clear",tolua_stratagus_vector_string__clear00);
   tolua_function(tolua_S,"empty",tolua_stratagus_vector_string__empty00);
   tolua_function(tolua_S,"size",tolua_stratagus_vector_string__size00);
  tolua_endmodule(tolua_S);
  tolua_cclass(tolua_S,"CButtonPanel","CButtonPanel","",NULL);
  tolua_beginmodule(tolua_S,"CButtonPanel");
   tolua_variable(tolua_S,"X",tolua_get_CButtonPanel_X,tolua_set_CButtonPanel_X);
   tolua_variable(tolua_S,"Y",tolua_get_CButtonPanel_Y,tolua_set_CButtonPanel_Y);
   tolua_variable(tolua_S,"Buttons",tolua_get_CButtonPanel_Buttons,tolua_set_CButtonPanel_Buttons);
   tolua_variable(tolua_S,"AutoCastBorderColorRGB",tolua_get_CButtonPanel_AutoCastBorderColorRGB,tolua_set_CButtonPanel_AutoCastBorderColorRGB);
   tolua_variable(tolua_S,"ShowCommandKey",tolua_get_CButtonPanel_ShowCommandKey,tolua_set_CButtonPanel_ShowCommandKey);
  tolua_endmodule(tolua_S);
  tolua_cclass(tolua_S,"CPieMenu","CPieMenu","",NULL);
  tolua_beginmodule(tolua_S,"CPieMenu");
   tolua_variable(tolua_S,"G",tolua_get_CPieMenu_G_ptr,tolua_set_CPieMenu_G_ptr);
   tolua_variable(tolua_S,"MouseButton",tolua_get_CPieMenu_MouseButton,tolua_set_CPieMenu_MouseButton);
   tolua_array(tolua_S,"X",tolua_get_stratagus_CPieMenu_X,tolua_set_stratagus_CPieMenu_X);
   tolua_array(tolua_S,"Y",tolua_get_stratagus_CPieMenu_Y,tolua_set_stratagus_CPieMenu_Y);
   tolua_function(tolua_S,"SetRadius",tolua_stratagus_CPieMenu_SetRadius00);
  tolua_endmodule(tolua_S);
  #ifdef __cplusplus
  tolua_cclass(tolua_S,"CResourceInfo","CResourceInfo","",tolua_collect_CResourceInfo);
  #else
  tolua_cclass(tolua_S,"CResourceInfo","CResourceInfo","",NULL);
  #endif
  tolua_beginmodule(tolua_S,"CResourceInfo");
   tolua_variable(tolua_S,"G",tolua_get_CResourceInfo_G_ptr,tolua_set_CResourceInfo_G_ptr);
   tolua_variable(tolua_S,"IconFrame",tolua_get_CResourceInfo_IconFrame,tolua_set_CResourceInfo_IconFrame);
   tolua_variable(tolua_S,"IconX",tolua_get_CResourceInfo_IconX,tolua_set_CResourceInfo_IconX);
   tolua_variable(tolua_S,"IconY",tolua_get_CResourceInfo_IconY,tolua_set_CResourceInfo_IconY);
   tolua_variable(tolua_S,"TextX",tolua_get_CResourceInfo_TextX,tolua_set_CResourceInfo_TextX);
   tolua_variable(tolua_S,"TextY",tolua_get_CResourceInfo_TextY,tolua_set_CResourceInfo_TextY);
  tolua_endmodule(tolua_S);
  tolua_cclass(tolua_S,"CInfoPanel","CInfoPanel","",NULL);
  tolua_beginmodule(tolua_S,"CInfoPanel");
   tolua_variable(tolua_S,"X",tolua_get_CInfoPanel_X,tolua_set_CInfoPanel_X);
   tolua_variable(tolua_S,"Y",tolua_get_CInfoPanel_Y,tolua_set_CInfoPanel_Y);
  tolua_endmodule(tolua_S);
  tolua_cclass(tolua_S,"CStatusLine","CStatusLine","",NULL);
  tolua_beginmodule(tolua_S,"CStatusLine");
   tolua_function(tolua_S,"Set",tolua_stratagus_CStatusLine_Set00);
   tolua_function(tolua_S,"Get",tolua_stratagus_CStatusLine_Get00);
   tolua_function(tolua_S,"Clear",tolua_stratagus_CStatusLine_Clear00);
   tolua_variable(tolua_S,"Width",tolua_get_CStatusLine_Width,tolua_set_CStatusLine_Width);
   tolua_variable(tolua_S,"TextX",tolua_get_CStatusLine_TextX,tolua_set_CStatusLine_TextX);
   tolua_variable(tolua_S,"TextY",tolua_get_CStatusLine_TextY,tolua_set_CStatusLine_TextY);
   tolua_variable(tolua_S,"Font",tolua_get_CStatusLine_Font_ptr,tolua_set_CStatusLine_Font_ptr);
  tolua_endmodule(tolua_S);
  tolua_cclass(tolua_S,"CUITimer","CUITimer","",NULL);
  tolua_beginmodule(tolua_S,"CUITimer");
   tolua_variable(tolua_S,"X",tolua_get_CUITimer_X,tolua_set_CUITimer_X);
   tolua_variable(tolua_S,"Y",tolua_get_CUITimer_Y,tolua_set_CUITimer_Y);
   tolua_variable(tolua_S,"Font",tolua_get_CUITimer_Font_ptr,tolua_set_CUITimer_Font_ptr);
  tolua_endmodule(tolua_S);
  tolua_cclass(tolua_S,"CUserInterface","CUserInterface","",NULL);
  tolua_beginmodule(tolua_S,"CUserInterface");
   tolua_variable(tolua_S,"NormalFontColor",tolua_get_CUserInterface_NormalFontColor,tolua_set_CUserInterface_NormalFontColor);
   tolua_variable(tolua_S,"ReverseFontColor",tolua_get_CUserInterface_ReverseFontColor,tolua_set_CUserInterface_ReverseFontColor);
   tolua_variable(tolua_S,"Fillers",tolua_get_CUserInterface_Fillers,tolua_set_CUserInterface_Fillers);
   tolua_array(tolua_S,"Resources",tolua_get_stratagus_CUserInterface_Resources,tolua_set_stratagus_CUserInterface_Resources);
   tolua_variable(tolua_S,"InfoPanel",tolua_get_CUserInterface_InfoPanel,tolua_set_CUserInterface_InfoPanel);
   tolua_variable(tolua_S,"SingleSelectedButton",tolua_get_CUserInterface_SingleSelectedButton_ptr,tolua_set_CUserInterface_SingleSelectedButton_ptr);
   tolua_variable(tolua_S,"SelectedButtons",tolua_get_CUserInterface_SelectedButtons,tolua_set_CUserInterface_SelectedButtons);
   tolua_variable(tolua_S,"MaxSelectedFont",tolua_get_CUserInterface_MaxSelectedFont_ptr,tolua_set_CUserInterface_MaxSelectedFont_ptr);
   tolua_variable(tolua_S,"MaxSelectedTextX",tolua_get_CUserInterface_MaxSelectedTextX,tolua_set_CUserInterface_MaxSelectedTextX);
   tolua_variable(tolua_S,"MaxSelectedTextY",tolua_get_CUserInterface_MaxSelectedTextY,tolua_set_CUserInterface_MaxSelectedTextY);
   tolua_variable(tolua_S,"SingleTrainingButton",tolua_get_CUserInterface_SingleTrainingButton_ptr,tolua_set_CUserInterface_SingleTrainingButton_ptr);
   tolua_variable(tolua_S,"TrainingButtons",tolua_get_CUserInterface_TrainingButtons,tolua_set_CUserInterface_TrainingButtons);
   tolua_variable(tolua_S,"TransportingButtons",tolua_get_CUserInterface_TransportingButtons,tolua_set_CUserInterface_TransportingButtons);
   tolua_variable(tolua_S,"CompletedBarColorRGB",tolua_get_CUserInterface_CompletedBarColorRGB,tolua_set_CUserInterface_CompletedBarColorRGB);
   tolua_variable(tolua_S,"CompletedBarShadow",tolua_get_CUserInterface_CompletedBarShadow,tolua_set_CUserInterface_CompletedBarShadow);
   tolua_variable(tolua_S,"ButtonPanel",tolua_get_CUserInterface_ButtonPanel,tolua_set_CUserInterface_ButtonPanel);
   tolua_variable(tolua_S,"PieMenu",tolua_get_CUserInterface_PieMenu,tolua_set_CUserInterface_PieMenu);
   tolua_variable(tolua_S,"MouseViewport",tolua_get_CUserInterface_MouseViewport_ptr,tolua_set_CUserInterface_MouseViewport_ptr);
   tolua_variable(tolua_S,"MapArea",tolua_get_CUserInterface_MapArea,tolua_set_CUserInterface_MapArea);
   tolua_variable(tolua_S,"MessageFont",tolua_get_CUserInterface_MessageFont_ptr,tolua_set_CUserInterface_MessageFont_ptr);
   tolua_variable(tolua_S,"MessageScrollSpeed",tolua_get_CUserInterface_MessageScrollSpeed,tolua_set_CUserInterface_MessageScrollSpeed);
   tolua_variable(tolua_S,"MenuButton",tolua_get_CUserInterface_MenuButton,tolua_set_CUserInterface_MenuButton);
   tolua_variable(tolua_S,"NetworkMenuButton",tolua_get_CUserInterface_NetworkMenuButton,tolua_set_CUserInterface_NetworkMenuButton);
   tolua_variable(tolua_S,"NetworkDiplomacyButton",tolua_get_CUserInterface_NetworkDiplomacyButton,tolua_set_CUserInterface_NetworkDiplomacyButton);
   tolua_variable(tolua_S,"Minimap",tolua_get_CUserInterface_Minimap,tolua_set_CUserInterface_Minimap);
   tolua_variable(tolua_S,"StatusLine",tolua_get_CUserInterface_StatusLine,tolua_set_CUserInterface_StatusLine);
   tolua_variable(tolua_S,"Timer",tolua_get_CUserInterface_Timer,tolua_set_CUserInterface_Timer);
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
  tolua_function(tolua_S,"FindButtonStyle",tolua_stratagus_FindButtonStyle00);
  tolua_function(tolua_S,"GetMouseScroll",tolua_stratagus_GetMouseScroll00);
  tolua_function(tolua_S,"SetMouseScroll",tolua_stratagus_SetMouseScroll00);
  tolua_function(tolua_S,"GetKeyScroll",tolua_stratagus_GetKeyScroll00);
  tolua_function(tolua_S,"SetKeyScroll",tolua_stratagus_SetKeyScroll00);
  tolua_function(tolua_S,"GetGrabMouse",tolua_stratagus_GetGrabMouse00);
  tolua_function(tolua_S,"SetGrabMouse",tolua_stratagus_SetGrabMouse00);
  tolua_function(tolua_S,"GetLeaveStops",tolua_stratagus_GetLeaveStops00);
  tolua_function(tolua_S,"SetLeaveStops",tolua_stratagus_SetLeaveStops00);
  tolua_function(tolua_S,"SetSavedMapPosition",tolua_stratagus_SetSavedMapPosition00);
  tolua_function(tolua_S,"GetDoubleClickDelay",tolua_stratagus_GetDoubleClickDelay00);
  tolua_function(tolua_S,"SetDoubleClickDelay",tolua_stratagus_SetDoubleClickDelay00);
  tolua_function(tolua_S,"GetHoldClickDelay",tolua_stratagus_GetHoldClickDelay00);
  tolua_function(tolua_S,"SetHoldClickDelay",tolua_stratagus_SetHoldClickDelay00);
  tolua_variable(tolua_S,"CursorX",tolua_get_CursorX,tolua_set_CursorX);
  tolua_variable(tolua_S,"CursorY",tolua_get_CursorY,tolua_set_CursorY);
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
  tolua_cclass(tolua_S,"Graphics","Graphics","",NULL);
  tolua_beginmodule(tolua_S,"Graphics");
   tolua_constant(tolua_S,"LEFT",Graphics::LEFT);
   tolua_constant(tolua_S,"CENTER",Graphics::CENTER);
   tolua_constant(tolua_S,"RIGHT",Graphics::RIGHT);
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
   tolua_function(tolua_S,"setBorderSize",tolua_stratagus_Widget_setBorderSize00);
   tolua_function(tolua_S,"getBorderSize",tolua_stratagus_Widget_getBorderSize00);
   tolua_function(tolua_S,"setEnabled",tolua_stratagus_Widget_setEnabled00);
   tolua_function(tolua_S,"isEnabled",tolua_stratagus_Widget_isEnabled00);
   tolua_function(tolua_S,"setVisible",tolua_stratagus_Widget_setVisible00);
   tolua_function(tolua_S,"isVisible",tolua_stratagus_Widget_isVisible00);
   tolua_function(tolua_S,"setBaseColor",tolua_stratagus_Widget_setBaseColor00);
   tolua_function(tolua_S,"getBaseColor",tolua_stratagus_Widget_getBaseColor00);
   tolua_function(tolua_S,"setForegroundColor",tolua_stratagus_Widget_setForegroundColor00);
   tolua_function(tolua_S,"getForegroundColor",tolua_stratagus_Widget_getForegroundColor00);
   tolua_function(tolua_S,"setBackgroundColor",tolua_stratagus_Widget_setBackgroundColor00);
   tolua_function(tolua_S,"getBackgroundColor",tolua_stratagus_Widget_getBackgroundColor00);
   tolua_function(tolua_S,"setDisabledColor",tolua_stratagus_Widget_setDisabledColor00);
   tolua_function(tolua_S,"getDisabledColor",tolua_stratagus_Widget_getDisabledColor00);
   tolua_function(tolua_S,"setGlobalFont",tolua_stratagus_Widget_setGlobalFont00);
   tolua_function(tolua_S,"setFont",tolua_stratagus_Widget_setFont00);
   tolua_function(tolua_S,"getHotKey",tolua_stratagus_Widget_getHotKey00);
   tolua_function(tolua_S,"setHotKey",tolua_stratagus_Widget_setHotKey00);
   tolua_function(tolua_S,"setHotKey",tolua_stratagus_Widget_setHotKey01);
   tolua_function(tolua_S,"addActionListener",tolua_stratagus_Widget_addActionListener00);
   tolua_function(tolua_S,"requestFocus",tolua_stratagus_Widget_requestFocus00);
  tolua_endmodule(tolua_S);

  { /* begin embedded lua code */
   int top = lua_gettop(tolua_S);
   static const unsigned char B[] = {
    10, 87,105,100,103,101,116, 46,115,101,116, 65, 99,116,105,
    111,110, 67, 97,108,108, 98, 97, 99,107, 32, 61, 32,102,117,
    110, 99,116,105,111,110, 40,119, 44, 32,102, 41, 10,119, 46,
     95, 97, 99,116,105,111,110, 99, 98, 32, 61, 32, 76,117, 97,
     65, 99,116,105,111,110, 76,105,115,116,101,110,101,114, 40,
    102, 41, 10,119, 58, 97,100,100, 65, 99,116,105,111,110, 76,
    105,115,116,101,110,101,114, 40,119, 46, 95, 97, 99,116,105,
    111,110, 99, 98, 41, 10,101,110,100, 45, 45, 45, 45, 45, 45,
     45, 45, 45, 45, 45, 45, 45, 45,32
   };
   tolua_dobuffer(tolua_S,(char*)B,sizeof(B),"tolua: embedded Lua code 3");
   lua_settop(tolua_S, top);
  } /* end of embedded lua code */

  tolua_cclass(tolua_S,"BasicContainer","BasicContainer","Widget",NULL);
  tolua_beginmodule(tolua_S,"BasicContainer");
  tolua_endmodule(tolua_S);
  #ifdef __cplusplus
  tolua_cclass(tolua_S,"ScrollArea","ScrollArea","BasicContainer",tolua_collect_ScrollArea);
  #else
  tolua_cclass(tolua_S,"ScrollArea","ScrollArea","BasicContainer",NULL);
  #endif
  tolua_beginmodule(tolua_S,"ScrollArea");
   tolua_function(tolua_S,"new",tolua_stratagus_ScrollArea_new00);
   tolua_function(tolua_S,"new_local",tolua_stratagus_ScrollArea_new00_local);
   tolua_function(tolua_S,".call",tolua_stratagus_ScrollArea_new00_local);
   tolua_function(tolua_S,"setContent",tolua_stratagus_ScrollArea_setContent00);
   tolua_function(tolua_S,"getContent",tolua_stratagus_ScrollArea_getContent00);
   tolua_function(tolua_S,"setScrollbarWidth",tolua_stratagus_ScrollArea_setScrollbarWidth00);
   tolua_function(tolua_S,"getScrollbarWidth",tolua_stratagus_ScrollArea_getScrollbarWidth00);
  tolua_endmodule(tolua_S);
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
  tolua_cclass(tolua_S,"Button","Button","Widget",NULL);
  tolua_beginmodule(tolua_S,"Button");
  tolua_endmodule(tolua_S);
  #ifdef __cplusplus
  tolua_cclass(tolua_S,"ButtonWidget","ButtonWidget","Button",tolua_collect_ButtonWidget);
  #else
  tolua_cclass(tolua_S,"ButtonWidget","ButtonWidget","Button",NULL);
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
  tolua_cclass(tolua_S,"ImageButton","ImageButton","Button",tolua_collect_ImageButton);
  #else
  tolua_cclass(tolua_S,"ImageButton","ImageButton","Button",NULL);
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
   tolua_function(tolua_S,"setAlignment",tolua_stratagus_Label_setAlignment00);
   tolua_function(tolua_S,"getAlignment",tolua_stratagus_Label_getAlignment00);
   tolua_function(tolua_S,"adjustSize",tolua_stratagus_Label_adjustSize00);
  tolua_endmodule(tolua_S);
  #ifdef __cplusplus
  tolua_cclass(tolua_S,"MultiLineLabel","MultiLineLabel","Widget",tolua_collect_MultiLineLabel);
  #else
  tolua_cclass(tolua_S,"MultiLineLabel","MultiLineLabel","Widget",NULL);
  #endif
  tolua_beginmodule(tolua_S,"MultiLineLabel");
   tolua_function(tolua_S,"new",tolua_stratagus_MultiLineLabel_new00);
   tolua_function(tolua_S,"new_local",tolua_stratagus_MultiLineLabel_new00_local);
   tolua_function(tolua_S,".call",tolua_stratagus_MultiLineLabel_new00_local);
   tolua_function(tolua_S,"new",tolua_stratagus_MultiLineLabel_new01);
   tolua_function(tolua_S,"new_local",tolua_stratagus_MultiLineLabel_new01_local);
   tolua_function(tolua_S,".call",tolua_stratagus_MultiLineLabel_new01_local);
   tolua_function(tolua_S,"setCaption",tolua_stratagus_MultiLineLabel_setCaption00);
   tolua_function(tolua_S,"getCaption",tolua_stratagus_MultiLineLabel_getCaption00);
   tolua_function(tolua_S,"setAlignment",tolua_stratagus_MultiLineLabel_setAlignment00);
   tolua_function(tolua_S,"getAlignment",tolua_stratagus_MultiLineLabel_getAlignment00);
   tolua_function(tolua_S,"setVerticalAlignment",tolua_stratagus_MultiLineLabel_setVerticalAlignment00);
   tolua_function(tolua_S,"getVerticalAlignment",tolua_stratagus_MultiLineLabel_getVerticalAlignment00);
   tolua_function(tolua_S,"setLineWidth",tolua_stratagus_MultiLineLabel_setLineWidth00);
   tolua_function(tolua_S,"getLineWidth",tolua_stratagus_MultiLineLabel_getLineWidth00);
   tolua_function(tolua_S,"adjustSize",tolua_stratagus_MultiLineLabel_adjustSize00);
   tolua_function(tolua_S,"draw",tolua_stratagus_MultiLineLabel_draw00);
   tolua_constant(tolua_S,"LEFT",MultiLineLabel::LEFT);
   tolua_constant(tolua_S,"CENTER",MultiLineLabel::CENTER);
   tolua_constant(tolua_S,"RIGHT",MultiLineLabel::RIGHT);
   tolua_constant(tolua_S,"TOP",MultiLineLabel::TOP);
   tolua_constant(tolua_S,"BOTTOM",MultiLineLabel::BOTTOM);
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
   tolua_function(tolua_S,"setText",tolua_stratagus_TextField_setText00);
   tolua_function(tolua_S,"getText",tolua_stratagus_TextField_getText00);
   tolua_function(tolua_S,"setMaxLengthBytes",tolua_stratagus_TextField_setMaxLengthBytes00);
   tolua_function(tolua_S,"getMaxLengthBytes",tolua_stratagus_TextField_getMaxLengthBytes00);
  tolua_endmodule(tolua_S);
  tolua_cclass(tolua_S,"ListBox","ListBox","Widget",NULL);
  tolua_beginmodule(tolua_S,"ListBox");
  tolua_endmodule(tolua_S);
  #ifdef __cplusplus
  tolua_cclass(tolua_S,"ListBoxWidget","ListBoxWidget","ScrollArea",tolua_collect_ListBoxWidget);
  #else
  tolua_cclass(tolua_S,"ListBoxWidget","ListBoxWidget","ScrollArea",NULL);
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
  tolua_cclass(tolua_S,"Window","Window","BasicContainer",tolua_collect_Window);
  #else
  tolua_cclass(tolua_S,"Window","Window","BasicContainer",NULL);
  #endif
  tolua_beginmodule(tolua_S,"Window");
   tolua_function(tolua_S,"new",tolua_stratagus_Window_new00);
   tolua_function(tolua_S,"new_local",tolua_stratagus_Window_new00_local);
   tolua_function(tolua_S,".call",tolua_stratagus_Window_new00_local);
   tolua_function(tolua_S,"new",tolua_stratagus_Window_new01);
   tolua_function(tolua_S,"new_local",tolua_stratagus_Window_new01_local);
   tolua_function(tolua_S,".call",tolua_stratagus_Window_new01_local);
   tolua_function(tolua_S,"new",tolua_stratagus_Window_new02);
   tolua_function(tolua_S,"new_local",tolua_stratagus_Window_new02_local);
   tolua_function(tolua_S,".call",tolua_stratagus_Window_new02_local);
   tolua_function(tolua_S,"setCaption",tolua_stratagus_Window_setCaption00);
   tolua_function(tolua_S,"getCaption",tolua_stratagus_Window_getCaption00);
   tolua_function(tolua_S,"setAlignment",tolua_stratagus_Window_setAlignment00);
   tolua_function(tolua_S,"getAlignment",tolua_stratagus_Window_getAlignment00);
   tolua_function(tolua_S,"setContent",tolua_stratagus_Window_setContent00);
   tolua_function(tolua_S,"getContent",tolua_stratagus_Window_getContent00);
   tolua_function(tolua_S,"setPadding",tolua_stratagus_Window_setPadding00);
   tolua_function(tolua_S,"getPadding",tolua_stratagus_Window_getPadding00);
   tolua_function(tolua_S,"setTitleBarHeight",tolua_stratagus_Window_setTitleBarHeight00);
   tolua_function(tolua_S,"getTitleBarHeight",tolua_stratagus_Window_getTitleBarHeight00);
   tolua_function(tolua_S,"setMovable",tolua_stratagus_Window_setMovable00);
   tolua_function(tolua_S,"isMovable",tolua_stratagus_Window_isMovable00);
   tolua_function(tolua_S,"resizeToContent",tolua_stratagus_Window_resizeToContent00);
   tolua_function(tolua_S,"setOpaque",tolua_stratagus_Window_setOpaque00);
   tolua_function(tolua_S,"isOpaque",tolua_stratagus_Window_isOpaque00);
  tolua_endmodule(tolua_S);
  #ifdef __cplusplus
  tolua_cclass(tolua_S,"Windows","Windows","Window",tolua_collect_Windows);
  #else
  tolua_cclass(tolua_S,"Windows","Windows","Window",NULL);
  #endif
  tolua_beginmodule(tolua_S,"Windows");
   tolua_function(tolua_S,"new",tolua_stratagus_Windows_new00);
   tolua_function(tolua_S,"new_local",tolua_stratagus_Windows_new00_local);
   tolua_function(tolua_S,".call",tolua_stratagus_Windows_new00_local);
   tolua_function(tolua_S,"add",tolua_stratagus_Windows_add00);
  tolua_endmodule(tolua_S);
  #ifdef __cplusplus
  tolua_cclass(tolua_S,"ScrollingWidget","ScrollingWidget","ScrollArea",tolua_collect_ScrollingWidget);
  #else
  tolua_cclass(tolua_S,"ScrollingWidget","ScrollingWidget","ScrollArea",NULL);
  #endif
  tolua_beginmodule(tolua_S,"ScrollingWidget");
   tolua_function(tolua_S,"new",tolua_stratagus_ScrollingWidget_new00);
   tolua_function(tolua_S,"new_local",tolua_stratagus_ScrollingWidget_new00_local);
   tolua_function(tolua_S,".call",tolua_stratagus_ScrollingWidget_new00_local);
   tolua_function(tolua_S,"add",tolua_stratagus_ScrollingWidget_add00);
   tolua_function(tolua_S,"restart",tolua_stratagus_ScrollingWidget_restart00);
   tolua_function(tolua_S,"setSpeed",tolua_stratagus_ScrollingWidget_setSpeed00);
   tolua_function(tolua_S,"getSpeed",tolua_stratagus_ScrollingWidget_getSpeed00);
  tolua_endmodule(tolua_S);
  tolua_cclass(tolua_S,"DropDown","DropDown","BasicContainer",NULL);
  tolua_beginmodule(tolua_S,"DropDown");
   tolua_function(tolua_S,"getSelected",tolua_stratagus_DropDown_getSelected00);
   tolua_function(tolua_S,"setSelected",tolua_stratagus_DropDown_setSelected00);
   tolua_function(tolua_S,"setScrollArea",tolua_stratagus_DropDown_setScrollArea00);
   tolua_function(tolua_S,"getScrollArea",tolua_stratagus_DropDown_getScrollArea00);
   tolua_function(tolua_S,"setListBox",tolua_stratagus_DropDown_setListBox00);
   tolua_function(tolua_S,"getListBox",tolua_stratagus_DropDown_getListBox00);
  tolua_endmodule(tolua_S);
  #ifdef __cplusplus
  tolua_cclass(tolua_S,"DropDownWidget","DropDownWidget","DropDown",tolua_collect_DropDownWidget);
  #else
  tolua_cclass(tolua_S,"DropDownWidget","DropDownWidget","DropDown",NULL);
  #endif
  tolua_beginmodule(tolua_S,"DropDownWidget");
   tolua_function(tolua_S,"new",tolua_stratagus_DropDownWidget_new00);
   tolua_function(tolua_S,"new_local",tolua_stratagus_DropDownWidget_new00_local);
   tolua_function(tolua_S,".call",tolua_stratagus_DropDownWidget_new00_local);
   tolua_function(tolua_S,"setList",tolua_stratagus_DropDownWidget_setList00);
   tolua_function(tolua_S,"getListBox",tolua_stratagus_DropDownWidget_getListBox00);
   tolua_function(tolua_S,"setSize",tolua_stratagus_DropDownWidget_setSize00);
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
   tolua_function(tolua_S,"setCaption",tolua_stratagus_StatBoxWidget_setCaption00);
   tolua_function(tolua_S,"getCaption",tolua_stratagus_StatBoxWidget_getCaption00);
   tolua_function(tolua_S,"setPercent",tolua_stratagus_StatBoxWidget_setPercent00);
   tolua_function(tolua_S,"getPercent",tolua_stratagus_StatBoxWidget_getPercent00);
  tolua_endmodule(tolua_S);
  #ifdef __cplusplus
  tolua_cclass(tolua_S,"Container","Container","BasicContainer",tolua_collect_Container);
  #else
  tolua_cclass(tolua_S,"Container","Container","BasicContainer",NULL);
  #endif
  tolua_beginmodule(tolua_S,"Container");
   tolua_function(tolua_S,"new",tolua_stratagus_Container_new00);
   tolua_function(tolua_S,"new_local",tolua_stratagus_Container_new00_local);
   tolua_function(tolua_S,".call",tolua_stratagus_Container_new00_local);
   tolua_function(tolua_S,"setOpaque",tolua_stratagus_Container_setOpaque00);
   tolua_function(tolua_S,"isOpaque",tolua_stratagus_Container_isOpaque00);
   tolua_function(tolua_S,"add",tolua_stratagus_Container_add00);
   tolua_function(tolua_S,"remove",tolua_stratagus_Container_remove00);
   tolua_function(tolua_S,"clear",tolua_stratagus_Container_clear00);
  tolua_endmodule(tolua_S);
  #ifdef __cplusplus
  tolua_cclass(tolua_S,"CMenuScreen","MenuScreen","Container",tolua_collect_MenuScreen);
  #else
  tolua_cclass(tolua_S,"CMenuScreen","MenuScreen","Container",NULL);
  #endif
  tolua_beginmodule(tolua_S,"CMenuScreen");
   tolua_function(tolua_S,"new",tolua_stratagus_CMenuScreen_new00);
   tolua_function(tolua_S,"new_local",tolua_stratagus_CMenuScreen_new00_local);
   tolua_function(tolua_S,".call",tolua_stratagus_CMenuScreen_new00_local);
   tolua_function(tolua_S,"run",tolua_stratagus_CMenuScreen_run00);
   tolua_function(tolua_S,"stop",tolua_stratagus_CMenuScreen_stop00);
   tolua_function(tolua_S,"stopAll",tolua_stratagus_CMenuScreen_stopAll00);
   tolua_function(tolua_S,"addLogicCallback",tolua_stratagus_CMenuScreen_addLogicCallback00);
   tolua_function(tolua_S,"setDrawMenusUnder",tolua_stratagus_CMenuScreen_setDrawMenusUnder00);
   tolua_function(tolua_S,"getDrawMenusUnder",tolua_stratagus_CMenuScreen_getDrawMenusUnder00);
  tolua_endmodule(tolua_S);

  { /* begin embedded lua code */
   int top = lua_gettop(tolua_S);
   static const unsigned char B[] = {
    10,102,117,110, 99,116,105,111,110, 32, 77,101,110,117, 83,
     99,114,101,101,110, 40, 41, 10,108,111, 99, 97,108, 32,109,
    101,110,117, 32, 61, 32, 67, 77,101,110,117, 83, 99,114,101,
    101,110, 40, 41, 10,108,111, 99, 97,108, 32,103,117,105, 99,
    104, 97,110, 97,100,100, 32, 61, 32, 67,111,110,116, 97,105,
    110,101,114, 46, 97,100,100, 10,102,117,110, 99,116,105,111,
    110, 32,109,101,110,117, 58, 97,100,100, 40,119,105,100,103,
    101,116, 44, 32,120, 44, 32,121, 41, 10,105,102, 32,110,111,
    116, 32,115,101,108,102, 46, 95, 97,100,100,101,100, 87,105,
    100,103,101,116,115, 32,116,104,101,110, 10,115,101,108,102,
     46, 95, 97,100,100,101,100, 87,105,100,103,101,116,115, 32,
     61, 32,123,125, 10,101,110,100, 10,115,101,108,102, 46, 95,
     97,100,100,101,100, 87,105,100,103,101,116,115, 91,119,105,
    100,103,101,116, 93, 32, 61, 32,116,114,117,101, 10,103,117,
    105, 99,104, 97,110, 97,100,100, 40,115,101,108,102, 44, 32,
    119,105,100,103,101,116, 44, 32,120, 44, 32,121, 41, 10,101,
    110,100, 10,114,101,116,117,114,110, 32,109,101,110,117, 10,
    101,110,100, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45,
     45, 45,32
   };
   tolua_dobuffer(tolua_S,(char*)B,sizeof(B),"tolua: embedded Lua code 4");
   lua_settop(tolua_S, top);
  } /* end of embedded lua code */

  tolua_function(tolua_S,"InitNetwork1",tolua_stratagus_InitNetwork100);
  tolua_function(tolua_S,"ExitNetwork1",tolua_stratagus_ExitNetwork100);
  tolua_function(tolua_S,"IsNetworkGame",tolua_stratagus_IsNetworkGame00);
  tolua_function(tolua_S,"NetworkSetupServerAddress",tolua_stratagus_NetworkSetupServerAddress00);
  tolua_function(tolua_S,"NetworkInitClientConnect",tolua_stratagus_NetworkInitClientConnect00);
  tolua_function(tolua_S,"NetworkInitServerConnect",tolua_stratagus_NetworkInitServerConnect00);
  tolua_function(tolua_S,"NetworkServerStartGame",tolua_stratagus_NetworkServerStartGame00);
  tolua_function(tolua_S,"NetworkProcessClientRequest",tolua_stratagus_NetworkProcessClientRequest00);
  tolua_function(tolua_S,"GetNetworkState",tolua_stratagus_GetNetworkState00);
  tolua_function(tolua_S,"NetworkServerResyncClients",tolua_stratagus_NetworkServerResyncClients00);
  tolua_function(tolua_S,"NetworkDetachFromServer",tolua_stratagus_NetworkDetachFromServer00);
  tolua_cclass(tolua_S,"CServerSetup","CServerSetup","",NULL);
  tolua_beginmodule(tolua_S,"CServerSetup");
   tolua_variable(tolua_S,"ResourcesOption",tolua_get_CServerSetup_unsigned_ResourcesOption,tolua_set_CServerSetup_unsigned_ResourcesOption);
   tolua_variable(tolua_S,"UnitsOption",tolua_get_CServerSetup_unsigned_UnitsOption,tolua_set_CServerSetup_unsigned_UnitsOption);
   tolua_variable(tolua_S,"FogOfWar",tolua_get_CServerSetup_unsigned_FogOfWar,tolua_set_CServerSetup_unsigned_FogOfWar);
   tolua_variable(tolua_S,"RevealMap",tolua_get_CServerSetup_unsigned_RevealMap,tolua_set_CServerSetup_unsigned_RevealMap);
   tolua_variable(tolua_S,"GameTypeOption",tolua_get_CServerSetup_unsigned_GameTypeOption,tolua_set_CServerSetup_unsigned_GameTypeOption);
   tolua_variable(tolua_S,"Difficulty",tolua_get_CServerSetup_unsigned_Difficulty,tolua_set_CServerSetup_unsigned_Difficulty);
   tolua_variable(tolua_S,"MapRichness",tolua_get_CServerSetup_unsigned_MapRichness,tolua_set_CServerSetup_unsigned_MapRichness);
   tolua_array(tolua_S,"CompOpt",tolua_get_stratagus_CServerSetup_CompOpt,tolua_set_stratagus_CServerSetup_CompOpt);
   tolua_array(tolua_S,"Ready",tolua_get_stratagus_CServerSetup_Ready,tolua_set_stratagus_CServerSetup_Ready);
   tolua_array(tolua_S,"LastFrame",tolua_get_stratagus_CServerSetup_LastFrame,tolua_set_stratagus_CServerSetup_LastFrame);
  tolua_endmodule(tolua_S);
  tolua_variable(tolua_S,"LocalSetupState",tolua_get_LocalSetupState,tolua_set_LocalSetupState);
  tolua_variable(tolua_S,"ServerSetupState",tolua_get_ServerSetupState,tolua_set_ServerSetupState);
  tolua_variable(tolua_S,"NetLocalHostsSlot",tolua_get_NetLocalHostsSlot,tolua_set_NetLocalHostsSlot);
  tolua_variable(tolua_S,"NetPlayerNameSize",tolua_get_NetPlayerNameSize,NULL);
  #ifdef __cplusplus
  tolua_cclass(tolua_S,"CNetworkHost","CNetworkHost","",tolua_collect_CNetworkHost);
  #else
  tolua_cclass(tolua_S,"CNetworkHost","CNetworkHost","",NULL);
  #endif
  tolua_beginmodule(tolua_S,"CNetworkHost");
   tolua_variable(tolua_S,"Host",tolua_get_CNetworkHost_unsigned_Host,tolua_set_CNetworkHost_unsigned_Host);
   tolua_variable(tolua_S,"Port",tolua_get_CNetworkHost_unsigned_Port,tolua_set_CNetworkHost_unsigned_Port);
   tolua_variable(tolua_S,"PlyNr",tolua_get_CNetworkHost_unsigned_PlyNr,tolua_set_CNetworkHost_unsigned_PlyNr);
   tolua_variable(tolua_S,"PlyName",tolua_get_CNetworkHost_PlyName,tolua_set_CNetworkHost_PlyName);
  tolua_endmodule(tolua_S);
  tolua_array(tolua_S,"Hosts",tolua_get_stratagus_Hosts,tolua_set_stratagus_Hosts);
  tolua_variable(tolua_S,"NetworkMapName",tolua_get_NetworkMapName,tolua_set_NetworkMapName);
  tolua_function(tolua_S,"NetworkGamePrepareGameSettings",tolua_stratagus_NetworkGamePrepareGameSettings00);
  tolua_function(tolua_S,"InitVideo",tolua_stratagus_InitVideo00);
  tolua_variable(tolua_S,"UseOpenGL",tolua_get_UseOpenGL,tolua_set_UseOpenGL);
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
  tolua_cclass(tolua_S,"CPlayerColorGraphic","CPlayerColorGraphic","CGraphic",NULL);
  tolua_beginmodule(tolua_S,"CPlayerColorGraphic");
   tolua_function(tolua_S,"New",tolua_stratagus_CPlayerColorGraphic_New00);
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
  tolua_variable(tolua_S,"UseGLTextureCompression",tolua_get_UseGLTextureCompression,tolua_set_UseGLTextureCompression);
  tolua_cclass(tolua_S,"CFont","CFont","",NULL);
  tolua_beginmodule(tolua_S,"CFont");
   tolua_function(tolua_S,"New",tolua_stratagus_CFont_New00);
   tolua_function(tolua_S,"Get",tolua_stratagus_CFont_Get00);
   tolua_function(tolua_S,"Height",tolua_stratagus_CFont_Height00);
   tolua_function(tolua_S,"Width",tolua_stratagus_CFont_Width00);
   tolua_function(tolua_S,"PlainText",tolua_stratagus_CFont_PlainText00);
  tolua_endmodule(tolua_S);
  tolua_constant(tolua_S,"MaxFontColors",MaxFontColors);
  tolua_cclass(tolua_S,"CFontColor","CFontColor","",NULL);
  tolua_beginmodule(tolua_S,"CFontColor");
   tolua_function(tolua_S,"New",tolua_stratagus_CFontColor_New00);
   tolua_function(tolua_S,"Get",tolua_stratagus_CFontColor_Get00);
   tolua_array(tolua_S,"Colors",tolua_get_stratagus_CFontColor_Colors,tolua_set_stratagus_CFontColor_Colors);
  tolua_endmodule(tolua_S);
  tolua_constant(tolua_S,"PlayerNeutral",PlayerNeutral);
  tolua_constant(tolua_S,"PlayerNobody",PlayerNobody);
  tolua_constant(tolua_S,"PlayerComputer",PlayerComputer);
  tolua_constant(tolua_S,"PlayerPerson",PlayerPerson);
  tolua_constant(tolua_S,"PlayerRescuePassive",PlayerRescuePassive);
  tolua_constant(tolua_S,"PlayerRescueActive",PlayerRescueActive);
  #ifdef __cplusplus
  tolua_cclass(tolua_S,"CPlayer","CPlayer","",tolua_collect_CPlayer);
  #else
  tolua_cclass(tolua_S,"CPlayer","CPlayer","",NULL);
  #endif
  tolua_beginmodule(tolua_S,"CPlayer");
   tolua_variable(tolua_S,"Index",tolua_get_CPlayer_Index,tolua_set_CPlayer_Index);
   tolua_variable(tolua_S,"Name",tolua_get_CPlayer_Name,tolua_set_CPlayer_Name);
   tolua_variable(tolua_S,"Type",tolua_get_CPlayer_Type,tolua_set_CPlayer_Type);
   tolua_variable(tolua_S,"AiName",tolua_get_CPlayer_AiName,tolua_set_CPlayer_AiName);
   tolua_variable(tolua_S,"StartX",tolua_get_CPlayer_StartX,tolua_set_CPlayer_StartX);
   tolua_variable(tolua_S,"StartY",tolua_get_CPlayer_StartY,tolua_set_CPlayer_StartY);
   tolua_function(tolua_S,"SetStartView",tolua_stratagus_CPlayer_SetStartView00);
   tolua_variable(tolua_S,"EnergyProductionRate",tolua_get_CPlayer_EnergyProductionRate,tolua_set_CPlayer_EnergyProductionRate);
   tolua_variable(tolua_S,"MagmaProductionRate",tolua_get_CPlayer_MagmaProductionRate,tolua_set_CPlayer_MagmaProductionRate);
   tolua_variable(tolua_S,"EnergyStored",tolua_get_CPlayer_EnergyStored,tolua_set_CPlayer_EnergyStored);
   tolua_variable(tolua_S,"MagmaStored",tolua_get_CPlayer_MagmaStored,tolua_set_CPlayer_MagmaStored);
   tolua_variable(tolua_S,"EnergyStorageCapacity",tolua_get_CPlayer_EnergyStorageCapacity,tolua_set_CPlayer_EnergyStorageCapacity);
   tolua_variable(tolua_S,"MagmaStorageCapacity",tolua_get_CPlayer_MagmaStorageCapacity,tolua_set_CPlayer_MagmaStorageCapacity);
   tolua_array(tolua_S,"UnitTypesCount",tolua_get_stratagus_CPlayer_UnitTypesCount,NULL);
   tolua_variable(tolua_S,"AiEnabled",tolua_get_CPlayer_AiEnabled,tolua_set_CPlayer_AiEnabled);
   tolua_array(tolua_S,"Units",tolua_get_stratagus_CPlayer_Units,tolua_set_stratagus_CPlayer_Units);
   tolua_variable(tolua_S,"TotalNumUnits",tolua_get_CPlayer_TotalNumUnits,tolua_set_CPlayer_TotalNumUnits);
   tolua_variable(tolua_S,"NumBuildings",tolua_get_CPlayer_NumBuildings,tolua_set_CPlayer_NumBuildings);
   tolua_variable(tolua_S,"UnitLimit",tolua_get_CPlayer_UnitLimit,tolua_set_CPlayer_UnitLimit);
   tolua_variable(tolua_S,"BuildingLimit",tolua_get_CPlayer_BuildingLimit,tolua_set_CPlayer_BuildingLimit);
   tolua_variable(tolua_S,"TotalUnitLimit",tolua_get_CPlayer_TotalUnitLimit,tolua_set_CPlayer_TotalUnitLimit);
   tolua_variable(tolua_S,"Score",tolua_get_CPlayer_Score,tolua_set_CPlayer_Score);
   tolua_variable(tolua_S,"TotalUnits",tolua_get_CPlayer_TotalUnits,tolua_set_CPlayer_TotalUnits);
   tolua_variable(tolua_S,"TotalBuildings",tolua_get_CPlayer_TotalBuildings,tolua_set_CPlayer_TotalBuildings);
   tolua_variable(tolua_S,"TotalEnergy",tolua_get_CPlayer_TotalEnergy,tolua_set_CPlayer_TotalEnergy);
   tolua_variable(tolua_S,"TotalMagma",tolua_get_CPlayer_TotalMagma,tolua_set_CPlayer_TotalMagma);
   tolua_variable(tolua_S,"TotalRazings",tolua_get_CPlayer_TotalRazings,tolua_set_CPlayer_TotalRazings);
   tolua_variable(tolua_S,"TotalKills",tolua_get_CPlayer_TotalKills,tolua_set_CPlayer_TotalKills);
   tolua_function(tolua_S,"IsEnemy",tolua_stratagus_CPlayer_IsEnemy00);
   tolua_function(tolua_S,"IsEnemy",tolua_stratagus_CPlayer_IsEnemy01);
   tolua_function(tolua_S,"IsAllied",tolua_stratagus_CPlayer_IsAllied00);
   tolua_function(tolua_S,"IsAllied",tolua_stratagus_CPlayer_IsAllied01);
   tolua_function(tolua_S,"IsSharedVision",tolua_stratagus_CPlayer_IsSharedVision00);
   tolua_function(tolua_S,"IsSharedVision",tolua_stratagus_CPlayer_IsSharedVision01);
   tolua_function(tolua_S,"IsBothSharedVision",tolua_stratagus_CPlayer_IsBothSharedVision00);
   tolua_function(tolua_S,"IsBothSharedVision",tolua_stratagus_CPlayer_IsBothSharedVision01);
   tolua_function(tolua_S,"IsTeamed",tolua_stratagus_CPlayer_IsTeamed00);
   tolua_function(tolua_S,"IsTeamed",tolua_stratagus_CPlayer_IsTeamed01);
  tolua_endmodule(tolua_S);
  tolua_array(tolua_S,"Players",tolua_get_stratagus_Players,tolua_set_stratagus_Players);
  tolua_variable(tolua_S,"ThisPlayer",tolua_get_ThisPlayer_ptr,tolua_set_ThisPlayer_ptr);
  tolua_function(tolua_S,"ChangeUnitsOwner",tolua_stratagus_ChangeUnitsOwner00);
  tolua_cclass(tolua_S,"CUnitType","CUnitType","",NULL);
  tolua_beginmodule(tolua_S,"CUnitType");
   tolua_variable(tolua_S,"Ident",tolua_get_CUnitType_Ident,tolua_set_CUnitType_Ident);
   tolua_variable(tolua_S,"Name",tolua_get_CUnitType_Name,tolua_set_CUnitType_Name);
   tolua_variable(tolua_S,"Slot",tolua_get_CUnitType_Slot,NULL);
   tolua_variable(tolua_S,"MinAttackRange",tolua_get_CUnitType_MinAttackRange,tolua_set_CUnitType_MinAttackRange);
   tolua_variable(tolua_S,"MaxEnergyUtilizationRate",tolua_get_CUnitType_MaxEnergyUtilizationRate,tolua_set_CUnitType_MaxEnergyUtilizationRate);
   tolua_variable(tolua_S,"MaxMagmaUtilizationRate",tolua_get_CUnitType_MaxMagmaUtilizationRate,tolua_set_CUnitType_MaxMagmaUtilizationRate);
   tolua_variable(tolua_S,"EnergyProductionRate",tolua_get_CUnitType_EnergyProductionRate,tolua_set_CUnitType_EnergyProductionRate);
   tolua_variable(tolua_S,"MagmaProductionRate",tolua_get_CUnitType_MagmaProductionRate,tolua_set_CUnitType_MagmaProductionRate);
   tolua_variable(tolua_S,"EnergyValue",tolua_get_CUnitType_EnergyValue,tolua_set_CUnitType_EnergyValue);
   tolua_variable(tolua_S,"MagmaValue",tolua_get_CUnitType_MagmaValue,tolua_set_CUnitType_MagmaValue);
   tolua_variable(tolua_S,"EnergyStorageCapacity",tolua_get_CUnitType_EnergyStorageCapacity,tolua_set_CUnitType_EnergyStorageCapacity);
   tolua_variable(tolua_S,"MagmaStorageCapacity",tolua_get_CUnitType_MagmaStorageCapacity,tolua_set_CUnitType_MagmaStorageCapacity);
  tolua_endmodule(tolua_S);
  tolua_function(tolua_S,"UnitTypeByIdent",tolua_stratagus_UnitTypeByIdent00);
  tolua_cclass(tolua_S,"CUnit","CUnit","",NULL);
  tolua_beginmodule(tolua_S,"CUnit");
   tolua_variable(tolua_S,"Slot",tolua_get_CUnit_Slot,NULL);
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
  tolua_function(tolua_S,"PlayFile",tolua_stratagus_PlayFile00);

  { /* begin embedded lua code */
   int top = lua_gettop(tolua_S);
   static const unsigned char B[] = {
    10,102,117,110, 99,116,105,111,110, 32, 80,108, 97,121, 83,
    111,117,110,100, 70,105,108,101, 40,102,105,108,101, 44, 32,
     99, 97,108,108, 98, 97, 99,107, 41, 10,114,101,116,117,114,
    110, 32, 80,108, 97,121, 70,105,108,101, 40,102,105,108,101,
     44, 32, 76,117, 97, 65, 99,116,105,111,110, 76,105,115,116,
    101,110,101,114, 58,110,101,119, 40, 99, 97,108,108, 98, 97,
     99,107, 41, 41, 10,101,110,100, 45, 45, 45, 45, 45, 45, 45,
     45, 45, 45, 45, 45, 45, 45,32
   };
   tolua_dobuffer(tolua_S,(char*)B,sizeof(B),"tolua: embedded Lua code 5");
   lua_settop(tolua_S, top);
  } /* end of embedded lua code */

  tolua_function(tolua_S,"PlayMusic",tolua_stratagus_PlayMusic00);
  tolua_function(tolua_S,"StopMusic",tolua_stratagus_StopMusic00);
  tolua_function(tolua_S,"SetChannelVolume",tolua_stratagus_SetChannelVolume00);
  tolua_function(tolua_S,"SetChannelStereo",tolua_stratagus_SetChannelStereo00);
  tolua_function(tolua_S,"StopChannel",tolua_stratagus_StopChannel00);
  tolua_function(tolua_S,"StopAllChannels",tolua_stratagus_StopAllChannels00);
  tolua_constant(tolua_S,"EditorNotRunning",EditorNotRunning);
  tolua_constant(tolua_S,"EditorStarted",EditorStarted);
  tolua_constant(tolua_S,"EditorCommandLine",EditorCommandLine);
  tolua_constant(tolua_S,"EditorEditing",EditorEditing);
  tolua_cclass(tolua_S,"CEditor","CEditor","",NULL);
  tolua_beginmodule(tolua_S,"CEditor");
   tolua_variable(tolua_S,"UnitTypes",tolua_get_CEditor_UnitTypes,tolua_set_CEditor_UnitTypes);
   tolua_variable(tolua_S,"StartUnit",tolua_get_CEditor_StartUnit_ptr,NULL);
   tolua_variable(tolua_S,"ShowTerrainFlags",tolua_get_CEditor_ShowTerrainFlags,tolua_set_CEditor_ShowTerrainFlags);
   tolua_variable(tolua_S,"Running",tolua_get_CEditor_Running,tolua_set_CEditor_Running);
   tolua_function(tolua_S,"TileSelectedPatch",tolua_stratagus_CEditor_TileSelectedPatch00);
  tolua_endmodule(tolua_S);
  tolua_function(tolua_S,"SetEditorSelectIcon",tolua_stratagus_SetEditorSelectIcon00);
  tolua_function(tolua_S,"SetEditorUnitsIcon",tolua_stratagus_SetEditorUnitsIcon00);
  tolua_function(tolua_S,"SetEditorPatchIcon",tolua_stratagus_SetEditorPatchIcon00);
  tolua_function(tolua_S,"SetEditorStartUnit",tolua_stratagus_SetEditorStartUnit00);
  tolua_variable(tolua_S,"Editor",tolua_get_Editor,tolua_set_Editor);
  tolua_function(tolua_S,"StartEditor",tolua_stratagus_StartEditor00);
  tolua_function(tolua_S,"EditorSaveMap",tolua_stratagus_EditorSaveMap00);
  tolua_function(tolua_S,"StartPatchEditor",tolua_stratagus_StartPatchEditor00);
  tolua_function(tolua_S,"IsReplayGame",tolua_stratagus_IsReplayGame00);
  tolua_function(tolua_S,"StartMap",tolua_stratagus_StartMap00);
  tolua_function(tolua_S,"StartReplay",tolua_stratagus_StartReplay00);
  tolua_function(tolua_S,"StartSavedGame",tolua_stratagus_StartSavedGame00);
  tolua_function(tolua_S,"SaveReplay",tolua_stratagus_SaveReplay00);
  tolua_constant(tolua_S,"GameNoResult",GameNoResult);
  tolua_constant(tolua_S,"GameVictory",GameVictory);
  tolua_constant(tolua_S,"GameDefeat",GameDefeat);
  tolua_constant(tolua_S,"GameDraw",GameDraw);
  tolua_constant(tolua_S,"GameQuitToMenu",GameQuitToMenu);
  tolua_constant(tolua_S,"GameRestart",GameRestart);
  tolua_variable(tolua_S,"GameResult",tolua_get_GameResult,tolua_set_GameResult);
  tolua_function(tolua_S,"StopGame",tolua_stratagus_StopGame00);
  tolua_variable(tolua_S,"GameRunning",tolua_get_GameRunning,tolua_set_GameRunning);
  tolua_function(tolua_S,"SetGamePaused",tolua_stratagus_SetGamePaused00);
  tolua_function(tolua_S,"GetGamePaused",tolua_stratagus_GetGamePaused00);
  tolua_variable(tolua_S,"GamePaused",tolua_get_GamePaused,tolua_set_GamePaused);
  tolua_function(tolua_S,"SetGameSpeed",tolua_stratagus_SetGameSpeed00);
  tolua_function(tolua_S,"GetGameSpeed",tolua_stratagus_GetGameSpeed00);
  tolua_variable(tolua_S,"GameSpeed",tolua_get_GameSpeed,tolua_set_GameSpeed);
  tolua_variable(tolua_S,"GameObserve",tolua_get_GameObserve,tolua_set_GameObserve);
  tolua_variable(tolua_S,"GameCycle",tolua_get_unsigned_GameCycle,tolua_set_unsigned_GameCycle);
  #ifdef __cplusplus
  tolua_cclass(tolua_S,"SettingsPresets","SettingsPresets","",tolua_collect_SettingsPresets);
  #else
  tolua_cclass(tolua_S,"SettingsPresets","SettingsPresets","",NULL);
  #endif
  tolua_beginmodule(tolua_S,"SettingsPresets");
   tolua_variable(tolua_S,"Team",tolua_get_SettingsPresets_Team,tolua_set_SettingsPresets_Team);
   tolua_variable(tolua_S,"Type",tolua_get_SettingsPresets_Type,tolua_set_SettingsPresets_Type);
  tolua_endmodule(tolua_S);
  tolua_cclass(tolua_S,"Settings","Settings","",NULL);
  tolua_beginmodule(tolua_S,"Settings");
   tolua_variable(tolua_S,"NetGameType",tolua_get_Settings_NetGameType,tolua_set_Settings_NetGameType);
   tolua_array(tolua_S,"Presets",tolua_get_stratagus_Settings_Presets,tolua_set_stratagus_Settings_Presets);
   tolua_variable(tolua_S,"Resources",tolua_get_Settings_Resources,tolua_set_Settings_Resources);
   tolua_variable(tolua_S,"NumUnits",tolua_get_Settings_NumUnits,tolua_set_Settings_NumUnits);
   tolua_variable(tolua_S,"Opponents",tolua_get_Settings_Opponents,tolua_set_Settings_Opponents);
   tolua_variable(tolua_S,"Difficulty",tolua_get_Settings_Difficulty,tolua_set_Settings_Difficulty);
   tolua_variable(tolua_S,"GameType",tolua_get_Settings_GameType,tolua_set_Settings_GameType);
   tolua_variable(tolua_S,"NoFogOfWar",tolua_get_Settings_NoFogOfWar,tolua_set_Settings_NoFogOfWar);
   tolua_variable(tolua_S,"RevealMap",tolua_get_Settings_RevealMap,tolua_set_Settings_RevealMap);
   tolua_variable(tolua_S,"MapRichness",tolua_get_Settings_MapRichness,tolua_set_Settings_MapRichness);
  tolua_endmodule(tolua_S);
  tolua_variable(tolua_S,"GameSettings",tolua_get_GameSettings,tolua_set_GameSettings);
  tolua_constant(tolua_S,"SettingsPresetMapDefault",SettingsPresetMapDefault);
  tolua_constant(tolua_S,"SettingsGameTypeMapDefault",SettingsGameTypeMapDefault);
  tolua_constant(tolua_S,"SettingsGameTypeMelee",SettingsGameTypeMelee);
  tolua_constant(tolua_S,"SettingsGameTypeFreeForAll",SettingsGameTypeFreeForAll);
  tolua_constant(tolua_S,"SettingsGameTypeTopVsBottom",SettingsGameTypeTopVsBottom);
  tolua_constant(tolua_S,"SettingsGameTypeLeftVsRight",SettingsGameTypeLeftVsRight);
  tolua_constant(tolua_S,"SettingsGameTypeManVsMachine",SettingsGameTypeManVsMachine);
  tolua_constant(tolua_S,"SettingsGameTypeManTeamVsMachine",SettingsGameTypeManTeamVsMachine);
  tolua_array(tolua_S,"AlliedUnitRecyclingEfficiency",tolua_get_stratagus_AlliedUnitRecyclingEfficiency,tolua_set_stratagus_AlliedUnitRecyclingEfficiency);
  tolua_array(tolua_S,"EnemyUnitRecyclingEfficiency",tolua_get_stratagus_EnemyUnitRecyclingEfficiency,tolua_set_stratagus_EnemyUnitRecyclingEfficiency);
  tolua_cclass(tolua_S,"CMapInfo","CMapInfo","",NULL);
  tolua_beginmodule(tolua_S,"CMapInfo");
   tolua_variable(tolua_S,"Description",tolua_get_CMapInfo_Description,tolua_set_CMapInfo_Description);
   tolua_variable(tolua_S,"MapWidth",tolua_get_CMapInfo_MapWidth,tolua_set_CMapInfo_MapWidth);
   tolua_variable(tolua_S,"MapHeight",tolua_get_CMapInfo_MapHeight,tolua_set_CMapInfo_MapHeight);
   tolua_array(tolua_S,"PlayerType",tolua_get_stratagus_CMapInfo_PlayerType,tolua_set_stratagus_CMapInfo_PlayerType);
  tolua_endmodule(tolua_S);
  tolua_cclass(tolua_S,"CMap","CMap","",NULL);
  tolua_beginmodule(tolua_S,"CMap");
   tolua_variable(tolua_S,"Info",tolua_get_CMap_Info,tolua_set_CMap_Info);
   tolua_variable(tolua_S,"PatchManager",tolua_get_CMap_PatchManager,tolua_set_CMap_PatchManager);
  tolua_endmodule(tolua_S);
  tolua_variable(tolua_S,"Map",tolua_get_Map,tolua_set_Map);
  tolua_cclass(tolua_S,"CPatchType","CPatchType","",NULL);
  tolua_beginmodule(tolua_S,"CPatchType");
   tolua_function(tolua_S,"getGraphic",tolua_stratagus_CPatchType_getGraphic00);
   tolua_function(tolua_S,"getTileWidth",tolua_stratagus_CPatchType_getTileWidth00);
   tolua_function(tolua_S,"getTileHeight",tolua_stratagus_CPatchType_getTileHeight00);
   tolua_function(tolua_S,"getFlag",tolua_stratagus_CPatchType_getFlag00);
  tolua_endmodule(tolua_S);
  tolua_cclass(tolua_S,"CPatch","CPatch","",NULL);
  tolua_beginmodule(tolua_S,"CPatch");
   tolua_function(tolua_S,"getType",tolua_stratagus_CPatch_getType00);
   tolua_function(tolua_S,"setPos",tolua_stratagus_CPatch_setPos00);
   tolua_function(tolua_S,"setX",tolua_stratagus_CPatch_setX00);
   tolua_function(tolua_S,"getX",tolua_stratagus_CPatch_getX00);
   tolua_function(tolua_S,"setY",tolua_stratagus_CPatch_setY00);
   tolua_function(tolua_S,"getY",tolua_stratagus_CPatch_getY00);
  tolua_endmodule(tolua_S);
  tolua_cclass(tolua_S,"CPatchManager","CPatchManager","",NULL);
  tolua_beginmodule(tolua_S,"CPatchManager");
   tolua_function(tolua_S,"add",tolua_stratagus_CPatchManager_add00);
   tolua_function(tolua_S,"moveToTop",tolua_stratagus_CPatchManager_moveToTop00);
   tolua_function(tolua_S,"moveToBottom",tolua_stratagus_CPatchManager_moveToBottom00);
   tolua_function(tolua_S,"getPatch",tolua_stratagus_CPatchManager_getPatch00);
   tolua_function(tolua_S,"getPatchTypeNames",tolua_stratagus_CPatchManager_getPatchTypeNames00);
   tolua_function(tolua_S,"getPatchTypeNamesUsingGraphic",tolua_stratagus_CPatchManager_getPatchTypeNamesUsingGraphic00);
   tolua_function(tolua_S,"newPatchType",tolua_stratagus_CPatchManager_newPatchType00);
   tolua_function(tolua_S,"computePatchSize",tolua_stratagus_CPatchManager_computePatchSize00);
  tolua_endmodule(tolua_S);
  tolua_variable(tolua_S,"AStarFixedUnitCrossingCost",tolua_get_AStarFixedUnitCrossingCost,tolua_set_AStarFixedUnitCrossingCost);
  tolua_variable(tolua_S,"AStarMovingUnitCrossingCost",tolua_get_AStarMovingUnitCrossingCost,tolua_set_AStarMovingUnitCrossingCost);
  tolua_variable(tolua_S,"AStarKnowUnseenTerrain",tolua_get_AStarKnowUnseenTerrain,tolua_set_AStarKnowUnseenTerrain);
  tolua_variable(tolua_S,"AStarUnknownTerrainCost",tolua_get_AStarUnknownTerrainCost,tolua_set_AStarUnknownTerrainCost);
  tolua_function(tolua_S,"GetNumOpponents",tolua_stratagus_GetNumOpponents00);
  tolua_function(tolua_S,"GetTimer",tolua_stratagus_GetTimer00);
  tolua_function(tolua_S,"ActionVictory",tolua_stratagus_ActionVictory00);
  tolua_function(tolua_S,"ActionDefeat",tolua_stratagus_ActionDefeat00);
  tolua_function(tolua_S,"ActionDraw",tolua_stratagus_ActionDraw00);
  tolua_function(tolua_S,"ActionSetTimer",tolua_stratagus_ActionSetTimer00);
  tolua_function(tolua_S,"ActionStartTimer",tolua_stratagus_ActionStartTimer00);
  tolua_function(tolua_S,"ActionStopTimer",tolua_stratagus_ActionStopTimer00);
  tolua_function(tolua_S,"SetTrigger",tolua_stratagus_SetTrigger00);
  #ifdef __cplusplus
  tolua_cclass(tolua_S,"CPosition","CPosition","",tolua_collect_CPosition);
  #else
  tolua_cclass(tolua_S,"CPosition","CPosition","",NULL);
  #endif
  tolua_beginmodule(tolua_S,"CPosition");
   tolua_function(tolua_S,"new",tolua_stratagus_CPosition_new00);
   tolua_function(tolua_S,"new_local",tolua_stratagus_CPosition_new00_local);
   tolua_function(tolua_S,".call",tolua_stratagus_CPosition_new00_local);
   tolua_variable(tolua_S,"x",tolua_get_CPosition_x,tolua_set_CPosition_x);
   tolua_variable(tolua_S,"y",tolua_get_CPosition_y,tolua_set_CPosition_y);
  tolua_endmodule(tolua_S);
  #ifdef __cplusplus
  tolua_cclass(tolua_S,"GraphicAnimation","GraphicAnimation","Animation",tolua_collect_GraphicAnimation);
  #else
  tolua_cclass(tolua_S,"GraphicAnimation","GraphicAnimation","Animation",NULL);
  #endif
  tolua_beginmodule(tolua_S,"GraphicAnimation");
   tolua_function(tolua_S,"new",tolua_stratagus_GraphicAnimation_new00);
   tolua_function(tolua_S,"new_local",tolua_stratagus_GraphicAnimation_new00_local);
   tolua_function(tolua_S,".call",tolua_stratagus_GraphicAnimation_new00_local);
   tolua_function(tolua_S,"clone",tolua_stratagus_GraphicAnimation_clone00);
  tolua_endmodule(tolua_S);
  tolua_cclass(tolua_S,"CParticle","CParticle","",NULL);
  tolua_beginmodule(tolua_S,"CParticle");
   tolua_function(tolua_S,"clone",tolua_stratagus_CParticle_clone00);
  tolua_endmodule(tolua_S);
  #ifdef __cplusplus
  tolua_cclass(tolua_S,"StaticParticle","StaticParticle","CParticle",tolua_collect_StaticParticle);
  #else
  tolua_cclass(tolua_S,"StaticParticle","StaticParticle","CParticle",NULL);
  #endif
  tolua_beginmodule(tolua_S,"StaticParticle");
   tolua_function(tolua_S,"new",tolua_stratagus_StaticParticle_new00);
   tolua_function(tolua_S,"new_local",tolua_stratagus_StaticParticle_new00_local);
   tolua_function(tolua_S,".call",tolua_stratagus_StaticParticle_new00_local);
  tolua_endmodule(tolua_S);
  #ifdef __cplusplus
  tolua_cclass(tolua_S,"CChunkParticle","CChunkParticle","CParticle",tolua_collect_CChunkParticle);
  #else
  tolua_cclass(tolua_S,"CChunkParticle","CChunkParticle","CParticle",NULL);
  #endif
  tolua_beginmodule(tolua_S,"CChunkParticle");
   tolua_function(tolua_S,"new",tolua_stratagus_CChunkParticle_new00);
   tolua_function(tolua_S,"new_local",tolua_stratagus_CChunkParticle_new00_local);
   tolua_function(tolua_S,".call",tolua_stratagus_CChunkParticle_new00_local);
  tolua_endmodule(tolua_S);
  tolua_cclass(tolua_S,"CSmokeParticle","CSmokeParticle","CParticle",NULL);
  tolua_beginmodule(tolua_S,"CSmokeParticle");
  tolua_endmodule(tolua_S);
  #ifdef __cplusplus
  tolua_cclass(tolua_S,"CParticleManager","CParticleManager","",tolua_collect_CParticleManager);
  #else
  tolua_cclass(tolua_S,"CParticleManager","CParticleManager","",NULL);
  #endif
  tolua_beginmodule(tolua_S,"CParticleManager");
   tolua_function(tolua_S,"new",tolua_stratagus_CParticleManager_new00);
   tolua_function(tolua_S,"new_local",tolua_stratagus_CParticleManager_new00_local);
   tolua_function(tolua_S,".call",tolua_stratagus_CParticleManager_new00_local);
   tolua_function(tolua_S,"delete",tolua_stratagus_CParticleManager_delete00);
   tolua_function(tolua_S,"add",tolua_stratagus_CParticleManager_add00);
  tolua_endmodule(tolua_S);
  tolua_variable(tolua_S,"ParticleManager",tolua_get_ParticleManager,tolua_set_ParticleManager);
  tolua_function(tolua_S,"Translate",tolua_stratagus_Translate00);
  tolua_function(tolua_S,"AddTranslation",tolua_stratagus_AddTranslation00);
  tolua_function(tolua_S,"LoadPO",tolua_stratagus_LoadPO00);
  tolua_function(tolua_S,"SetTranslationsFiles",tolua_stratagus_SetTranslationsFiles00);
  tolua_variable(tolua_S,"StratagusTranslation",tolua_get_StratagusTranslation,tolua_set_StratagusTranslation);
  tolua_variable(tolua_S,"GameTranslation",tolua_get_GameTranslation,tolua_set_GameTranslation);
  tolua_function(tolua_S,"SaveGame",tolua_stratagus_SaveGame00);
  tolua_function(tolua_S,"_",tolua_stratagus__00);
  tolua_function(tolua_S,"SyncRand",tolua_stratagus_SyncRand00);
  tolua_function(tolua_S,"SyncRand",tolua_stratagus_SyncRand01);
  tolua_function(tolua_S,"Exit",tolua_stratagus_Exit00);
  tolua_variable(tolua_S,"CliMapName",tolua_get_CliMapName,tolua_set_CliMapName);
 tolua_endmodule(tolua_S);
 return 1;
}


#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 501
 TOLUA_API int luaopen_stratagus (lua_State* tolua_S) {
 return tolua_stratagus_open(tolua_S);
};
#endif

