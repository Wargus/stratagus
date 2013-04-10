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

#include <UnitTest++.h>

#include "stratagus.h"
#include "network.h"
#include "net_message.h"

void FillCustomValue(CNetworkCommand *obj)
{
	obj->Dest = 0x1234;
	obj->Unit = 0x5678;
	obj->X = 0x9ABC;
	obj->Y = 0xDEF0;
}

void FillCustomValue(CNetworkExtendedCommand *obj)
{
	obj->ExtendedType = 11;
	obj->Arg1 = 22;
	obj->Arg2 = 0x1234;
	obj->Arg3 = 0x5678;
	obj->Arg4 = 0x9ABC;
}

void FillCustomValue(CNetworkChat *obj)
{
	obj->Text = "abcdefghijklmnopqrstuvwxyz";
}
void FillCustomValue(CNetworkCommandSync *obj)
{
	obj->syncSeed = 0x01234567;
	obj->syncHash = 0x89ABCDEF;
}
void FillCustomValue(CNetworkCommandQuit *obj)
{
	obj->player = 0x0123;
}
void FillCustomValue(CNetworkSelection *obj)
{
	for (int i = 0; i != 10; ++i) {
		obj->Units.push_back(0x0123 * i);
	}
}

void FillCustomValue(CNetworkPacketHeader *obj)
{
	obj->Cycle = 42;
	for (int i = 0; i != MaxNetworkCommands; ++i) {
		obj->Type[i] = 0x05 + i * 0x12;
	}
}

template <typename T>
bool Comp(const T &lhs, const T &rhs)
{
	return memcmp(&lhs, &rhs, sizeof(T)) == 0;
}

bool Comp(const CNetworkChat &lhs, const CNetworkChat &rhs)
{
	return lhs.Text == rhs.Text;
}

bool Comp(const CNetworkSelection &lhs, const CNetworkSelection &rhs)
{
	return lhs.Units == rhs.Units;
}


template <typename T>
bool CheckSerialization()
{
	T obj1;

	FillCustomValue(&obj1);
	unsigned char *buffer = new unsigned char [obj1.Size()];
	obj1.Serialize(buffer);

	T obj2;
	obj2.Deserialize(buffer);
	bool res = Comp(obj1, obj2);
	delete [] buffer;
	return res;
}

TEST(CNetworkCommand)
{
	CHECK(CheckSerialization<CNetworkCommand>());
}
TEST(CNetworkExtendedCommand)
{
	CHECK(CheckSerialization<CNetworkExtendedCommand>());
}
TEST(CNetworkChat)
{
	CHECK(CheckSerialization<CNetworkChat>());
}
TEST(CNetworkCommandSync)
{
	CHECK(CheckSerialization<CNetworkCommandSync>());
}
TEST(CNetworkCommandQuit)
{
	CHECK(CheckSerialization<CNetworkCommandQuit>());
}
TEST(CNetworkSelection)
{
	CHECK(CheckSerialization<CNetworkSelection>());
}
TEST(CNetworkPacketHeader)
{
	CHECK(CheckSerialization<CNetworkPacketHeader>());
}
//TEST(CNetworkPacket)

