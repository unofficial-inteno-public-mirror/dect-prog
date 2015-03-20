
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




#define BUF_SIZE 500

static struct bin_img preloader;
static struct bin_img *pr = &preloader;





static void read_preloader(void) {

	int fd, size, ret, sz_ht;
	struct stat s;

	fd = open("preloader", O_RDONLY);
	if (fd == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	fstat(fd, &s);
  
	pr->size = s.st_size;
	pr->size_msb = (uint8_t) (pr->size >> 8);
	pr->size_lsb = (uint8_t) pr->size;

	printf("size: %d\n", pr->size);
	printf("size_msb: %d\n", pr->size_msb);
	printf("size_msb_x: %x\n", pr->size_msb);
	printf("size_lsb: %d\n", pr->size_lsb);
	printf("size_lsb_x: %x\n", pr->size_lsb);
  
	pr->img = malloc(pr->size);
  
	ret = read(fd, pr->img, pr->size);
	if (ret == -1) {
		perror("read");
		exit(EXIT_FAILURE);
	}

	close(fd);
}




static void calculate_checksum(void) {
  
	uint8_t chk=0;
	int i;
	uint8_t * FlashLoaderCodePtr = pr->img;

	// Calculate Checksum of flash loader
	for (i=0; i< pr->size; i++) {
		chk^=FlashLoaderCodePtr[i];
	}

	pr->checksum = chk;
	printf("checksum: %x\n", pr->checksum);
}


static void send_size(event_t *e) {

	read_preloader();
	calculate_checksum();

	/* Reply */
	e->out[0] = SOH;
	e->out[1] = pr->size_lsb;
	e->out[2] = pr->size_msb;
	e->outcount = 3;

	printf("SOH\n");

}


static void send_preloader(int fd) {
  
	util_write(pr->img, pr->size, fd);
	free(pr->img);
	
}

static void send_ack(int fd) {
  
	uint8_t r[1];

	r[0] = ACK;

	util_write(r, 1, fd);
}

#define PRELOADER_START 1

static void send_preloader_start(int fd) {
  
	uint8_t r[1];

	r[0] = PRELOADER_START;

	util_write(r, 1, fd);
}



void init_boot_state(int dect_fd) {
	
	printf("BOOT_STATE\n");

	tty_set_raw(dect_fd);
	tty_set_baud(dect_fd, B19200);
	
}


void handle_boot_package(event_t *e) {

	//  RosPrimitiveType primitive = ((ApifpccEmptySignalType *) buf)->Primitive;
	//  struct packet *p = (struct packet *)buf;

	
	switch (e->in[0]) {

	case SOH:
		printf("SOH\n");
		break;
	case STX:
		printf("\n\n\nSTX\n");
		send_size(e);
		break;
	case ETX:
		printf("ETX\n");
		break;
	case ACK:
		printf("ACK\n");
		send_preloader(e->fd);
		break;
	case NACK:
		printf("NACK\n\n");
		break;
	default:
		if (e->in[0] == pr->checksum) {
			printf("Checksum ok!\n");
			send_ack(e->fd);
			printf("state: PRELOADER_STATE\n");
			//state = PRELOADER_STATE;
		/* 	/\* printf("send_preloader_start()\n"); *\/ */
		/* 	/\* sleep(1); *\/ */
		/* 	/\* send_preloader_start(); *\/ */
		} else {
			printf("Unknown boot packet: %x\n", e->in[0]);
		}
		break;
	}

}





struct state_handler boot_handler = {
	.state = BOOT_STATE,
	.init_state = init_boot_state,
	.event_handler = handle_boot_package,
};

struct state_handler * boot_state = &boot_handler;
