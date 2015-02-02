#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main(void)
{
	pid_t pid;
	pid = getpid();
	printf("fuck %lu\n", pid);

	pause();
}
