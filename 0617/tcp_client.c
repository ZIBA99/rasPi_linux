#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define TCP_PORT 5100

int main(int argc, char **argv)
{
    int ssock;
    struct sockaddr_in servaddr;
    char mesg[BUFSIZ];

    if(argc < 2) {
        printf("Usage : %s IP_ADDRESS\n", argv[0]);
        return -1;
    }

    /* 소켓 생성 */
    if((ssock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        return -1;
    }

    /* 소켓이 접속할 주소 지정 */
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;

    /* 문자열을 네트워크 주소로 변환 */
    if(inet_pton(AF_INET, argv[1], &(servaddr.sin_addr)) <= 0) {
        perror("inet_pton()");
        return -1;
    }
    servaddr.sin_port = htons(TCP_PORT);

    /* 지정한 주소로 접속 */
    if(connect(ssock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect()");
        return -1;
    }

    /* 키보드로부터 데이터 받아서 서버에 전송 */
    while(fgets(mesg, BUFSIZ, stdin) != NULL) {
        if(send(ssock, mesg, strlen(mesg), 0) <= 0) {
            perror("send()");
            return -1;
        }

        /* 서버로부터 데이터 읽기 */
        memset(mesg, 0, BUFSIZ);
        if(recv(ssock, mesg, BUFSIZ, 0) <= 0) {
            perror("recv()");
            return -1;
        }

        /* 받은 문자열 출력 */
        printf("Received data : %s", mesg);

        /* 종료 조건 */
        if(strncmp(mesg, "q", 1) == 0)
            break;
    }

    /* 소켓 닫기 */
    close(ssock);

    return 0;
}

