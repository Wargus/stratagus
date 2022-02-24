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
#include "replay.h"
#include "results.h"
#include "interface.h"
#include "map.h"
#include "tileset.h"
#include "minimap.h"
#include "network.h"
#include "netconnect.h"

extern bool IsReplayGame();
extern void StartMap(const std::string &filename, bool clean = true);
extern void StartReplay(const std::string &filename, bool reveal = true);
extern void StartSavedGame(const std::string &filename);
extern int SaveReplay(const std::string &filename);

extern std::string NetworkMapName;
extern std::string NetworkMapFragmentName;

#include <LuaBridge/LuaBridge.h>

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
    luabridge::getGlobalNamespace(Lua).beginNamespace("sg")
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
            .addArrayProperty("Colors", +[](const CFontColor *color, int idx) { return color->Colors[idx]; },
                                        +[](CFontColor *color, int idx, CColor &value) { color->Colors[idx] = value; })
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
            .addArrayProperty("Presets", +[](const Settings *settings, int idx) { return settings->Presets[idx]; })
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
            .addArrayProperty("PlayerType", +[](const CMapInfo *info, int idx) { return static_cast<int>(info->PlayerType[idx]); },
                                            +[](CMapInfo *info, int idx, int value) { info->PlayerType[idx] = static_cast<PlayerTypes>(value); })
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
            .addArrayProperty("CompOpt", +[](const CServerSetup *s, int idx) { return static_cast<int>(s->CompOpt[idx]); },
                                         +[](CServerSetup *s, int idx, int value) { s->CompOpt[idx] = static_cast<SlotOption>(value); })
            .addArrayProperty("Ready", +[](const CServerSetup *s, int idx) { return s->Ready[idx]; },
                                       +[](CServerSetup *s, int idx, int value) { s->Ready[idx] = value; })
            .addArrayProperty("Race", +[](const CServerSetup *s, int idx) { return s->ServerGameSettings.Presets[idx].Race; },
                                      +[](CServerSetup *s, int idx, int value) { s->ServerGameSettings.Presets[idx].Race = value; })
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
        .addArrayProperty("Hosts", +[](int idx) { return Hosts[idx]; })
        .addProperty("NetworkMapName", &NetworkMapName)
        .addProperty("NetworkMapFragmentName", &NetworkMapFragmentName)
        .addFunction("NetworkGamePrepareGameSettings", NetworkGamePrepareGameSettings)
        // particle.pkg
        // pathfinder.pkg
        // player.pkg
        // sound.pkg
        // stratagus.pkg
        // translate.pkg
        // trigger.pkg
        // ui.pkg
        // unit.pkg
        // unittype.pkg
        // upgrade.pkg
        // video.pkg
    .endNamespace();

    // Backwards compatibility: copy everything above from StratagusEngine to the global namespace
    lua_getglobal(Lua, "sg");
    lua_pushnil(Lua);
    // [nil, table]
    while (lua_next(Lua, -2)) {
        // [value, key, table]
        lua_pushvalue(Lua, -2);
        // [key, value, key, table]
        lua_pushvalue(Lua, -2);
        // [value, key, value, key, table]
        lua_settable(Lua, LUA_GLOBALSINDEX);
        // [value, key, table]
        lua_pop(Lua, 1);
        // [key, table]
    }
    // [table]
    lua_pop(Lua, 1);

    // any newly exposed APIs should go below

    return 0;
}
