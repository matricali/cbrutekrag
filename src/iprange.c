/*
 * Copyright (C) 2018-2024 Jorge Matricali <jorgematricali@gmail.com>
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

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h> /* ntohl */
#endif

#include <errno.h> /* errno */
#include <stdio.h>
#include <stdlib.h> /* exit */
#include <string.h> /* strchr */

#include "iprange.h"

/**
 * @brief Convert an IPv4 address in string format to a 32-bit host-order value.
 *
 * @param ipstr The IPv4 address in string format (e.g., "192.168.1.1").
 *
 * @return The 32-bit host-order value of the IPv4 address.
 *
 * @note Exits the program with an error message if the address is invalid.
 */
in_addr_t a_to_hl(const char *ipstr)
{
	struct in_addr in;

#ifdef _WIN32
	if (inet_pton(AF_INET, ipstr, &in) != 1) {
#else
	if (!inet_aton(ipstr, &in)) {
#endif
		fprintf(stderr, "Invalid address %s!\n", ipstr);
		exit(1);
	}

	return (in_addr_t)ntohl(in.s_addr);
}

/**
 * @brief Compute the netmask address given a prefix length.
 *
 * @param prefix The prefix length (0 to 32).
 *
 * @return The netmask address as a 32-bit integer.
 */
in_addr_t netmask(int prefix)
{
	if (prefix == 0) {
		return (~((in_addr_t)-1));
	} else {
		return (~(((in_addr_t)1 << (32 - prefix)) - 1));
	}
}

/**
 * @brief Compute the network address given an IP address and a prefix length.
 *
 * @param addr The IP address as a 32-bit integer.
 * @param prefix The prefix length (0 to 32).
 *
 * @return The network address as a 32-bit integer.
 */
in_addr_t network(in_addr_t addr, int prefix)
{
	return (addr & netmask(prefix));
}

/**
 * @brief Compute the broadcast address given an IP address and a prefix length.
 *
 * @param addr The IP address as a 32-bit integer.
 * @param prefix The prefix length (0 to 32).
 *
 * @return The broadcast address as a 32-bit integer.
 */
in_addr_t broadcast(in_addr_t addr, int prefix)
{
	return (addr | ~netmask(prefix));
}

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
network_addr_t str_to_netaddr(const char *ipstr)
{
	long prefix = 32;
	char *prefixstr;
	network_addr_t netaddr;
	char *ipstr_copy = strdup(ipstr); // Make a copy of ipstr to modify

	if ((prefixstr = strchr(ipstr_copy, '/'))) {
		*prefixstr = '\0';
		prefixstr++;
		errno = 0;
		prefix = strtol(prefixstr, (char **)NULL, 10);
		if (errno || (*prefixstr == '\0') || (prefix < 0) ||
		    (prefix > 32)) {
			fprintf(stderr, "Invalid prefix /%s...!\n", prefixstr);
			free(ipstr_copy);
			exit(1);
		}
	}

	netaddr.pfx = (int)prefix;
	netaddr.addr = network(a_to_hl(ipstr_copy), (int)prefix);

	free(ipstr_copy);
	return (netaddr);
}
