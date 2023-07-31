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
/**@name mdns.cpp - LAN server discovery. */
//
//      (c) Copyright 2020 by Tim Felgentreff
//
//      This package is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
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

#include "mdns.h"
#include "game.h"
#include "net_lowlevel.h"
#include "network.h"

static int service_callback(int sock, const struct sockaddr* from, size_t addrlen, mdns_entry_type_t entry,
                            uint16_t query_id, uint16_t rtype, uint16_t rclass, uint32_t ttl, const void* data,
                            size_t size, size_t name_offset, size_t name_length, size_t record_offset,
                            size_t record_length, void* user_data) {
    if (rtype == MDNS_RECORDTYPE_PTR && entry == MDNS_ENTRYTYPE_QUESTION) {
        char namebuffer[256];
        mdns_string_t service = mdns_record_parse_ptr(data, size, record_offset, record_length, namebuffer, sizeof(namebuffer));
        std::string offeredService = GameName;
        if (strncmp(offeredService.c_str(), service.str, offeredService.size()) == 0) {
            uint16_t unicast = (rclass & MDNS_UNICAST_RESPONSE);
            std::string hostname = NetGetHostname();
            if (!unicast) {
                addrlen = 0;
            }
            char buffer[256];
            unsigned long ips[20];
            int numIps = NetSocketAddr(ips, 20);
            for (int i = 0; i < numIps; i++) {
                mdns_query_answer(sock, from, addrlen, buffer, sizeof(buffer), query_id,
                                  offeredService.c_str(), offeredService.size(),
                                  hostname.c_str(), hostname.size(),
                                  ips[i], nullptr, CNetworkParameter::Instance.localPort,
                                  nullptr, 0);
            }
        }
    }
    return 0;
}

void MDNS::AnswerServerQueries() {
    if (serviceSocket == -1) {
        // When recieving, a socket can recieve data from all network interfaces
        struct sockaddr_in sock_addr;
        memset(&sock_addr, 0, sizeof(struct sockaddr_in));
        sock_addr.sin_family = AF_INET;
#ifdef _WIN32
        sock_addr.sin_addr = in4addr_any;
#else
        sock_addr.sin_addr.s_addr = INADDR_ANY;
#endif
        sock_addr.sin_port = htons(MDNS_PORT);
#ifdef __APPLE__
        sock_addr.sin_len = sizeof(struct sockaddr_in);
#endif
        serviceSocket = mdns_socket_open_ipv4(&sock_addr);
    }

    char buffer[1024];
    mdns_socket_listen(serviceSocket, buffer, sizeof(buffer), service_callback, nullptr);
}

static int query_callback(int sock, const struct sockaddr* from, size_t addrlen, mdns_entry_type_t entry,
                          uint16_t query_id, uint16_t rtype, uint16_t rclass, uint32_t ttl, const void* data,
                          size_t size, size_t name_offset, size_t name_length, size_t record_offset,
                          size_t record_length, void* user_data) {
    std::function<void(char*)> *cb = static_cast<std::function<void(char*)>*>(user_data);
    if (rtype == MDNS_RECORDTYPE_A && entry == MDNS_ENTRYTYPE_ADDITIONAL) {
        struct sockaddr_in addr;
        mdns_record_parse_a(data, size, record_offset, record_length, &addr);
        char buf[24] = {'\0'};
        sprintf(buf, "%d.%d.%d.%d", NIPQUAD(ntohl(addr.sin_addr.s_addr)));
        (*cb)(buf);
    }
    return 0;
}

void MDNS::QueryServers(std::function<void(char*)> callback) {
    char buffer[1024];
    if (numSockets == -1) {
        // When sending, each socket can only send to one network interface
        // Thus we need to open one socket for each interface
        unsigned long ips[20];
        numSockets = NetSocketAddr(ips, 20);
        struct sockaddr_in sock_addr;
        for (int i = 0; i < numSockets; i++) {
            memset(&sock_addr, 0, sizeof(struct sockaddr_in));
            sock_addr.sin_family = AF_INET;
            sock_addr.sin_addr.s_addr = ips[i];
#ifdef __APPLE__
            sock_addr.sin_len = sizeof(struct sockaddr_in);
#endif
            querySockets[i] = mdns_socket_open_ipv4(&sock_addr);
        }
    }
    for (int i = 0; i < numSockets; i++) {
        queryId = mdns_query_send(querySockets[i], MDNS_RECORDTYPE_PTR,
                                  GameName.c_str(), GameName.size(),
                                  buffer, sizeof(buffer), 0);
        int responses = mdns_query_recv(querySockets[i], buffer,
                                        sizeof(buffer), query_callback,
                                        &callback, queryId);
    }
}

MDNS::~MDNS() {
    if (serviceSocket != -1) {
	mdns_socket_close(serviceSocket);
	serviceSocket = -1;
    }

    for (int i = 0; i < numSockets; i++) {
        mdns_socket_close(querySockets[i]);
        numSockets = -1;
        queryId = -1;
    }
}
