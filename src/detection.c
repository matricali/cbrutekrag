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

#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <libssh/libssh.h>

#include "cbrutekrag.h"
#include "detection.h"
#include "log.h"
#include "progressbar.h"
#include "target.h"
#include "macrowrapper.h"

#define BUF_SIZE 1024
#define BANNER_LEN 256

size_t scan_counter = 0;
btkg_target_list_t filtered;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
const long timeout = 5;

int detection_login_methods(btkg_context_t *context, const char *hostname,
			    uint16_t port)
{
	ssh_session session;
	int verbosity = 0;

	if (context->verbose & CBRUTEKRAG_VERBOSE_SSHLIB) {
		verbosity = SSH_LOG_PROTOCOL;
	} else {
		verbosity = SSH_LOG_NOLOG;
	}

	session = ssh_new();

	if (session == NULL) {
		log_error("Cant create SSH session.");
		return -1;
	}

	ssh_options_set(session, SSH_OPTIONS_HOST, hostname);
	ssh_options_set(session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
	ssh_options_set(session, SSH_OPTIONS_PORT, &port);
#if LIBSSH_VERSION_MAYOR > 0 ||                                                \
	(LIBSSH_VERSION_MAYOR == 0 && LIBSSH_VERSION_MINOR >= 6)
	ssh_options_set(session, SSH_OPTIONS_KEY_EXCHANGE, "none");
	ssh_options_set(session, SSH_OPTIONS_HOSTKEYS, "none");
#endif
	ssh_options_set(session, SSH_OPTIONS_TIMEOUT, &timeout);
	ssh_options_set(session, SSH_OPTIONS_USER, NULL);

	int r;
	r = ssh_connect(session);
	if (r != SSH_OK) {
		ssh_free(session);
		if (context->verbose & CBRUTEKRAG_VERBOSE_MODE) {
			log_error("[!] Error connecting to %s:%d %s.", hostname,
				  port, ssh_get_error(session));
		}
		return -1;
	}

	r = ssh_userauth_none(session, NULL);

	if (r == SSH_AUTH_SUCCESS) {
		log_debug(
			"[!] %s:%d - Server without authentication. (not eligible)",
			hostname, port);
		ssh_disconnect(session);
		ssh_free(session);

		return -1;
	}

	if (r == SSH_AUTH_ERROR) {
		log_debug(
			"[!] %s:%d - ssh_userauth_none(): A serious error happened. (not eligible)",
			hostname, port);
		ssh_disconnect(session);
		ssh_free(session);

		return -1;
	}

	int method = 0;

	method = ssh_userauth_list(session, NULL);

	if (method & (int)SSH_AUTH_METHOD_PASSWORD) {
		ssh_disconnect(session);
		ssh_free(session);
		return 0;
	}

	ssh_disconnect(session);
	ssh_free(session);
	return -1;
}

int detection_detect_ssh(btkg_context_t *ctx, const char *hostname,
			 uint16_t port, long tm)
{
	struct sockaddr_in addr;
	int sockfd;
	long ret;
	char buffer[BUF_SIZE];
	char banner[BANNER_LEN];
	size_t banner_len;
	fd_set fdset;

	sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockfd < 0) {
		log_error("Error creating socket!");
		sockfd = 0;
		return -1;
	}
	fcntl(sockfd, F_SETFL, O_NONBLOCK);

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(hostname);

	ret = connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));

	FD_ZERO(&fdset);
	FdSet(sockfd, &fdset);

	/* Connection timeout */
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 500000;

	if (select(sockfd + 1, NULL, &fdset, NULL, &tv) == 1) {
		int so_error;
		socklen_t len = sizeof so_error;

		getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len);
		if (so_error != 0) {
			log_debug(
				"[!] %s:%d - Error connecting to the server! (%s)",
				hostname, port, strerror(so_error));
			close(sockfd);
			sockfd = 0;
			return -1;
		}
	} else {
		close(sockfd);
		sockfd = 0;
		return -1;
	}

	// Set to blocking mode again...
	if ((ret = fcntl(sockfd, F_GETFL, NULL)) < 0) {
		log_error("Error fcntl(..., F_GETFL) (%s)\n",
			  strerror((int)ret));
		close(sockfd);
		sockfd = 0;
		return -2;
	}

	long arg = 0;
	arg &= (~O_NONBLOCK);

	if ((ret = fcntl(sockfd, F_SETFL, arg)) < 0) {
		log_error("Error fcntl(..., F_SETFL) (%s)\n",
			  strerror((int)ret));
		close(sockfd);
		sockfd = 0;
		return -1;
	}

	log_debug("[+] %s:%d - Connected.", hostname, port);

	/* Send/Receive timeout */
	struct timeval timeout;
	timeout.tv_sec = tm;
	timeout.tv_usec = 0;

	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
		   sizeof(timeout));
	setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
		   sizeof(timeout));

	memset(buffer, 0, BUF_SIZE);

	// RECIBIR BANNER
	banner[0] = 0;
	ret = recvfrom(sockfd, buffer, BUF_SIZE, 0, NULL, NULL);
	if (ret < 0) {
		log_debug("%s:%d - Error receiving banner!", hostname, port);
		close(sockfd);
		sockfd = 0;
		return -1;
	}

	banner_len = strcspn(buffer, "\r\n");
	if (banner_len > 0) {
		if (banner_len > BANNER_LEN)
			banner_len = BANNER_LEN;
		strncpy(banner, buffer, banner_len);
		banner[banner_len] = 0;
	}

	if (strstr(banner, "SSH-") != banner) {
		/* It's not a SSH server */
		log_warn(
			"[!] %s:%d - "
			"\033[91mIt's not a SSH server (tcpwrapped)\033[0m skipping.",
			hostname, port);
		close(sockfd);
		sockfd = 0;
		return -1;
	}

	if (strstr(banner, "SSH-2.0-OpenSSH") != banner) {
		/* It's not a OpenSSH server */
		log_warn("[!] %s:%d - %s "
			 "\033[91mIt's not a OpenSSH server\033[0m%s",
			 hostname, port, banner,
			 (ctx->non_openssh != 1) ? " skipping." : ".");

		close(sockfd);
		sockfd = 0;

		if (ctx->non_openssh != 1)
			return -1;
	} else {
		/* We'll check if it's a REAL OpenSSH Server */
		char *pkt1 = "SSH-2.0-OpenSSH_7.5";
		char *pkt2 = "\n";
		char *pkt3 = "asd\n      ";
		char *search = "Protocol mismatch.";

		ret = sendto(sockfd, pkt1, sizeof(pkt1), 0,
			     (struct sockaddr *)&addr, sizeof(addr));

		if (ret < 0) {
			log_error("[!] %s:%d - Error sending data pkt1!!",
				  hostname, port);
			close(sockfd);
			sockfd = 0;
			return -1;
		}

		ret = sendto(sockfd, pkt2, sizeof(pkt2), 0,
			     (struct sockaddr *)&addr, sizeof(addr));

		if (ret < 0) {
			log_error("[!] %s:%d - Error sending data pkt2!!",
				  hostname, port);
			close(sockfd);
			sockfd = 0;
			return -1;
		}

		ret = sendto(sockfd, pkt3, sizeof(pkt3), 0,
			     (struct sockaddr *)&addr, sizeof(addr));

		if (ret < 0) {
			log_error("[!] %s:%d - Error sending data pkt3!!",
				  hostname, port);
			close(sockfd);
			sockfd = 0;
			return -1;
		}

		ret = recvfrom(sockfd, buffer, BUF_SIZE, 0, NULL, NULL);
		if (ret < 0) {
			log_error("[!] %s:%d - Error receiving response!!",
				  hostname, port);
			close(sockfd);
			sockfd = 0;
			return -1;
		}

		close(sockfd);
		sockfd = 0;

		if (strstr(buffer, search) == NULL) {
			log_warn(
				"[!] %s:%d - %s \033[91mPOSSIBLE HONEYPOT!\033[0m%s",
				hostname, port, banner,
				(ctx->allow_honeypots != 1) ? " skipping." :
							      ".");

			if (ctx->allow_honeypots != 1)
				return -1;
		}
	}

	if (detection_login_methods(ctx, hostname, port) != 0) {
		log_warn("[!] %s:%d - %s \033[91mThe server doesn't "
			 "accept password authentication method\033[0m",
			 hostname, port, banner);
		return -1;
	}

	log_info("[!] %s:%d - %s", hostname, port, banner);

	return 0;
}

void *detection_process(void *ptr)
{
	btkg_detection_args_t *args = (btkg_detection_args_t *)ptr;
	btkg_target_list_t *target_list = args->target_list;
	btkg_context_t *context = args->context;
	FILE *output = args->output;

	for (;;) {
		pthread_mutex_lock(&mutex);
		if (scan_counter >= target_list->length) {
			pthread_mutex_unlock(&mutex);
			break;
		}
		btkg_target_t *current_target =
			target_list->targets[scan_counter];
		scan_counter++;

		if (context->progress_bar) {
			char str[40];
			snprintf(str, 40, "[%zu/%zu] %zu OK - %s:%d",
				 scan_counter, target_list->length,
				 filtered.length, current_target->host,
				 current_target->port);
			progressbar_render(scan_counter, target_list->length,
					   str, 0);
		}
		pthread_mutex_unlock(&mutex);

		if (context->dry_run) {
			pthread_mutex_lock(&mutex);
			log_info("Scanning %s:%d", current_target->host,
				 current_target->port);
			btkg_target_list_append(&filtered, current_target);
			pthread_mutex_unlock(&mutex);
			continue;
		}

		if (detection_detect_ssh(context, current_target->host,
					 current_target->port, 1) == 0) {
			pthread_mutex_lock(&mutex);
			btkg_target_list_append(&filtered, current_target);
			if (output != NULL)
				fprintf(output, "%s:%d %s\n",
					current_target->host,
					current_target->port, "BANNER");
			pthread_mutex_unlock(&mutex);
		}
	}
	pthread_exit(NULL);
	return NULL;
}

void detection_start(btkg_context_t *context, btkg_target_list_t *source,
		     btkg_target_list_t *target, size_t max_threads)
{
	btkg_target_list_init(&filtered);
	btkg_detection_args_t args;

	memset(&args, 0, sizeof(btkg_detection_args_t));
	args.context = context;
	args.target_list = source;

	pthread_t scan_threads[max_threads];
	int ret;

	FILE *output = NULL;

	/* Output file */
	if (context->scan_output != NULL) {
		output = fopen(context->scan_output, "a");
		if (output == NULL) {
			log_error("Error opening output file. (%s)",
				  context->scan_output);
			exit(EXIT_FAILURE);
		}
	}

	args.output = output;

	for (size_t i = 0; i < max_threads; i++) {
		if ((ret = pthread_create(&scan_threads[i], NULL,
					  &detection_process, (void *)&args))) {
			log_error("Thread creation failed: %d\n", ret);
		}
	}

	for (size_t i = 0; i < max_threads; i++) {
		ret = pthread_join(scan_threads[i], NULL);
		if (ret != 0) {
			log_error("Cannot join thread no: %d\n", ret);
		}
	}

	if (args.output != NULL)
		fclose(output);

	if (context->progress_bar)
		progressbar_render(1, 1, NULL, 0);

	*target = filtered;
}
