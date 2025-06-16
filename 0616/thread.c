#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>

sem_t *sem;               /* 세마포어를 위한 전역 변수 */
static int cnt = 0;       /* 세마포어에서 사용할 임계 구역 변수 */

void p()                   /* 세마포어의 P 연산 (sem_wait) */
{
    sem_wait(sem);
}

void v()                   /* 세마포어의 V 연산 (sem_post) */
{
    sem_post(sem);
}

void *pthreadV(void *arg)  /* V 연산을 수행하는 스레드 함수 */
{
    int i;
    for(i = 0; i < 10; i++) {
        p();              /* P 연산 수행 */
        cnt++;
        printf("Thread: cnt = %d\n", cnt);
        fflush(stdout);
        v();              /* V 연산 수행 */
        usleep(100000);   /* 0.1초 대기 */
    }
    return NULL;
}

int main(int argc, char **argv)
{
    pthread_t ptv;
    const char *name = "iposex_sem";  /* 세마포어 이름 */
    unsigned int value = 7;            /* 세마포어 초기값 */

    /* 세마포어 생성 및 초기화 */
    sem = sem_open(name, O_CREAT, 0644, value);
    if(sem == SEM_FAILED) {
        perror("sem_open");
        return -1;
    }

    /* 스레드 생성 */
    pthread_create(&ptv, NULL, pthreadV, NULL);

    /* 메인 스레드는 100번 대기 */
    int i;
    for(i = 0; i < 100; i++) {
        usleep(1000);
    }

    /* 스레드 종료 대기 */
    pthread_join(ptv, NULL);

    /* 세마포어 닫기 및 삭제 */
    sem_close(sem);
    sem_unlink(name);

    printf("Main thread 종료\n");

    return 0;
}
