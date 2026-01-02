#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "utils.h"

void handle_error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void log_msg(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

int initialize_socket(const char* IPversion) {
    // ソケットの初期化 (IPv4: AF_INET, IPv6: AF_INET6)
    int domain = (strcmp(IPversion, "IPv6") == 0) ? AF_INET6 : AF_INET;
    int tcp_socket = socket(domain, SOCK_STREAM, 0);
    if (tcp_socket < 0) handle_error("Socket creation failed");

    return tcp_socket;
}

void close_socket(int socket_fd) {
    // ソケットのクローズ
    if (close(socket_fd) < 0) {
        handle_error("Close failed");
    }
}