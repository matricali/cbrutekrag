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
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "cbrutekrag.h"
#include "detection.h"
#include "log.h"
#include "progressbar.h"
#include "target.h"
#include "wordlist.h"

#define BUF_SIZE 1024

int scan_counter = 0;
btkg_target_list_t filtered;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int detection_detect_ssh(char* serverAddr, unsigned int serverPort, unsigned int tm)
{
    struct sockaddr_in addr;
    int sockfd, ret;
    char buffer[BUF_SIZE];
    char* banner = "";
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
    addr.sin_port = htons(serverPort);
    addr.sin_addr.s_addr = inet_addr(serverAddr);

    ret = connect(sockfd, (struct sockaddr*)&addr, sizeof(addr));

    FD_ZERO(&fdset);
    FD_SET(sockfd, &fdset);

    /* Connection timeout */
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 500000;

    if (select(sockfd + 1, NULL, &fdset, NULL, &tv) == 1) {
        int so_error;
        socklen_t len = sizeof so_error;

        getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len);
        if (so_error != 0) {
            log_debug("[!] %s:%d - Error connecting to the server! (%s)", serverAddr, serverPort, strerror(so_error));
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
        log_error("Error fcntl(..., F_GETFL) (%s)\n", strerror(ret));
        close(sockfd);
        sockfd = 0;
        return -2;
    }

    long arg = 0;
    arg &= (~O_NONBLOCK);

    if ((ret = fcntl(sockfd, F_SETFL, arg)) < 0) {
        log_error("Error fcntl(..., F_SETFL) (%s)\n", strerror(ret));
        close(sockfd);
        sockfd = 0;
        return -1;
    }

    log_debug("[+] %s:%d - Connected.", serverAddr, serverPort);

    /* Send/Receive timeout */
    struct timeval timeout;
    timeout.tv_sec = tm;
    timeout.tv_usec = 0;

    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout,
        sizeof(timeout));
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout,
        sizeof(timeout));

    memset(buffer, 0, BUF_SIZE);

    // RECIBIR BANNER
    ret = recvfrom(sockfd, buffer, BUF_SIZE, 0, NULL, NULL);
    if (ret < 0) {
        log_debug("%s:%d - Error receiving banner!", serverAddr, serverPort);
        close(sockfd);
        sockfd = 0;
        return -1;
    }

    buffer[strcspn(buffer, "\n")] = 0;
    if (strlen(buffer) > 0) {
        banner = calloc(strlen(buffer), 1);
        if (banner == NULL) {
            log_error("calloc failed");
            exit(EXIT_FAILURE);
        }
        strncpy(banner, buffer, strlen(buffer));
    }

    char* pkt1 = "SSH-2.0-OpenSSH_7.5";
    char* pkt2 = "\n";
    char* pkt3 = "asd\n      ";
    char* search = "Protocol mismatch.";

    ret = sendto(sockfd, pkt1, sizeof(pkt1), 0, (struct sockaddr*)&addr, sizeof(addr));

    if (ret < 0) {
        log_error("[!] %s:%d - Error sending data pkt1!!", serverAddr, serverPort);
        close(sockfd);
        sockfd = 0;
        return -1;
    }

    ret = sendto(sockfd, pkt2, sizeof(pkt2), 0, (struct sockaddr*)&addr, sizeof(addr));

    if (ret < 0) {
        log_error("[!] %s:%d - Error sending data pkt2!!", serverAddr, serverPort);
        close(sockfd);
        sockfd = 0;
        return -1;
    }

    ret = sendto(sockfd, pkt3, sizeof(pkt3), 0, (struct sockaddr*)&addr, sizeof(addr));

    if (ret < 0) {
        log_error("[!] %s:%d - Error sending data pkt3!!", serverAddr, serverPort);
        close(sockfd);
        sockfd = 0;
        return -1;
    }

    ret = recvfrom(sockfd, buffer, BUF_SIZE, 0, NULL, NULL);
    if (ret < 0) {
        log_error("[!] %s:%d - Error receiving response!!", serverAddr, serverPort);
        close(sockfd);
        sockfd = 0;
        return -1;
    }

    close(sockfd);
    sockfd = 0;

    if (strstr(buffer, search) != NULL) {
        log_debug("[+] %s:%d - %s", serverAddr, serverPort, banner);
        return 0;
    }

    log_warn("[!] %s:%d - \033[91m(POSSIBLE HONEYPOT!)\033[0m %s", serverAddr, serverPort, banner);
    return 0;
}

void* detection_process(void* ptr)
{
    btkg_detection_args_t* args = (btkg_detection_args_t*)ptr;
    btkg_target_list_t* target_list = args->target_list;

    while (1) {
        pthread_mutex_lock(&mutex);
        if (scan_counter >= target_list->length - 1) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        scan_counter++;
        btkg_target_t current_target = target_list->targets[scan_counter];

        if (args->context->progress_bar) {
            char str[40];
            snprintf(str, 40, "[%d/%zu] %zu OK - %s:%d", scan_counter, target_list->length, filtered.length,
                current_target.host, current_target.port);
            progressbar_render(scan_counter, target_list->length, str, -1);
        }
        pthread_mutex_unlock(&mutex);

        if (args->context->dry_run) {
            pthread_mutex_lock(&mutex);
            log_info("Scanning %s:%d", current_target.host, current_target.port);
            btkg_target_list_append(&filtered, current_target);
            pthread_mutex_unlock(&mutex);
            continue;
        }

        if (detection_detect_ssh(current_target.host, current_target.port, 1) == 0) {
            pthread_mutex_lock(&mutex);
            btkg_target_list_append(&filtered, current_target);
            pthread_mutex_unlock(&mutex);
        }
    }
    pthread_exit(NULL);
    return NULL;
}

void detection_start(btkg_context_t* context, btkg_target_list_t* source, btkg_target_list_t* target, int max_threads)
{
    btkg_target_list_init(&filtered);
    btkg_detection_args_t args;

    memset(&args, 0, sizeof(btkg_detection_args_t));
    args.context = context;
    args.target_list = source;

    pthread_t scan_threads[max_threads];
    int ret;

    for (int i = 0; i < max_threads; i++) {
        if ((ret = pthread_create(&scan_threads[i], NULL, &detection_process, (void*)&args))) {
            log_error("Thread creation failed: %d\n", ret);
        }
    }

    for (int i = 0; i < max_threads; i++) {
        ret = pthread_join(scan_threads[i], NULL);
        if (ret != 0) {
            log_error("Cannot join thread no: %d\n", ret);
        }
    }

    if (context->progress_bar)
        progressbar_render(1, 1, NULL, -1);

    *target = filtered;
}
