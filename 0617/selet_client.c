#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <time.h> 

#define SERVER_PORT 5100

#define SERVER_IP "192.168.0.2"

int main(int argc, char **argv)
{
    int sock;
    struct sockaddr_in servaddr;
    char mesg[BUFSIZ];
    int n;

    srand(time(NULL) + getpid());//아이디 랜덤

    /* 소켓 생성 */
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        return -1;
    }

    /* 서버 주소 설정 */
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    
    /* 명령줄 인자로 서버 IP를 받으면 사용, 아니면 기본값 사용 */
    if(argc > 1) {
        inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
    } else {
        inet_pton(AF_INET, SERVER_IP, &servaddr.sin_addr);
    }
    
    servaddr.sin_port = htons(SERVER_PORT);

    /* 서버에 연결 */
    if(connect(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect()");
        return -1;
    }

    printf("Connected to server. Type a message (q to quit):\n");

    /* 클라이언트 ID 설정 (랜덤) */
    int client_id = rand() % 1000;
    printf("Your client ID: %d\n", client_id);

    while(1) {
        /* 사용자 입력 받기 */
        memset(mesg, 0, BUFSIZ);
        printf("Client %d > ", client_id);
        fgets(mesg, BUFSIZ, stdin);
        
        /* 종료 조건 확인 */
        if(strncmp(mesg, "q", 1) == 0) {
            printf("Disconnecting from server...\n");
            write(sock, mesg, strlen(mesg)); // 'q' 메시지 전송
            break;
        }

        /* 서버로 메시지 전송 */
        if(write(sock, mesg, strlen(mesg)) <= 0) {
            perror("write()");
            break;
        }

        /* 서버로부터 응답 수신 */
        memset(mesg, 0, BUFSIZ);
        n = read(sock, mesg, BUFSIZ);
        if(n <= 0) {
            printf("Server closed connection\n");
            break;
        }

        printf("Server response: %s", mesg);
    }

    /* 소켓 닫기 */
    close(sock);
    return 0;
}

