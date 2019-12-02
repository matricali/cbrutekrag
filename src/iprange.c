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

#include "iprange.h"

#include <arpa/inet.h> /* ntohl */
#include <errno.h> /* errno */
#include <stdio.h>
#include <stdlib.h> /* exit */
#include <string.h> /* strchr */

/**
 * Convert an A.B.C.D address into a 32-bit host-order value
 * @param ipstr
 * @return
 */
in_addr_t a_to_hl(const char* ipstr)
{
    struct in_addr in;

    if (!inet_aton(ipstr, &in)) {
        fprintf(stderr, "Invalid address %s!\n", ipstr);
        exit(1);
    }

    return (ntohl(in.s_addr));
}

/**
 * Compute netmask address given prefix
 * @param prefix
 * @return
 */
in_addr_t netmask(int prefix)
{
    if (prefix == 0) {
        return (~((in_addr_t)-1));
    } else {
        return (~((1 << (32 - prefix)) - 1));
    }
}

/**
 * Compute network address given address and prefix
 * @param addr
 * @param prefix
 * @return
 */
in_addr_t network(in_addr_t addr, int prefix)
{
    return (addr & netmask(prefix));
}

/**
 * Compute broadcast address given address and prefix
 * @param addr
 * @param prefix
 * @return
 */
in_addr_t broadcast(in_addr_t addr, int prefix)
{
    return (addr | ~netmask(prefix));
}

/**
 * Convert a network address char string into a host-order network
 * address and an integer prefix value
 * @param ipstr
 * @return
 */
network_addr_t str_to_netaddr(const char* ipstr)
{
    long int prefix = 32;
    char* prefixstr;
    network_addr_t netaddr;

    if ((prefixstr = strchr(ipstr, '/'))) {
        *prefixstr = '\0';
        prefixstr++;
        errno = 0;
        prefix = strtol(prefixstr, (char**)NULL, 10);
        if (errno || (*prefixstr == '\0') || (prefix < 0) || (prefix > 32)) {
            fprintf(stderr, "Invalid prefix /%s...!\n", prefixstr);
            exit(1);
        }
    }

    netaddr.pfx = (int)prefix;
    netaddr.addr = network(a_to_hl(ipstr), prefix);

    return (netaddr);
}
