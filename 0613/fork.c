#include <stdio.h>
#include <unistd.h>

static int g_var = 1;
char str[] = "PID"; // 세미콜론(;)이 빠져있었습니다

int main (int argc,char **argv)
{
        int var;
        pid_t pid;
        var = 92;

        if((pid = fork()) < 0){
                perror("[ERROR] : fork()");
        }else if(pid == 0){
                g_var++;
                var++;
                printf("Parent %s from child Process(%d) : %d\n", str, getpid(), getppid()); // gettppid() → getppid()로 수정, pid 인자 제거
                sleep(1);
        }
        printf("pid = %d, GLobal var = %d, var = %d\n", getpid(), g_var, var);

        return 0;
}

