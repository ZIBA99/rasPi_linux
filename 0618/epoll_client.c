#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>

#define SERVER_PORT 5100
#define SERVER_IP "192.168.0.2"
#define MAX_EVENTS 10
#define BUFFER_SIZE 1024

// 소켓을 논블로킹 모드로 설정하는 함수
void setnonblocking(int sock)
{
    int opts;
    opts = fcntl(sock, F_GETFL);
    if (opts < 0) {
        perror("fcntl(F_GETFL)");
        exit(EXIT_FAILURE);
    }
    opts = (opts | O_NONBLOCK);
    if (fcntl(sock, F_SETFL, opts) < 0) {
        perror("fcntl(F_SETFL)");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    int n;

    // epoll 관련 변수
    int epfd;
    struct epoll_event ev;
    struct epoll_event events[MAX_EVENTS];
    int nfds;

    // 소켓 생성
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        return -1;
    }

    // 서버 주소 설정
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;

    // 명령줄 인자로 서버 IP를 받으면 사용, 아니면 기본값 사용
    if (argc > 1) {
        inet_pton(AF_INET, argv[1], &server_addr.sin_addr);
    } else {
        inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);
    }

    server_addr.sin_port = htons(SERVER_PORT);

    // 서버에 연결
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect()");
        return -1;
    }

    printf("Connected to server %s:%d\n", SERVER_IP, SERVER_PORT);

    // 소켓을 논블로킹 모드로 설정
    setnonblocking(sock);

    // epoll 인스턴스 생성
    epfd = epoll_create1(0);
    if (epfd == -1) {
        perror("epoll_create1()");
        return -1;
    }

    // 서버 소켓을 epoll에 등록
    ev.events = EPOLLIN | EPOLLET; // 읽기 이벤트 및 에지 트리거 모드
    ev.data.fd = sock;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &ev) == -1) {
        perror("epoll_ctl()");
        return -1;
    }

    // 표준 입력을 epoll에 등록
    ev.events = EPOLLIN;
    ev.data.fd = STDIN_FILENO;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev) == -1) {
        perror("epoll_ctl()");
        return -1;
    }

    printf("Client ready. Type a message (q to quit):\n");

    while (1) {
        nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("epoll_wait()");
            break;
        }

        for (int i = 0; i < nfds; i++) {
            // 서버로부터 데이터 수신
            if (events[i].data.fd == sock) {
                memset(buffer, 0, BUFFER_SIZE);
                n = read(sock, buffer, BUFFER_SIZE);
                if (n <= 0) {
                    if (n < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            continue; // 논블로킹 모드에서 데이터가 없는 경우
                        }
                        perror("read()");
                    }
                    printf("Server disconnected\n");
                    close(sock);
                    return 0;
                } else {
                    printf("Server response: %s", buffer);
                }
            }
            // 사용자 입력 처리
            else if (events[i].data.fd == STDIN_FILENO) {
                memset(buffer, 0, BUFFER_SIZE);
                n = read(STDIN_FILENO, buffer, BUFFER_SIZE);
                if (n > 0) {
                    // 종료 조건 확인
                    if (strncmp(buffer, "q", 1) == 0) {
                        printf("Disconnecting from server...\n");
                        close(sock);
                        return 0;
                    }

                    // 서버로 메시지 전송
                    if (write(sock, buffer, n) <= 0) {
                        perror("write()");
                        close(sock);
                        return -1;
                    }
                }
            }
        }
    }

    close(sock);
    return 0;
}

