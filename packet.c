#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Api/FpGeneral/ApiFpGeneral.h>
#include <Api/FpMm/ApiFpMm.h>
#include <RosPrimitiv.h>

#include "error.h"
#include "packet.h"
#include "prog.h"



#define BUSMAIL_PACKET_HEADER 0x10
#define BUSMAIL_HEADER_SIZE 3
#define BUSMAIL_PACKET_OVER_HEAD 4
#define API_PROG_ID 0x00
#define API_TEST 0x01

#define PACKET_TYPE_MASK (1 << 7)
#define INFORMATION_FRAME (0 << 7)
#define CONTROL_FRAME (1 << 7)

#define CONTROL_FRAME_MASK ((1 << 7) | (1 << 6))
#define UNNUMBERED_CONTROL_FRAME ((1 << 7) | (1 << 6))
#define SUPERVISORY_CONTROL_FRAME ((1 << 7) | (0 << 6))

/* Information frame */
#define TX_SEQ_MASK ((1 << 6) | (1 << 5) | (1 << 4))
#define TX_SEQ_OFFSET 4
#define RX_SEQ_MASK  ((1 << 2) | (1 << 1) | (1 << 0))
#define RX_SEQ_OFFSET 0
#define PF_MASK ((1 << 3))
#define PF_OFFSET 3

/* Supervisory control frame */
#define SUID_MASK ((1 << 5) | (1 << 4))
#define SUID_RR   ((0 << 1) | (0 << 0))
#define SUID_REJ  ((0 << 1) | (1 << 0))
#define SUID_RNR  ((1 << 1) | (0 << 0))
#define SUID_OFFSET 4
#define NO_PF 0
#define PF 1


#define POLL_FINAL (1 << 3)
#define SAMB_POLL_SET 0xc8
#define SAMB_NO_POLL_SET 0xc0

uint8_t rx_next;
uint8_t tx_next;

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




static uint8_t make_supervisory_frame(uint8_t suid, uint8_t pf, uint8_t tx_seq) {
	
	uint8_t header, rx_next;

	if (tx_seq < 7) {
		rx_next = tx_seq + 1;
	} else {
		rx_next = 0;
	}
	
	header = ( (suid << SUID_OFFSET) | (pf << PF_OFFSET) | rx_next );

	return header;
}


static uint8_t make_info_frame(uint8_t tx_next, uint8_t pf, uint8_t rx_next) {
	
	uint8_t header;

	header = ( (tx_next << TX_SEQ_OFFSET) | (pf << PF_OFFSET) | rx_next );

	return header;
}



static busmail_t * busmail_send(uint8_t * data, int size, busmail_t *m, int fd) {

	uint8_t tx_seq, rx_seq;
	busmail_t * r;	

	r = malloc(BUSMAIL_PACKET_OVER_HEAD - 1 + size);
	if (!r) {
		exit_failure("malloc");
	}

	tx_seq = (m->frame_header & TX_SEQ_MASK) >> TX_SEQ_OFFSET;
	rx_seq = (m->frame_header & RX_SEQ_MASK) >> RX_SEQ_OFFSET;
		
	r->frame_header = make_info_frame(tx_seq, NO_PF, rx_seq);
	r->program_id = API_PROG_ID;
	r->task_id = API_TEST;
	memcpy(&(r->mail_header), data, size);
	/* r->mail_header =  */
	/* r->mail_data[0] = 0; */

	send_packet(r, BUSMAIL_PACKET_OVER_HEAD - 1 + size, fd);
	free(r);

}


static void supervisory_control_frame(packet_t *p) {
	
	busmail_t * m = (busmail_t *) &p->data[0];
	uint8_t rx_seq, pf, suid;

	rx_seq = (m->frame_header & RX_SEQ_MASK) >> RX_SEQ_OFFSET;
	pf = (m->frame_header & PF_MASK) >> PF_OFFSET;
	suid = (m->frame_header & SUID_MASK) >> SUID_OFFSET;

	printf("frame_header: %02x\n", m->frame_header);

	
	printf("suid: ");
	
	switch (suid & SUID_MASK) {
		
	case SUID_RR:
		printf("SUID_RR\n");
		break;

	case SUID_REJ:
		printf("SUID_REJ\n");
		break;

	case SUID_RNR:
		printf("SUID_RNR\n");
		break;
	}

	printf("rx_seq: %d\n", rx_seq);
	printf("pf: %d\n", pf);

}



static void information_frame(packet_t *p) {

	busmail_t * m = (busmail_t *) &p->data[0];

	uint8_t tx_seq, rx_seq, pf, sh, ih;
	

	/* Drop unwanted frames */
	if( m->program_id != API_PROG_ID ) {
		return;
	}

	packet_dump(p);
	
	tx_seq = (m->frame_header & TX_SEQ_MASK) >> TX_SEQ_OFFSET;
	rx_seq = (m->frame_header & RX_SEQ_MASK) >> RX_SEQ_OFFSET;
	pf = (m->frame_header & PF_MASK) >> PF_OFFSET;


	printf("frame_header: %02x\n", m->frame_header);
	printf("tx_seq: %d\n", tx_seq);
	printf("rx_seq: %d\n", rx_seq);
	printf("pf: %d\n", pf);

	switch (m->mail_header) {
		
	case API_FP_RESET_IND:
		printf("API_FP_RESET_IND\n");
		
		ApiFpMmStartProtocolReqType * r = malloc(sizeof(ApiFpMmStartProtocolReqType));
		r->Primitive = API_FP_MM_START_PROTOCOL_REQ;

		printf("API_FP_MM_START_PROTOCOL_REQ\n");
		busmail_send((uint8_t *)r, sizeof(ApiFpMmStartProtocolReqType), m, p->fd);
		free(r);


		break;

	case API_SCL_STATUS_IND:
		printf("API_SCL_STATUS_IND\n");
		/* just ack the package */
		sh = make_supervisory_frame(SUID_RR, NO_PF, tx_seq);
		printf("header: %02x\n", sh);
		printf("SUID_RR\n");
		send_packet(&sh, 1, p->fd);
		break;

		
	}


}


int packet_get(packet_t *p, buffer_t *b) {
	
	int i, start, stop, size;
	uint8_t crc = 0, crc_calc = 0;
	uint8_t buf[5000];

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

	/* Do we have a full header? */
	if (buffer_size(b) < 2) {
		return -1;
	}
	buffer_read(b, buf + 1, 2);

	/* Packet size */
	size = (((uint32_t) buf[1] << 8) | buf[2]);
	
	/* Do we have a full packet? */
	if (BUSMAIL_PACKET_OVER_HEAD + size >= b->count - start) {
		return -1;
	}
	buffer_read(b, buf + 3, size + 1);
	
	/* Read packet checksum */
	crc = (( (uint8_t) buf[start + BUSMAIL_PACKET_OVER_HEAD + size - 1]));

	/* Calculate checksum over data portion */
	for (i = 0; i < size; i++) {
		crc_calc += buf[i + 3];
	}

	if (crc != crc_calc) {
		printf("Drop packet: bad packet checksum: %x != %x\n", crc, crc_calc);
		return -1;
	}

	/* Copy data portion to packet */
	memcpy(p->data, buf + 3, size);
	p->size = size;

	return 0;
}


void packet_dump(packet_t *p) {
	
	int i;

	printf("[PACKET %d] - ", p->size);
	for (i = 0; i < p->size; i++) {
		printf("%02x ", p->data[i]);
	}
	printf("\n");
}


void packet_dispatch(packet_t *p) {

	busmail_t * m = (busmail_t *) &p->data[0];
	
	/* Drop invalid packets */
	/* if (packet_inspect(p) < 0) { */
	/* 	printf("dropped packet\n"); */
	/* }  */
	

	/* Route packet based on type */

	switch (m->frame_header & PACKET_TYPE_MASK) {
		
	case INFORMATION_FRAME:
		information_frame(p);
		break;

	case CONTROL_FRAME:

		switch (m->frame_header & CONTROL_FRAME_MASK) {
			
		case UNNUMBERED_CONTROL_FRAME:
			printf("UNNUMBERED_CONTROL_FRAME\n");
			unnumbered_control_frame(p);
			break;

		case SUPERVISORY_CONTROL_FRAME:
			printf("SUPERVISORY_CONTROL_FRAME\n");
			supervisory_control_frame(p);
			break;
		}

	break;

	default:
		printf("Unknown packet header: %x\n", m->frame_header);

	}



}

