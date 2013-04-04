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
/**@name test_udpsocket.cpp - The test file for net_lowlevel.cpp. */
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

#include "network/udpsocket.h"

#include "net_lowlevel.h"


class AutoNetwork
{
public:
	AutoNetwork() { NetInit(); }
	~AutoNetwork() { NetExit(); }
};

TEST_FIXTURE(AutoNetwork, CHost)
{
	const CHost host1("127.0.0.1", 6502);
	const CHost host2(htonl(0x7F000001), htons(6502));

	CHECK(host1 == host2);
	CHECK_EQUAL(std::string("127.0.0.1:6502"), host2.toString());
}

class Foo
{
public:
	Foo() { memset(&data, 0, sizeof(data)); }

	void Fill()
	{
		for (int i = 0; i != 42; ++i) {
			data[i] = i;
		}
	}

	bool Check() const
	{
		for (int i = 0; i != 42; ++i) {
			if (data[i] != i) {
				return false;
			}
		}
		return true;
	}
public:
	char data[42];
};

TEST_FIXTURE(AutoNetwork, CUDPSocket)
{
	const CHost host1("127.0.0.1", 6501);
	const CHost host2("localhost", 6502);

	CUDPSocket socket1;
	CUDPSocket socket2;

	socket1.Open(host1);
	socket2.Open(host2);

	CHECK(socket1.IsValid());
	CHECK(socket2.IsValid());

	Foo foo2;
	foo2.Fill();

	socket1.Send(host2, &foo2, sizeof(foo2));

	Foo foo1;
	CHECK(foo1.Check() == false);

	CHost from;
	CHECK(socket2.HasDataToRead(1000));
	CHECK_EQUAL(int(sizeof(foo1)), socket2.Recv(&foo1, sizeof(foo1) + 4, &from));

	CHECK(host1 == from);
	CHECK(foo1.Check() == true);
	socket2.Close();
	CHECK(socket2.IsValid() == false);
}
