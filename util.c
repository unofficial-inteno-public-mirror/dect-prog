#include <stdio.h>
#include <unistd.h>

#include "util.h"

void util_write(void *data, int size, int fd) {

	util_dump(data, size, "[WRITE]");
	write(fd, data, size);

	return;
}


void util_dump(unsigned char *buf, int size, char *start) {

	int i, dumpsize;
	int maxdump = 100;
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
