#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#define MAX_BUFFER_SIZE 1024
#define MAX_EVENTS 10

int main()
{
    int server_fd;
    struct sockaddr_in server_addr;
    char buffer[MAX_BUFFER_SIZE];

    // 创建socket文件描述符
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8000);
    if (inet_pton(AF_INET, "127.0.0.1", &(server_addr.sin_addr)) <= 0)
    {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // 连接到服务器
    if (connect(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("Failed to connect to server");
        exit(EXIT_FAILURE);
    }

    // 将server_fd添加进内核事件表
    int epoll_fd = epoll_create1(0);
    struct epoll_event event, events[MAX_EVENTS];
    event.events = EPOLLIN;
    event.data.fd = server_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event);
   
   //将stdin添加进内核事件表
    event.events = EPOLLIN;
    event.data.fd = fileno(stdin);
    epoll_ctl(epoll_fd,EPOLL_CTL_ADD,fileno(stdin),&event);

    printf("Connected to the server.\n\n");
    
    while (1)
    {
      
        int eventCount = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
      
        for (int i = 0;i < eventCount; i++)
        {
            if (events[i].data.fd == server_fd)
            {
                // 接收服务器回复的消息
                ssize_t bytes_received = recv(server_fd, buffer, sizeof(buffer), 0);
                if (bytes_received == -1)
                {
                    perror("Failed to receive data");
                    exit(EXIT_FAILURE);
                }

                if (bytes_received == 0)
                {
                    printf("Server disconnected , please retry connect\n");
                    exit(EXIT_FAILURE);
                }
                else
                {
                    buffer[bytes_received] = '\0';
                    printf("Received data from server: %s\n\n", buffer);
                }
            }
            if (events[i].data.fd == fileno(stdin))
            {
                // 从标准输入读入用户输入的消息
                fgets(buffer, sizeof(buffer), stdin);
                // 发送消息给服务器
                ssize_t bytes_sent = send(server_fd, buffer, strlen(buffer), 0);
                if (bytes_sent == -1)
                {
                    perror("Failed to send data");
                    exit(EXIT_FAILURE);
                }
                printf("Send success!!!\n\n");
                if(strncmp(buffer,"quit",4)==0)
                {
                    printf("Client disconnected with server successfully\n");
                    return 0;
                }
            }
        }
    }

    close(server_fd);

    return 0;
}




/**
 * 当客户端的socket连接意外断开时，服务端会收到一个SIGPIPE信号。这个信号通常是由客户端主动关闭socket连接引起的，但也有可能是由于网络问题或其他原因导致的。

在Linux系统中，可以使用signalfd函数来注册SIGPIPE信号，并将其传递给应用程序。然后，应用程序可以使用epoll_wait函数来监听SIGPIPE信号的到来。
一旦收到SIGPIPE信号，应用程序就可以执行相应的处理逻辑，例如重新建立连接、发送错误消息等。

需要注意的是，在使用epoll模型监控socket连接时，需要确保应用程序能够正确处理SIGPIPE信号，否则可能会导致程序崩溃或出现其他异常情况。
 * 
*/