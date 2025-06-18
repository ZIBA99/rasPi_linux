#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#define SERVER_PORT 5100  // SEVER_PORT에서 SERVER_PORT로 수정
#define MAX_EVENT 32

void setnonblocking(int fd)
{
    int opts = fcntl(fd, F_GETFL);
    opts |= O_NONBLOCK;  // 0_NONBLOCK에서 O_NONBLOCK으로 수정
    fcntl(fd, F_SETFL, opts);
}

int main(int argc, char **argv)
{
    int ssock, csock;
    socklen_t clen;  // scoklen_t에서 socklen_t로 수정
    int n, epfd, nfds = 1;
    struct sockaddr_in servaddr, cliaddr;
    struct epoll_event ev;
    struct epoll_event events[MAX_EVENT];  // MAX,EVENT에서 MAX_EVENT로 수정
    char mesg[BUFSIZ];

    if((ssock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("socket()");
        return -1;
    }

    setnonblocking(ssock);

    memset(&servaddr, 0, sizeof(servaddr));  // sizof에서 sizeof로 수정
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);  // INDDR_ANY에서 INADDR_ANY로 수정
    servaddr.sin_port = htons(SERVER_PORT);
    if(bind(ssock, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){  // sevaddr에서 servaddr로 수정
        perror("bind()");
        return -1;
    }

    if(listen(ssock, 8) < 0){
        perror("listen()");  // perrpr에서 perror로 수정
        return -1;
    }

    epfd = epoll_create(MAX_EVENT);
    if(epfd == -1){
        perror("epoll_create()");
        return 1;
    }

    ev.events = EPOLLIN;  // envents에서 events로 수정
    ev.data.fd = ssock;

    if(epoll_ctl(epfd, EPOLL_CTL_ADD, ssock, &ev) == -1){
        perror("epoll_ctl()");
        return 1;
    }

    do{
        nfds = epoll_wait(epfd, events, MAX_EVENT, 500);  // epooll_wait에서 epoll_wait로 수정, nfds 값 저장
        for(int i = 0; i < nfds; i++){  // evets에서 events로 수정
            if(events[i].data.fd == ssock){
                clen = sizeof(struct sockaddr_in);
                csock = accept(ssock, (struct sockaddr*)&cliaddr, &clen);  // $clen에서 &clen으로 수정
                if(csock > 0){
                    inet_ntop(AF_INET, &cliaddr.sin_addr, mesg, BUFSIZ);
                    printf("Client is connected : %s\n", mesg);

                    setnonblocking(csock);

                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.fd = csock;  // ev,data.fd에서 ev.data.fd로 수정

                    epoll_ctl(epfd, EPOLL_CTL_ADD, csock, &ev);  // csockm에서 csock으로 수정
                    nfds++;
                    continue;
                }
            }else if(events[i].events & EPOLLIN){
                if(events[i].data.fd < 0) continue;  // coutinue에서 continue로 수정

                memset(mesg, 0, sizeof(mesg));

                n = read(events[i].data.fd, mesg, sizeof(mesg));  // 괄호 닫기 수정
                if(n > 0)                if(n > 0){
                    printf("Received data : %s", mesg);
                    write(events[i].data.fd, mesg, n); /* 에코 */
                    // 소켓을 닫거나 epoll에서 제거하지 않음
                } else if (n == 0) { // 클라이언트가 연결을 정상적으로 종료했을 때
                    printf("Client disconnected\n");
                    close(events[i].data.fd);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                    nfds--;
                } else { // read 오류 발생 시 (EAGAIN/EWOULDBLOCK 제외)
                     if (errno != EAGAIN && errno != EWOULDBLOCK) {
                        perror("read error");
                        close(events[i].data.fd);
                        epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                        nfds--;
                     }
                }
  // 괄호 수정
            }
        }
    }while(strncmp(mesg, "q", 1));

    close(ssock);

    return 0;
}

