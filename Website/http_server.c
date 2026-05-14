#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

void handle_client(int client_socket) {
    FILE *html_file;
    char buffer[1024];
    size_t bytes_read;

    printf("🔍 Handling new client request...\n");

    html_file = fopen("homework1.html", "r");
    if (html_file == NULL) {
        perror("❌ Failed to open homework1.html");
        const char *not_found = "HTTP/1.1 404 Not Found\r\n"
                                "Content-Type: text/plain\r\n"
                                "Content-Length: 15\r\n"
                                "\r\n"
                                "File not found.\n";
        send(client_socket, not_found, strlen(not_found), 0);
        close(client_socket);
        return;
    }

    // ファイルの長さを調べる
    fseek(html_file, 0, SEEK_END);
    long file_size = ftell(html_file);
    rewind(html_file);

    // HTTPヘッダーを作成（Content-Length付き）
    char header[256];
    snprintf(header, sizeof(header),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/html; charset=UTF-8\r\n"
             "Content-Length: %ld\r\n"
             "\r\n", file_size);

    send(client_socket, header, strlen(header), 0);

    // ファイル内容を送信
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), html_file)) > 0) {
        send(client_socket, buffer, bytes_read, 0);
    }

    fclose(html_file);
    close(client_socket);

    printf("✨ Done sending page (%ld bytes)!\n", file_size);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_len = sizeof(client_address);

    // ソケット作成
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // アドレスとポート設定
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(8080);

    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // 接続待機
    if (listen(server_socket, 5) == -1) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("✅ Server running at http://localhost:8080\n");

    // クライアントからの接続を待つ
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len);
        if (client_socket == -1) {
            perror("Accept failed");
            continue;
        }
        handle_client(client_socket);
    }

    close(server_socket);
    return 0;
}