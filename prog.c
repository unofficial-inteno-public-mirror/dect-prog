
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/epoll.h>
#include <termios.h>

#include "dect.h"
#include "tty.h"
#include "error.h"
#include "state.h"
#include "boot.h"
#include "util.h"
#include "prog.h"






void init_prog_state(int dect_fd) {
	
	printf("PROG_STATE\n");

	tty_set_raw(dect_fd);
	tty_set_baud(dect_fd, B115200);
}


void handle_prog_package(event_t *e) {

	
	switch (e->in[0]) {

	default:
		printf("Unknown packet: %x\n", e->in[0]);

	}
	
}





struct state_handler prog_handler = {
	.state = PROG_STATE,
	.init_state = init_prog_state,
	.event_handler = handle_prog_package,
};

struct state_handler * prog_state = &prog_handler;
