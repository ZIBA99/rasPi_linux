#include <stdio.h>
#include <unistd.h>

int main()
{
    printf("Hello, "); 
    printf("world!");
    fflush(stdout);
    _exit(1);
}
