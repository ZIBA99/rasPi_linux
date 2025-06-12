#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char **argv)
{
    struct stat statbuf;

    if (argc < 3) {
        fprintf(stderr, "Usage: %s file1 file2\n", argv[0]);
        return -1;
    }

    /* 1. 파일 존재 여부 확인 */
    if (stat(argv[1], &statbuf) < 0) {
        perror("stat");
        return -1;
    }

    /* 2. set-group-ID 설정 및 그룹 실행 권한 해제 */
    if (chmod(argv[1], (statbuf.st_mode & ~S_IXGRP) | S_ISGID) < 0) {
        perror("chmod");
        return -1;
    }

    /* 3. 두 번째 파일의 권한을 644 (rw-r--r--)로 설정 */
    if (chmod(argv[2], S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) < 0) {
        perror("chmod");
        return -1;
    }

    return 0;
}

