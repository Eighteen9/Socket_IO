#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#define MAX_EVENTS 10
#define MAX_BUFFER_SIZE 1024

int main() {
    int server_fd, client_fd, epoll_fd, event_count, i;
    struct sockaddr_in server_addr, client_addr;
    struct epoll_event event, events[MAX_EVENTS];
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
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // 绑定服务器地址到socket
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Failed to bind");
        exit(EXIT_FAILURE);
    }

    // 开始监听连接请求
    if (listen(server_fd, 5) == -1) {
        perror("Failed to listen");
        exit(EXIT_FAILURE);
    }

    // 创建epoll文件描述符
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("Failed to create epoll file descriptor");
        exit(EXIT_FAILURE);
    }

     // 将server_fd添加到epoll监控中
    event.events = EPOLLIN;
    event.data.fd = server_fd;
    if (    (epoll_fd, EPOLL_CTL_ADD, server_fd, &event) == -1) {
        perror("Failed to add server_fd to epoll");
        exit(EXIT_FAILURE);
    }

    printf("Server started. Waiting for connections...\n");

    while (1) {
        // 等待事件触发
        event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (event_count == -1) {
            perror("epoll_wait error"); 
            exit(EXIT_FAILURE);
        }

        //events[]里面存放了活跃事件，
        //与select类似，只能检测活跃的事件数量，具体是那个事件，仍然需要遍历进行对比!!!
        for (i = 0; i < event_count; i++) {     
            if (events[i].data.fd == server_fd) {//deal with server_socket
                // 有新的连接请求
                socklen_t client_addr_len = sizeof(client_addr);
                client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
                if (client_fd == -1) {
                    perror("Failed to accept connection");
                    exit(EXIT_FAILURE);
                }
                printf("Accepted new connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                // 将新的连接添加到epoll监控中
                event.events = EPOLLIN;
                event.data.fd = client_fd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1) {
                    perror("Failed to add client_fd to epoll");
                    exit(EXIT_FAILURE);
                }
            } else {//deal with client_socket
                // 接收客户端发送的消息
                ssize_t bytes_read = recv(events[i].data.fd, buffer, sizeof(buffer), 0);
                if (bytes_read == -1) {
                    perror("Failed to receive data");
                    exit(EXIT_FAILURE);
                }

                if (bytes_read == 0) {
                    // 客户端断开连接
                    printf("Client disconnected: %d\n", events[i].data.fd);
                    close(events[i].data.fd);
                } else {
                    // 打印客户端发送的消息
                    buffer[bytes_read] = '\0';
                    printf("Received data from client %d: %s", events[i].data.fd, buffer);
                    // 回复消息给客户端
                    send(events[i].data.fd, buffer, bytes_read, 0);
                }
            }
        }
    }

    close(server_fd);

    return 0;
}