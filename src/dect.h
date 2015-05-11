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



#endif /* DECT_H */
