#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "error.h"
#include "busmail.h"
#include "fifo.h"


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


typedef struct {
	uint8_t * data;
	int size;
	uint8_t task_id;
} tx_packet_t;


#define CLIENT_PKT_DATA_SIZE 1000
#define CLIENT_PKT_TYPE 5
#define CLIENT_PKT_HEADER_SIZE 8

typedef struct {
	uint32_t size;
	uint32_t type;
	uint8_t data[CLIENT_PKT_DATA_SIZE];
} client_packet_t;



/* Module scope variables */
static void * tx_fifo;
static uint8_t tx_seq_l, rx_seq_l, tx_seq_r, rx_seq_r;
static int busmail_fd;
static void (*application_frame) (busmail_t *);
extern void * client_list;
client_packet_t client_p;


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
		tx_seq_l = 0;
		rx_seq_l = 0;
		tx_seq_r = 0;
		rx_seq_r = 0;

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




static uint8_t make_supervisory_frame(uint8_t suid, uint8_t pf) {
	
	uint8_t header;

	header = ( (suid << SUID_OFFSET) | (pf << PF_OFFSET) | rx_seq_l );

	return header;
}


static uint8_t make_info_frame(uint8_t pf) {
	
	uint8_t header;

	header = ( (tx_seq_l << TX_SEQ_OFFSET) | (pf << PF_OFFSET) | rx_seq_l );

	return header;
}



busmail_tx(uint8_t * data, int size, uint8_t pf, uint8_t task_id) {

	uint8_t tx_seq_tmp, rx_seq_tmp;
	busmail_t * r;	
	
	r = malloc(BUSMAIL_PACKET_OVER_HEAD - 1 + size);
	if (!r) {
		exit_failure("malloc");
	}

		
	r->frame_header = make_info_frame(pf);
	r->program_id = API_PROG_ID;
	r->task_id = task_id;
	memcpy(&(r->mail_header), data, size);

	tx_seq_tmp = (r->frame_header & TX_SEQ_MASK) >> TX_SEQ_OFFSET;
	rx_seq_tmp = (r->frame_header & RX_SEQ_MASK) >> RX_SEQ_OFFSET;

	printf("BUSMAIL_SEND_INFO\n");
	printf("tx_seq_l: %d\n", tx_seq_l);
	printf("rx_seq_l: %d\n", rx_seq_l);
	printf("pf: %d\n", pf);

	printf("frame_header: %x\n", (r->frame_header));
	
	send_packet(r, BUSMAIL_PACKET_OVER_HEAD - 1 + size, busmail_fd);
	free(r);
	
	/* Update packet counter */
	tx_seq_l++;

}


void busmail_send(uint8_t * data, int size) {
	
	tx_packet_t * tx = calloc(sizeof(tx_packet_t), 1);
	
	tx->data = malloc(size);
	memcpy(tx->data, data, size);
	tx->task_id = API_TEST;
	tx->size = size;
	
	util_dump(tx->data, tx->size, "fifo_add");
	//fifo_add(tx_fifo, tx);
       
	busmail_tx(tx->data, tx->size, PF, tx->task_id);
	free(tx);
}


/* Needed for test commands */
void busmail_send0(uint8_t * data, int size) {

	tx_packet_t * tx = calloc(sizeof(tx_packet_t), 1);
	
	tx->data = malloc(size);
	memcpy(tx->data, data, size);
	tx->task_id = 0;
	tx->size = size;

	//fifo_add(tx_fifo, tx);

	busmail_tx(tx->data, tx->size, PF, tx->task_id);
	free(tx);
}


void busmail_ack(void) {

	uint8_t sh, rx_seq_tmp;

	sh = make_supervisory_frame(SUID_RR, NO_PF);
	rx_seq_tmp = (sh & RX_SEQ_MASK) >> RX_SEQ_OFFSET;
	
	printf("\nWRITE: BUSMAIL_ACK %d\n", rx_seq_tmp);
	send_packet(&sh, 1, busmail_fd);

}


static void supervisory_control_frame(packet_t *p) {
	
	busmail_t * m = (busmail_t *) &p->data[0];
	uint8_t pf, suid;

	rx_seq_r = (m->frame_header & RX_SEQ_MASK) >> RX_SEQ_OFFSET;
	pf = (m->frame_header & PF_MASK) >> PF_OFFSET;
	suid = (m->frame_header & SUID_MASK) >> SUID_OFFSET;

	packet_dump(p);
	
	switch (suid) {
		
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

	printf("rx_seq_r: %d\n", rx_seq_r);
	printf("pf: %d\n", pf);

}


static void send_to_client(int fd) {

	printf("send_to_client: %d\n", fd);
	
	if (send(fd, &client_p, client_p.size, 0) == -1) {
		perror("send");
	}
}

static void information_frame(packet_t *p) {

	busmail_t * m = (busmail_t *) &p->data[0];
	uint8_t pf, sh, ih;
	tx_packet_t * tx;
	int ack = true;


	/* Drop unwanted frames */
	if( m->program_id != API_PROG_ID ) {
		return;
	}

	packet_dump(p);
	
	/* Update busmail packet counters */
	tx_seq_r = (m->frame_header & TX_SEQ_MASK) >> TX_SEQ_OFFSET;
	rx_seq_r = (m->frame_header & RX_SEQ_MASK) >> RX_SEQ_OFFSET;

	pf = (m->frame_header & PF_MASK) >> PF_OFFSET;

	printf("frame_header: %02x\n", m->frame_header);
	printf("tx_seq_r: %d\n", tx_seq_r);
	printf("rx_seq_r: %d\n", rx_seq_r);
	printf("pf: %d\n", pf);

	rx_seq_l = tx_seq_r + 1;
	if (rx_seq_l == 7) {
		rx_seq_l = 0;
	}

	/* Process application frame. The application frame callback will enqueue 
	 outgoing packets on tx_fifo and directly transmit packages with busmail_send() */
	application_frame(m);

	/* Send packet to connected client */
	client_p.type = CLIENT_PKT_TYPE;
	client_p.size = CLIENT_PKT_HEADER_SIZE + p->size - 3;
	memcpy(&(client_p.data), &(p->data[3]), p->size - 3);
	util_dump(&p->data[3], p->size - 3, "[TO CLIENT]");
	list_each(client_list, send_to_client);

	/* Always ack with control frame */
	busmail_ack();
}


int busmail_get(packet_t *p, buffer_t *b) {
	
	int i, start, stop, size, read = 0;
	uint8_t crc = 0, crc_calc = 0;
	uint8_t buf[5000];

	/* start = buffer_find(b, BUSMAIL_PACKET_HEADER); */
	/* if (start < 0) { */
	/* 	return -1; */
	/* } */

	/* Do we have a start of frame? */
	while (buffer_read(b, buf, 1) > 0) {
		read++;
		if (buf[0] == BUSMAIL_PACKET_HEADER) {
			break;
		}
	}
	
	/* Return if we did not read any data */
	if (read == 0) {
		return -1;
	}

	/* Do we have a full header? */
	if (buffer_size(b) < 2) {
		buffer_rewind(b, 1);
		return -1;
	}
	buffer_read(b, buf + 1, 2);

	/* Packet size */
	size = (((uint32_t) buf[1] << 8) | buf[2]);
	
	/* Do we have a full packet? */
	/* Data + crc */
	if (1 + size > buffer_size(b)) {
		buffer_rewind(b, 3);
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

	printf("\n[PACKET %d] - ", p->size);
	for (i = 0; i < p->size; i++) {
		printf("%02x ", p->data[i]);
	}
	printf("\n");
}


void busmail_dispatch(packet_t *p) {

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
			supervisory_control_frame(p);
			break;
		}

	break;

	default:
		printf("Unknown packet header: %x\n", m->frame_header);

	}



}

int busmail_init(int fd, void (*app_handler)(busmail_t *)) {

	printf("busmail_init\n");
	busmail_fd = fd;
	application_frame = app_handler;


	tx_fifo = fifo_new();
}
