#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define TCP_PORT 5100

int main(int argc, char **argv)
{
    int sock;
    struct sockaddr_in servaddr;
    char buffer[BUFSIZ];
    int n;

    if(argc < 2) {
        printf("Usage: %s IP_ADDRESS\n", argv[0]);
        return -1;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0) {
        perror("socket()");
        return -1;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    if(inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        perror("inet_pton()");
        return -1;
    }
    servaddr.sin_port = htons(TCP_PORT);

    if(connect(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect()");
        return -1;
    }

    // 서버에 메시지 보내기
    const char *msg = "Hello Server!";
    if(send(sock, msg, strlen(msg), 0) < 0) {
        perror("send()");
        return -1;
    }
    printf("Sent message: %s\n", msg);

    // 우아한 종료: 더 이상 송신하지 않음 (읽기는 계속 가능)
    if(shutdown(sock, SHUT_WR) < 0) {
        perror("shutdown()");
        return -1;
    }
    printf("Shutdown write side of the socket\n");

    // 서버로부터 응답 받기
    while((n = recv(sock, buffer, BUFSIZ - 1, 0)) > 0) {
        buffer[n] = '\0';
        printf("Received: %s\n", buffer);
    }

    if(n < 0) {
        perror("recv()");
    } else {
        printf("Server closed connection\n");
    }

    // 소켓 완전 종료
    close(sock);

    return 0;
}
