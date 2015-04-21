#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <Api/FpGeneral/ApiFpGeneral.h>
#include <Api/FpCc/ApiFpCc.h>
#include <Api/FpMm/ApiFpMm.h>
#include <RosPrimitiv.h>
#include <Api/RsStandard.h>

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

uint8_t tx_seq_l, rx_seq_l, tx_seq_r, rx_seq_r;

int busmail_fd;

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



static busmail_send(uint8_t * data, int size, uint8_t pf) {

	uint8_t tx_seq_tmp, rx_seq_tmp;
	busmail_t * r;	

	r = malloc(BUSMAIL_PACKET_OVER_HEAD - 1 + size);
	if (!r) {
		exit_failure("malloc");
	}

		
	r->frame_header = make_info_frame(pf);
	r->program_id = API_PROG_ID;
	r->task_id = API_TEST;
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


static busmail_ack(void) {

	uint8_t sh, rx_seq_tmp;

	sh = make_supervisory_frame(SUID_RR, NO_PF);
	rx_seq_tmp = (sh & RX_SEQ_MASK) >> RX_SEQ_OFFSET;
	
	printf("BUSMAIL_ACK %d\n", rx_seq_tmp);
	send_packet(&sh, 1, busmail_fd);

}

static void supervisory_control_frame(packet_t *p) {
	
	busmail_t * m = (busmail_t *) &p->data[0];
	uint8_t pf, suid;

	rx_seq_r = (m->frame_header & RX_SEQ_MASK) >> RX_SEQ_OFFSET;
	pf = (m->frame_header & PF_MASK) >> PF_OFFSET;
	suid = (m->frame_header & SUID_MASK) >> SUID_OFFSET;

	printf("frame_header: %02x\n", m->frame_header);

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


static void fw_version_cfm(busmail_t *m) {

	ApiFpGetFwVersionCfmType * p = (ApiFpGetFwVersionCfmType *) &m->mail_header;

	printf("fw_version_cfm\n");
	
	if (p->Status == RSS_SUCCESS) {
		printf("Status: RSS_SUCCESS\n");
	} else {
		printf("Status: RSS_FAIL: %x\n", p->Status);
	}

	printf("VersionHex %x\n", (uint)p->VersionHex);
	
	if (p->DectType == API_EU_DECT) {
		printf("DectType: API_EU_DECT\n");
	} else {
		printf("DectType: BOGUS\n");
	}

}




static void application_frame(busmail_t *m) {
	
	int i;

	switch (m->mail_header) {
		
	case API_FP_RESET_IND:
		printf("API_FP_RESET_IND\n");


		printf("API_FP_GET_FW_VERSION_REQ\n");
		ApiFpGetFwVersionReqType m1 = { .Primitive = API_FP_GET_FW_VERSION_REQ, };
		busmail_send((uint8_t *)&m1, sizeof(ApiFpGetFwVersionReqType), PF);


		/* /\* Start protocol *\/ */
		/* ApiFpMmStartProtocolReqType * r = malloc(sizeof(ApiFpMmStartProtocolReqType)); */
		/* r->Primitive = API_FP_MM_EXT_HIGHER_LAYER_CAP2_REQ; */

		/* printf("API_FP_MM_START_PROTOCOL_REQ\n"); */
		/* busmail_send((uint8_t *)r, sizeof(ApiFpMmStartProtocolReqType)); */
		/* free(r); */

		/* ApiFpCcFeaturesReqType * r = (ApiFpCcFeaturesReqType*) malloc(sizeof(ApiFpCcFeaturesReqType)); */

		/* r->Primitive = API_FP_CC_FEATURES_REQ; */
		/* r->ApiFpCcFeature = API_FP_CC_EXTENDED_TERMINAL_ID_SUPPORT; */

		/* printf("API_FP_CC_FEATURES_REQ\n"); */
		/* busmail_send((uint8_t *)r, sizeof(ApiFpCcFeaturesReqType)); */
		/* free(r); */
		
		

		break;


	case API_FP_GET_FW_VERSION_CFM:
		printf("API_FP_GET_FW_VERSION_CFM\n");
		fw_version_cfm(m);
		/* just ack the package */
		busmail_ack();

		break;


	case API_SCL_STATUS_IND:
		printf("API_SCL_STATUS_IND\n");
		/* just ack the package */
		busmail_ack();

		break;


	case API_FP_CC_FEATURES_CFM:
		printf("API_FP_CC_FEATURES_CFM\n");


		/* Start protocol */
		ApiFpMmStartProtocolReqType * r1 = malloc(sizeof(ApiFpMmStartProtocolReqType));
		r1->Primitive = API_FP_MM_START_PROTOCOL_REQ;

		printf("API_FP_MM_START_PROTOCOL_REQ\n");
		busmail_send((uint8_t *)r1, sizeof(ApiFpMmStartProtocolReqType), NO_PF);
		free(r1);


		/* Start registration */
		ApiFpMmSetRegistrationModeReqType r2 = { .Primitive = API_FP_MM_SET_REGISTRATION_MODE_REQ, \
							.RegistrationEnabled = true, .DeleteLastHandset = false};

		printf("API_FP_MM_SET_REGISTRATION_MODE_REQ\n");
		busmail_send((uint8_t *)&r2, sizeof(ApiFpMmStartProtocolReqType), PF);


	case API_FP_MM_SET_REGISTRATION_MODE_CFM:
		printf("API_FP_MM_SET_REGISTRATION_MODE_CFM\n");
		/* just ack the package */
		busmail_ack();

		break;

	}
}



static void information_frame(packet_t *p) {

	busmail_t * m = (busmail_t *) &p->data[0];

	uint8_t pf, sh, ih;
	

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

	/* Process application frame */
	application_frame(m);
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

int busmail_init(int fd) {

	printf("busmail_init\n");
	busmail_fd = fd;
}
