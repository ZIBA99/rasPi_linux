#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>    // wait()를 위해 필요
#include <sys/socket.h>  // socketpair()

int main(int argc, char **argv)
{
    int ret, sock_fd[2];
    int status;
    char buf[] = "Hello World", line[BUFSIZ];
    pid_t pid;

    // 소켓 페어 생성 (UNIX 도메인, 스트림)
    ret = socketpair(AF_LOCAL, SOCK_STREAM, 0, sock_fd);
    if (ret == -1) {
        perror("socketpair()");
        return -1;
    }

    printf("socket 1 : %d\n", sock_fd[0]);
    printf("socket 2 : %d\n", sock_fd[1]);

    if ((pid = fork()) < 0) {
        perror("fork()");
    } else if (pid == 0) {
        // 자식 프로세스: 데이터 전송
        write(sock_fd[0], buf, strlen(buf) + 1);
        printf("Data sent : %s\n", buf);
        close(sock_fd[0]);
    } else {
        // 부모 프로세스: 데이터 수신
        wait(&status);  // 자식 프로세스 종료 대기

        read(sock_fd[1], line, BUFSIZ);
        printf("Received data : %s\n", line);
        close(sock_fd[1]);
    }

    return 0;
}

