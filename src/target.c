/*
Copyright (c) 2014-2018 Jorge Matricali

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iprange.h"
#include "log.h"
#include "target.h"

/**
 * Check if given port is valid
 */
int btkg_target_port_is_valid(int port)
{
    return port >= 1 && port <= 65535;
}

/**
 * Initialize btkg_target_list_t
 */
void btkg_target_list_init(btkg_target_list_t* target_list)
{
    target_list->length = 0;
    target_list->targets = NULL;
}

/**
 * Split target into btkg_target_t structure
 */
btkg_target_t target_parse(char* line)
{
    btkg_target_t ret = { .host = NULL, .port = 22 };
    char* ptr = strtok(line, ":");

    char* host = NULL;
    int port = 0;

    if (ptr != NULL) {
        host = strdup(ptr);
        ptr = strtok(NULL, "´");

        if (ptr != NULL) {
            port = atoi(ptr);
            if (!btkg_target_port_is_valid(port)) {
                log_error("WARNING: Invalid port (%d)", port);
                return ret;
            }
            ret.port = port;
        }
        ret.host = host;
    }

    return ret;
}

/**
 * Append btkg_target_t into given btkg_target_list_t
 */
void btkg_target_list_append(btkg_target_list_t* target_list, btkg_target_t target)
{
    btkg_target_t* targets = target_list->targets;

    if (targets == NULL) {
        targets = malloc(sizeof(target));
        *targets = target;
    } else {
        targets = realloc(targets, sizeof(target) * (target_list->length + 1));
        *(targets + target_list->length) = target;
    }

    target_list->length = target_list->length + 1;
    target_list->targets = targets;
}

/**
 * Parse CIDR block and appends as many btkg_target_t as needed into the given btkg_target_list_t
 */
void btkg_target_list_append_range(btkg_target_list_t* target_list, const char* range, unsigned int port)
{
    char* netmask_s;
    netmask_s = strchr(range, '/');

    if (netmask_s == NULL) {
        btkg_target_t target = {
            .host = strdup(range),
            .port = port
        };
        btkg_target_list_append(target_list, target);
        return;
    }

    struct in_addr in;
    in_addr_t lo, hi;
    network_addr_t netaddr;

    netaddr = str_to_netaddr(range);
    lo = netaddr.addr;
    hi = broadcast(netaddr.addr, netaddr.pfx);

    for (in_addr_t x = lo; x < hi; x++) {
        btkg_target_t new_target;
        in.s_addr = htonl(x);
        new_target.host = strdup(inet_ntoa(in));
        new_target.port = port;
        btkg_target_list_append(target_list, new_target);
    }
}

/**
 * Loads targets from a given file and append them into the given btkg_target_list_t
 */
void btkg_target_list_load(btkg_target_list_t* target_list, char* filename)
{
    FILE* fp;
    ssize_t read;
    char* temp = 0;
    size_t len;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        log_error("Error opening file. (%s)", filename);
        exit(EXIT_FAILURE);
    }

    int lines = 0;
    for (lines = 0; (read = getline(&temp, &len, fp)) != -1; lines++) {
        strtok(temp, "\n");
        btkg_target_t ret = target_parse(temp);

        if (ret.host == NULL) {
            log_error("WARNING: An error ocurred parsing '%s' on line #%d", filename, lines);
            continue;
        }

        btkg_target_list_append_range(target_list, ret.host, ret.port);
    }

    free(temp);
    temp = NULL;

    fclose(fp);
}
