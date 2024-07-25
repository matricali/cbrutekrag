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

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
typedef int socklen_t;
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libssh/libssh.h>
#include <pthread.h>

#include "cbrutekrag.h"
#include "detection.h"
#include "log.h"
#include "macrowrapper.h"
#include "progressbar.h"
#include "target.h"

#define BUF_SIZE 1024
#define BANNER_LEN 256

size_t scan_counter = 0;
btkg_target_list_t filtered;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int detection_login_methods(ssh_session session, const char *hostname, int port)
{
	int rc = 0;

	rc = ssh_userauth_none(session, NULL);

	if (rc == SSH_AUTH_SUCCESS) {
		log_debug(
			"[!] %s:%d - Server without authentication. (not eligible)",
			hostname, port);

		return -1;
	}

	if (rc == SSH_AUTH_ERROR) {
		log_debug(
			"[!] %s:%d - ssh_userauth_none(): A serious error happened. (not eligible)",
			hostname, port);

		return -1;
	}

	int method = ssh_userauth_list(session, NULL);

	if (method & (int)SSH_AUTH_METHOD_PASSWORD) {
		return 0;
	}

	return -1;
}

int detection_detect_ssh(btkg_context_t *context, const char *hostname,
			 uint16_t port, long tm)
{
	int verbosity = 0;

	btkg_options_t *options = &context->options;

	if (options->verbose & CBRUTEKRAG_VERBOSE_SSHLIB) {
		verbosity = SSH_LOG_PROTOCOL;
	} else {
		verbosity = SSH_LOG_NOLOG;
	}

	ssh_session session = ssh_new();

	if (session == NULL) {
		log_error("Cant create SSH session.");
		return -1;
	}

	ssh_options_set(session, SSH_OPTIONS_HOST, hostname);
	ssh_options_set(session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
	ssh_options_set(session, SSH_OPTIONS_PORT, &port);
#if LIBSSH_VERSION_MAJOR > 0 ||                                                \
	(LIBSSH_VERSION_MAJOR == 0 && LIBSSH_VERSION_MINOR >= 6)
	ssh_options_set(session, SSH_OPTIONS_KEY_EXCHANGE, "none");
	ssh_options_set(session, SSH_OPTIONS_HOSTKEYS, "none");
#endif
	ssh_options_set(session, SSH_OPTIONS_TIMEOUT, &tm);
	ssh_options_set(session, SSH_OPTIONS_USER, NULL);

	int rc = ssh_connect(session);

	if (rc != SSH_OK) {
		if (options->verbose & CBRUTEKRAG_VERBOSE_MODE) {
			log_error("[!] Error connecting to %s:%d %s.", hostname,
				  port, ssh_get_error(session));
		}
		ssh_free(session);

		return 1;
	}

	log_debug("[+] %s:%d - Connected.", hostname, port);

	const char *banner = ssh_get_serverbanner(session);
	if (!banner) {
		log_debug("%s:%d - Error receiving banner!", hostname, port);

		ssh_disconnect(session);
		ssh_free(session);

		return -1;
	}

	if (strstr(banner, "SSH-") != banner) {
		log_warn(
			"[!] %s:%d - It's not a SSH server (tcpwrapped) skipping.",
			hostname, port);
		ssh_disconnect(session);
		ssh_free(session);

		return -1;
	}

	if (strstr(banner, "SSH-2.0-OpenSSH") != banner) {
		log_warn("[!] %s:%d - %s It's not a OpenSSH server", hostname,
			 port, banner);

		if (options->non_openssh != 1) {
			ssh_disconnect(session);
			ssh_free(session);

			return -1;
		}
	}

	/* Check authentication methods */
	rc = ssh_userauth_none(session, NULL);

	if (rc == SSH_AUTH_SUCCESS) {
		log_debug(
			"[!] %s:%d - Server without authentication. (not eligible)",
			hostname, port);
		ssh_disconnect(session);
		ssh_free(session);

		return -1;
	}

	if (rc == SSH_AUTH_ERROR) {
		log_debug(
			"[!] %s:%d - ssh_userauth_none(): A serious error happened. (not eligible)",
			hostname, port);
		ssh_disconnect(session);
		ssh_free(session);

		return -1;
	}

	if (detection_login_methods(session, hostname, port) != 0) {
		log_warn(
			"[!] %s:%d - %s The server doesn't accept password authentication method",
			hostname, port, banner);
		return -1;
	}

	log_info("[!] %s:%d - %s", hostname, port, banner);

	ssh_disconnect(session);
	ssh_free(session);

	return 0;
}

void *detection_process(void *ptr)
{
	btkg_detection_args_t *args = (btkg_detection_args_t *)ptr;
	btkg_target_list_t *target_list = args->target_list;
	btkg_context_t *context = args->context;
	btkg_options_t *options = &context->options;

	for (;;) {
		pthread_mutex_lock(&mutex);
		if (scan_counter >= target_list->length) {
			pthread_mutex_unlock(&mutex);
			break;
		}
		btkg_target_t *current_target =
			&target_list->targets[scan_counter];
		scan_counter++;

		if (options->progress_bar) {
			char str[40];
			snprintf(str, 40, "[%zu/%zu] %zu OK - %s:%d",
				 scan_counter, target_list->length,
				 filtered.length, current_target->host,
				 current_target->port);
			progressbar_render(scan_counter, target_list->length,
					   str, 0);
		}
		pthread_mutex_unlock(&mutex);

		if (options->dry_run) {
			pthread_mutex_lock(&mutex);
			log_info("Scanning %s:%d", current_target->host,
				 current_target->port);
			btkg_target_list_append(&filtered, current_target);
			pthread_mutex_unlock(&mutex);
			continue;
		}

		if (detection_detect_ssh(context, current_target->host,
					 current_target->port,
					 options->timeout) == 0) {
			pthread_mutex_lock(&mutex);
			btkg_target_list_append(&filtered, current_target);
			pthread_mutex_unlock(&mutex);
		}
	}
	pthread_exit(NULL);
	return NULL;
}

void detection_start(btkg_context_t *context, btkg_target_list_t *source,
		     btkg_target_list_t *targets, size_t max_threads)
{
	btkg_target_list_init(&filtered);
	btkg_options_t *options = &context->options;
	btkg_detection_args_t args;

	memset(&args, 0, sizeof(btkg_detection_args_t));
	args.context = context;
	args.target_list = source;

	pthread_t scan_threads[max_threads];
	int ret;

	for (size_t i = 0; i < max_threads; i++) {
		log_debug("Creating thread: %ld", i);
		if ((ret = pthread_create(&scan_threads[i], NULL,
					  *detection_process, (void *)&args))) {
			log_error("Thread creation failed: %d\n", ret);
		}
	}

	for (size_t i = 0; i < max_threads; i++) {
		ret = pthread_join(scan_threads[i], NULL);
		if (ret != 0) {
			log_error("Cannot join thread no: %d\n", ret);
		}
	}

	if (options->progress_bar)
		progressbar_render(1, 1, NULL, 0);

	*targets = filtered;
}
