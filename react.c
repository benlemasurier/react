/*
 * react - watch a file, when it changes, react.
 * ben lemasurier 2k12
 * https://github.com/benlemasurier/react
 *
 * license: free as in beer.
 *
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/inotify.h>
#include <sys/select.h>
#include <sys/signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "config.h"

#define EVENT_SIZE    (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))

struct react {
	int fd;
	int wd;
	int logging;
	char *file;
	char *command;
	char *progname;
	bool debug;
	bool foreground;
} react;

void fail(const char *on)
{
	perror(on);
	exit(EXIT_FAILURE);
}

void quit(void)
{
	if(react.wd)
		inotify_rm_watch(react.fd, react.wd);

	if(react.fd)
		close(react.fd);

	syslog(LOG_NOTICE, "shutting down");
	closelog();
}

void sigquit(int signum)
{
	quit();
}

void watch(void)
{
	int ret;
	char buf[EVENT_BUF_LEN];
	struct timeval time;
	fd_set fds;

	/* timeout every 5 minutes */
	time.tv_sec  = 300;
	time.tv_usec = 0;

	FD_ZERO(&fds);
	FD_SET(react.fd, &fds);
	ret = select(react.fd + 1, &fds, NULL, NULL, &time);
	if(ret < 0)
		fail("select");
	else if(FD_ISSET(react.fd, &fds)) {
		int i = 0;
		int length = 0;

		if((length = read(react.fd, buf, EVENT_BUF_LEN)) < 0)
			fail("read");

		while(i < length) {
			struct inotify_event *e = (struct inotify_event *) &buf[i];

			i += (EVENT_SIZE + e->len);
		}

		if(react.debug)
			syslog(LOG_NOTICE, "%s changed, calling %s\n", react.file, react.command);

		system(react.command);
	}
}

static void usage(void)
{
	printf("%s [-fv] <file to watch> <command>\n", react.progname);
	exit(EXIT_SUCCESS);
}

static void version(void)
{
	printf("react %d.%d\n",
			REACT_VERSION_MAJOR, REACT_VERSION_MINOR);
}

int main(int argc, char **argv)
{
	int arg;
	pid_t pid, sid;

	/* defaults */
	memset(&react, 0, sizeof(struct react));
	react.progname = argv[0];
	react.logging = LOG_CONS | LOG_PID | LOG_NDELAY;
	react.foreground = false;

	/* quit cleanly */
	atexit(quit);
	signal(SIGINT, sigquit);

	while((arg = getopt(argc, argv, "dfv")) != -1) {
		switch(arg) {
			case 'd':
				react.debug = true;
				react.logging |= LOG_PERROR;
				break;
			case 'f':
				react.foreground = true;
				break;
			case 'v':
				version();
				exit(EXIT_SUCCESS);
			case '?':
				if(optopt == 'h')
					fprintf(stderr, "Option -%c requires an argument.\n", optopt);
				else if(isprint(optopt))
					fprintf(stderr, "Unknown option '-%c'.\n", optopt);
				else
					fprintf(stderr, "Unknown option '\\x%x'\n", optopt);
		}
	}

	/* command-line options */
	if(argc < 3) usage();
	react.file    = argv[argc - 2];
	react.command = argv[argc - 1];

	/* logging */
	openlog(react.progname, react.logging, LOG_LOCAL3);

	/* daemonize? */
	if(!react.foreground) {
		pid = fork();
		if(pid < 0)
			exit(EXIT_FAILURE);
		else if(pid > 0)
			exit(EXIT_SUCCESS);

		umask(0);
		sid = setsid();
		if(sid < 0)
			exit(EXIT_FAILURE);
	}

	/* initialize inotify */
	if((react.fd = inotify_init()) < 0)
		fail("inotify_init");

	if((react.wd = inotify_add_watch(react.fd, react.file, IN_MODIFY)) < 0)
		fail(react.file);

	while(1)
		watch();

	exit(EXIT_SUCCESS);
}
