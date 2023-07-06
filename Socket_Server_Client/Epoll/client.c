#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define MAX_BUFFER_SIZE 1024

int main() {
    int server_fd;
    struct sockaddr_in server_addr;
    char buffer[MAX_BUFFER_SIZE];

    // 创建socket文件描述符
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8000);
    if (inet_pton(AF_INET, "127.0.0.1", &(server_addr.sin_addr)) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // 连接到服务器
    if (connect(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Failed to connect to server");
        exit(EXIT_FAILURE);
    }

    printf("Connected to the server.\n");

    while (1) {
        // 从标准输入读入用户输入的消息
        printf("Enter a message: ");
        fgets(buffer, sizeof(buffer), stdin);

        // 发送消息给服务器
        ssize_t bytes_sent = send(server_fd, buffer, strlen(buffer), 0);
        if (bytes_sent == -1) {
            perror("Failed to send data");
            exit(EXIT_FAILURE);
        }

        // 接收服务器回复的消息
        ssize_t bytes_received = recv(server_fd, buffer, sizeof(buffer), 0);
        if (bytes_received == -1) {
            perror("Failed to receive data");
            exit(EXIT_FAILURE);
        }

        if (bytes_received == 0) {
            printf("Server disconnected\n");
            break;
        } else {
            buffer[bytes_received] = '\0';
            printf("Received response from server: %s", buffer);
        }
    }

    close(server_fd);

    return 0;
}