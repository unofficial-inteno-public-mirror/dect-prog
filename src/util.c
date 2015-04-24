#include <stdio.h>
#include <unistd.h>

#include "util.h"
#include "dect.h"



void util_dump(unsigned char *buf, int size, char *start) {

	int i, dumpsize;
	int maxdump = 10000;
	//unsigned char* cdata = (unsigned char*)buf;

	printf("%s", start);
	printf("[%04d] - ",size);
    
	if ( size > maxdump ) {
		dumpsize = maxdump; 
	} else {
		dumpsize = size; 
	}

	for (i=0 ; i<dumpsize ; i++) {
		printf("%02x ",buf[i]);
	}
	printf("\n");
	
}


int check_args(int argc, char * argv[], config_t * c) {

	/* Check arguments */
	if (argc < 2) {
		err_exit("Usage: %s <prog | app>", argv[0]);
	}


	/* Select operating mode */
	if (strcmp("prog", argv[1]) == 0) {
		c->mode = PROG_MODE;
		return 0;
	} else if (strcmp("app", argv[1]) == 0) {
		c->mode = APP_MODE;
		return 0;
	} else {
		return -1;
	}
}
