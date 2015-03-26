
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

#include "MRtxDef.h"
#include "MailDef.h"


#define BUF_SIZE 500

static struct bin_img flashloader;
static struct bin_img *pr = &flashloader;

enum fl_state {
	NEW_PACKET,
};

int fl_state;

uint8_t packetbuf[BUF_SIZE];

//-----------------------------------------------------------------------------
//   
//   Packet format
//   =============
//
//  +-----+----------------- 
//  | 0   | Header
//  +-----+----------------- 
//  | 1   | Length lsb          n=Length
//  +-----+----------------- 
//  | 2   | Length msb
//  +-----+----------------- 
//  | 3   | Payload 1
//  +-----+----------------- 
//  | .   | ...
//  +-----+----------------- 
//  | .   | ... 
//  +-----+----------------- 
//  | 2+n | Payload n
//  +-----+----------------- 
//  | 3+n | crc lsb
//  +-----+----------------- 
//  | 4+n | crc msb
//  +-----+----------------- 
//   
//   
//-----------------------------------------------------------------------------


static uint8_t * make_tx_packet(uint8_t * tx, void * packet, int data_size) {
  
  uint8_t * data = (uint8_t *) packet;
  int i;
  uint16_t crc = 0;
  
  tx[0] = UART_PACKET_HEADER;
  tx[1] = (uint8_t) data_size;
  tx[2] = (uint8_t) data_size >> 8;
  
  for (i=0; i<data_size; i++) {
    crc = UpdateCrc(data[i], crc);
    tx[3 + i] = data[i];
  }
  
  tx[3 + data_size] = (uint8_t) crc;
  tx[4 + data_size] = (uint8_t) (crc >> 8);
  
  return tx;
}


static int inspect_rx(event_t *e) {
	
	uint32_t data_size = 0, i;
	uint16_t crc = 0, crc_calc = 0;
	
	/* Check header */
	if (e->in[0] =! UART_PACKET_HEADER) {
		printf("Drop packet: no header\n");
		return -1;
	}

	/* Check size */
	if (e->incount < PACKET_OVER_HEAD) {
		printf("Drop packet: packet size smaller then PACKET_OVER_HEAD %d < %d\n",
		       e->incount, PACKET_OVER_HEAD);
		return -1;
	}

	/* Do we have a full packet? */
	data_size = (((uint32_t) e->in[2] << 8) | e->in[1]);
	if (e->incount < (data_size + PACKET_OVER_HEAD)) {
		printf("Drop packet: not a full packet incount: %d < packet size: %d\n",
		       e->incount, data_size + PACKET_OVER_HEAD);
		return -1;
	}
	
	/* Read packet checksum */
	crc = (( ((uint16_t) e->in[e->incount - 1]) << 8) | e->in[e->incount - 2]);

	/* Calculate checksum over data portion */
	for (i = 0; i < data_size; i++) {
		crc_calc = UpdateCrc(e->in[i + 3], crc_calc);
	}

	if (crc != crc_calc) {
		printf("Drop packet: bad packet checksum: %x != %x\n", crc, crc_calc);
		return -1;
	}

	return 0;
}


static send_packet(void * data, int data_size, int fd) {

  int tx_size = data_size + PACKET_OVER_HEAD;
  uint8_t * tx = malloc(tx_size);
  
  make_tx_packet(tx, data, data_size);
  util_dump(tx, tx_size, "[WRITE]");
  write(fd, tx, tx_size);
  free(tx);
}


static void get_sw_version(void) {
  
}


static void read_flashloader(void) {


  
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

	uint8_t c[2];

	/* Reply */
	c[0] = pr->size_msb;
	c[1] = pr->size_lsb;

	util_dump(c, 2, "[WRITE]");
	write(e->fd, c, 2);
}


static void send_flashloader(event_t *e) {
  
	/* memcpy(e->out, pr->img, pr->size); */
	/* e->outcount = pr->size; */

	util_dump(pr->img, pr->size, "[WRITE]");
	write(e->fd, pr->img, pr->size);

}

static void send_start(event_t *e) {
  
	e->out[0] = 1;
	e->outcount = 1;
}

static void sw_version_req(int fd) {
  
	SwVersionReqType *r = malloc(sizeof(SwVersionReqType));
  
	r->Primitive = READ_SW_VERSION_REQ;
	send_packet(r, sizeof(SwVersionReqType), fd);
	free(r);
}


static void sw_version_cfm(event_t *e) {
  
	SwVersionCfmType *p = (SwVersionCfmType *) &e->in[3];

	printf("version: %d\n", p->Version);
	printf("revision: %d\n", p->Revision);
	printf("flashloader id: %d\n", p->FlashLoaderId);
	
}




void init_flashloader_state(int dect_fd) {
	
	printf("FLASHLOADER_STATE\n");
	
	usleep(300*1000);
	sw_version_req(dect_fd);
	/* read_firmware(); */
	/* calculate_checksum(); */

}




void handle_flashloader_package(event_t *e) {

	
	if (inspect_rx(e) < 0) {
		printf("dropped packet\n");
	} 

	
	switch (e->in[3]) {

	case READ_SW_VERSION_CFM:
		printf("READ_SW_VERSION_CFM\n");
		sw_version_cfm(e);
		break;
		
	default:
		printf("Unknown flashloader packet: %x\n", e->in[0]);
		break;
	}

}





struct state_handler flashloader_handler = {
	.state = FLASHLOADER_STATE,
	.init_state = init_flashloader_state,
	.event_handler = handle_flashloader_package,
};

struct state_handler * flashloader_state = &flashloader_handler;
