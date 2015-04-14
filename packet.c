#include <stdio.h>
#include <stdlib.h>

#include "packet.h"
#include "prog.h"


#define BUSMAIL_PACKET_HEADER 0x10
#define BUSMAIL_HEADER_SIZE 3
#define BUSMAIL_PACKET_OVER_HEAD 4

#define PACKET_TYPE_MASK (1 << 7)
#define INFORMATION_FRAME (0 << 7)
#define CONTROL_FRAME (1 << 7)

#define CONTROL_FRAME_MASK ((1 << 7) | (1 << 6))
#define UNNUMBERED_CONTROL_FRAME ((1 << 7) | (1 << 6))
#define SUPERVISORY_CONTROL_FRAME ((1 << 7) | (0 << 6))

#define POLL_FINAL (1 << 3)
#define SAMB_POLL_SET 0xc8
#define SAMB_NO_POLL_SET 0xc0


static uint8_t * make_tx_packet(uint8_t * tx, void * packet, int data_size) {
  
  uint8_t * data = (uint8_t *) packet;
  int i;
  uint8_t crc = 0;
  
  tx[0] = BUSMAIL_PACKET_HEADER;
  tx[1] = (uint8_t) (data_size >> 8);
  tx[2] = (uint8_t) data_size;

  /* Calculate checksum over data portion */
  for (i = 0; i < data_size; i++) {
	  crc += data[i];
	  tx[3 + i] = data[i];
  }

  
  tx[3 + data_size] = crc;

  
  return tx;
}



static send_packet(void * data, int data_size, int fd) {

  int tx_size = data_size + BUSMAIL_PACKET_OVER_HEAD;
  uint8_t * tx = malloc(tx_size);
  
  make_tx_packet(tx, data, data_size);
  util_dump(tx, tx_size, "[WRITE]");
  write(fd, tx, tx_size);
  free(tx);
}




static void unnumbered_control_frame(packet_t *p) {
	
	uint8_t header = p->data[0];
	uint8_t data;
	
	if (header == SAMB_POLL_SET) {
		printf("SAMB. Reset Rx and Tx counters\n");

		printf("Reply SAMB_NO_POLL_SET. \n");
		data = SAMB_NO_POLL_SET;
		send_packet(&data, 1, p->fd);
		
	} else {
		printf("Bad unnumbered control frame\n");
	}
}


static int packet_inspect(packet_t *p) {
	
	uint32_t data_size = 0, i;
	uint8_t crc = 0, crc_calc = 0;
	
	/* Check header */
	if (p->data[0] =! BUSMAIL_PACKET_HEADER) {
		printf("Drop packet: no header\n");
		return -1;
	}

	/* Check size */
	if (p->size < BUSMAIL_PACKET_OVER_HEAD) {
		printf("Drop packet: packet size smaller then BUSMAIL_PACKET_OVER_HEAD %d < %d\n",
		       p->size, BUSMAIL_PACKET_OVER_HEAD);
		return -1;
	}

	/* Do we have a full packet? */
	data_size = (((uint32_t) p->data[1] << 8) | p->data[2]);
	if (p->size < (data_size + BUSMAIL_PACKET_OVER_HEAD)) {
		printf("Drop packet: not a full packet incount: %d < packet size: %d\n",
		       p->size, data_size + BUSMAIL_PACKET_OVER_HEAD);
		return -1;
	}
	
	/* Read packet checksum */
	crc = (( (uint8_t) p->data[p->size - 1]));

	/* Calculate checksum over data portion */
	for (i = 0; i < data_size; i++) {
		crc_calc += p->data[i + 3];
	}

	if (crc != crc_calc) {
		printf("Drop packet: bad packet checksum: %x != %x\n", crc, crc_calc);
		return -1;
	}

	return 0;
}




int packet_get(packet_t *p, buffer_t *b) {
	
	int i, start, stop, size;
	uint8_t crc = 0, crc_calc = 0;
	uint8_t buf[10];


	/* start = buffer_find(b, BUSMAIL_PACKET_HEADER); */
	/* if (start < 0) { */
	/* 	return -1; */
	/* } */

	/* Do we have a start of frame? */	
	while (buffer_read(b, buf, 1) > 0) {
		if (buf[0] == BUSMAIL_PACKET_HEADER) {
			break;
		}
	}

	printf("got header %x\n", buf[0]);
	/* printf("buffer_size: %d\n", buffer_size(b)); */
	/* buffer_dump(b); */
	/* return 0; */

	/* Do we have a full header? */
	if (buffer_size(b) < 2) {
		return -1;
	}
	buffer_read(b, buf + 1, 2);

	/* Packet size */
	size = (((uint32_t) buf[1] << 8) | buf[2]);
	printf("size: %d\n", size);


	/* Do we have a full packet? */
	if (BUSMAIL_PACKET_OVER_HEAD + size >= b->count - start) {
		return -1;
	}

	/* Read packet checksum */
	crc = (( (uint8_t) b->in[start + BUSMAIL_PACKET_OVER_HEAD + size - 1]));

	/* Calculate checksum over data portion */
	for (i = 0; i < size; i++) {
		crc_calc += b->in[i + 3];
	}

	if (crc != crc_calc) {
		printf("Drop packet: bad packet checksum: %x != %x\n", crc, crc_calc);
		return -1;
	}


	printf("got packet\n");

}


void packet_dump(packet_t *p) {


}


void packet_dispatch(packet_t *p) {

	uint8_t header;

	/* Drop invalid packets */
	/* if (packet_inspect(p) < 0) { */
	/* 	printf("dropped packet\n"); */
	/* }  */
	

	/* Route packet based on type */
	header = p->data[0];
	switch (header & PACKET_TYPE_MASK) {
		
	case INFORMATION_FRAME:
		printf("INFORMATION_FRAME\n");
		break;

	case CONTROL_FRAME:

		switch (header & CONTROL_FRAME_MASK) {
			
		case UNNUMBERED_CONTROL_FRAME:
			printf("UNNUMBERED_CONTROL_FRAME\n");
			unnumbered_control_frame(p);
			break;

		case SUPERVISORY_CONTROL_FRAME:
			printf("SUPERVISORY_CONTROL_FRAME\n");
			break;
		}

	break;

	default:
		printf("Unknown packet header: %x\n", header);

	}



}

