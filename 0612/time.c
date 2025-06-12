#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>  // putenv, tzset 선언 위해 추가

int main(int argc, char **argv)
{
    time_t rawtime;
    struct tm *tm;
    char buf[BUFSIZ];
    struct timeval mytime;

    time(&rawtime);
    printf("time : %u\n", (unsigned)rawtime);

    gettimeofday(&mytime, NULL);
    printf("gettimeofday : %ld/%ld\n", (long)mytime.tv_sec, (long)mytime.tv_usec);

    printf("ctime : %s", ctime(&rawtime));

    putenv("TZ=PST3PDT");
    tzset();

    tm = localtime(&rawtime);
    printf("asctime : %s", asctime(tm));

    strftime(buf, sizeof(buf), "%a %b %e %H:%M:%S %Y", tm);
    printf("strftime : %s\n", buf);

    return 0;
}

