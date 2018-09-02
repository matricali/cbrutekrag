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

/*
 * File:   iprange.h
 * Author: Jorge Matricali <jorgematricali@gmail.com>
 *
 * Created on 19 de agosto de 2018, 00:57
 */

#ifndef IPRANGE_H
#define IPRANGE_H

#include <arpa/inet.h> /* in_addr_t */

typedef struct network_addr {
    in_addr_t addr;
    int pfx;
} network_addr_t;

/**
 * Convert an A.B.C.D address into a 32-bit host-order value
 * @param ipstr
 * @return
 */
in_addr_t a_to_hl(const char *ipstr);

/**
 * Compute netmask address given prefix
 * @param prefix
 * @return
 */
in_addr_t netmask(int prefix);

/**
 * Compute network address given address and prefix
 * @param addr
 * @param prefix
 * @return
 */
in_addr_t network(in_addr_t addr, int prefix);

/**
 * Compute broadcast address given address and prefix
 * @param addr
 * @param prefix
 * @return
 */
in_addr_t broadcast(in_addr_t addr, int prefix);

/**
 * Convert a network address char string into a host-order network
 * address and an integer prefix value
 * @param ipstr
 * @return
 */
network_addr_t str_to_netaddr(const char *ipstr);

#endif /* IPRANGE_H */
