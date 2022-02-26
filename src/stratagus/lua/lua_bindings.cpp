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
    using Int7Array = CArray<int, MaxCosts>;
    using ReadyArray = CArray<const uint8_t, PlayerMax>;
    using SlotOptionArray = CArray<const SlotOption, PlayerMax>;
    using ConstIntArray = CArray<const int, PlayerMax>;
    using ConstInt7Array = CArray<const int, MaxCosts>;
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
                (*thiz)[index] = value;
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
                luaL_error(L, "Read-only array");
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
            .addStaticFunction("New", &CFont::New)
            .addStaticFunction("Get", &CFont::Get)
            .addFunction("Height", &CFont::Height)
            .addFunction("Width", +[](const CFont* font, std::string &text) { return font->Width(text); })
        .endClass()
        .addProperty("MaxFontColors", +[]() { return MaxFontColors; })
        .beginClass<CFontColor>("CFontColor")
            .addStaticFunction("New", &CFontColor::New)
            .addStaticFunction("Get", &CFontColor::Get)
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
            .addProperty("MapRichness", +[](const Settings*) { PrintOnStdOut("GameSettings.MapRichness is deprecated."); return 0; },
                                        +[](Settings*, int) { PrintOnStdOut("GameSettings.MapRichness is deprecated."); })
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
            .addProperty("MapRichness", +[](const CServerSetup*) { PrintOnStdOut("ServerSetup.MapRichness is deprecated."); return 0; },
                                        +[](CServerSetup*, int) { PrintOnStdOut("ServerSetup.MapRichness is deprecated."); })
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
            // .addFunction("GetUnit", &CPlayer::GetUnit) TODO:
            .addFunction("GetUnitCount", &CPlayer::GetUnitCount)
            .addFunction("IsEnemy", +[](const CPlayer *p, CPlayer &player) -> bool { return p->IsEnemy(player); }) // TODO: unit overload
            .addFunction("IsAllied", +[](const CPlayer *p, CPlayer &player) -> bool { return p->IsAllied(player); }) // TODO: unit overload
            .addFunction("HasSharedVisionWith", &CPlayer::HasSharedVisionWith)
            .addFunction("IsTeamed", +[](const CPlayer *p, CPlayer &player) -> bool { return p->IsTeamed(player); }) // TODO: unit overload
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
        // .addFunction("PlayFile", PlayFile) // TODO: once we defined LuaActionListener class
        .addFunction("PlaySoundFile", +[](lua_State *l) {
            if (lua_gettop(l) != 2) {
                return luaL_error(l, "wrong number of arguments");
            }
            if (!lua_isstring(l, 1)) {
                return luaL_error(l, "arg 1 must be string");
            }
            // int r = PlayFile(lua_tostring(l, 1), );
            // return PlayFile(file, LuaActionListener:new(callback))
            return 0;
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
        .addProperty("FoodCost", +[]() { return FoodCost; })
        .addProperty("ScoreCost", +[]() { return ScoreCost; })
        .addProperty("ManaResCost", +[]() { return ManaResCost; })
        .addProperty("FreeWorkersCount", +[]() { return FreeWorkersCount; })
        .addProperty("MaxResouceInfo", +[]() { return MaxResourceInfo; })
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
        .addFunction("SetTranslationFiles", SetTranslationsFiles)
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
        // unit.pkg
        // unittype.pkg
        // upgrade.pkg
        // video.pkg
    .endNamespace();

    // Backwards compatibility: forward everything above from sg to the global namespace
    Assert(lua_getmetatable(Lua, LUA_GLOBALSINDEX) == 0);
    lua_newtable(Lua); // metatable
    lua_pushstring(Lua, "__index"); // key, metatable
    lua_getglobal(Lua, "sg"); // sg table, key, metatable
    lua_pushcclosure(Lua, +[](lua_State *l) {
        // Args: table, name
        lua_pushvalue(l, 2); // [name]
        lua_gettable(l, lua_upvalueindex(1)); // [value from sg]
        return 1;
    }, 1); // closure, key, metatable
    lua_rawset(Lua, -3); // metatable
    lua_setmetatable(Lua, LUA_GLOBALSINDEX);

    // any newly exposed APIs should go below

    return 0;
}
