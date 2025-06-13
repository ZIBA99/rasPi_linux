#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>

int main()
{
        struct rlimit rlim;

        getrlimit(RLIMIT_NPROC, &rlim);  // RLMIT_NPROC → RLIMIT_NPROC
        printf("max user processes : %lu / %lu\n", rlim.rlim_cur, rlim.rlim_max);

        getrlimit(RLIMIT_NOFILE, &rlim);  // RLMIT_NOFILE → RLIMIT_NOFILE
        printf("file size : %lu / %lu\n", rlim.rlim_cur, rlim.rlim_max);

        getrlimit(RLIMIT_RSS, &rlim);  // getrlimt → getrlimit
        printf("max memory size : %lu / %lu\n", rlim.rlim_cur, rlim.rlim_max);

        getrlimit(RLIMIT_CPU, &rlim);  // getrlimt → getrlimit, RLIMT_CPU → RLIMIT_CPU
        if(rlim.rlim_cur == RLIM_INFINITY){  // RLIM_LNFLNTTY → RLIM_INFINITY
                printf("cpu time : UNLIMIT\n");  // UNLIMT → UNLIMIT
        }

        return 0;
}

