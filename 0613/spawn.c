#include <stdio.h>
#include <unistd.h>
#include <spawn.h>
#include <sys/wait.h> // waitpid 함수를 사용하기 위해 필요한 헤더 추가

extern char **environ;

int system(char *cmd)
{
        pid_t pid; // 변수명 오타 수정 (pidi → pid)
        int status;
        char *argv[]= {"sh", "-c", cmd, NULL};

        posix_spawn(&pid, "/bin/sh", NULL, NULL, argv, environ);

        waitpid(pid, &status, 0);

        return status;
}

int main(int argc, char **argv, char **envp)
{
        while(*envp)
                printf("%s\n", *envp++);

        system("who");
        system("nocommand");
        system("cal");

        return 0;
}

