#ifndef DECT_H
#define DECT_H

#define MAX_LINE 16384
#define MAX_MAIL_SIZE 4098
#define MAX_LISTENERS 10
#define PKT_DATA_SIZE 100
#define MAX_NR_HANDSETS 20
#define MAX_NR_PHONES 4

#define MAX(a,b) ((a) > (b) ? (a) : (b))

#ifndef RSOFFSETOF
  /*! \def RSOFFSETOF(type, field)
  * Computes the byte offset of \a field from the beginning of \a type. */
  #define RSOFFSETOF(type, field) ((size_t)(&((type*)0)->field))
#endif

#define DEFAULT_HOTPLUG_PATH    "/sbin/hotplug-call"




struct info {
	const char *name;
	struct packet *pkt;
};


typedef struct dect_state {
	int reg_state;
} dect_state;

enum {
	PROG_MODE,
	APP_MODE,
	NVS_MODE,
};


enum reg_state {
	DISABLED,
	ENABLED,
};


enum dect_action {
	DECT_INIT,
	REG_START,
	REG_STOP,
};


enum packet_type {
	GET_STATUS,
	REGISTRATION,
	PING_HSET,
	DELETE_HSET,
	RESPONSE,
	DECT_PACKET,
	CLIENT_PACKET,
	LIST_HANDSETS,
	ULE_START,
	INIT,
	ZWITCH,
	RADIO,
	RELOAD_CONFIG,
};


enum packet_resp_type {
	OK,
	ERROR,
	DATA,
};





enum boot_type {
  SOH = 0x01,
  STX = 0x02,
  ETX = 0x03,
  ACK = 0x06,
  NACK = 0x15,
};





#define PACKET_HEADER \
	uint32_t size; \
	uint32_t type;
	

typedef struct packet {
	PACKET_HEADER
	uint8_t data[PKT_DATA_SIZE];
} packet_t;


typedef struct client_packet {
	PACKET_HEADER
	uint8_t data;
} client_packet;


struct dect_packet {
	PACKET_HEADER
	uint8_t data[MAX_MAIL_SIZE];
};


struct hset {
	uint8_t registered;
	uint8_t present;
	uint8_t pinging;
	uint8_t ipui[5];
};


struct status_packet {
	PACKET_HEADER
	uint8_t dect_init;
	uint8_t reg_mode;
	uint8_t radio;
	struct hset handset[MAX_NR_HANDSETS];
};

struct config {
	uint8_t radio;
};



typedef struct packet_header {
	PACKET_HEADER
} packet_header_t;


#endif /* DECT_H */
