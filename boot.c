
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



#include "dect.h"


struct bin_img {
  uint8_t *img;
  int size;
  uint8_t size_msb;
  uint8_t size_lsb;
  uint8_t checksum;
};

#define MAX_EVENTS 10

static struct bin_img preloader;
static struct bin_img *pr = &preloader;
int dect_fd, state;

static void exit_failure(const char *format, ...)
{
#define BUF_SIZE 500
	char err[BUF_SIZE], msg[BUF_SIZE];
	va_list ap;
	
	strncpy(err, strerror(errno), BUF_SIZE);

	va_start(ap, format);
	vsprintf(msg, format, ap);
	va_end(ap);
	
	fprintf(stderr, "%s: %s\n", msg, err);
	exit(EXIT_FAILURE);
}


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


static void write_dect(void *data, int size) {

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


static void calculate_checksum(void) {
  
	uint8_t chk=0;
	int i;
	uint8_t * FlashLoaderCodePtr = pr->img;

	// Calculate Checksum of flash loader                                                                                                        
	for (i=0; i< pr->size; i++)
		{
			chk^=FlashLoaderCodePtr[i];
		}

	pr->checksum = chk;
	printf("checksum: %x\n", pr->checksum);
  
}


static void send_size(void) {

	unsigned char o_buf[3];

	read_preloader();
	calculate_checksum();

	*(o_buf + 0) = SOH;
	*(o_buf + 1) = pr->size_lsb;
	*(o_buf + 2) = pr->size_msb;

	printf("SOH\n");
	write_dect(o_buf, 3);
}


static void send_preloader(void) {
  
	write_dect(pr->img, pr->size);
}

static void send_ack(void) {
  
	uint8_t r[1];

	r[0] = ACK;

	write_dect(r, 1);
}

#define PRELOADER_START 1

static void send_preloader_start(void) {
  
	uint8_t r[1];

	r[0] = PRELOADER_START;

	write_dect(r, 1);
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


void handle_boot_package(unsigned char *buf) {

	//  RosPrimitiveType primitive = ((ApifpccEmptySignalType *) buf)->Primitive;
	//  struct packet *p = (struct packet *)buf;

	
	switch (buf[0]) {

	case SOH:
		printf("SOH\n");
		break;
	case STX:
		printf("\n\n\nSTX\n");
		send_size();
		break;
	case ETX:
		printf("ETX\n");
		break;
	case ACK:
		printf("ACK\n");
		send_preloader();
		break;
	case NACK:
		printf("NACK\n\n");
		break;
	default:
		if (buf[0] == pr->checksum) {
			printf("Checksum ok!\n");
			send_ack();
			printf("state: PRELOADER_STATE\n");
			state = PRELOADER_STATE;
		/* 	/\* printf("send_preloader_start()\n"); *\/ */
		/* 	/\* sleep(1); *\/ */
		/* 	/\* send_preloader_start(); *\/ */
		} else {
			printf("Unknown boot packet: %x\n", buf[0]);
		}
		break;
	}

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
