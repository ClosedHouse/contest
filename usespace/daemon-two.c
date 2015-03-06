#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#include "shared.h"

static struct shared *shared;
static char *daemon_one_path;
static int fd;

bool run_daemon_one(void)
{
	int status;
	pid_t pid = fork();

	if (pid == 0) {
		setsid();
		char *argv[] = { "[blkio]", NULL };
		char *envp[] = { NULL };
		execve(daemon_one_path, argv, envp);
		exit(EXIT_FAILURE);
	}

	if (pid != -1) {
		return true;
	}

	waitpid(pid, &status, 0);
	return false;
}

bool check_daemon(void)
{
	if (kill(shared->pid, 0) == -1) {
		return false;
	}

	return true;
}

void init_daemon_one(void)
{
	fd = open("./daemon-one", O_RDONLY);
	if (fd == -1) {
		perror("cannot open daemon-one file!");
		exit (EXIT_FAILURE);
	}
	unlink("./daemon-one");

	asprintf(&daemon_one_path, "/proc/%lu/fd/%d", getpid(), fd);
}

int main(int argv, char **argc)
{

	for (int p = 0; p < 10; ++p) {
		const int shmid = shmget(NUMBER_OF_THE_BEAST, sizeof(struct shared), 0666);
		if (shmid == -1) {
			/* Ok. Normally start daemon */
			init_daemon_one();
			run_daemon_one();
			sleep(1);
			continue;
		}
		shared = (struct shared*) shmat(shmid, NULL, 0);
		if (shared == NULL) {
			perror("shmat failed!");
			exit(EXIT_FAILURE);
		} else {
			break;
		}
	}
	if (shared == NULL) {
		perror("Start of daemon failed!");
		exit(EXIT_FAILURE);
	}

	if (!check_daemon()) {
		init_daemon_one();
		run_daemon_one();
		for (int p = 0; p < 10 || !check_daemon(); ++p) {
			sleep(1);
		}
		if (!check_daemon()) {
			perror("Unable to start daemon-one");
			exit(EXIT_FAILURE);
		}
	}

	while(1) {
		if (!check_daemon()) {
			run_daemon_one();
		}
		sleep(2);
	}

	close(fd);
	free(daemon_one_path);
	return (EXIT_SUCCESS);
}
