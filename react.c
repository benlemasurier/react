/*
 * react - watch a file, when it changes, react.
 * ben lemasurier 2k12
 *
 * license: free as in give me credit. beer is cool too.
 *
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <sys/select.h>
#include <sys/signal.h>
#include "config.h"

#define EVENT_SIZE    (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))

struct react {
  int fd;
  int wd;
  char *file;
  char *command;
} react;

void
fail(const char *on)
{
  perror(on);
  exit(EXIT_FAILURE);
}

void
quit(void)
{
  if(react.wd)
    inotify_rm_watch(react.fd, react.wd);

  if(react.fd)
    close(react.fd);
}

void
sigquit(int signum)
{
  quit();
}

void
watch(void)
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

    system(react.command);
  }
}

static void
usage(void)
{
  printf("react <file to watch> <command to run>");
  exit(EXIT_SUCCESS);
}

int
main(int argc, char **argv)
{
  memset(&react, 0, sizeof(struct react));

  /* quit cleanly */
  atexit(quit);
  signal(SIGINT, sigquit);

  /* command-line options */
  if(argc < 3) usage();
  react.file    = argv[1];
  react.command = argv[2];

  /* initialize inotify */
  if((react.fd = inotify_init()) < 0)
    fail("inotify_init");

  if((react.wd = inotify_add_watch(react.fd, react.file, IN_MODIFY)) < 0)
    fail(react.file);

  while(1)
    watch();

  exit(EXIT_SUCCESS);
}
