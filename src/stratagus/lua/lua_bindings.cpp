//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
//      lua_bindings.cpp - bindings of C++ objects and functions to Lua globals
//
//      (c) Copyright 2022 by Tim Felgentreff
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

#include "stratagus.h"
#include "script.h"
#include "settings.h"

#include "ai.h"
#include "editor.h"
#include "font.h"
#include "game.h"
#include "interface.h"
#include "iolib.h"
#include "map.h"
#include "minimap.h"
#include "movie.h"
#include "netconnect.h"
#include "network.h"
#include "particle.h"
#include "pathfinder.h"
#include "player.h"
#include "replay.h"
#include "results.h"
#include "sound_server.h"
#include "sound.h"
#include "tileset.h"
#include "translate.h"
#include "ui.h"
#include "unit_manager.h"
#include "unit.h"
#include "unittype.h"
#include "upgrade.h"
#include "video.h"
#include "widgets.h"


#include <LuaBridge/LuaBridge.h>


extern bool IsReplayGame();
extern void StartMap(const std::string &filename, bool clean = true);
extern void StartReplay(const std::string &filename, bool reveal = true);
extern void StartSavedGame(const std::string &filename);
extern int SaveReplay(const std::string &filename);
extern std::string NetworkMapName;
extern std::string NetworkMapFragmentName;
extern std::string CliMapName;
extern int GetNumOpponents(int player);
extern int GetTimer();
extern void ActionVictory();
extern void ActionDefeat();
extern void ActionDraw();
extern void ActionSetTimer(int cycles, bool increasing);
extern void ActionStartTimer();
extern void ActionStopTimer();
extern void SetTrigger(int trigger);
extern void ShowFullImage(const std::string &filename, unsigned int timeOutInSecond);

template <class T, int SZ>
struct CArray {
    CArray(T *array) {
        this->array = array;
    }

    int size() {
        return SZ;
    }

    T& operator[](int idx) {
        return array[idx];
    }

    T operator[](int idx) const {
        return array[idx];
    }

    T *array;
};

template <class T>
struct ExposedVector {
    ExposedVector(const std::vector<T> *v) {
        this->vectorPtr = const_cast<std::vector<T> *>(v);
    }

    std::vector<T> *vectorPtr;
};

/**
 * @brief Expose C++ functions, variables, classes, and structures to Lua.
 * 
 * This replaces the generated code from tolua++. For backwards compatibility,
 * we export the old exposed APIs first. This should be kept until the end of
 * the 3.x series. These APIs are copied from the "sg" module to the global
 * namespace so that games continue to work. Any new API will only be exposed
 * on the "sg" module.
 */
int tolua_stratagus_open(lua_State *Lua) {
    using IntArray = CArray<int, PlayerMax>;
    using Int7Array = CArray<const int, MaxCosts>;
    using Int8Array = CArray<const int, 8>;
    using ReadyArray = CArray<const uint8_t, PlayerMax>;
    using SlotOptionArray = CArray<const SlotOption, PlayerMax>;
    using ConstIntArray = CArray<const int, PlayerMax>;
    using ConstInt7Array = CArray<const int, MaxCosts>;
    using CResourceInfoArray = CArray<const CResourceInfo, MaxResourceInfo>;
    using ConstInt2048Array = CArray<const int, UnitTypeMax>;
    using CColorArray = CArray<const CColor, MaxFontColors>;
    using SettingsPresetsArray = CArray<const SettingsPresets, PlayerMax>;
    using PlayerTypesArray = CArray<const PlayerTypes, PlayerMax>;
    using SettingsPresetsRaceArray = CArray<const SettingsPresets, PlayerMax>;
    using CNetworkHostArray = CArray<CNetworkHost, PlayerMax>;
    using CPlayerArray = CArray<CPlayer, PlayerMax>;

    luabridge::getGlobalNamespace(Lua).beginNamespace("sg")
        // proxy array definitions
        .beginClass<IntArray>("__IntArray")
            .addFunction("__index", +[](IntArray* thiz, int index, lua_State* L) -> int {
                if (index < 0 || index >= PlayerMax)
                    luaL_error(L, "Invalid index access in table %d", index);
                return (*thiz)[index];
            })
            .addFunction("__newindex", +[](IntArray* thiz, int index, const int value, lua_State* L) {
                if (index < 0 || index >= PlayerMax)
                    luaL_error(L, "Invalid index access in table %d", index);
                (*thiz)[index] = value;
            })
        .endClass()
        .beginClass<Int7Array>("__Int7Array")
            .addFunction("__index", +[](Int7Array* thiz, int index, lua_State* L) -> int {
                if (index < 0 || index > MaxCosts)
                    luaL_error(L, "Invalid index access in table %d", index);
                return (*thiz)[index];
            })
            .addFunction("__newindex", +[](Int7Array* thiz, int index, const int value, lua_State* L) {
                if (index < 0 || index > MaxCosts)
                    luaL_error(L, "Invalid index access in table %d", index);
                const_cast<int *>(thiz->array)[index] = value;
            })
        .endClass()
        .beginClass<Int8Array>("__Int8Array")
            .addFunction("__index", +[](Int8Array* thiz, int index, lua_State* L) -> int {
                if (index < 0 || index > 8)
                    luaL_error(L, "Invalid index access in table %d", index);
                return (*thiz)[index];
            })
            .addFunction("__newindex", +[](Int8Array* thiz, int index, const int value, lua_State* L) {
                if (index < 0 || index > 8)
                    luaL_error(L, "Invalid index access in table %d", index);
                const_cast<int *>(thiz->array)[index] = value;
            })
        .endClass()
        .beginClass<CResourceInfoArray>("__CResourceInfoArray")
            .addFunction("__index", +[](CResourceInfoArray* thiz, int index, lua_State* L) {
                if (index < 0 || index > MaxResourceInfo)
                    luaL_error(L, "Invalid index access in table %d", index);
                return (*thiz)[index];
            })
            .addFunction("__newindex", +[](CResourceInfoArray* thiz, int index, const CResourceInfo &value, lua_State* L) {
                luaL_error(L, "Read-only array");
            })
        .endClass()
        .beginClass<ReadyArray>("__ReadyArray")
            .addFunction("__index", +[](ReadyArray* thiz, int index, lua_State* L) -> int {
                if (index < 0 || index >= PlayerMax)
                    luaL_error(L, "Invalid index access in table %d", index);
                return (*thiz)[index];
            })
            .addFunction("__newindex", +[](ReadyArray* thiz, int index, const int value, lua_State* L) {
                if (index < 0 || index >= PlayerMax)
                    luaL_error(L, "Invalid index access in table %d", index);
                const_cast<uint8_t *>(thiz->array)[index] = value;
            })
        .endClass()
        .beginClass<SlotOptionArray>("__SlotOptionArray")
            .addFunction("__index", +[](SlotOptionArray* thiz, int index, lua_State* L) -> int {
                if (index < 0 || index >= PlayerMax)
                    luaL_error(L, "Invalid index access in table %d", index);
                return static_cast<int>((*thiz)[index]);
            })
            .addFunction("__newindex", +[](SlotOptionArray* thiz, int index, const int value, lua_State* L) {
                if (index < 0 || index >= PlayerMax)
                    luaL_error(L, "Invalid index access in table %d", index);
                const_cast<SlotOption *>(thiz->array)[index] = static_cast<SlotOption>(value);
            })
        .endClass()
        .beginClass<ConstIntArray>("__ConstIntArray")
            .addFunction("__index", +[](ConstIntArray* thiz, int index, lua_State* L) -> int {
                if (index < 0 || index >= PlayerMax)
                    luaL_error(L, "Invalid index access in table %d", index);
                return (*thiz)[index];
            })
            .addFunction("__newindex", +[](ConstIntArray* thiz, int index, const int value, lua_State* L) {
                luaL_error(L, "Read-only array");
            })
        .endClass()
        .beginClass<ConstInt7Array>("__ConstInt7Array")
            .addFunction("__index", +[](ConstInt7Array* thiz, int index, lua_State* L) -> int {
                if (index < 0 || index >= MaxCosts)
                    luaL_error(L, "Invalid index access in table %d", index);
                return (*thiz)[index];
            })
            .addFunction("__newindex", +[](ConstInt7Array* thiz, int index, const int value, lua_State* L) {
                if (index < 0 || index > MaxCosts)
                    luaL_error(L, "Invalid index access in table %d", index);
                const_cast<int *>(thiz->array)[index] = value;
            })
        .endClass()
        .beginClass<ConstInt2048Array>("__ConstInt2048Array")
            .addFunction("__index", +[](ConstInt2048Array* thiz, int index, lua_State* L) -> int {
                if (index < 0 || index >= UnitTypeMax)
                    luaL_error(L, "Invalid index access in table %d", index);
                return (*thiz)[index];
            })
            .addFunction("__newindex", +[](ConstInt2048Array* thiz, int index, const int value, lua_State* L) {
                luaL_error(L, "Read-only array");
            })
        .endClass()
        .beginClass<CColorArray>("__CColorArray")
            .addFunction("__index", +[](CColorArray* thiz, int index, lua_State* L) -> CColor {
                if (index < 0 || index >= MaxFontColors)
                    luaL_error(L, "Invalid index access in table %d", index);
                return (*thiz)[index];
            })
            .addFunction("__newindex", +[](CColorArray* thiz, int index, const CColor &value, lua_State* L) {
                if (index < 0 || index >= MaxFontColors)
                    luaL_error(L, "Invalid index access in table %d", index);
                const_cast<CColor *>(thiz->array)[index] = value;
            })
        .endClass()
        .beginClass<SettingsPresetsArray>("__SettingsPresetsArray")
            .addFunction("__index", +[](SettingsPresetsArray* thiz, int index, lua_State* L) -> SettingsPresets {
                if (index < 0 || index >= PlayerMax)
                    luaL_error(L, "Invalid index access in table %d", index);
                return (*thiz)[index];
            })
            .addFunction("__newindex", +[](SettingsPresetsArray* thiz, int index, const SettingsPresets &value, lua_State* L) {
                luaL_error(L, "Read-only array");
            })
        .endClass()
        .beginClass<PlayerTypesArray>("__PlayerTypesArray")
            .addFunction("__index", +[](PlayerTypesArray* thiz, int index, lua_State* L) -> PlayerTypes {
                if (index < 0 || index >= PlayerMax)
                    luaL_error(L, "Invalid index access in table %d", index);
                
                return (*thiz)[index];
            })
            .addFunction("__newindex", +[](PlayerTypesArray* thiz, int index, const PlayerTypes &value, lua_State* L) {
                luaL_error(L, "Read-only array");
            })
        .endClass()
        .beginClass<SettingsPresetsRaceArray>("__SettingsPresetsRaceArray")
            .addFunction("__index", +[](SettingsPresetsRaceArray* thiz, int index, lua_State* L) -> int {
                if (index < 0 || index >= PlayerMax)
                    luaL_error(L, "Invalid index access in table %d", index);
                return (*thiz)[index].Race;
            })
            .addFunction("__newindex", +[](SettingsPresetsRaceArray* thiz, int index, int value, lua_State* L) {
                if (index < 0 || index >= PlayerMax)
                    luaL_error(L, "Invalid index access in table %d", index);
                const_cast<SettingsPresets *>(thiz->array)[index].Race = value;
            })
        .endClass()
        .beginClass<CNetworkHostArray>("__CNetworkHostArray")
            .addFunction("__index", +[](CNetworkHostArray* thiz, int index, lua_State* L) -> CNetworkHost {
                if (index < 0 || index >= PlayerMax)
                    luaL_error(L, "Invalid index access in table %d", index);
                
                return (*thiz)[index];
            })
            .addFunction("__newindex", +[](CNetworkHostArray* thiz, int index, const CNetworkHost &value, lua_State* L) {
                luaL_error(L, "Read-only array");
            })
        .endClass()
        .beginClass<CPlayerArray>("__CPlayerArray")
            .addFunction("__index", +[](CPlayerArray* thiz, int index, lua_State* L) -> CPlayer {
                if (index < 0 || index >= PlayerMax)
                    luaL_error(L, "Invalid index access in table %d", index);
                return (*thiz)[index];
            })
            .addFunction("__newindex", +[](CPlayerArray* thiz, int index, const CPlayer &value, lua_State* L) {
                luaL_error(L, "Read-only array");
            })
        .endClass()

        // ai.pkg
        .addFunction("AiAttackWithForceAt", AiAttackWithForceAt)
        .addFunction("AiAttackWithForce", AiAttackWithForce)
        // editor.pkg
        .addFunction("StartEditor", StartEditor)
        .addFunction("EditorSaveMap", EditorSaveMap)
        .addProperty("EditorNotRunning", +[]() { return EditorNotRunning; })
        .addProperty("EditorStarted", +[]() { return EditorStarted; })
        .addProperty("EditorCommandLine", +[]() { return EditorCommandLine; })
        .addProperty("EditorEditing", +[]() { return EditorEditing; })
        .beginClass<CEditor>("CEditor")
            .addProperty("UnitTypes", &CEditor::UnitTypes)
            .addProperty("TerrainEditable", &CEditor::TerrainEditable)
            // .addProperty("CUnitType", &CEditor::StartUnit)
            .addProperty("WriteCompressedMaps", &CEditor::WriteCompressedMaps)
            .addProperty("Running", +[](const CEditor *editor) { return static_cast<int>(editor->Running); },
                                    +[](CEditor *editor, int arg) { editor->Running = static_cast<EditorRunningType>(arg); })
            .addFunction("CreateRandomMap", &CEditor::CreateRandomMap)
        .endClass()
        .addProperty("Editor", &Editor)
        // font.pkg
        .beginClass<CFont>("CFont")
            .addStaticFunction("New", +[](lua_State *l) {
                int nargs = lua_gettop(l);
                std::string ident(luabridge::Stack<std::string>::get(l, nargs - 1));
                CGraphic *g = luabridge::Stack<CGraphic*>::get(l, nargs);
                return CFont::New(ident, g);
            })
            .addStaticFunction("Get", +[](lua_State *l) {
                int nargs = lua_gettop(l);
                std::string ident(lua_tostring(l, nargs));
                return CFont::Get(ident);
            })
            .addFunction("Height", &CFont::Height)
            .addFunction("Width", +[](const CFont* font, std::string &text) { return font->Width(text); })
        .endClass()
        .addProperty("MaxFontColors", +[]() { return MaxFontColors; })
        .beginClass<CFontColor>("CFontColor")
            .addStaticFunction("New", +[](lua_State *l) {
                int nargs = lua_gettop(l);
                std::string ident(lua_tostring(l, nargs));
                return CFontColor::New(ident);
            })
            .addStaticFunction("Get", +[](lua_State *l) {
                int nargs = lua_gettop(l);
                std::string ident(lua_tostring(l, nargs));
                return CFontColor::Get(ident);
            })
            .addProperty("Colors", +[](const CFontColor* fc) { return static_cast<CColorArray>(fc->Colors); })
        .endClass()
        // game.pkg
        .addFunction("IsReplayGame", IsReplayGame)
        .addFunction("StartMap", StartMap)
        .addFunction("StartReplay", StartReplay)
        .addFunction("StartSavedGame", StartSavedGame)
        .addFunction("SaveReplay", SaveReplay)
        .addProperty("GameNoResult", +[]() { return GameNoResult; })
        .addProperty("GameVictory", +[]() { return GameVictory; })
        .addProperty("GameDefeat", +[]() { return GameDefeat; })
        .addProperty("GameDraw", +[]() { return GameDraw; })
        .addProperty("GameQuitToMenu", +[]() { return GameQuitToMenu; })
        .addProperty("GameRestart", +[]() { return GameRestart; })
        .addProperty("GameResult", +[]() { return GameResult; })
        .addFunction("StopGame", StopGame)
        .addProperty("GameRunning", &GameRunning)
        .addFunction("SetGamePaused", SetGamePaused)
        .addFunction("GetGamePaused", GetGamePaused)
        .addProperty("GamePaused", +[]() { return GetGamePaused(); }, +[](bool paused) { SetGamePaused(paused); })
        .addFunction("SetGameSpeed", SetGameSpeed)
        .addFunction("GetGameSpeed", GetGameSpeed)
        .addProperty("GameSpeed", +[]() { return GetGameSpeed(); }, +[](int speed) { SetGameSpeed(speed); })
        .addProperty("GameObserve", &GameObserve)
        .addProperty("GameEstablishing", &GameEstablishing)
        .addProperty("GameCycle", &GameCycle, false)
        .addProperty("FastForwardCycle", &FastForwardCycle)
        .beginClass<SettingsPresets>("SettingsPresets")
            .addProperty("PlayerColor", &SettingsPresets::PlayerColor)
            .addProperty("AIScript", &SettingsPresets::AIScript)
            .addProperty("Race", &SettingsPresets::Race)
            .addProperty("Team", &SettingsPresets::Team)
            .addProperty("Type", &SettingsPresets::Type)
        .endClass()
        .beginClass<Settings>("Settings")
            .addProperty("NetGameType", +[](const Settings* settings) { return static_cast<int>(settings->NetGameType); },
                                        +[](Settings* settings, int type) { settings->NetGameType = static_cast<NetGameTypes>(type); })
            .addProperty("Presets", +[](const Settings *settings) { return static_cast<SettingsPresetsArray>(settings->Presets); })
            .addProperty("Resources", &Settings::Resources)
            .addProperty("NumUnits", &Settings::NumUnits)
            .addProperty("Opponents", &Settings::Opponents)
            .addProperty("Difficulty", &Settings::Difficulty)
            .addProperty("GameType", +[](const Settings* settings) { return static_cast<int>(settings->GameType); },
                                     +[](Settings* settings, int type) { settings->GameType = static_cast<GameTypes>(type); })
            .addProperty("NoFogOfWar", +[](const Settings* settings) { return static_cast<bool>(settings->NoFogOfWar); },
                                       +[](Settings* settings, bool v) { settings->NoFogOfWar = v; })
            .addProperty("Inside", +[](const Settings* settings) { return static_cast<bool>(settings->Inside); },
                                   +[](Settings* settings, bool v) { settings->Inside = v; })
            .addProperty("RevealMap", +[](const Settings* settings) { return static_cast<int>(settings->RevealMap); },
                                      +[](Settings* settings, int type) { settings->RevealMap = static_cast<MapRevealModes>(type); })
            .addProperty("MapRichness", +[](const Settings*) { PrintOnStdOut("[DEPRECATION WARNING] GameSettings.MapRichness is deprecated."); return 0; },
                                        +[](Settings*, int) { PrintOnStdOut("[DEPRECATION WARNING] GameSettings.MapRichness is deprecated."); })
        .endClass()
        .addProperty("GameSettings", &GameSettings, false)
        .addProperty("SettingsPresetMapDefault", +[]() { return SettingsPresetMapDefault; })
        .addProperty("SettingsGameTypeMapDefault", +[]() { return static_cast<int>(GameTypes::SettingsGameTypeMapDefault); })
        .addProperty("SettingsGameTypeMelee", +[]() { return static_cast<int>(GameTypes::SettingsGameTypeMelee); })
        .addProperty("SettingsGameTypeFreeForAll", +[]() { return static_cast<int>(GameTypes::SettingsGameTypeFreeForAll); })
        .addProperty("SettingsGameTypeTopVsBottom", +[]() { return static_cast<int>(GameTypes::SettingsGameTypeTopVsBottom); })
        .addProperty("SettingsGameTypeLeftVsRight", +[]() { return static_cast<int>(GameTypes::SettingsGameTypeLeftVsRight); })
        .addProperty("SettingsGameTypeManVsMachine", +[]() { return static_cast<int>(GameTypes::SettingsGameTypeManVsMachine); })
        .addProperty("SettingsGameTypeManTeamVsMachine", +[]() { return static_cast<int>(GameTypes::SettingsGameTypeManTeamVsMachine); })
        .addProperty("SettingsGameTypeMachineVsMachine", +[]() { return static_cast<int>(GameTypes::SettingsGameTypeMachineVsMachine); })
        .addProperty("SettingsGameTypeMachineVsMachineTraining", +[]() { return static_cast<int>(GameTypes::SettingsGameTypeMachineVsMachineTraining); })
        // map.pkg
        .beginClass<CMapInfo>("CMapInfo")
            .addProperty("Description", &CMapInfo::Description)
            .addProperty("Filename", &CMapInfo::Filename)
            .addProperty("Preamble", &CMapInfo::Preamble)
            .addProperty("Postamble", &CMapInfo::Postamble)
            .addProperty("MapWidth", &CMapInfo::MapWidth)
            .addProperty("MapHeight", &CMapInfo::MapHeight)
            .addProperty("PlayerType", +[](const CMapInfo *info) { return static_cast<PlayerTypesArray>(info->PlayerType); })
        .endClass()
        .beginClass<CTileset>("CTileset")
            .addProperty("Name", &CTileset::Name)
        .endClass()
        .beginClass<CMap>("CMap")
            .addProperty("Info", &CMap::Info)
            .addProperty("Tileset", &CMap::Tileset)
        .endClass()
        .addProperty("Map", &Map, false)
        .addFunction("SetTile", +[](int tile, int w, int h, int value = 0) { SetTile(tile, w, h, value); })
        // minimap.pkg
        .beginClass<CMinimap>("CMinimap")
            .addProperty("X", &CMinimap::X)
            .addProperty("Y", &CMinimap::Y)
            .addProperty("W", &CMinimap::W)
            .addProperty("H", &CMinimap::H)
            .addProperty("WithTerrain", &CMinimap::WithTerrain)
            .addProperty("ShowSelected", &CMinimap::ShowSelected)
            .addProperty("Transparent", &CMinimap::Transparent)
        .endClass()
        // network.pkg
        .addFunction("InitNetwork1", InitNetwork1)
        .addFunction("ExitNetwork1", ExitNetwork1)
        .addFunction("IsNetworkGame", IsNetworkGame)
        .addFunction("NetworkSetupServerAddress", NetworkSetupServerAddress)
        .addFunction("NetworkInitClientConnect", NetworkInitClientConnect)
        .addFunction("NetworkInitServerConnect", NetworkInitServerConnect)
        .addFunction("NetworkServerStartGame", NetworkServerStartGame)
        .addFunction("NetworkProcessClientRequest", NetworkProcessClientRequest)
        .addFunction("GetNetworkState", GetNetworkState)
        .addFunction("NetworkServerResyncClients", NetworkServerResyncClients)
        .addFunction("NetworkDetachFromServer", NetworkDetachFromServer)
        .beginClass<CServerSetup>("ServerSetup")
            .addProperty("ResourcesOption", +[](const CServerSetup *s) { return s->ServerGameSettings.Resources; },
                                            +[](CServerSetup *s, int value) { s->ServerGameSettings.Resources = value; })
            .addProperty("UnitsOption", +[](const CServerSetup *s) { return s->ServerGameSettings.NumUnits; },
                                        +[](CServerSetup *s, int value) { s->ServerGameSettings.NumUnits = value; })
            .addProperty("FogOfWar", +[](const CServerSetup *s) { return s->ServerGameSettings.NoFogOfWar != 1; },
                                     +[](CServerSetup *s, int value) { s->ServerGameSettings.NoFogOfWar = value ? 0 : 1; })
            .addProperty("Inside", +[](const CServerSetup *s) { return s->ServerGameSettings.Inside == 1; },
                                   +[](CServerSetup *s, int value) { s->ServerGameSettings.Inside = value ? 1 : 0; })
            .addProperty("RevealMap", +[](const CServerSetup *s) { return static_cast<int>(s->ServerGameSettings.RevealMap); },
                                      +[](CServerSetup *s, int value) { s->ServerGameSettings.RevealMap = static_cast<MapRevealModes>(value); })
            .addProperty("GameTypeOption", +[](const CServerSetup *s) { return static_cast<int>(s->ServerGameSettings.GameType); },
                                           +[](CServerSetup *s, int value) { s->ServerGameSettings.GameType = static_cast<GameTypes>(value); })
            .addProperty("Difficulty", +[](const CServerSetup *s) { return s->ServerGameSettings.Difficulty; },
                                       +[](CServerSetup *s, int value) { s->ServerGameSettings.Difficulty = value; })
            .addProperty("MapRichness", +[](const CServerSetup*) { PrintOnStdOut("[DEPRECATION WARNING] ServerSetup.MapRichness is deprecated."); return 0; },
                                        +[](CServerSetup*, int) { PrintOnStdOut("[DEPRECATION WARNING] ServerSetup.MapRichness is deprecated."); })
            .addProperty("Opponents", +[](const CServerSetup *s) { return s->ServerGameSettings.Opponents; },
                                       +[](CServerSetup *s, int value) { s->ServerGameSettings.Opponents = value; })
            .addProperty("CompOpt", +[](const CServerSetup *s) { return static_cast<SlotOptionArray>(s->CompOpt); })
            .addProperty("Ready", +[](const CServerSetup *s) { return static_cast<ReadyArray>(s->Ready); })
            .addProperty("Race", +[](const CServerSetup *s) { return static_cast<SettingsPresetsRaceArray>(s->ServerGameSettings.Presets); })
        .endClass()
        .addProperty("LocalSetupState", &LocalSetupState, false)
        .addProperty("ServerSetupState", &ServerSetupState, false)
        .addProperty("NetLocalHostsSlot", &NetLocalHostsSlot, false)
        .addProperty("NetPlayerNameSize", &NetLocalHostsSlot, false)
        .beginClass<CNetworkHost>("CNetworkHost")
            .addProperty("Host", &CNetworkHost::Host, false)
            .addProperty("Port", &CNetworkHost::Port, false)
            .addProperty("PlyNr", &CNetworkHost::PlyNr, false)
            .addProperty("PlyName", +[](const CNetworkHost *s) { return std::string(s->PlyName); })
        .endClass()
        .addProperty("Hosts", +[]() { return static_cast<CNetworkHostArray>(Hosts); })
        .addProperty("NetworkMapName", &NetworkMapName)
        .addProperty("NetworkMapFragmentName", &NetworkMapFragmentName)
        .addFunction("NetworkGamePrepareGameSettings", NetworkGamePrepareGameSettings)
        // particle.pkg
        .beginClass<CPosition>("CPosition")
            .addConstructor<void (*) (int, int)>()
            .addProperty("x", &CPosition::x)
            .addProperty("y", &CPosition::y)
        .endClass()
        .beginClass<GraphicAnimation>("GraphicAnimation")
            .addConstructor<void (*) (CGraphic *, int)>()
            .addFunction("clone", &GraphicAnimation::clone)
        .endClass()
        .beginClass<CParticle>("CParticle")
            .addFunction("clone", &CParticle::clone)
            .addFunction("setDrawLevel", &CParticle::setDrawLevel)
        .endClass()
        .deriveClass<StaticParticle, CParticle>("StaticParticle")
            .addConstructor<void (*) (CPosition, GraphicAnimation *, int)>()
        .endClass()
        .deriveClass<CChunkParticle, CParticle>("CChunkParticle")
            .addConstructor<void (*) (CPosition, GraphicAnimation *, GraphicAnimation *, GraphicAnimation *, int, int, int, int, int)>()
            .addFunction("getSmokeDrawLevel", &CChunkParticle::getSmokeDrawLevel)
            .addFunction("getDestroyDrawLevel", &CChunkParticle::getDestroyDrawLevel)      
            .addFunction("setSmokeDrawLevel", &CChunkParticle::setSmokeDrawLevel)
            .addFunction("setDestroyDrawLevel", &CChunkParticle::setDestroyDrawLevel)
        .endClass()
        .deriveClass<CSmokeParticle, CParticle>("CSmokeParticle")
            .addConstructor<void (*) (CPosition, GraphicAnimation *, float, float, int)>()
        .endClass()
        .deriveClass<CRadialParticle, CParticle>("CRadialParticle")
            .addConstructor<void (*) (CPosition, GraphicAnimation *, int, int)>()
        .endClass()
        .beginClass<CParticleManager>("CParticleManager")
            .addConstructor<void (*) ()>()
            .addFunction("add", &CParticleManager::add)
        .endClass()
        .addProperty("CParticleManager", &ParticleManager)
        // pathfinder.pkg
        .addProperty("AStarFixedUnitCrossingCost", +[]() { return GetAStarFixedUnitCrossingCost(); }, +[](int v) { SetAStarFixedUnitCrossingCost(v); })
        .addProperty("AStarMovingUnitCrossingCost", +[]() { return GetAStarMovingUnitCrossingCost(); }, +[](int v) { SetAStarMovingUnitCrossingCost(v); })
        .addProperty("AStarKnowUnseenTerrain", &AStarKnowUnseenTerrain)
        .addProperty("AStarUnknownTerrainCost", +[]() { return GetAStarUnknownTerrainCost(); }, +[](int v) { SetAStarUnknownTerrainCost(v); })
        // player.pkg
        .addProperty("PlayerNeutral", +[]() { return static_cast<int>(PlayerTypes::PlayerNeutral); })
        .addProperty("PlayerNobody", +[]() { return static_cast<int>(PlayerTypes::PlayerNobody); })
        .addProperty("PlayerComputer", +[]() { return static_cast<int>(PlayerTypes::PlayerComputer); })
        .addProperty("PlayerPerson", +[]() { return static_cast<int>(PlayerTypes::PlayerPerson); })
        .addProperty("PlayerRescuePassive", +[]() { return static_cast<int>(PlayerTypes::PlayerRescuePassive); })
        .addProperty("PlayerRescueActive", +[]() { return static_cast<int>(PlayerTypes::PlayerRescueActive); })
        .beginClass<CPlayer>("CPlayer")
            .addProperty("Index", &CPlayer::Index)
            .addProperty("Name", &CPlayer::Name)
            .addProperty("Type", +[](const CPlayer *p) { return static_cast<int>(p->Type); }, +[](CPlayer *p, int t) { p->Type = static_cast<PlayerTypes>(t); })
            .addProperty("Race", &CPlayer::Race)
            .addProperty("AiName", &CPlayer::AiName)
            .addProperty("StartPos", &CPlayer::StartPos)
            .addFunction("SetStartView", &CPlayer::SetStartView)
            .addProperty("Resources", +[](const CPlayer *p) { return static_cast<ConstInt7Array>(p->Resources); })
            .addProperty("StoredResources", +[](const CPlayer *p) { return static_cast<ConstInt7Array>(p->StoredResources); })
            .addProperty("Incomes", +[](const CPlayer *p) { return static_cast<ConstInt7Array>(p->Incomes); })
            .addProperty("Revenue", +[](const CPlayer *p) { return static_cast<ConstInt7Array>(p->Revenue); })
            .addProperty("UnitTypesCount", +[](const CPlayer *p) { return static_cast<ConstInt2048Array>(p->UnitTypesCount); })
            .addProperty("UnitTypesAiActiveCount", +[](const CPlayer *p) { return static_cast<ConstInt2048Array>(p->UnitTypesAiActiveCount); })
            .addProperty("AiEnabled", &CPlayer::AiEnabled)
            .addProperty("NumBuildings", &CPlayer::NumBuildings)
            .addProperty("Supply", &CPlayer::Supply)
            .addProperty("Demand", &CPlayer::Demand)
            .addProperty("UnitLimit", &CPlayer::UnitLimit)
            .addProperty("BuildingLimit", &CPlayer::BuildingLimit)
            .addProperty("TotalUnitLimit", &CPlayer::TotalUnitLimit)
            .addProperty("Score", &CPlayer::Score)
            .addProperty("TotalUnits", &CPlayer::TotalUnits)
            .addProperty("TotalBuildings", &CPlayer::TotalBuildings)
            .addProperty("TotalResources", +[](const CPlayer *p) { return static_cast<ConstInt7Array>(p->TotalResources); })
            .addProperty("TotalRazings", &CPlayer::TotalRazings)
            .addProperty("TotalKills", &CPlayer::TotalKills)
            .addProperty("SpeedResourcesHarvest", +[](const CPlayer *p) { return static_cast<ConstInt7Array>(p->SpeedResourcesHarvest); })
            .addProperty("SpeedResourcesReturn", +[](const CPlayer *p) { return static_cast<ConstInt7Array>(p->SpeedResourcesReturn); })
            .addProperty("SpeedBuild", &CPlayer::SpeedBuild)
            .addProperty("SpeedTrain", &CPlayer::SpeedTrain)
            .addProperty("SpeedUpgrade", &CPlayer::SpeedUpgrade)
            .addProperty("SpeedResearch", &CPlayer::SpeedResearch)
            .addFunction("GetUnit", &CPlayer::GetUnit)
            .addFunction("GetUnitCount", &CPlayer::GetUnitCount)
            .addFunction("IsEnemy", +[](const CPlayer *p, lua_State *l) -> bool {
                if (luabridge::Stack<CUnit&>::isInstance(l, 2)) {
                    return p->IsEnemy(luabridge::Stack<CUnit&>::get(l, 2));
                } else if (luabridge::Stack<CPlayer&>::isInstance(l, 2)) {
                    return p->IsEnemy(luabridge::Stack<CPlayer&>::get(l, 2));
                } else {
                    return false;
                }
            })
            .addFunction("IsAllied", +[](const CPlayer *p, lua_State *l) -> bool {
                if (luabridge::Stack<CUnit&>::isInstance(l, 2)) {
                    return p->IsAllied(luabridge::Stack<CUnit&>::get(l, 2));
                } else if (luabridge::Stack<CPlayer&>::isInstance(l, 2)) {
                    return p->IsAllied(luabridge::Stack<CPlayer&>::get(l, 2));
                } else {
                    return false;
                }
            })
            .addFunction("HasSharedVisionWith", &CPlayer::HasSharedVisionWith)
            .addFunction("IsTeamed", +[](const CPlayer *p, lua_State *l) -> bool {
                if (luabridge::Stack<CUnit&>::isInstance(l, 2)) {
                    return p->IsTeamed(luabridge::Stack<CUnit&>::get(l, 2));
                } else if (luabridge::Stack<CPlayer&>::isInstance(l, 2)) {
                    return p->IsTeamed(luabridge::Stack<CPlayer&>::get(l, 2));
                } else {
                    return false;
                }
            })
        .endClass()
        .addProperty("Players", +[]() { return static_cast<CPlayerArray>(Players); })
        .addProperty("ThisPlayer", &ThisPlayer)
        // sound.pkg
        .addFunction("GetEffectsVolume", GetEffectsVolume)
        .addFunction("SetEffectsVolume", SetEffectsVolume)
        .addFunction("GetMusicVolume", GetMusicVolume)
        .addFunction("SetMusicVolume", SetMusicVolume)
        .addFunction("SetEffectsEnabled", SetEffectsEnabled)
        .addFunction("IsEffectsEnabled", IsEffectsEnabled)
        .addFunction("SetMusicEnabled", SetMusicEnabled)
        .addFunction("IsMusicEnabled", IsMusicEnabled)
        .addFunction("PlayFile", PlayFile)
        .addFunction("PlaySoundFile", +[](lua_State *l) {
            if (lua_gettop(l) != 2) {
                return luaL_error(l, "wrong number of arguments");
            }
            if (!lua_isstring(l, 1)) {
                return luaL_error(l, "arg 1 must be string");
            }
            int r = PlayFile(lua_tostring(l, 1), new LuaActionListener(l, 2));
            lua_pushnumber(l, r);
            return 1;
        })
        .addFunction("PlayMusic", +[](std::string &file) { return PlayMusic(file); })
        .addFunction("StopMusic", StopMusic)
        .addFunction("IsMusicPlaying", IsMusicPlaying)
        .addFunction("SetChannelVolume", SetChannelVolume)
        .addFunction("SetChannelStereo", SetChannelStereo)
        .addFunction("StopChannel", StopChannel)
        .addFunction("StopAllChannels", StopAllChannels)
        // stratagus.pkg
        .beginClass<CArray<CIcon, 0>>("__CIconsAccessor")
            .addFunction("__index", +[](CArray<CIcon, 0>* thiz, std::string &key) -> CIcon* {
                return CIcon::Get(key);
            })
        .endClass()
        .addProperty("Icons", +[]() { return static_cast<CArray<CIcon, 0>>(nullptr); })
        .beginClass<CArray<CUpgrade, 0>>("__CUpgradesAccessor")
            .addFunction("__index", +[](CArray<CUpgrade, 0>* thiz, std::string &key) -> CUpgrade* {
                return CUpgrade::Get(key);
            })
        .endClass()
        .addProperty("Fonts", +[]() { return static_cast<CArray<CFont, 0>>(nullptr); })
        .beginClass<CArray<CFont, 0>>("__CFontsAccessor")
            .addFunction("__index", +[](CArray<CFont, 0>* thiz, std::string &key) -> CFont* {
                return CFont::Get(key);
            })
        .endClass()
        .addProperty("Fonts", +[]() { return static_cast<CArray<CFont, 0>>(nullptr); })
        .beginClass<CArray<CFontColor, 0>>("__CFontColorsAccessor")
            .addFunction("__index", +[](CArray<CFontColor, 0>* thiz, std::string &key) -> CFontColor* {
                return CFontColor::Get(key);
            })
        .endClass()
        .addProperty("FontColors", +[]() { return static_cast<CArray<CFontColor, 0>>(nullptr); })
        .beginClass<CArray<CUnitType, 0>>("__CUnitTypesAccessor")
            .addFunction("__index", +[](CArray<CUnitType, 0>* thiz, std::string &key) -> CUnitType* {
                return UnitTypeByIdent(key);
            })
        .endClass()
        .addProperty("UnitTypes", +[]() { return static_cast<CArray<CUnitType, 0>>(nullptr); })
        .addProperty("MaxCosts", +[]() { return MaxCosts; })
        .addProperty("FoodCost", +[]() -> int { return FoodCost; })
        .addProperty("ScoreCost", +[]() -> int { return ScoreCost; })
        .addProperty("ManaResCost", +[]() -> int { return ManaResCost; })
        .addProperty("FreeWorkersCount", +[]() -> int { return FreeWorkersCount; })
        .addProperty("MaxResouceInfo", +[]() -> int { return MaxResourceInfo; })
        .addProperty("PlayerMax", +[]() { return PlayerMax; })
        .addProperty("PlayerNumNeutral", +[]() { return PlayerNumNeutral; })
        .addProperty("InfiniteRepairRange", +[]() { return InfiniteRepairRange; })
        .addProperty("NoButton", +[]() { return NoButton; })
        .addProperty("LeftButton", +[]() { return LeftButton; })
        .addProperty("MiddleButton", +[]() { return MiddleButton; })
        .addProperty("RightButton", +[]() { return RightButton; })
        .addProperty("UpButton", +[]() { return UpButton; })
        .addProperty("DownButton", +[]() { return DownButton; })
        .addFunction("SaveGame", SaveGame)
        .addFunction("DeleteSaveGame", DeleteSaveGame)
        .addFunction("_", Translate)
        .addFunction("SyncRand", +[](lua_State *l) {
            switch (lua_gettop(l)) {
                case 0:
                    lua_pushinteger(l, SyncRand());
                    return 1;
                case 1:
                    if (lua_isnumber(l, 1)) {
                        lua_pushinteger(l, SyncRand(lua_tonumber(l, 1)));
                        return 1;
                    }
                default:
                    return luaL_error(l, "SyncRand needs none or a single integer argument");
            }
        })
        .addFunction("CanAccessFile", CanAccessFile)
        .addFunction("Exit", Exit)
        .addProperty("CliMapName", +[]() { return CliMapName; })
        .addProperty("StratagusLibPath", +[]() { return StratagusLibPath; })
        .addProperty("IsDebugEnabled", +[]() { return IsDebugEnabled; })
        // translate.pkg
        .addFunction("Translate", Translate)
        .addFunction("AddTranslation", AddTranslation)
        .addFunction("LoadPO", LoadPO)
        .addFunction("SetTranslationsFiles", SetTranslationsFiles)
        // trigger.pkg
        .addFunction("GetNumOpponents", GetNumOpponents)
        .addFunction("GetTimer", GetTimer)
        .addFunction("ActionVictory", ActionVictory)
        .addFunction("ActionDefeat", ActionDefeat)
        .addFunction("ActionDraw", ActionDraw)
        .addFunction("ActionSetTimer", ActionSetTimer)
        .addFunction("ActionStartTimer", ActionStartTimer)
        .addFunction("ActionStopTimer", ActionStopTimer)
        .addFunction("SetTrigger", SetTrigger)
        // ui.pkg
        .addProperty("VIEWPORT_SINGLE", +[]() { return VIEWPORT_SINGLE; })
        .addProperty("VIEWPORT_SPLIT_HORIZ", +[]() { return VIEWPORT_SPLIT_HORIZ; })
        .addProperty("VIEWPORT_SPLIT_HORIZ3", +[]() { return VIEWPORT_SPLIT_HORIZ3; })
        .addProperty("VIEWPORT_SPLIT_VERT", +[]() { return VIEWPORT_SPLIT_VERT; })
        .addProperty("VIEWPORT_QUAD", +[]() { return VIEWPORT_QUAD; })

        .beginClass<LuaActionListener>("LuaActionListener")
            .addStaticFunction("__call", +[](lua_State *l) {
                // custom constructor, first arg is the table, second the lua function
                return LuaActionListener(l, lua_gettop(l) < 2 ? 0 : 2);
            })
        .endClass()

        .beginClass<CUIButton>("CUIButton")
            .addConstructor<void (*) ()>()
            .addStaticFunction("new", +[](lua_State *l) {
                return new CUIButton(); // C++ lifetime
            })
            .addStaticFunction("new_local", +[](lua_State *l) {
                return CUIButton(); // Lua lifetime
            })
            .addProperty("X", &CUIButton::X)
            .addProperty("Y", &CUIButton::Y)
            .addProperty("Text", &CUIButton::Text)
            .addProperty("Style", &CUIButton::Style)
            .addProperty("Callback", &CUIButton::Callback)
            .addFunction("SetCallback", +[](CUIButton *btn, lua_State *l) { btn->Callback = new LuaActionListener(l, 2); })
        .endClass()

        .beginClass<CMapArea>("CMapArea")
            .addProperty("X", &CMapArea::X)
            .addProperty("Y", &CMapArea::Y)
            .addProperty("EndX", &CMapArea::EndX)
            .addProperty("EndY", &CMapArea::EndY)
            .addProperty("ScrollPaddingLeft", &CMapArea::ScrollPaddingLeft)
            .addProperty("ScrollPaddingRight", &CMapArea::ScrollPaddingRight)
            .addProperty("ScrollPaddingTop", &CMapArea::ScrollPaddingTop)
            .addProperty("ScrollPaddingBottom", &CMapArea::ScrollPaddingBottom)
        .endClass()

        .beginClass<CViewport>("CViewport")
        .endClass()

        .beginClass<CFiller>("CFiller")
            .addConstructor<void (*) ()>()
            .addStaticFunction("new_local", +[](lua_State *l) -> CFiller {  
                return CFiller();
            })
            .addProperty("G", &CFiller::G)
            .addProperty("X", &CFiller::X)
            .addProperty("Y", &CFiller::Y)
        .endClass()

        .beginClass<CButtonPanel>("CButtonPanel")
            .addProperty("X", &CButtonPanel::X)
            .addProperty("Y", &CButtonPanel::Y)
            .addProperty("Buttons", +[](const CButtonPanel *p) -> ExposedVector<CUIButton> { return &(p->Buttons); })
            .addProperty("AutoCastBorderColorRGB", &CButtonPanel::AutoCastBorderColorRGB)
            .addProperty("ShowCommandKey", &CButtonPanel::ShowCommandKey)
        .endClass()

        .beginClass<CPieMenu>("CPieMenu")
            .addProperty("G", &CPieMenu::G)
            .addProperty("MouseButton", &CPieMenu::MouseButton)
            .addProperty("X", +[](const CPieMenu* m) { return static_cast<Int8Array>(m->X); })
            .addProperty("Y", +[](const CPieMenu* m) { return static_cast<Int8Array>(m->Y); })
            .addFunction("SetRadius", &CPieMenu::SetRadius)
        .endClass()

        .beginClass<CResourceInfo>("CResourceInfo")
            .addProperty("G", &CResourceInfo::G)
            .addProperty("IconFrame", &CResourceInfo::IconFrame)
            .addProperty("IconX", &CResourceInfo::IconX)
            .addProperty("IconY", &CResourceInfo::IconY)
            .addProperty("IconWidth", &CResourceInfo::IconWidth)
            .addProperty("TextX", &CResourceInfo::TextX)
            .addProperty("TextY", &CResourceInfo::TextY)
        .endClass()

        .beginClass<CInfoPanel>("CInfoPanel")
            .addProperty("G", &CInfoPanel::G)
            .addProperty("X", &CInfoPanel::X)
            .addProperty("Y", &CInfoPanel::Y)
        .endClass()

        .beginClass<CUIUserButton>("CUIUserButton")
            .addConstructor<void (*) ()>()
            .addProperty("Clicked", &CUIUserButton::Clicked)
            .addProperty("Button", &CUIUserButton::Button)
        .endClass()

        .beginClass<CStatusLine>("CStatusLine")
            .addFunction("Set", &CStatusLine::Set)
            .addFunction("Get", &CStatusLine::Get)
            .addFunction("Clear", &CStatusLine::Clear)

            .addProperty("Width", &CStatusLine::Width)
            .addProperty("TextX", &CStatusLine::TextX)
            .addProperty("TextY", &CStatusLine::TextY)
            .addProperty("Font", &CStatusLine::Font)
        .endClass()

        .beginClass<CUITimer>("CUITimer")
            .addProperty("X", &CUITimer::X)
            .addProperty("Y", &CUITimer::Y)
            .addProperty("Font", &CUITimer::Font)
        .endClass()

        .beginClass<ExposedVector<CFiller>>("__vector_CFiller")
            .addFunction("clear", +[](ExposedVector<CFiller> *thiz) { thiz->vectorPtr->clear(); })
            .addFunction("push_back", +[](ExposedVector<CFiller> *thiz, CFiller filler) { thiz->vectorPtr->push_back(filler); })
        .endClass()
        .beginClass<ExposedVector<CUIButton>>("__vector_CUIButton")
            .addFunction("clear", +[](ExposedVector<CUIButton> *thiz) { thiz->vectorPtr->clear(); })
            .addFunction("push_back", +[](ExposedVector<CUIButton> *thiz, CUIButton btn) { thiz->vectorPtr->push_back(btn); })
        .endClass()
        .beginClass<ExposedVector<CUIUserButton>>("__vector_CUIUserButton")
            .addFunction("clear", +[](ExposedVector<CUIUserButton> *thiz) { thiz->vectorPtr->clear(); })
        .endClass()
        .beginClass<ExposedVector<std::string>>("__vector_string")
            .addFunction("clear", +[](ExposedVector<std::string> *thiz) { thiz->vectorPtr->clear(); })
        .endClass()
        .beginClass<ExposedVector<int>>("__vector_int")
            .addFunction("clear", +[](ExposedVector<int> *thiz) { thiz->vectorPtr->clear(); })
        .endClass()

        .beginClass<CUserInterface>("CUserInterface")
            .addProperty("NormalFontColor", &CUserInterface::NormalFontColor)
            .addProperty("ReverseFontColor", &CUserInterface::ReverseFontColor)

            .addProperty("Fillers", +[](const CUserInterface *ui) -> ExposedVector<CFiller> { return &(ui->Fillers); })

            .addProperty("Resources", +[](const CUserInterface *ui) { return static_cast<CResourceInfoArray>(ui->Resources); })
            .addProperty("InfoPanel", &CUserInterface::InfoPanel)
            .addProperty("SingleSelectedButton", &CUserInterface::SingleSelectedButton)

            .addProperty("SelectedButtons", +[](const CUserInterface *ui) -> ExposedVector<CUIButton> { return &(ui->SelectedButtons); })

            .addProperty("MaxSelectedFont", &CUserInterface::MaxSelectedFont)
            .addProperty("MaxSelectedTextX", &CUserInterface::MaxSelectedTextX)
            .addProperty("MaxSelectedTextY", &CUserInterface::MaxSelectedTextY)

            .addProperty("SingleTrainingButton", &CUserInterface::SingleTrainingButton)
            .addProperty("TrainingButtons", +[](const CUserInterface *ui) -> ExposedVector<CUIButton> { return &(ui->TrainingButtons); })
            .addProperty("UpgradingButton", &CUserInterface::UpgradingButton)
            .addProperty("ResearchingButton", &CUserInterface::ResearchingButton)
            .addProperty("TransportingButtons", +[](const CUserInterface *ui) -> ExposedVector<CUIButton> { return &(ui->TransportingButtons); })

            .addProperty("LifeBarColorNames", +[](const CUserInterface *ui) -> ExposedVector<std::string> { return &(ui->LifeBarColorNames); })
            .addProperty("LifeBarColorNames", +[](const CUserInterface *ui) -> ExposedVector<int> { return &(ui->LifeBarPercents); })
            .addProperty("LifeBarYOffset", &CUserInterface::LifeBarYOffset)
            .addProperty("LifeBarPadding", &CUserInterface::LifeBarPadding)
            .addProperty("LifeBarBorder", &CUserInterface::LifeBarBorder)

            .addProperty("CompletedBarColorRGB", &CUserInterface::CompletedBarColorRGB)
            .addProperty("CompletedBarShadow", &CUserInterface::CompletedBarShadow)

            .addProperty("ButtonPanel", &CUserInterface::ButtonPanel)

            .addProperty("PieMenu", &CUserInterface::PieMenu)

            .addProperty("MouseViewport", &CUserInterface::MouseViewport)
            .addProperty("MapArea", &CUserInterface::MapArea)

            .addProperty("MessageFont", &CUserInterface::MessageFont)
            .addProperty("MessageScrollSpeed", &CUserInterface::MessageScrollSpeed)

            .addProperty("MenuButton", &CUserInterface::MenuButton)
            .addProperty("NetworkMenuButton", &CUserInterface::NetworkMenuButton)
            .addProperty("NetworkDiplomacyButton", &CUserInterface::NetworkDiplomacyButton)

            .addProperty("UserButtons", +[](const CUserInterface *ui) -> ExposedVector<CUIUserButton> { return &(ui->UserButtons); })

            .addProperty("Minimap", &CUserInterface::Minimap)
            .addProperty("StatusLine", &CUserInterface::StatusLine)
            .addProperty("Timer", &CUserInterface::Timer)

            .addProperty("EditorSettingsAreaTopLeft", &CUserInterface::EditorSettingsAreaTopLeft)
            .addProperty("EditorSettingsAreaBottomRight", &CUserInterface::EditorSettingsAreaBottomRight)
            .addProperty("EditorButtonAreaTopLeft", &CUserInterface::EditorButtonAreaTopLeft)
            .addProperty("EditorButtonAreaBottomRight", &CUserInterface::EditorButtonAreaBottomRight)
        .endClass()

        .addProperty("UI", &UI)

        .beginClass<CIcon>("CIcon")
            .addStaticFunction("New", +[](lua_State *l) {
                int nargs = lua_gettop(l);
                std::string ident(lua_tostring(l, nargs));
                return CIcon::New(ident);
            })
            .addStaticFunction("Get", +[](lua_State *l) {
                int nargs = lua_gettop(l);
                std::string ident(lua_tostring(l, nargs));
                return CIcon::Get(ident);
            })
            .addProperty("Ident", +[](const CIcon *i) { return i->GetIdent(); })
            .addProperty("G", &CIcon::G)
            .addProperty("Frame", &CIcon::Frame)
        .endClass()

        .beginClass<ButtonStyle>("__ButtonStyle")
        .endClass()

        .addFunction("FindButtonStyle", FindButtonStyle)

        .addFunction("GetMouseScroll", GetMouseScroll)
        .addFunction("SetMouseScroll", SetMouseScroll)
        .addFunction("GetKeyScroll", GetKeyScroll)
        .addFunction("SetKeyScroll", SetKeyScroll)
        .addFunction("GetGrabMouse", GetGrabMouse)
        .addFunction("SetGrabMouse", SetGrabMouse)
        .addFunction("GetLeaveStops", GetLeaveStops)
        .addFunction("SetLeaveStops", SetLeaveStops)
        .addFunction("GetDoubleClickDelay", GetDoubleClickDelay)
        .addFunction("SetDoubleClickDelay", SetDoubleClickDelay)
        .addFunction("GetHoldClickDelay", GetHoldClickDelay)
        .addFunction("SetHoldClickDelay", SetHoldClickDelay)

        .addProperty("CursorScreenPos", &CursorScreenPos)

        //
        //  Guichan
        //

        .beginClass<gcn::Color>("Color")
            .addConstructor<void (*) (int, int, int, int)>()
            .addProperty("r", &gcn::Color::r)
            .addProperty("g", &gcn::Color::g)
            .addProperty("b", &gcn::Color::b)
            .addProperty("a", &gcn::Color::a)
        .endClass()

        .beginClass<gcn::Graphics>("Graphics")
            .addStaticProperty("LEFT", +[]() { return gcn::Graphics::LEFT; })
            .addStaticProperty("CENTER", +[]() { return gcn::Graphics::CENTER; })
            .addStaticProperty("RIGHT", +[]() { return gcn::Graphics::RIGHT; })
        .endClass()
/*
        .beginClass<Widget>("Widget")
            .addFunction("setWidth", &::setWidth)
            virtual int getWidth() const;
            .addFunction("setHeight", &::setHeight)
            virtual int getHeight() const;
            .addFunction("setSize", &::setSize)
            .addFunction("setX", &::setX)
            virtual int getX() const;
            .addFunction("setY", &::setY)
            virtual int getY() const;
            .addFunction("setPosition", &::setPosition)
            .addFunction("setBorderSize", &::setBorderSize)
            virtual unsigned int getBorderSize() const;
            .addFunction("setEnabled", &::setEnabled)
            virtual bool isEnabled() const;
            .addFunction("setVisible", &::setVisible)
            virtual bool isVisible() const;

            .addFunction("setDirty", &::setDirty)

            .addFunction("setBaseColor", &::setBaseColor)
            virtual const Color &getBaseColor() const;
            .addFunction("setForegroundColor", &::setForegroundColor)
            virtual const Color &getForegroundColor() const;
            .addFunction("setBackgroundColor", &::setBackgroundColor)
            virtual const Color &getBackgroundColor() const;
            .addFunction("setDisabledColor", &::setDisabledColor)
            virtual const Color &getDisabledColor() const;

            .addStaticFunction("setGlobalFont", &::setGlobalFont)
            .addFunction("setForegroundColor", &::setForegroundColor)
            .addFunction("setBackgroundColor", &::setBackgroundColor)
            .addFunction("setBaseColor", &::setBaseColor)
            .addFunction("setSize", &::setSize)
            .addFunction("setBorderSize", &::setBorderSize)
            .addFunction("setFont", &::setFont)

            virtual int getHotKey() const;
            .addFunction("setHotKey", &::setHotKey)
            .addFunction("setHotKey", &::setHotKey)

            .addFunction("requestFocus", &::requestFocus)

            .addFunction("addActionListener", &::addActionListener)
            .addFunction("addMouseListener", &::addMouseListener)
            .addFunction("addKeyListener", &::addKeyListener)
        .endClass()

        $[
        Widget.setActionCallback = function(w, f)
        local lal = LuaActionListener(f)
        if not w._actioncb then
            w._actioncb = {}
        end
        table.insert(w._actioncb, lal)
        w:addActionListener(lal)
        end

        Widget.setMouseCallback = function(w, f)
        local lal = LuaActionListener(f)
        if not w._mousecb then
            w._mousecb = {}
        end
        table.insert(w._mousecb, lal)
        w:addMouseListener(lal)
        end

        Widget.setKeyCallback = function(w, f)
        local lal = LuaActionListener(f)
        if not w._keycb then
            w._keycb = {}
        end
        table.insert(w._keycb, lal)
        w:addKeyListener(lal)
        end
        $]

        .deriveClass<BasicContainer, Widget>("BasicContainer")
        .endClass()

        .deriveClass<ScrollArea, BasicContainer>("ScrollArea")
            ScrollArea();
            .addFunction("setContent", &::setContent)
            .addFunction("getContent", &::getContent)
            .addFunction("setScrollbarWidth", &::setScrollbarWidth)
            .addFunction("getScrollbarWidth", &::getScrollbarWidth)
            .addFunction("scrollToBottom", &::scrollToBottom)
            .addFunction("scrollToTop", &::scrollToTop)
        .endClass()

        .deriveClass<ImageWidget, Widget>("ImageWidget")
            ImageWidget(CGraphic *image);
                ImageWidget(Mng *image);
                ImageWidget(Movie *image);
        .endClass()

        .deriveClass<Button, Widget>("Button")
        .endClass()

        .deriveClass<ButtonWidget, Button>("ButtonWidget")
            ButtonWidget(const std::string caption);
            .addFunction("setCaption", &::setCaption)
            virtual const std::string &getCaption() const;
            .addFunction("adjustSize", &::adjustSize)
        .endClass()

        .deriveClass<ImageButton, Button>("ImageButton")
            ImageButton();
            ImageButton(const std::string caption);

            .addFunction("setNormalImage", &::setNormalImage)
            .addFunction("setPressedImage", &::setPressedImage)
            .addFunction("setDisabledImage", &::setDisabledImage)
        .endClass()

        .deriveClass<RadioButton, Widget>("RadioButton")
            RadioButton();
            RadioButton(const std::string caption, const std::string group, bool marked = false);

            .addFunction("isMarked", &::isMarked)
            .addFunction("setMarked", &::setMarked)
            virtual const std::string &getCaption() const;
            .addFunction("setCaption", &::setCaption)
            .addFunction("setGroup", &::setGroup)
            virtual const std::string &getGroup() const;
            .addFunction("adjustSize", &::adjustSize)
        .endClass()

        .deriveClass<ImageRadioButton, RadioButton>("ImageRadioButton")
            ImageRadioButton();
            ImageRadioButton(const std::string caption, const std::string group, bool marked = false);

            .addFunction("setUncheckedNormalImage", &::setUncheckedNormalImage)
            .addFunction("setUncheckedPressedImage", &::setUncheckedPressedImage)
            .addFunction("setUncheckedDisabledImage", &::setUncheckedDisabledImage)
            .addFunction("setCheckedNormalImage", &::setCheckedNormalImage)
            .addFunction("setCheckedPressedImage", &::setCheckedPressedImage)
            .addFunction("setCheckedDisabledImage", &::setCheckedDisabledImage)
        .endClass()

        .deriveClass<CheckBox, Widget>("CheckBox")
            CheckBox();
            CheckBox(const std::string caption, bool marked = false);

            virtual bool isMarked() const;
            .addFunction("setMarked", &::setMarked)
            virtual const std::string &getCaption() const;
            .addFunction("setCaption", &::setCaption)
            .addFunction("adjustSize", &::adjustSize)
        .endClass()

        .deriveClass<ImageCheckBox, CheckBox>("ImageCheckBox")
            ImageCheckBox();
            ImageCheckBox(const std::string caption, bool marked = false);

            .addFunction("setUncheckedNormalImage", &::setUncheckedNormalImage)
            .addFunction("setUncheckedPressedImage", &::setUncheckedPressedImage)
            .addFunction("setUncheckedDisabledImage", &::setUncheckedDisabledImage)
            .addFunction("setCheckedNormalImage", &::setCheckedNormalImage)
            .addFunction("setCheckedPressedImage", &::setCheckedPressedImage)
            .addFunction("setCheckedDisabledImage", &::setCheckedDisabledImage)
        .endClass()

        .deriveClass<Slider, Widget>("Slider")
            Slider(double scaleEnd = 1.0);
            Slider(double scaleStart, double scaleEnd);
            .addFunction("setScale", &::setScale)
            virtual double getScaleStart() const;
            .addFunction("setScaleStart", &::setScaleStart)
            virtual double getScaleEnd() const;
            .addFunction("setScaleEnd", &::setScaleEnd)
            .addFunction("getValue", &::getValue)
            .addFunction("setValue", &::setValue)
            .addFunction("setMarkerLength", &::setMarkerLength)
            virtual int getMarkerLength() const;
            .addFunction("setOrientation", &::setOrientation)
            virtual unsigned int getOrientation() const;
            .addFunction("setStepLength", &::setStepLength)
            virtual double getStepLength() const;

            enum { HORIZONTAL = 0, VERTICAL };
        .endClass()

        .deriveClass<ImageSlider, Slider>("ImageSlider")
            ImageSlider(double scaleEnd = 1.0);
            ImageSlider(double scaleStart, double scaleEnd);
            .addFunction("setMarkerImage", &::setMarkerImage)
            .addFunction("setBackgroundImage", &::setBackgroundImage)
            .addFunction("setDisabledBackgroundImage", &::setDisabledBackgroundImage)
        .endClass()

        .deriveClass<Label, Widget>("Label")
            Label(const std::string caption);
            const std::string &getCaption() const;
            .addFunction("setCaption", &::setCaption)
            .addFunction("setAlignment", &::setAlignment)
            virtual unsigned    .addFunction("getAlignment", &::getAlignment)
            .addFunction("adjustSize", &::adjustSize)
        .endClass()

        .deriveClass<MultiLineLabel, Widget>("MultiLineLabel")
            MultiLineLabel();
            MultiLineLabel(const std::string caption);

            .addFunction("setCaption", &::setCaption)
            virtual const std::string &getCaption() const;
            .addFunction("setAlignment", &::setAlignment)
            virtual unsigned    .addFunction("getAlignment", &::getAlignment)
            .addFunction("setVerticalAlignment", &::setVerticalAlignment)
            virtual unsigned    .addFunction("getVerticalAlignment", &::getVerticalAlignment)
            .addFunction("setLineWidth", &::setLineWidth)
            .addFunction("getLineWidth", &::getLineWidth)
            .addFunction("adjustSize", &::adjustSize)
            .addFunction("draw", &::draw)

            enum {
                LEFT = 0,
                CENTER,
                RIGHT,
                TOP,
                BOTTOM
            };
        .endClass()

        .deriveClass<TextBox, Widget>("TextBox")
            TextBox(const std::string text);
            .addFunction("setEditable", &::setEditable)
                virtual std::string getText();
        .endClass()

        .deriveClass<TextField, Widget>("TextField")
            TextField(const std::string text);
            .addFunction("setText", &::setText)
            virtual std::string &getText();
            .addFunction("setPassword", &::setPassword)
        .endClass()

        .deriveClass<ImageTextField, Widget>("ImageTextField")
            ImageTextField(const std::string text);
            .addFunction("setText", &::setText)
            virtual std::string &getText();
            .addFunction("setItemImage", &::setItemImage)
            .addFunction("setPassword", &::setPassword)
        .endClass()

        .deriveClass<ListBox, Widget>("ListBox")
        .endClass()

        .deriveClass<ImageListBox, ListBox>("ImageListBox")
        .endClass()

        .deriveClass<ListBoxWidget, ScrollArea>("ListBoxWidget")
            ListBoxWidget(unsigned int width, unsigned int height);
            .addFunction("setList", &::setList)
            .addFunction("setSelected", &::setSelected)
            .addFunction("getSelected", &::getSelected)
        .endClass()

        .deriveClass<ImageListBoxWidget, ListBoxWidget>("ImageListBoxWidget")
            ImageListBoxWidget(unsigned int width, unsigned int height);
            .addFunction("setList", &::setList)
            .addFunction("setSelected", &::setSelected)
            .addFunction("getSelected", &::getSelected)
            .addFunction("setItemImage", &::setItemImage)
            .addFunction("setUpButtonImage", &::setUpButtonImage)
            .addFunction("setUpPressedButtonImage", &::setUpPressedButtonImage)
            .addFunction("setDownButtonImage", &::setDownButtonImage)
            .addFunction("setDownPressedButtonImage", &::setDownPressedButtonImage)
            .addFunction("setLeftButtonImage", &::setLeftButtonImage)
            .addFunction("setLeftPressedButtonImage", &::setLeftPressedButtonImage)
            .addFunction("setRightButtonImage", &::setRightButtonImage)
            .addFunction("setRightPressedButtonImage", &::setRightPressedButtonImage)
            .addFunction("setHBarImage", &::setHBarImage)
            .addFunction("setVBarImage", &::setVBarImage)
            .addFunction("setMarkerImage", &::setMarkerImage)
        .endClass()

        .deriveClass<Window, BasicContainer>("Window")
            Window();
            Window(const std::string caption);
            Window(Widget *content, const std::string caption = "");

            .addFunction("setCaption", &::setCaption)
            virtual const std::string &getCaption() const;
            .addFunction("setAlignment", &::setAlignment)
            virtual unsigned int getAlignment() const;
            .addFunction("setContent", &::setContent)
            virtual Widget* getContent() const;
            .addFunction("setPadding", &::setPadding)
            virtual unsigned int getPadding() const;
            .addFunction("setTitleBarHeight", &::setTitleBarHeight)
            virtual unsigned    .addFunction("getTitleBarHeight", &::getTitleBarHeight)
            .addFunction("setMovable", &::setMovable)
            virtual bool isMovable() const;
            .addFunction("resizeToContent", &::resizeToContent)
            .addFunction("setOpaque", &::setOpaque)
            .addFunction("isOpaque", &::isOpaque)
        .endClass()

        .deriveClass<Windows, Window>("Windows")
            Windows(const std::string text, int width, int height);
            .addFunction("add", &::add)
        .endClass()

        .deriveClass<ScrollingWidget, ScrollArea>("ScrollingWidget")
            ScrollingWidget(int width, int height);
            .addFunction("add", &::add)
            .addFunction("restart", &::restart)
            .addFunction("setSpeed", &::setSpeed)
            .addFunction("getSpeed", &::getSpeed)
        .endClass()

        .deriveClass<DropDown, BasicContainer>("DropDown")
            .addFunction("getSelected", &::getSelected)
            .addFunction("setSelected", &::setSelected)
            .addFunction("setScrollArea", &::setScrollArea)
            .addFunction("getScrollArea", &::getScrollArea)
            .addFunction("setListBox", &::setListBox)
            .addFunction("getListBox", &::getListBox)
        .endClass()

        .deriveClass<DropDownWidget, DropDown>("DropDownWidget")
            DropDownWidget();
            .addFunction("setList", &::setList)
            .addFunction("getListBox", &::getListBox)
            .addFunction("setSize", &::setSize)
        .endClass()

        .deriveClass<ImageDropDownWidget, DropDown>("ImageDropDownWidget")
            ImageDropDownWidget();
            .addFunction("setList", &::setList)
            .addFunction("getListBox", &::getListBox)
            .addFunction("setSize", &::setSize)
            .addFunction("setItemImage", &::setItemImage)
            .addFunction("setDownNormalImage", &::setDownNormalImage)
            .addFunction("setDownPressedImage", &::setDownPressedImage)
        .endClass()

        .deriveClass<StatBoxWidget, Widget>("StatBoxWidget")
            StatBoxWidget(int width, int height);

            .addFunction("setCaption", &::setCaption)
            const std::string &getCaption() const;
            .addFunction("setPercent", &::setPercent)
            int getPercent() const;
        .endClass()

        .deriveClass<Container, BasicContainer>("Container")
            Container();
            .addFunction("setOpaque", &::setOpaque)
            virtual bool isOpaque() const;
            .addFunction("add", &::add)
            .addFunction("remove", &::remove)
            .addFunction("clear", &::clear)
        .endClass()

        .deriveClass<MenuScreen, Container>("MenuScreen")
            MenuScreen();

            .addFunction("run", &::run)
            .addFunction("stop", &::stop)
            .addFunction("stopAll", &::stopAll)
            .addFunction("addLogicCallback", &::addLogicCallback)
            .addFunction("setDrawMenusUnder", &::setDrawMenusUnder)
            .addFunction("getDrawMenusUnder", &::getDrawMenusUnder)
        .endClass()
        $renaming MenuScreen @ CMenuScreen

        $[
        function MenuScreen()
        local menu = CMenuScreen()

        -- Store the widget in the container. This way we keep a reference
        -- to the widget until the container gets deleted.
        local guichanadd = Container.add
        function menu:add(widget, x, y)
            if not self._addedWidgets then
            self._addedWidgets = {}
            end
            self._addedWidgets[widget] = true
            guichanadd(self, widget, x, y)
        end

        return menu
        end
        $]

        */
        .addFunction("CenterOnMessage", CenterOnMessage)
        .addFunction("ToggleShowMessages", ToggleShowMessages)
        .addFunction("SetMaxMessageCount", SetMaxMessageCount)
        .addFunction("UiFindIdleWorker", UiFindIdleWorker)
        .addFunction("CycleViewportMode", CycleViewportMode)
        .addFunction("UiToggleTerrain", UiToggleTerrain)
        .addFunction("UiTrackUnit", UiTrackUnit)
        .addFunction("SetNewViewportMode", +[](int mode) { SetNewViewportMode(static_cast<ViewportModeType>(mode)); })
        .addFunction("SetDefaultTextColors", SetDefaultTextColors)
        // unit.pkg
        .beginClass<Vec2i>("Vec2i")
            .addProperty("x", &Vec2i::x)
            .addProperty("y", &Vec2i::y)
        .endClass()
        .beginClass<CUnit>("CUnit")
            .addProperty("tilePos", &CUnit::tilePos, false)
            .addProperty("Type", &CUnit::Type, false)
            .addProperty("Player", &CUnit::Player, false)
            .addProperty("Goal", &CUnit::Goal)
            .addProperty("Active", +[](const CUnit* unit) { return static_cast<int>(unit->Active); }, +[](CUnit *unit, int value) { unit->Active = value; })
            .addProperty("ResourcesHeld", &CUnit::ResourcesHeld)
        .endClass()
        .beginClass<CPreference>("CPreference")
            .addProperty("ShowSightRange", &CPreference::ShowSightRange)
            .addProperty("ShowReactionRange", &CPreference::ShowReactionRange)
            .addProperty("ShowAttackRange", &CPreference::ShowAttackRange)
            .addProperty("ShowMessages", &CPreference::ShowMessages)
            .addProperty("ShowNoSelectionStats", &CPreference::ShowNoSelectionStats)
            .addProperty("BigScreen", &CPreference::BigScreen)
            .addProperty("PauseOnLeave", &CPreference::PauseOnLeave)
            .addProperty("AiExplores", &CPreference::AiExplores)
            .addProperty("GrayscaleIcons", &CPreference::GrayscaleIcons)
            .addProperty("IconsShift", &CPreference::IconsShift)
            .addProperty("StereoSound", &CPreference::StereoSound)
            .addProperty("MineNotifications", &CPreference::MineNotifications)
            .addProperty("DeselectInMine", &CPreference::DeselectInMine)
            .addProperty("NoStatusLineTooltips", &CPreference::NoStatusLineTooltips)
            .addProperty("SimplifiedAutoTargeting", &CPreference::SimplifiedAutoTargeting)
            .addProperty("AiChecksDependencies", &CPreference::AiChecksDependencies)
            .addProperty("HardwareCursor", &CPreference::HardwareCursor)
            .addProperty("SelectionRectangleIndicatesDamage", &CPreference::SelectionRectangleIndicatesDamage)
            .addProperty("ShowOrders", &CPreference::ShowOrders)
            .addProperty("ShowNameDelay", &CPreference::ShowNameDelay)
            .addProperty("ShowNameTime", &CPreference::ShowNameTime)
            .addProperty("AutosaveMinutes", &CPreference::AutosaveMinutes)
            .addProperty("IconFrameG", &CPreference::IconFrameG)
            .addProperty("PressedIconFrameG", &CPreference::PressedIconFrameG)
        .endClass()
        .beginClass<CUnitManager>("CUnitManager")
            .addFunction("GetSlotUnit", +[](const CUnitManager *m, int idx) { return m->GetSlotUnit(idx); })
        .endClass()
        .addProperty("UnitManager", &UnitManager, false)
        .addProperty("Preference", &Preference, false)
        .addFunction("GetUnitUnderCursor", GetUnitUnderCursor)
        .addFunction("UnitNumber", +[](CUnit &unit) { return UnitNumber(unit); })
        // unittype.pkg
        .beginClass<CUnitType>("CUnitType")
            .addProperty("Ident", &CUnitType::Ident, false)
            .addProperty("Name", &CUnitType::Name, false)
            .addProperty("Slot", &CUnitType::Slot, false)
            .addProperty("MinAttackRange", &CUnitType::MinAttackRange)
            .addProperty("ClicksToExplode", &CUnitType::ClicksToExplode)
            .addProperty("GivesResource", &CUnitType::GivesResource)
            .addProperty("TileWidth", &CUnitType::TileWidth, false)
            .addProperty("TileHeight", &CUnitType::TileHeight, false)
        .endClass()
        .addFunction("UnitTypeByIdent", UnitTypeByIdent)
        .addProperty("UnitTypeHumanWall", &UnitTypeHumanWall)
        .addProperty("UnitTypeOrcWall", &UnitTypeOrcWall)
        .addFunction("SetMapStat", SetMapStat)
        .addFunction("SetMapSound", SetMapSound)
        // upgrade.pkg
        .beginClass<CUpgrade>("CUpgrade")
            .addStaticFunction("New", +[](lua_State *l) {
                int nargs = lua_gettop(l);
                std::string ident(lua_tostring(l, nargs));
                return CUpgrade::New(ident);
            })
            .addStaticFunction("Get", +[](lua_State *l) {
                int nargs = lua_gettop(l);
                std::string ident(lua_tostring(l, nargs));
                return CUpgrade::Get(ident);
            })
            .addProperty("Name", &CUpgrade::Name)
            .addProperty("Costs", +[](const CUpgrade *u) { return static_cast<Int7Array>(u->Costs); })
            .addProperty("Icon", &CUpgrade::Icon)
        .endClass()
        // video.pkg
        .addFunction("InitVideo", InitVideo)
        .addFunction("PlayMovie", PlayMovie)
        .addFunction("ShowFullImage", ShowFullImage)
        .addFunction("SaveMapPNG", SaveMapPNG)
        .beginClass<CVideo>("CVideo")
            .addProperty("Width", &CVideo::Width, false)
            .addProperty("Height", &CVideo::Height, false)
            .addProperty("Depth", &CVideo::Depth, false)
            .addProperty("FullScreen", &CVideo::FullScreen, false)
            .addFunction("ResizeScreen", &CVideo::ResizeScreen)
        .endClass()
        .addProperty("Video", &Video, false)
        .addFunction("ToggleFullScreen", ToggleFullScreen)
        .beginClass<CGraphic>("CGraphic")
            .addStaticFunction("New", +[](lua_State *l) {
                int nargs = lua_gettop(l);
                std::string name(luabridge::Stack<std::string>::get(l, 2));
                int w = 0, h = 0;
                if (nargs > 2) {
                    w = luabridge::Stack<int>::get(l, nargs - 1);
                    if (nargs > 3) {
                        h = luabridge::Stack<int>::get(l, nargs);
                    }
                }
                return CGraphic::New(name, w, h);
            })
            .addStaticFunction("ForceNew", +[](lua_State *l) {
                int nargs = lua_gettop(l);
                return CGraphic::ForceNew((luabridge::Stack<std::string>::get(l, nargs - 2)), luabridge::Stack<int>::get(l, nargs - 1), luabridge::Stack<int>::get(l, nargs));
            })
            .addStaticFunction("Get", +[](lua_State *l) {
                int nargs = lua_gettop(l);
                std::string ident(luabridge::Stack<std::string>::get(l, nargs));
                return CGraphic::Get(ident);
            })
            .addFunction("Free", &CGraphic::Free)
            .addFunction("Load", &CGraphic::Load)
            .addFunction("Resize", &CGraphic::Resize)
            .addFunction("SetPaletteColor", &CGraphic::SetPaletteColor)
        .endClass()
        .deriveClass<CPlayerColorGraphic, CGraphic>("CPlayerColorGraphic")
        	.addStaticFunction("New", +[](lua_State *l) {
                int nargs = lua_gettop(l);
                return CPlayerColorGraphic::New((luabridge::Stack<std::string>::get(l, nargs - 2)), luabridge::Stack<int>::get(l, nargs - 1), luabridge::Stack<int>::get(l, nargs));
            })
            .addStaticFunction("ForceNew", +[](lua_State *l) {
                int nargs = lua_gettop(l);
                return CPlayerColorGraphic::ForceNew((luabridge::Stack<std::string>::get(l, nargs - 2)), luabridge::Stack<int>::get(l, nargs - 1), luabridge::Stack<int>::get(l, nargs));
            })
	        .addStaticFunction("Get", +[](lua_State *l) {
                int nargs = lua_gettop(l);
                std::string ident(luabridge::Stack<std::string>::get(l, nargs));
                return CPlayerColorGraphic::Get(ident);
            })
        .endClass()
        .beginClass<CColor>("CColor")
            .addStaticFunction("__call", +[](lua_State *l) {
                int nargs = lua_gettop(l);
                uint8_t r = luabridge::Stack<uint8_t>::get(l, 2);
                uint8_t g = luabridge::Stack<uint8_t>::get(l, 3);
                uint8_t b = luabridge::Stack<uint8_t>::get(l, 4);
                uint8_t a = nargs == 5 ? luabridge::Stack<uint8_t>::get(l, 5) : 0;
                return CColor(r, g, b, a);
            })
            .addProperty("R", &CColor::R)
            .addProperty("G", &CColor::G)
            .addProperty("B", &CColor::B)
            .addProperty("A", &CColor::A)
        .endClass()
        .addFunction("SetColorCycleAll", SetColorCycleAll)
        .addFunction("ClearAllColorCyclingRange", ClearAllColorCyclingRange)
        .addFunction("AddColorCyclingRange", AddColorCyclingRange)
        .beginClass<Mng>("Mng")
            .addConstructor<void (*)()>()
            .addFunction("Load", &Mng::Load)
            .addFunction("Draw", &Mng::Draw)
            .addFunction("Reset", &Mng::Reset)
        .endClass()
        .beginClass<Movie>("Movie")
            .addConstructor<void (*)()>()
            .addFunction("Load", &Movie::Load)
            .addFunction("IsPlaying", &Movie::IsPlaying)
        .endClass()
    .endNamespace();

    // Backwards compatibility: forward everything above from sg to the global namespace
    Assert(lua_getmetatable(Lua, LUA_GLOBALSINDEX) == 0);
    lua_newtable(Lua); // metatable
    lua_pushstring(Lua, "__index"); // key, metatable
    lua_getglobal(Lua, "sg"); // sg table, key, metatable
    lua_pushcclosure(Lua, +[](lua_State *l) {
        // Args: table, name
        lua_pushvalue(l, 2); // [name]
#ifdef DEBUG
        lua_pushvalue(l, -1);
        const char *key = lua_tostring(l, -1);
        lua_pop(l, 1);
        PrintOnStdOut("[DEPRECATION WARNING]: Accessing global '%s'\n" _C_ key);
#endif
        lua_gettable(l, lua_upvalueindex(1)); // [value from sg]
        return 1;
    }, 1); // closure, key, metatable
    lua_rawset(Lua, -3); // metatable
    lua_setmetatable(Lua, LUA_GLOBALSINDEX);

    // any newly exposed APIs should go below

    return 0;
}
