#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>


#include <Api/FpGeneral/ApiFpGeneral.h>
#include <Api/FpCc/ApiFpCc.h>
#include <Api/FpMm/ApiFpMm.h>
#include <Api/ProdTest/ApiProdTest.h>
#include <Api/ProdTest/ApiProdTest.h>
#include <RosPrimitiv.h>
#include <Api/RsStandard.h>
#include <termios.h>


#include "tty.h"
#include "error.h"
#include "state.h"
#include "boot.h"
#include "util.h"
#include "app.h"
#include "buffer.h"
#include "busmail.h"


#define INBUF_SIZE 5000

buffer_t * buf;
int reset_ind = 0;



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
		

		if (reset_ind == 0) {
			reset_ind = 1;

			printf("\nWRITE: API_FP_GET_FW_VERSION_REQ\n");
			ApiFpGetFwVersionReqType m1 = { .Primitive = API_FP_GET_FW_VERSION_REQ, };
			busmail_send((uint8_t *)&m1, sizeof(ApiFpGetFwVersionReqType), PF);

		} else {
			busmail_ack();
		}

		break;

	case API_PROD_TEST_CFM:
		printf("API_PROD_TEST_CFM\n");
		busmail_ack();
		break;

	case RTX_EAP_HW_TEST_CFM:
		printf("RTX_EAP_HW_TEST_CFM\n");
		busmail_ack();
		break;

	case API_FP_GET_FW_VERSION_CFM:
		printf("API_FP_GET_FW_VERSION_CFM\n");
		fw_version_cfm(m);

		/* printf("\nWRITE: NvsDefault\n"); */
		/* uint8_t data[] = {0x66, 0xf0, 0x00, 0x00, 0x02, 0x01, 0x01, 0x00, 0x01}; */
		/* busmail_send0(data, sizeof(data), PF); */


		/* printf("\nWRITE: GetId\n"); */
		/* uint8_t data[] = {0x66, 0xf0, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00}; */
		/* busmail_send0(data, sizeof(data), PF); */

		//	       #define PT_CMD_SET_ID 0x001B
		/* printf("\nWRITE: SetId\n"); */
		/* uint8_t data[] = {0x66, 0xf0, 0x00, 0x00, 0x1b, 0x00, 0x05, 0x00, 0x02, 0x3f, 0x80, 0x00, 0xf8}; */
		/* busmail_send0(data, sizeof(data), PF); */
		
		/* printf("Get NVS\n"); */
		/* uint8_t data[] = {0x66, 0xf0, 0x00, 0x00, 0x01, 0x01, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff}; */
		/* busmail_send0(data, sizeof(data), PF); */

		/* #define PT_CMD_SET_NVS 0x0100 */
		/* printf("Set NVS\n"); */
		/* uint8_t data[] = {0x66, 0xf0, 0x00, 0x00, 0x00, 0x01, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x02, 0x3f, 0x80, 0x00, 0xf8, 0x25, 0xc0, 0x01, 0x00, 0xf8, 0x23}; */
		/* busmail_send0(data, sizeof(data), PF); */



		/* #define PT_CMD_GET_BAND_GAP 0x0300 */
		/* printf("\nWRITE: GetBandGap\n"); */
		/* uint8_t data[] = {0x66, 0xf0, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00 }; */
		/* busmail_send0(data, sizeof(data), PF); */


		/* Start protocol */
		printf("\nWRITE: API_FP_MM_START_PROTOCOL_REQ\n");
		ApiFpMmStartProtocolReqType * r = malloc(sizeof(ApiFpMmStartProtocolReqType));
		r->Primitive = API_FP_MM_START_PROTOCOL_REQ;
		busmail_send((uint8_t *)r, sizeof(ApiFpMmStartProtocolReqType), PF);
		free(r);
		

		/* Start registration */
		printf("\nWRITE: API_FP_MM_SET_REGISTRATION_MODE_REQ\n");
		ApiFpMmSetRegistrationModeReqType r2 = { .Primitive = API_FP_MM_SET_REGISTRATION_MODE_REQ, \
							.RegistrationEnabled = true, .DeleteLastHandset = false};
		busmail_send((uint8_t *)&r2, sizeof(ApiFpMmStartProtocolReqType), PF);

		break;


	case API_SCL_STATUS_IND:
		printf("API_SCL_STATUS_IND\n");
		/* just ack the package */
		busmail_ack();
		break;


	case API_FP_MM_SET_REGISTRATION_MODE_CFM:
		printf("API_FP_MM_SET_REGISTRATION_MODE_CFM\n");
		/* just ack the package */
		busmail_ack();
		break;
	}
}





void init_app_state(int dect_fd) {
	
	printf("APP_STATE\n");

	tty_set_raw(dect_fd);
	tty_set_baud(dect_fd, B115200);

	printf("RESET_DECT\n");
	system("/usr/bin/dect-reset > /dev/null");

	/* Init input buffer */
	buf = buffer_new(500);
	
	/* Init busmail subsystem */
	busmail_init(dect_fd, application_frame);
	
}


void handle_app_package(event_t *e) {

	uint8_t header;
	packet_t packet;
	packet_t *p = &packet;
	p->fd = e->fd;
	p->size = 0;

	//util_dump(e->in, e->incount, "\n[READ]");

	/* Add input to buffer */
	if (buffer_write(buf, e->in, e->incount) == 0) {
		printf("buffer full\n");
	}
	
	/* Process whole packets in buffer */
	while(busmail_get(p, buf) == 0) {
		busmail_dispatch(p);
	}
}





struct state_handler app_handler = {
	.state = APP_STATE,
	.init_state = init_app_state,
	.event_handler = handle_app_package,
};

struct state_handler * app_state = &app_handler;
