#include <stdio.h>
#include <unistd.h>

int main(void)
{
	printf ("hello, world!\n");
	_exit(10);
	return 0;
}
