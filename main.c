#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <termios.h>
#include <stdio.h>

#include "dect.h"
#include "tty.h"
#include "error.h"
#include "boot.h"
#include "state.h"
#include "util.h"



#define MAX_EVENTS 10
#define BUF_SIZE 500

int dect_fd;



void write_dect(void *data, int size) {

	util_dump(data, size, "[WRITE]");
	write(dect_fd, data, size);

	return;
}





int main(void) {
	
	struct epoll_event ev, events[MAX_EVENTS];
	int state = BOOT_STATE;
	int epoll_fd, nfds, i, count;
	uint8_t buf[BUF_SIZE];
	void (*state_event_handler)(uint8_t *buf);
 	
	epoll_fd = epoll_create(10);
	if (epoll_fd == -1) {
		exit_failure("epoll_create\n");
	}

	dect_fd = open("/dev/ttyUSB1", O_RDWR);
	if (dect_fd == -1) {
		exit_failure("open\n");
	}


	ev.events = EPOLLIN;
	ev.data.fd = dect_fd;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, dect_fd, &ev) == -1) {
		exit_failure("epoll_ctl\n");
	}
	
	state_add_handler(boot_state);
	
	/* Initial transition */
	state_transition(BOOT_STATE);

	for(;;) {

		nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
		if (nfds == -1) {
			exit_failure("epoll_wait\n");
		}

		for (i = 0; i < nfds; ++i) {
			if (events[i].data.fd == dect_fd) {
				count = read(dect_fd, buf, BUF_SIZE);
				util_dump(buf, count, "[READ]");

				/* Dispatch to current event handler */
				state_event_handler = state_get_handler();
				state_event_handler(buf);
			}
		}
		
	}

	return 0;
}
