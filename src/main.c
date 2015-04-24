#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>

#include "dect.h"
#include "tty.h"
#include "error.h"
#include "boot.h"
#include "state.h"
#include "util.h"
#include "prog.h"



#define MAX_EVENTS 10
#define BUF_SIZE 50000


int main(int argc, char * argv[]) {
	
	struct epoll_event ev, events[MAX_EVENTS];
	int state = BOOT_STATE;
	int epoll_fd, nfds, i, count;
	uint8_t inbuf[BUF_SIZE];
	uint8_t outbuf[BUF_SIZE];
	void (*state_event_handler)(event_t *e);
	int dect_fd;
	event_t event;
	event_t *e = &event;
	config_t c;
	config_t *config = &c;

	e->in = inbuf;
	e->out = outbuf;

	setbuf(stdout, NULL);

	/* Setup input */
	epoll_fd = epoll_create(10);
	if (epoll_fd == -1) {
		exit_failure("epoll_create\n");
	}

	dect_fd = open("/dev/ttyUSB1", O_RDWR);
	if (dect_fd == -1) {
		exit_failure("open\n");
	}

	/* Setup epoll instance */
	ev.events = EPOLLIN;
	ev.data.fd = dect_fd;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, dect_fd, &ev) == -1) {
		exit_failure("epoll_ctl\n");
	}
	

	/* Check user arguments and init config */
	if (check_args(argc, argv, config) < 0) {
		err_exit("Usage: %s <boot | prog>", argv[0]);
	}

	/* Initial transition */
	if (config->mode == BOOT_MODE) {

		/* Program new firmware */
		state_add_handler(boot_state, dect_fd);
		state_transition(BOOT_STATE);

	} else if (config->mode == PROG_MODE) {

		/* Firmware written, init firmware */
		state_add_handler(prog_state, dect_fd);
		state_transition(PROG_STATE);

	} else {
		err_exit("No known operating mode selected\n");
	}


	for(;;) {

		nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
		if (nfds == -1) {
			exit_failure("epoll_wait\n");
		}

		for (i = 0; i < nfds; ++i) {
			if (events[i].data.fd == dect_fd) {

				e->fd = dect_fd;
				e->incount = read(e->fd, e->in, BUF_SIZE);
				//util_dump(e->in, e->incount, "[READ]");
				
				/* Dispatch to current event handler */
				state_event_handler = state_get_handler();
				state_event_handler(e);

				/* Write reply if there is one */
				if (e->outcount > 0) {
					util_dump(e->out, e->outcount, "[WRITE]");
					write(e->fd, e->out, e->outcount);

				
				}

				/* Reset event_t */
				e->outcount = 0;
				e->incount = 0;
				memset(e->out, 0, BUF_SIZE);
				memset(e->in, 0, BUF_SIZE);
			}
		}
		
	}

	return 0;
}
