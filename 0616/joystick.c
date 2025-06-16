#include <stdio.h>
#include <stdlib.h>     /* exit() 함수를 위해 필요 */
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

#define JOY_DEV "/dev/input/event2" 

int main()
{
    int fd;
    struct input_event ie;

    if ((fd = open(JOY_DEV, O_RDONLY)) == -1) {
        perror("opening device");
        exit(EXIT_FAILURE);
    }

    while(read(fd, &ie, sizeof(struct input_event))) {
        printf("time %ld.%06ld\ttype %d\tcode %-3d\tvalue %d\n",
               ie.time.tv_sec, ie.time.tv_usec, ie.type, ie.code, ie.value);
        if(ie.type) {
            switch(ie.code) {
                case KEY_UP: printf("Up\n"); break;
                case KEY_DOWN: printf("Down\n"); break;
                case KEY_LEFT: printf("Left\n"); break;
                case KEY_RIGHT: printf("Right\n"); break;
                case KEY_ENTER: printf("Enter\n"); break;
                default: printf("Default\n"); break;
            }
        }
    }

    return 0;
}


//struct input_event{
//	struct timeval time;
//	__u16 type;
//	__u16 code;
//	__s32 value;
//};

//struct timeval{
//	long tv_sec;
//	long tv_usec;
//};
