#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/socket.h>

void handle_error(const char *msg) {
    do {
        perror(msg); exit(EXIT_FAILURE);
    } while (0);
}

void log_msg(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

int initialize_socket() {
    // ソケットの初期化 IPv6の通信はAF_INET6を使用
    int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_socket < 0) handle_error("Socket creation failed");

    return tcp_socket;
}

void close_socket(int socket_fd) {
    // ソケットのクローズ
    if (close(socket_fd) < 0) {
        handle_error("Close failed");
    }
}