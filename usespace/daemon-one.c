#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

void spawn_again(int signum)
{
	(void) signum; /* unused */

	pid_t pid = fork();
	if (pid == 0) {
		setsid();
		return;
	}
	exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
	struct sigaction new_action = { 0 };
	new_action.sa_handler = &spawn_again;
	sigaction (SIGTERM, &new_action, NULL);

	/* do something */
	while(1) {};
}
