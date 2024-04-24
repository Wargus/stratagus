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
//
//      (c) Copyright 2020 by the Stratagus Developers
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


#ifndef NET_SERIALIZATION_H
#define NET_SERIALIZATION_H

#include <string>
#include <vector>

extern size_t serialize32(unsigned char *buf, uint32_t data);
extern size_t serialize32(unsigned char *buf, int32_t data);
extern size_t serialize16(unsigned char *buf, uint16_t data);
extern size_t serialize16(unsigned char *buf, int16_t data);
extern size_t serialize8(unsigned char *buf, uint8_t data);
extern size_t serialize8(unsigned char *buf, int8_t data);
template <int N>
extern size_t serialize(unsigned char *buf, const char(&data)[N]);
extern size_t serialize(unsigned char *buf, const std::string &s);
extern size_t serialize(unsigned char *buf, const std::vector<unsigned char> &data);
extern size_t deserialize32(const unsigned char *buf, uint32_t *data);
extern size_t deserialize32(const unsigned char *buf, int32_t *data);
extern size_t deserialize16(const unsigned char *buf, uint16_t *data);
extern size_t deserialize16(const unsigned char *buf, int16_t *data);
extern size_t deserialize8(const unsigned char *buf, uint8_t *data);
extern size_t deserialize8(const unsigned char *buf, int8_t *data);
template <int N>
extern size_t deserialize(const unsigned char *buf, char(&data)[N]);
extern size_t deserialize(const unsigned char *buf, std::string &s);
extern size_t deserialize(const unsigned char *buf, std::vector<unsigned char> &data);

#endif
