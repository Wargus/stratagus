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
/**@name test_net_lowlevel.cpp - The test file for net_lowlevel.cpp. */
//
//      (c) Copyright 2013 by Joris Dauphin
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

#include <doctest.h>

#include "stratagus.h"
#include "netconnect.h"

namespace {

void ClearNetworkHosts()
{
	for (int i = 0; i < PlayerMax; ++i) {
		Hosts[i].Clear();
	}
}

void SetHost(int hostIndex, int playerIndex, const char *name)
{
	Hosts[hostIndex].Host = 0x7F000001 + hostIndex;
	Hosts[hostIndex].Port = 10000 + hostIndex;
	Hosts[hostIndex].PlyNr = playerIndex;
	Hosts[hostIndex].SetName(name);
}

}

void FillCustomValue(CNetworkHost *obj)
{
	obj->Host = 0x12345678;
	obj->Port = 0x9ABC;
	for (int i = 0; i != sizeof(obj->PlyName); ++i) {
		obj->PlyName[i] = i + 1;
	}
	obj->PlyNr = 0xDEF0;
}

void FillCustomValue(CServerSetup *obj)
{
	obj->Clear();
	obj->ServerGameSettings.Resources = 1;
	obj->ServerGameSettings.NumUnits = 2;
	obj->ServerGameSettings.Opponents = 3;
	obj->ServerGameSettings.Difficulty = 4;
	obj->ServerGameSettings.GameType = GameTypes::SettingsGameTypeFreeForAll;
	obj->ServerGameSettings.FoV = FieldOfViewTypes::cShadowCasting;
	obj->ServerGameSettings.RevealMap = MapRevealModes::cExplored;
	obj->ServerGameSettings.DefeatReveal = RevealTypes::cBuildingsOnly;
	obj->ServerGameSettings.NoFogOfWar = 1;
	obj->ServerGameSettings.Inside = 1;
	obj->ServerGameSettings.AiExplores = 0;
	obj->ServerGameSettings.SimplifiedAutoTargeting = 1;
	obj->ServerGameSettings.AiChecksDependencies = 1;
	obj->ServerGameSettings.AllyDepositsAllowed = 1;
	obj->ServerGameSettings.UserGameSettings = 0x1A2B3;
	for (int i = 0; i != PlayerMax; ++i) {
		obj->ServerGameSettings.Presets[i].Race = i % 2;
		obj->ServerGameSettings.Presets[i].PlayerColor = i;
		obj->ServerGameSettings.Presets[i].Team = i % 4;
		obj->ServerGameSettings.Presets[i].Type =
			i % 2 == 0 ? PlayerTypes::PlayerPerson : PlayerTypes::PlayerComputer;
		obj->ServerGameSettings.Presets[i].AIScript = i % 2 == 0 ? "ai-passive" : "ai-active";
		obj->CompOpt[i] = static_cast<SlotOption>(i % 3);
	}
	for (int i = 0; i != PlayerMax; ++i) {
		obj->Ready[i] = i + 11;
	}
}

void FillCustomValue(CInitMessage_Header *obj)
{
	*obj = CInitMessage_Header(0x12, 0x34);
}

void FillCustomValue(CInitMessage_Hello *obj)
{
	for (int i = 0; i != sizeof(obj->PlyName); ++i) {
		obj->PlyName[i] = i + 1;
	}
	obj->Stratagus = 0x12345678;
	obj->Version = 0x90ABCDEF;
}

void FillCustomValue(CInitMessage_Config *obj)
{
#if 0
	obj->hostsCount = PlayerMax;
#endif
	obj->clientIndex = 3;
	for (int i = 0; i != PlayerMax; ++i) {
		FillCustomValue(&obj->hosts[i]);
	}
}

void FillCustomValue(CInitMessage_EngineMismatch *obj)
{
	obj->Stratagus = 0x01020304;
}

#if 0
void FillCustomValue(CInitMessage_ProtocolMismatch *obj)
{
	obj->Version = 0x01020304;
}
#endif
void FillCustomValue(CInitMessage_Welcome *obj)
{
	obj->Lag = 0x01020304;
	obj->gameCyclesPerUpdate = 0x05060708;
	for (int i = 0; i != PlayerMax; ++i) {
		FillCustomValue(&obj->hosts[i]);
	}
}

void FillCustomValue(CInitMessage_Map *obj)
{
	obj->MapUID = 0x01234567;
	for (int i = 0; i != sizeof(obj->MapPath); ++i) {
		obj->MapPath[i] = 1 + i;
	}
}

void FillCustomValue(CInitMessage_State *obj)
{
	FillCustomValue(&obj->State);
}

void FillCustomValue(CInitMessage_Resync *obj)
{
	for (int i = 0; i != PlayerMax; ++i) {
		FillCustomValue(&obj->hosts[i]);
	}
}

template <typename T>
bool CheckSerialization()
{
	T obj1;

	FillCustomValue(&obj1);

	std::vector<unsigned char> buffer(obj1.Size());
	unsigned char *end = buffer.data() + obj1.Serialize(buffer.data());
	bool res = size_t(end - buffer.data()) == obj1.Size();
	T obj2;
	obj2.Deserialize(buffer.data());
	res &= memcmp(&obj1, &obj2, sizeof(T)) == 0; // may fail with padding
	return res;
}

template <typename T>
bool CheckSerialization_return()
{
	T obj1;

	memset(&obj1, 0, sizeof(T));
	FillCustomValue(&obj1);
	std::vector<unsigned char> buffer = obj1.Serialize();

	T obj2;
	memset(&obj2, 0, sizeof(T));
	obj2.Deserialize(buffer.data());
	bool res = memcmp(&obj1, &obj2, sizeof(T)) == 0; // may fail with padding
	return res;
}


TEST_CASE("CNetworkHost")
{
	CHECK(CheckSerialization<CNetworkHost>());
}

TEST_CASE("CServerSetup")
{
	CServerSetup obj1;
	FillCustomValue(&obj1);

	std::vector<unsigned char> buffer(obj1.Size());
	unsigned char *end = buffer.data() + obj1.Serialize(buffer.data());
	CHECK(size_t(end - buffer.data()) == obj1.Size());

	CServerSetup obj2;
	obj2.Deserialize(buffer.data());
	CHECK(obj1 == obj2);
}

TEST_CASE("Settings bitfield preserves simulation-affecting flags")
{
	Settings settings;
	settings.Init();
	settings.NoFogOfWar = 1;
	settings.Inside = 1;
	settings.AiExplores = 0;
	settings.SimplifiedAutoTargeting = 1;
	settings.AiChecksDependencies = 1;
	settings.AllyDepositsAllowed = 1;
	settings.UserGameSettings = 0x03A5A5A5;

	Settings restored;
	restored.Init();
	restored.setBitfield(settings.getBitfield());

	CHECK(static_cast<unsigned>(restored.NoFogOfWar) == static_cast<unsigned>(settings.NoFogOfWar));
	CHECK(static_cast<unsigned>(restored.Inside) == static_cast<unsigned>(settings.Inside));
	CHECK(static_cast<unsigned>(restored.AiExplores) == static_cast<unsigned>(settings.AiExplores));
	CHECK(static_cast<unsigned>(restored.SimplifiedAutoTargeting)
	      == static_cast<unsigned>(settings.SimplifiedAutoTargeting));
	CHECK(static_cast<unsigned>(restored.AiChecksDependencies)
	      == static_cast<unsigned>(settings.AiChecksDependencies));
	CHECK(static_cast<unsigned>(restored.AllyDepositsAllowed)
	      == static_cast<unsigned>(settings.AllyDepositsAllowed));
	CHECK(static_cast<unsigned>(restored.UserGameSettings) == static_cast<unsigned>(settings.UserGameSettings));
}

TEST_CASE("Network setup sync tracks ready by host and race by assigned player slot")
{
	constexpr auto hostIndex = 3;
	constexpr auto playerIndex = 7;
	ClearNetworkHosts();
	SetHost(hostIndex, playerIndex, "client");

	CServerSetup server;
	CServerSetup local;
	server.Ready[hostIndex] = 1;
	local.Ready[hostIndex] = 1;
	server.ServerGameSettings.Presets[playerIndex].Race = 1;
	local.ServerGameSettings.Presets[playerIndex].Race = 1;

	CHECK(NetworkGetPlayerIndexForHost(hostIndex) == playerIndex);
	CHECK(NetworkIsLocalSetupInSync(server, local, hostIndex));

	local.ServerGameSettings.Presets[hostIndex].Race = 2;
	CHECK(NetworkIsLocalSetupInSync(server, local, hostIndex));

	local.ServerGameSettings.Presets[playerIndex].Race = 3;
	CHECK_FALSE(NetworkIsLocalSetupInSync(server, local, hostIndex));

	local.ServerGameSettings.Presets[playerIndex].Race = 1;
	local.Ready[hostIndex] = 0;
	CHECK_FALSE(NetworkIsLocalSetupInSync(server, local, hostIndex));
}

TEST_CASE("Network setup sync handles invalid host slots without reading player presets")
{
	constexpr auto hostIndex = 4;
	constexpr auto playerIndex = 4;
	ClearNetworkHosts();

	CServerSetup server;
	CServerSetup local;

	local.ServerGameSettings.Presets[playerIndex].Race = 2;
	CHECK(NetworkGetPlayerIndexForHost(hostIndex) == -1);
	CHECK(NetworkIsLocalSetupInSync(server, local, hostIndex));

	local.Ready[hostIndex] = 1;
	CHECK_FALSE(NetworkIsLocalSetupInSync(server, local, hostIndex));
}

TEST_CASE("Client setup state changes apply host-indexed ready and player-indexed race")
{
	constexpr auto hostIndex = 5;
	constexpr auto playerIndex = 2;
	constexpr auto playerIndex2 = 5;
	ClearNetworkHosts();
	SetHost(hostIndex, playerIndex, "client");

	CServerSetup server;
	CServerSetup client;
	server.Ready[hostIndex] = 0;
	server.ServerGameSettings.Presets[playerIndex].Race = 1;
	server.ServerGameSettings.Presets[playerIndex2].Race = 4;
	client.Ready[hostIndex] = 1;
	client.ServerGameSettings.Presets[playerIndex].Race = 3;
	client.ServerGameSettings.Presets[playerIndex2].Race = 8;

	NetworkApplyClientSetupStateChange(server, client, hostIndex);

	CHECK(server.Ready[hostIndex] == 1);
	CHECK(server.ServerGameSettings.Presets[playerIndex].Race == 3);
	CHECK(server.ServerGameSettings.Presets[playerIndex2].Race == 4);
}

TEST_CASE("Client setup state changes ignore race updates for invalid host slots")
{
	constexpr auto hostIndex = 4;
	constexpr auto playerIndex = 4;
	ClearNetworkHosts();

	CServerSetup server;
	CServerSetup client;
	server.ServerGameSettings.Presets[playerIndex].Race = 1;
	client.Ready[hostIndex] = 1;
	client.ServerGameSettings.Presets[playerIndex].Race = 3;

	NetworkApplyClientSetupStateChange(server, client, hostIndex);

	CHECK(server.Ready[hostIndex] == 1);
	CHECK(server.ServerGameSettings.Presets[playerIndex].Race == 1);
}

TEST_CASE("Dedicated server detection follows the host's assigned player slot")
{
	CNetworkHost host;
	host.SetName("server");
	host.PlyNr = 1;

	CServerSetup setup;
	setup.Clear();
	setup.CompOpt[0] = SlotOption::Closed;
	setup.CompOpt[1] = SlotOption::Available;

	CHECK_FALSE(NetworkIsDedicatedServerHost(host, setup));

	setup.CompOpt[1] = SlotOption::Closed;
	CHECK(NetworkIsDedicatedServerHost(host, setup));
}

TEST_CASE("Dedicated server detection preserves slot zero dedicated setup")
{
	CNetworkHost host;
	host.SetName("server");
	host.PlyNr = 0;

	CServerSetup setup;
	setup.Clear();
	setup.CompOpt[0] = SlotOption::Available;

	CHECK_FALSE(NetworkIsDedicatedServerHost(host, setup));

	setup.CompOpt[0] = SlotOption::Closed;
	CHECK(NetworkIsDedicatedServerHost(host, setup));
}

TEST_CASE("Network host compaction preserves sparse host to player assignments")
{
	constexpr auto hostIndexA = 0;
	constexpr auto hostIndexB = 3;
	constexpr auto hostIndexC = 9;
	constexpr auto hostIndexD = PlayerMax - 1;
	constexpr auto playerIndexA = 0;
	constexpr auto playerIndexB = 7;
	constexpr auto playerIndexC = 2;
	constexpr auto playerIndexD = 11;
	ClearNetworkHosts();
	SetHost(hostIndexA, playerIndexA, "server");
	SetHost(hostIndexB, playerIndexB, "client-a");
	SetHost(hostIndexC, playerIndexC, "client-b");
	SetHost(hostIndexD, playerIndexD, "client-c");

	NetworkCompactHosts();

	CHECK(Hosts[0].IsValid());
	CHECK(Hosts[0].PlyNr == playerIndexA);
	CHECK(Hosts[1].IsValid());
	CHECK(Hosts[1].PlyNr == playerIndexB);
	CHECK(Hosts[2].IsValid());
	CHECK(Hosts[2].PlyNr == playerIndexC);
	CHECK(Hosts[3].IsValid());
	CHECK(Hosts[3].PlyNr == playerIndexD);
	for (int i = 4; i < PlayerMax; ++i) {
		CHECK_FALSE(Hosts[i].IsValid());
	}
}

TEST_CASE("Network remote host indices include only compact valid clients")
{
	constexpr auto hostIndexA = 0;
	constexpr auto hostIndexB = 1;
	constexpr auto hostIndexC = 2;
	constexpr auto playerIndexA = 0;
	constexpr auto playerIndexB = 4;
	constexpr auto playerIndexC = 8;
	ClearNetworkHosts();
	SetHost(hostIndexA, playerIndexA, "server");
	SetHost(hostIndexB, playerIndexB, "client-a");
	SetHost(hostIndexC, playerIndexC, "client-b");

	const std::vector<int> remoteHostIndices = NetworkRemoteHostIndices();

	REQUIRE(remoteHostIndices.size() == 2);
	CHECK(remoteHostIndices[0] == hostIndexB);
	CHECK(remoteHostIndices[1] == hostIndexC);
}

TEST_CASE("CInitMessage_Header")
{
	CHECK(CheckSerialization<CInitMessage_Header>());
}

TEST_CASE("CInitMessage_Hello")
{
	CHECK(CheckSerialization_return<CInitMessage_Hello>());
}

TEST_CASE("CInitMessage_Config")
{
	CHECK(CheckSerialization_return<CInitMessage_Config>());
}

TEST_CASE("CInitMessage_EngineMismatch")
{
	CHECK(CheckSerialization_return<CInitMessage_EngineMismatch>());
}

#if 0
TEST_CASE("CInitMessage_ProtocolMismatch")
{
	CHECK(CheckSerialization_return<CInitMessage_ProtocolMismatch>());
}
#endif

TEST_CASE("CInitMessage_Welcome")
{
	CHECK(CheckSerialization_return<CInitMessage_Welcome>());
}

TEST_CASE("CInitMessage_Map")
{
	CHECK(CheckSerialization_return<CInitMessage_Map>());
}

TEST_CASE("CInitMessage_State")
{
	CHECK(CInitMessage_State::Size() > 1024);

	CServerSetup state;
	FillCustomValue(&state);
	const CInitMessage_State obj1(MessageInit_FromServer, state);
	std::vector<unsigned char> buffer = obj1.Serialize();

	CInitMessage_State obj2;
	obj2.Deserialize(buffer.data());
	CHECK(obj2.GetHeader().GetType() == MessageInit_FromServer);
	CHECK(obj2.GetHeader().GetSubType() == ICMState);
	CHECK(obj2.State == state);
}

TEST_CASE("CInitMessage_Resync")
{
	CHECK(CheckSerialization_return<CInitMessage_Resync>());
}
