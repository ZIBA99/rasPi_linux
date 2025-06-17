#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERVER_PORT 5100

int main(int argc, char **argv)
{
    int ssock;
    socklen_t clen;
    int n;
    struct sockaddr_in servaddr, cliaddr;
    char mesg[BUFSIZ];

    fd_set readfd;              /* select()용 fd_set 자료형 */
    int maxfd, client_index, start_index;
    int client_fd[5] = {0};     /* 최대 5명 클라이언트 소켓 저장 */

    /* 서버 소켓 디스크립터 생성 */
    if((ssock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        return -1;
    }
    
    /* 소켓 재사용 옵션 설정 */
    int opt = 1;
    if(setsockopt(ssock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt()");
        return -1;
    }

    /* 운영체제에 서비스 등록 */
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERVER_PORT);

    if(bind(ssock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind()");
        return -1;
    }

    /* 클라이언트 대기 큐 생성 */
    if(listen(ssock, 8) < 0) {
        perror("listen()");
        return -1;
    }

    FD_ZERO(&readfd);           /* fd_set 초기화 */
    maxfd = ssock;              /* 최대 fd는 서버 소켓 */
    client_index = 0;

    printf("Server started. Waiting for connections...\n");

    do {
        FD_ZERO(&readfd);
        FD_SET(ssock, &readfd); /* 서버 소켓 읽기 감지 설정 */

        /* 클라이언트 소켓들을 fd_set에 추가 */
        for(start_index = 0; start_index < client_index; start_index++) {
            FD_SET(client_fd[start_index], &readfd);
            if(client_fd[start_index] > maxfd)
                maxfd = client_fd[start_index];
        }
        maxfd = maxfd + 1;

        /* 읽기 가능한 소켓만 조사 (블로킹) */
        if(select(maxfd, &readfd, NULL, NULL, NULL) < 0) {
            perror("select()");
            return -1;
        }

        /* 서버 소켓이 읽기 가능하면 클라이언트 접속 요청 처리 */
        if(FD_ISSET(ssock, &readfd)) {
            clen = sizeof(struct sockaddr_in);
            int csock = accept(ssock, (struct sockaddr *)&cliaddr, &clen);
            if(csock < 0) {
                perror("accept()");
                return -1;
            } else {
                inet_ntop(AF_INET, &cliaddr.sin_addr, mesg, BUFSIZ);
                printf("Client is connected : %s\n", mesg);

                /* 새 클라이언트 소켓을 fd_set에 추가 */
                FD_SET(csock, &readfd);
                client_fd[client_index] = csock;
                client_index++;

                if(client_index == 5) break; /* 최대 5명 제한 */
                continue;
            }
        }

        /* 읽기 가능했던 클라이언트 소켓 처리 */
        for(start_index = 0; start_index < client_index; start_index++) {
            if(FD_ISSET(client_fd[start_index], &readfd)) {
                memset(mesg, 0, sizeof(mesg));
                n = read(client_fd[start_index], mesg, sizeof(mesg));
                if(n > 0) {
                    printf("Received data from client %d: %s", start_index, mesg);
                    write(client_fd[start_index], mesg, n); /* 에코 */
                    
                    // 'q'를 입력받으면 해당 클라이언트 연결 종료
                    if(strncmp(mesg, "q", 1) == 0) {
                        printf("Client %d disconnected\n", start_index);
                        close(client_fd[start_index]);
                        FD_CLR(client_fd[start_index], &readfd);
                        
                        /* 배열에서 해당 클라이언트 제거 */
                        for(int i = start_index; i < client_index - 1; i++) {
                            client_fd[i] = client_fd[i + 1];
                        }
                        client_index--;
                        start_index--; /* 인덱스 보정 */
                    }
                } else if(n == 0) {
                    // 클라이언트가 연결을 종료한 경우
                    printf("Client %d disconnected\n", start_index);
                    close(client_fd[start_index]);
                    FD_CLR(client_fd[start_index], &readfd);
                    
                    /* 배열에서 해당 클라이언트 제거 */
                    for(int i = start_index; i < client_index - 1; i++) {
                        client_fd[i] = client_fd[i + 1];
                    }
                    client_index--;
                    start_index--; /* 인덱스 보정 */
                }
            }
        }
    } while(1); // 무한 루프로 변경

    close(ssock); /* 서버 소켓 닫기 */

    return 0;
}

