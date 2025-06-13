#include <stdio.h>
#include <unistd.h>     // pipe(), fork(), read(), write(), close() 함수 사용
#include <sys/wait.h>   // waitpid() 함수 사용


int main(int argc, char **argv)
{
    pid_t pid;
    int pfd[2];             // 파이프용 파일 디스크립터 배열: pfd[0] 읽기, pfd[1] 쓰기
    char line[BUFSIZ];      // 데이터를 읽어올 버퍼
    int status;

    // 파이프 생성
    if (pipe(pfd) < 0) {
        perror("pipe()");
        return -1;
    }

    // 프로세스 생성 (fork)
    if ((pid = fork()) < 0) {
        perror("fork()");
        return -1;
    } 
    else if (pid == 0) {  // 자식 프로세스
        close(pfd[0]);    // 읽기용 파이프 닫기 (자식은 쓰기만 함)
        dup2(pfd[1], 1);  // 표준출력(1)을 파이프 쓰기 디스크립터로 변경
        execl("/bin/date", "date", NULL);  // date 명령어 실행
        close(pfd[1]);    // 쓰기용 파이프 닫기
        _exit(127);       // execl 실패 시 종료
    } 
    else {  // 부모 프로세스
        close(pfd[1]);    // 쓰기용 파이프 닫기 (부모는 읽기만 함)
        if (read(pfd[0], line, BUFSIZ) < 0) {  // 파이프에서 데이터 읽기
            perror("read()");
            return -1;
        }
        printf("%s", line);  // 읽은 데이터 출력
        close(pfd[0]);       // 읽기용 파이프 닫기
        waitpid(pid, &status, 0);  // 자식 프로세스 종료 대기
    }

    return 0;
}

