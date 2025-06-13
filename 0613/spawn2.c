#include <stdio.h>
#include <unistd.h>
#include <spawn.h>
#include <sys/wait.h> // waitpid 함수 사용을 위한 헤더

extern char **environ;

int system(char *cmd)
{
        pid_t pid;
        int status;
        posix_spawn_file_actions_t actions; // 변수명 오타 수정 (action → actions)
        posix_spawnattr_t attrs; // 변수명 오타 수정 (posix_apawnattr_t → posix_spawnattr_t)
        char *argv[]= {"sh", "-c", cmd, NULL};

        posix_spawn_file_actions_init(&actions);
        posix_spawnattr_init(&attrs);
        posix_spawnattr_setflags(&attrs, POSIX_SPAWN_SETSCHEDULER); // 함수명 및 인자 오타 수정

        posix_spawn(&pid, "/bin/sh", &actions, &attrs, argv, environ);
        
        waitpid(pid, &status, 0); // 이 줄이 누락되었습니다
        
        // 자원 해제 추가
        posix_spawn_file_actions_destroy(&actions);
        posix_spawnattr_destroy(&attrs);

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

