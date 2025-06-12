#include <stdio.h>
#include <signal.h>   /* signal() 함수 사용 */
#include <stdlib.h>   /* exit() 함수 사용 */
#include <string.h>   /* strsignal() 함수 사용 */
#include <unistd.h>

static void printSigset(sigset_t *set);   /* 현재 sigset_t에 설정된 시그널 표시 */
static void sigHandler(int signo);         /* 시그널 처리용 핸들러 */

int main(int argc, char **argv)
{
    sigset_t pset;

    sigemptyset(&pset);                   /* 모두 0으로 초기화 */
    sigaddset(&pset, SIGQUIT);            /* SIGQUIT 추가 */
    sigaddset(&pset, SIGRTMIN);           /* SIGRTMIN 추가 */
    sigprocmask(SIG_BLOCK, &pset, NULL);  /* 현재 시그널 마스크에 추가 */

    printSigset(&pset);                   /* 현재 설정된 sigset_t 출력 */

    /* SIGINT 시그널 처리용 핸들러 등록 */
    if(signal(SIGINT, sigHandler) == SIG_ERR) {
        perror("signal() : SIGINT");
        return -1;
    }

    /* SIGUSR1 시그널 처리용 핸들러 등록 */
    if(signal(SIGUSR1, sigHandler) == SIG_ERR) {
        perror("signal() : SIGUSR1");
        return -1;
    }

    /* SIGUSR2 시그널 처리용 핸들러 등록 */
    if(signal(SIGUSR2, sigHandler) == SIG_ERR) {
        perror("signal() : SIGUSR2");
        return -1;
    }

    /* SIGPIPE 시그널 무시 */
    if(signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        perror("signal() : SIGPIPE");
        return -1;
    }

    /* 시그널 처리 대기 */
    while(1) pause();

    return 0;
}

static void sigHandler(int signo)
{
    if(signo == SIGINT) {
        printf("SIGINT is catched : %d\n", signo);
        exit(0);
    } else if(signo == SIGUSR1) {
        printf("SIGUSR1 is catched\n");
    } else if(signo == SIGUSR2) {
        printf("SIGUSR2 is catched\n");
    } else if(signo == SIGQUIT) {
        printf("SIGQUIT is catched\n");

        sigset_t uset;
        sigemptyset(&uset);
        sigaddset(&uset, SIGINT);
        sigprocmask(SIG_UNBLOCK, &uset, NULL);
    } else {
        fprintf(stderr, "Catched signal : %s\n", strsignal(signo));
    }
}

static void printSigset(sigset_t *set)
{
    int i;
    for(i = 1; i < NSIG; ++i) {
        printf("%d ", sigismember(set, i) ? 1 : 0);
    }
    putchar('\n');
}

