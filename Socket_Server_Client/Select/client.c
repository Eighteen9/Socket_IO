#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#define SERVER_PORT 11277
int main(int argc, char *argv[])
{
    // 创建套接字
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);

    // 组装socket地址结构____server
    struct sockaddr_in client_addr_in;
    client_addr_in.sin_family = AF_INET;
    client_addr_in.sin_port = htons(SERVER_PORT);
    client_addr_in.sin_addr.s_addr = inet_addr("127.0.0.1");

    // 进行连接
    if (connect(client_socket, (struct sockaddr *)&client_addr_in, sizeof(struct sockaddr)) < 0)
    {
        perror("connect");
        return -1;
    }

    printf("连接到服务器...\n");

    fd_set clientFdSet;

    // 发送数据
    while (1)
    {
        // add client_socket to fd_set
        FD_ZERO(&clientFdSet);
        FD_SET(client_socket, &clientFdSet);
        // add standard input to fd_set
        FD_SET(0, &clientFdSet);
        int fd_max = client_socket;
        struct timeval mytime;
        mytime.tv_sec = 10;
        mytime.tv_usec = 0;
        int activity = select(fd_max + 1, &clientFdSet, NULL, NULL, NULL);
        if (activity == 0)
        {
            printf("time out\n");
            continue;
        }
        else
        {
            if (FD_ISSET(client_socket, &clientFdSet))
            {
                // 接收server发来的数据
                char buffer_recv[1024];
                buffer_recv[0] = '\0';
                printf("Message from server:");
                int iDataNum = read(client_socket, buffer_recv, 1024);
                buffer_recv[iDataNum] = '\0';
                if (strcmp(buffer_recv, "quit") == 0)
                {
                    printf("%s\n", buffer_recv);
                    printf("此时服务器已经单方面断开了连接\n");
                    break;
                }
                printf("%s\n", buffer_recv);
            }

            if (FD_ISSET(0, &clientFdSet))
            {
                char buffer_send[1024];
                printf("Client message send success!!!\n");
                bzero(buffer_send, 1024);
                fgets(buffer_send, 1024, stdin);
                printf("\n");
                send(client_socket, buffer_send, strlen(buffer_send), 0);
                if (strcmp(buffer_send, "quit") == 0)
                {
                    printf("与服务器的连接已断开\n");
                    break;
                }
            }
        }
    }
}
