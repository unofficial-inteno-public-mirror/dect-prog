
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
#include "preloader.h"




#define BUF_SIZE 500

static struct bin_img flashloader;
static struct bin_img *pr = &flashloader;

//--------------------------------------------------------------------------
//     PC                            TARGET
//     ==                            ======
//             PRELOADER_START
//    --------------------------------->
//             PRELOADER_READY
//    <---------------------------------
//           PRELOADER_BAUD_xxxx
//    --------------------------------->
//         PRELOADER_NEW_BAUDRATE
//    --------------------------------->
//       PRELOADER_NEW_BAUDRATE_READY
//    <---------------------------------
//             msb code length
//    --------------------------------->
//            lsb code length
//    --------------------------------->
//                 code
//    --------------------------------->
//                 ....
// 			       ....
//                 code
//    --------------------------------->
//                 chk
//    <---------------------------------
//
//     Boot loader down loaded now
//
//--------------------------------------------------------------------------




static void read_flashloader(void) {

	int fd, size, ret, sz_ht;
	struct stat s;

	fd = open("flashloader", O_RDONLY);
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


	/* Reply */
	e->out[0] = SOH;
	e->out[1] = pr->size_lsb;
	e->out[2] = pr->size_msb;
	e->outcount = 3;

	printf("SOH\n");

}


static void send_flashloader(event_t *e) {
  
	memcpy(e->out, pr->img, pr->size);
	e->outcount = pr->size;
}

static void send_start(event_t *e) {
  
	e->out[0] = 1;
	e->outcount = 1;
}


static void set_baudrate(event_t *e) {

	/* e->out[0] = PRELOADER_START; */
	/* e->out[1] = PRELOADER_BAUD_115200; */
	/* //	e->out[2] = PRELOADER_NEW_BAUDRATE; */
	/* e->outcount = 2; */

	uint8_t c[2];

	c[0] = PRELOADER_BAUD_115200;
	
	util_dump(c, 1, "[WRITE]");
	write(e->fd, c, 1);

	usleep(300*1000);
	tty_set_baud(e->fd, B115200);
	usleep(300*1000);
	
	c[0] = PRELOADER_NEW_BAUDRATE;
	util_dump(c, 1, "[WRITE]");
	write(e->fd, c, 1);

}


void init_preloader_state(int dect_fd) {
	
	uint8_t c = PRELOADER_START;
	
	printf("PRELOADER_STATE\n");

	/* /\* write ack *\/ */
	/* c[0] = 6; */
	/* util_dump(c, 1, "[WRITE]"); */
	/* write(dect_fd, c, 1); */
	
	/* read_flashloader(); */
	/* calculate_checksum(); */

	/* tty_set_raw(dect_fd); */
	usleep(300*1000);
	tty_set_baud(dect_fd, B9600);


	util_dump(&c, 1, "[WRITE]");
	write(dect_fd, &c, 1);
	
}


void handle_preloader_package(event_t *e) {

	
	switch (e->in[0]) {
	       
	case PRELOADER_READY:
		printf("PRELOADER_READY\n");
		set_baudrate(e);
		break;
		
	case PRELOADER_NEW_BAUDRATE_READY:
		printf("PRELOADER_NEW_BAUDRATE_READY\n");
		break;

	default:
		printf("Unknown preloader packet: %x\n", e->in[0]);
		break;
	}

}





struct state_handler preloader_handler = {
	.state = PRELOADER_STATE,
	.init_state = init_preloader_state,
	.event_handler = handle_preloader_package,
};

struct state_handler * preloader_state = &preloader_handler;
