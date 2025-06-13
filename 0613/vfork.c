#include <stdio.h>
#include <unistd.h>

static int g_var = 1;
char str[] = "PID"; // 세미콜론(;)이 빠져있었습니다

int main (int argc,char **argv)
{
        int var;
        pid_t pid;
        var = 88;

        if((pid = vfork()) < 0){
                perror("[ERROR] : fork()");
        }else if(pid == 0){
                g_var++;
                var++;
		printf("Parent %s from child Process(%d) : %d\n", str, getpid(), getppid());
		printf("pid = %d, GLobal var = %d, var = %d\n", getpid(), g_var, var);
                _exit(0);
        }else{
		printf("Child %s from Parent Process (%d) : %d\n", str, getpid(), pid);
	}
        printf("pid = %d, GLobal var = %d, var = %d\n", getpid(), g_var, var);

        return 0;
}

