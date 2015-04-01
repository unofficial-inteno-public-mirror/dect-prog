#include <stdio.h>
#include <unistd.h>

#include "util.h"



void util_dump(unsigned char *buf, int size, char *start) {

	int i, dumpsize;
	int maxdump = 10000;
	unsigned char* cdata = (unsigned char*)buf;

	printf("%s", start);
	printf("[%04d] - ",size);
    
	if ( size > maxdump ) {
		dumpsize = maxdump; 
	} else {
		dumpsize = size; 
	}

	for (i=0 ; i<dumpsize ; i++) {
		printf("%02x ",cdata[i]);
	}
	printf("\n");
	
}
