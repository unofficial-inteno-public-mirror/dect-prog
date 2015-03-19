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

#define MAX_EVENTS 10
#define BUF_SIZE 500

int dect_fd, state;



void write_dect(void *data, int size) {

	int i, dumpsize;
	int maxdump = 100;
	unsigned char* cdata = (unsigned char*)data;
	printf("[WDECT][%04d] - ",size);
    
	if ( size > maxdump ) {
		dumpsize = maxdump; 
	} else {
		dumpsize = size; 
	}

	for (i=0 ; i<dumpsize ; i++) {
		printf("%02x ",cdata[i]);
	}
	printf("\n");

	write(dect_fd, data, size);

	return;
}


static void dump_package(unsigned char *buf, int size) {

	int i, dumpsize;
	int maxdump = 100;
	unsigned char* cdata = (unsigned char*)buf;

	printf("[RDECT][%04d] - ",size);
    
	if ( size > maxdump ) {
		dumpsize = maxdump; 
	} else {
		dumpsize = size; 
	}

	for (i=0 ; i<dumpsize ; i++) {
		printf("%02x ",cdata[i]);
	}
	printf("\n");
	
}



int main(void) {
	
	struct epoll_event ev, events[MAX_EVENTS];
	int state = BOOT_STATE;
	int epoll_fd, nfds, i, count;
	uint8_t buf[BUF_SIZE];

	epoll_fd = epoll_create(10);
	if (epoll_fd == -1) {
		exit_failure("epoll_create\n");
	}

	dect_fd = open("/dev/ttyUSB1", O_RDWR);
	if (dect_fd == -1) {
		exit_failure("open\n");
	}

	tty_set_raw(dect_fd);
	tty_set_baud(dect_fd, B19200);
	

	ev.events = EPOLLIN;
	ev.data.fd = dect_fd;

	if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, dect_fd, &ev) == -1) {
		exit_failure("epoll_ctl\n");
	}

	for(;;) {

		nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
		if (nfds == -1) {
			exit_failure("epoll_wait\n");
		}

		for (i = 0; i < nfds; ++i) {
			if (events[i].data.fd == dect_fd) {
				count = read(dect_fd, buf, BUF_SIZE);
				dump_package(buf, count);
				handle_boot_package(buf);
			}
		}
		
	}

	return 0;
}
