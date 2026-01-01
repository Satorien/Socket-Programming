#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "utils.h"

#define PORT_NUMBER 8080
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 64

void connect_client_to_server(int client_socket_fd) {
    // サーバアドレス構造体
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    // ホストバイトオーダーからネットワークバイトオーダーへ変換
    server_address.sin_port = htons(PORT_NUMBER);
    inet_pton(AF_INET, SERVER_IP, &server_address.sin_addr);

    // サーバへの接続
    if (connect(client_socket_fd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        handle_error("Connection to server failed");
    }
}

char* generate_request(const char *function, const char *query) {
    // リクエストの生成
    char *request = calloc(BUFFER_SIZE, 1);

    if (strcmp(function, "calc") == 0) {
        snprintf(request, BUFFER_SIZE, "GET /calc?query=%s HTTP/1.1\r\n", query);
    } else {
        handle_error("Unsupported function");
    }
    return request;
}

void send_request_and_get_response(int client_socket_fd, char *request) {
    write(client_socket_fd, request, strlen(request));
    log_msg("Request sent:\n%s\n", request);
    char *buffer = calloc(BUFFER_SIZE, 1);
    read(client_socket_fd, buffer, BUFFER_SIZE - 1);
    log_msg("Response from server:\n%s\n\n", buffer);
    free(buffer);
}

int main(int argc, char **argv){
    // 引数の数をチェック
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <function> <query>\n", argv[0]);
        fprintf(stderr, "Example: %s calc 18+20\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // ソケットの初期化
    int client_socket = initialize_socket();
    log_msg("Client socket initialized.\n");

    // サーバへの接続
    connect_client_to_server(client_socket);
    log_msg("Connected to server %s:%d.\n", SERVER_IP, PORT_NUMBER);

    // リクエストの送信とレスポンスの受信
    char *function = argv[1];
    char *query = argv[2];
    char *request = generate_request(function, query);
    send_request_and_get_response(client_socket, request);
    log_msg("Request sent and response received.\n");

    // ソケットのクローズ
    close_socket(client_socket);
    log_msg("Client socket closed.\n");
    free(request);
    return 0;
}