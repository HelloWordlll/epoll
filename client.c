/*************************************************************************
	> File Name: client.c
	> Author: 
	> Mail: 
	> Created Time: Sat 17 Feb 2024 04:50:51 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    // 创建一个TCP socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);

    // 连接到服务端
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port = htons(8080);

    connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address));
    char buf[125], mes[125];
    printf("connet ok\n");
    while(1){
    // 发送数据
    memset(&mes, 0, sizeof(mes));
    memset(&buf, 0, sizeof(buf));
    int n = read(0, mes, 125);
    if (n > 0 && mes[n - 1] == '\n') {
        mes[n - 1] = '\0'; // 将末尾的换行符替换为字符串结束符
    }
    send(client_socket, mes, n, 0);
    printf("send : %s\n", mes);
    if(!strncmp(mes, "quit", 4))break;
    // 接收数据
    recv(client_socket, buf, sizeof(buf), 0);
    printf("收到的数据: %s\n", buf);
    }
    // 关闭连接
    close(client_socket);

    return 0;
}
