#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h> // errno를 사용하기 위해 추가

#define SERVER_PORT 5100 			/* 서버의 포트 번호 */
#define MAX_EVENT 	10 // MAX_EVENT를 조금 더 여유있게 설정 (필수는 아님)
#define BUFFER_SIZE 1024 // 버퍼 크기 정의

/* 파일 디스크립터를 넌블로킹 모드로 설정 */
void setnonblocking(int fd)
{
    int opts = fcntl(fd, F_GETFL);
    opts |= O_NONBLOCK;
    fcntl(fd, F_SETFL, opts);
}

int main(int argc, char **argv)
{
    int ssock, n;
    struct sockaddr_in servaddr;
    struct epoll_event ev;
    struct epoll_event events[MAX_EVENT];
    char mesg[BUFFER_SIZE]; // 버퍼 크기 변경
    int nfds; // epoll_wait 반환 값 저장 변수

	if(argc < 2) {
		printf("usage : %s IP_ADDR\n", argv[0]);
		return -1;
	}

    /* 서버 소켓 디스크립터를 연다. */
    if((ssock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        return -1;
    }

    memset(&servaddr, 0, sizeof(servaddr)); 	/* 운영체제에 서비스 등록 */
    servaddr.sin_family = AF_INET;
	inet_pton(AF_INET, argv[1], &(servaddr.sin_addr.s_addr));
    servaddr.sin_port = htons(SERVER_PORT);

    printf("Connecting to server %s:%d...\n", argv[1], SERVER_PORT);
	if(connect(ssock, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect()");
        close(ssock); // 연결 실패 시 소켓 닫기
        return -1;
    }
    printf("Connected to server %s:%d\n", argv[1], SERVER_PORT);


    setnonblocking(ssock); 			/* 서버 소켓을 넌블로킹 모드로 설정 */

    /* epoll_create() 함수를 이용해서 커널에 등록 */
    int epfd = epoll_create1(0); // epoll_create1 사용 권장 (flags=0)
    if(epfd == -1) {
        perror("epoll_create1()");
        close(ssock);
        return 1;
    }

    /* epoll_ctl() 함수를 통해 감시하고 싶은 파일 디스크립터들을 등록 */

    // 표준 입력 (STDIN_FILENO) 등록
    ev.events = EPOLLIN;
    ev.data.fd = STDIN_FILENO; // 표준 입력은 0번 파일 디스크립터
    if(epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev) == -1) {
        perror("epoll_ctl(STDIN_FILENO)");
        close(ssock);
        close(epfd);
        return 1;
    }

    // 서버 소켓 등록
    ev.events = EPOLLIN | EPOLLET; // 서버 소켓은 에지 트리거 모드 사용
    ev.data.fd = ssock;
    if(epoll_ctl(epfd, EPOLL_CTL_ADD, ssock, &ev) == -1) {
        perror("epoll_ctl(ssock)");
        close(ssock);
        close(epfd);
        return 1;
    }

    printf("Client ready. Type a message (q to quit):\n");

    // 계속 채팅을 위한 무한 루프
    while (1) {
        // epoll_wait로 이벤트 대기
        nfds = epoll_wait(epfd, events, MAX_EVENT, -1); // -1은 무한 대기
        if (nfds == -1) {
            perror("epoll_wait()");
            break; // 오류 발생 시 루프 종료
        }

        // 발생한 이벤트 처리
        for(int i = 0; i < nfds; i++) {
            // 표준 입력 이벤트 (사용자 입력)
            if(events[i].data.fd == STDIN_FILENO) {
                memset(mesg, 0, sizeof(mesg));
                // 표준 입력에서 읽기
                n = read(STDIN_FILENO, mesg, sizeof(mesg));
                if (n > 0) {
                    // 개행 문자 제거 (fgets 대신 read를 사용했으므로 수동 처리 필요)
                    if (mesg[n-1] == '\n') {
                        mesg[n-1] = '\0';
                        n--; // 개행 문자 길이만큼 줄임
                    } else {
                         mesg[n] = '\0'; // read는 널 종료를 보장하지 않으므로 수동 추가
                    }


                    // 'q' 입력 시 종료
                    if (strcmp(mesg, "q") == 0) {
                        printf("Disconnecting from server...\n");
                        // 서버에 종료 메시지를 보낼 수도 있습니다 (선택 사항)
                        // write(ssock, "quit", strlen("quit"));
                        goto end_loop; // 루프 종료
                    }

                    // 서버로 메시지 전송
                    if (write(ssock, mesg, n) == -1) {
                         perror("write to server failed");
                         goto end_loop; // 전송 실패 시 종료
                    }
                } else if (n == 0) {
                    // EOF (Ctrl+D) 입력 시 종료
                    printf("EOF received on stdin. Disconnecting...\n");
                    goto end_loop;
                } else {
                     // read 오류 처리 (논블로킹이 아니므로 EAGAIN/EWOULDBLOCK은 발생 안 함)
                     perror("read from stdin failed");
                     goto end_loop;
                }
            }
            // 서버 소켓 이벤트 (서버로부터 데이터 수신)
            else if(events[i].data.fd == ssock) {
                memset(mesg, 0, sizeof(mesg));
                // 서버 소켓에서 읽기 (에지 트리거이므로 모든 데이터를 읽어야 함)
                // 간단한 에코 서버에서는 한 번 read로 충분할 수 있지만,
                // 실제로는 루프를 돌며 EAGAIN/EWOULDBLOCK이 반환될 때까지 읽는 것이 안전합니다.
                // 여기서는 간단하게 한 번만 읽도록 유지합니다.
                n = read(events[i].data.fd, mesg, sizeof(mesg) - 1); // 널 종료를 위해 1바이트 남김

				if(n > 0) {
                    mesg[n] = '\0'; // 널 종료
             	    printf("Server response: %s\n", mesg);
                } else if (n == 0) { // 서버가 연결을 정상적으로 종료
                    printf("Server disconnected\n");
                    goto end_loop; // 루프 종료
                } else { // read 오류 발생
                     if (errno != EAGAIN && errno != EWOULDBLOCK) {
                        perror("read from server failed");
                        goto end_loop; // 오류 발생 시 종료
                     }
                     // EAGAIN 또는 EWOULDBLOCK은 데이터가 아직 준비되지 않았다는 의미이므로 무시
                }
            }
        }
    }

end_loop: // 종료 지점 레이블

    // 소켓과 epoll 인스턴스 닫기
    close(ssock);
    close(epfd);

    printf("Client terminated.\n");

    return 0;
}

