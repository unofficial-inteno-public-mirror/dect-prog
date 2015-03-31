
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

#define SECTOR_ERASE_CMD 0x30

static struct bin_img flashloader;
static struct bin_img *pr = &flashloader;

enum fl_state {
	NEW_PACKET,
};

int fl_state;

uint8_t packetbuf[BUF_SIZE];

/* CommonFlashMemoryInterfaceType   MyCfi; */


 /* case SC14441_EXT_QSPI_FLASH_BMC: */
 /* { */
 /* 	 ReadQSPIFlashTypeCfmType  QSPIFlash; */
 /* 	 ReadQSPiFlashType(InstanceData, &QSPIFlash); */
 /* 	 Cfi->FlashID          = EXTERNAL_QSPI_FLASH_ID; */
 /* 	 Cfi->MinBlockSize     = QSPIFlash.SectorSize; */
 /* 	 Cfi->MemorySize       = DataOptions->Size; */
 /* 	 Cfi->FlashStartAddress= StartAddress; */
 /* 	 Cfi->ChipEraseCmd     = 0x10; */
 /* 	 Cfi->SectorEraseCmd   = 0x30; */
 /* 	 return(Err); */
 /* } */






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



static void qspi_flash_type_req(event_t *e) {

	ReadProprietaryDataReqType *r = malloc(sizeof(ReadProprietaryDataReqType));
  
	r->Primitive = READ_PROPRIETARY_DATA_REQ;
	r->RequestID = 0;

	send_packet(r, sizeof(ReadProprietaryDataReqType), e->fd);
	free(r);

}


/* /// Read Proprietary Data QSPI flashtype Confirm                                                                     */
/* typedef struct */
/* { */
/* 	PrimitiveType     Primitive;      /\*!< Primitive *\/ */
/* 	uint8             RequestID;      /\*!< Request ID *\/ */
/* 	uint8             MaxFreqSingle;  /\*!< MaxFreqSingle *\/ */
/* 	uint8             MaxFreqQuad;    /\*!< MaxFreqQuad   *\/ */
/* 	uint8             Manufacturer;   /\*!< Manufacturer *\/ */
/* 	uint16            DeviceId;       /\*!< DeviceId      *\/ */
/* 	uint32            TotalSize;      /\*!< TotalSize     *\/ */
/* 	uint32            SectorSize;     /\*!< SectorSize    *\/ */
/* 5 + 2 + 8 = 15
/* } ReadQSPIFlashTypeCfmType; */

typedef struct __attribute__((__packed__))
{
  PrimitiveType     Primitive;      /*!< Primitive */
  uint8_t           RequestID;      /*!< Request ID */
  uint8_t           MaxFreqSingle;  /*!< MaxFreqSingle */
  uint8_t           MaxFreqQuad;    /*!< MaxFreqQuad   */
  uint8_t           Manufacturer;   /*!< Manufacturer */
  uint16_t          DeviceId;       /*!< DeviceId      */
  uint32_t          TotalSize;      /*!< TotalSize     */
  uint32_t          SectorSize;     /*!< SectorSize    */
} flash_type_t;



static void qspi_flash_type_cfm(event_t *e) {

	flash_type_t flash;
	flash_type_t *f = &flash;  
	int i;


	memcpy(f, &(e->in[3]), sizeof(flash_type_t));

	printf("Primitive: %x\n", f->Primitive);
	printf("RequestId: %x\n", f->RequestID);
	printf("MaxFreqSingle: %x\n", f->MaxFreqSingle);
	printf("MaxFreqQuad: %x\n", f->MaxFreqQuad);
	printf("Manufacturer: %x\n", f->Manufacturer);
	printf("DeviceId: %x\n",  f->DeviceId);
	printf("TotalSize: %x\n", (int) f->TotalSize);
	printf("SectorSize: %x\n",(int) f->SectorSize);

}


/*
   ErrorCodeType Err;
   uint16     mailLength = sizeof(WriteConfigReqType);
   MyMailType *MailPtr   = (MyMailType *)Alloc(mailLength);
   MyMailType *RxMailPtr = NULL;

   MAIL_PTR->WriteConfigReq.Primitive     = WRITE_CONFIG_REQ;
   MAIL_PTR->WriteConfigReq.FirstProgWord = FirstProgWord;
   MAIL_PTR->WriteConfigReq.SecondProgWord= SecondProgWord;
   MAIL_PTR->WriteConfigReq.OffsetAddress = OffsetAddress;
   MAIL_PTR->WriteConfigReq.Config        = Config;

   Err=DeliverMail(InstanceData, MailPtr, mailLength, &RxMailPtr, QUICK_CONFIRM_TIMEOUT);
   if (Err==_NO_ERROR)
   {
       if ((RX_MAIL_PTR->WriteConfigCfm.Primitive==WRITE_CONFIG_CFM) &&
           (RX_MAIL_PTR->WriteConfigCfm.Confirm==TRUE))


        case SC14441_EXT_QSPI_FLASH_BMC:
             RETURN_ON_ERROR(ReadChip441ID(InstanceData));
             Err=WriteConfig(InstanceData, QSPI_FLASH_CONFIG, QSPI_XIP_OFFSET 0xF0000, 0, 0);

*/



static void config_target(event_t *e) {
	
	WriteConfigReqType *r = malloc(sizeof(WriteConfigReqType));
  
	r->Primitive = WRITE_CONFIG_REQ;
	r->FirstProgWord = 0;
	r->SecondProgWord = 0;
	r->OffsetAddress = 0xf0000;
	r->Config = QSPI_FLASH_CONFIG;

	send_packet(r, sizeof(WriteConfigReqType), e->fd);
	free(r);
}


static void write_config_cfm(event_t *e) {
	
	 WriteConfigCfmType *p = (WriteConfigCfmType *) &e->in[3];
	
	 if(p->Confirm == TRUE) {
		 printf("Confirm: TRUE\n");
	 }

}

static void read_firmware(void) {
	
	int fd;
	struct stat s;

	fd = open("target.bin", O_RDONLY);
	if (fd == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	fstat(fd, &s);
	pr->size = s.st_size;

	printf("Size: 0x%x\n", (int)pr->size);
	printf("Size: %d\n", (int)pr->size);
	
	
	
	close(fd);
	
}


typedef struct __attribute__((__packed__))
{
  PrimitiveType     Primitive;      /*!< Primitive */
  uint32_t          Address;        /*!< Memory address  */
  uint16_t           EraseCommand;   /*!< Erase command Chip/Sector/Block */    
} erase_flash_req_t;

typedef struct __attribute__((__packed__))
{
  PrimitiveType     Primitive;      /*!< Primitive */
  uint32_t            Address;        /*!< Memory address  */
  uint8_t          Confirm;        /*!< Confirm TRUE/FALSE */
} erase_flash_cfm_t;



static void erase_flash_req(event_t *e) {
	
	erase_flash_req_t *p = malloc(sizeof(erase_flash_req_t));
	
	p->Primitive = FLASH_ERASE_REQ;
	p->Address = 0;
	p->EraseCommand = SECTOR_ERASE_CMD;

	printf("Address: %x\n", (int) p->Address);
	printf("EraseCommand: %x\n", (int) p->EraseCommand);
	
	send_packet(p, sizeof(erase_flash_req_t), e->fd);
	free(p);
	
}


static void flash_erase_cfm(event_t *e) {
	
	erase_flash_cfm_t  *p = (erase_flash_cfm_t *) &e->in[3];
	  
	printf("Address: %x\n", (int) p->Address);
	if(p->Confirm == TRUE) {
		printf("Confirm: TRUE\n");
	} else {
		printf("Confirm: FALSE\n");
	}

}



void init_flashloader_state(int dect_fd) {
	
	printf("FLASHLOADER_STATE\n");
	
	usleep(300*1000);
	sw_version_req(dect_fd);
	read_firmware();
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
		printf("WRITE_CONFIG_REQ\n");
		config_target(e);
		break;
		
	case WRITE_CONFIG_CFM:
		printf("WRITE_CONFIG_CFM\n");
		write_config_cfm(e);
		printf("READ_PROPRIETARY_DATA_REQ\n");
		qspi_flash_type_req(e);
		break;

	case READ_PROPRIETARY_DATA_CFM:
		printf("READ_PROPRIETARY_DATA_CFM\n");
		qspi_flash_type_cfm(e);
		
		/* Erase flash */
		printf("FLASH_ERASE_REQ\n");
		erase_flash_req(e);
		break;

	case FLASH_ERASE_CFM:
		printf("FLASH_ERASE_CFM\n");
		flash_erase_cfm(e);
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
