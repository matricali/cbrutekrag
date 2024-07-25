/*
 * Copyright (C) 2018 Jorge Matricali <jorgematricali@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef IPRANGE_H
#define IPRANGE_H

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef int in_addr_t;
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h> /* in_addr_t */
#endif

typedef struct network_addr {
	in_addr_t addr;
	int pfx;
} network_addr_t;

/**
 * @brief Convert an IPv4 address in string format to a 32-bit host-order value.
 *
 * @param ipstr The IPv4 address in string format (e.g., "192.168.1.1").
 *
 * @return The 32-bit host-order value of the IPv4 address.
 *
 * @note Exits the program with an error message if the address is invalid.
 */
in_addr_t a_to_hl(const char *ipstr);

/**
 * @brief Compute the netmask address given a prefix length.
 *
 * @param prefix The prefix length (0 to 32).
 *
 * @return The netmask address as a 32-bit integer.
 */
in_addr_t netmask(int prefix);

/**
 * @brief Compute the network address given an IP address and a prefix length.
 *
 * @param addr The IP address as a 32-bit integer.
 * @param prefix The prefix length (0 to 32).
 *
 * @return The network address as a 32-bit integer.
 */
in_addr_t network(in_addr_t addr, int prefix);

/**
 * @brief Compute the broadcast address given an IP address and a prefix length.
 *
 * @param addr The IP address as a 32-bit integer.
 * @param prefix The prefix length (0 to 32).
 *
 * @return The broadcast address as a 32-bit integer.
 */
in_addr_t broadcast(in_addr_t addr, int prefix);

/**
 * @brief Convert a network address in string format into a host-order network address
 *        and an integer prefix value.
 *
 * @param ipstr The network address in string format (e.g., "192.168.1.0/24").
 *
 * @return A network_addr_t structure containing the network address and prefix.
 *
 * @note Exits the program with an error message if the address or prefix is invalid.
 */
network_addr_t str_to_netaddr(const char *ipstr);

#endif /* IPRANGE_H */
