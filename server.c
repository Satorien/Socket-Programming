#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "utils.h"

#define PORT_NUMBER 8080
#define LISTEN_BACKLOG 5
#define BUFFER_SIZE 64

volatile sig_atomic_t is_server_running = 1;

void signal_handler() {
    is_server_running = 0;
}

void bind_socket(int socket_fd) {
    // サーバアドレス構造体: sin_family, sin_addr, sin_port
    struct sockaddr_in address_info; 
    address_info.sin_family = AF_INET;
    address_info.sin_addr.s_addr = INADDR_ANY;
    address_info.sin_port = htons(PORT_NUMBER);

    // ソケットのバインド
    if (bind(socket_fd, (struct sockaddr *) &address_info, sizeof(address_info)) < 0) {
        handle_error("Bind failed");
    }
}

void listen_for_connections(int socket_fd) {
    // 接続待機
    if (listen(socket_fd, LISTEN_BACKLOG) < 0) {
        handle_error("Listen failed");
    }
}

int accept_connection(int server_socket_fd) {
    // クライアントからの接続受付
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);

    int client_socket_fd = accept(server_socket_fd, (struct sockaddr *) &client_address, &client_address_len);
    if (client_socket_fd < 0) {
        // シグナルによる中断の場合は-1を返す
        if (errno == EINTR) {
            return -1;
        }
        handle_error("Accept failed");
    }

    return client_socket_fd;
}

char* calculate_query(const char *query) {
    // クエリの計算処理
    // 例: "query=2+10" -> 12
    int operand1, operand2;
    char operator;
    char *buffer = calloc(BUFFER_SIZE, 1);
    sscanf(query, "query=%d%c%d", &operand1, &operator, &operand2);
    switch (operator) {
        case '+': snprintf(buffer, BUFFER_SIZE, "%d", operand1 + operand2); return buffer;
        default: handle_error("Unsupported operator"); return NULL;
    }
}

void handle_client_request(int client_socket_fd) {
    // リクエストの処理
    char *buffer = calloc(BUFFER_SIZE, 1);
    read(client_socket_fd, buffer, BUFFER_SIZE - 1);
    log_msg("Request received:\n%s\n", buffer);

    // /calc エンドポイントの処理
    if (strncmp(buffer, "GET /calc?", 10) == 0) {
        char *calculation = calculate_query(buffer + 10);
        char response[512];
        snprintf(response, sizeof(response), "HTTP/1.1 200 OK\r\nContent-Length:2\r\n\r\n%s", calculation);
        write(client_socket_fd, response, strlen(response));
        log_msg("Response sent:\n%s\n", response);
    } else {
        handle_error("Unsupported request method");
    }
}

int main(){
    // ソケットの初期化
    int server_socket = initialize_socket();
    log_msg("Server socket initialized.\n");

    // アドレスの再利用を有効化
    int optval = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
        handle_error("setsockopt SO_REUSEADDR failed");
    }

    // ソケットのバインド
    bind_socket(server_socket);
    log_msg("Server socket bound to port %d.\n", PORT_NUMBER);

    // クライアントからの接続を待機
    listen_for_connections(server_socket);
    log_msg("Server is listening on port %d...\n----------\n", PORT_NUMBER);

    // シグナルハンドラの設定
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; // SA_RESTARTを設定しない（システムコールを自動再開しない）
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        handle_error("sigaction failed");
    }

    // リクエストの受信と処理
    while (is_server_running) {
        int client_socket = accept_connection(server_socket);
        
        if (client_socket < 0) continue;
        
        log_msg("Client connected.\n");

        handle_client_request(client_socket);
        log_msg("Client request handled.\n");

        close_socket(client_socket);
        log_msg("Client socket closed.\n----------\n");
    }

    // サーバソケットのクローズ
    close_socket(server_socket);
    log_msg("\r\nServer shutdown gracefully.\n");
    return 0;
}