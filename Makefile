STAGING_DIR=/home/jonash/projects/inteno/iopsys-dg301/staging_dir/target-mips_uClibc-0.9.33.2

INCLUDE_PATHS = \
		-I$(STAGING_DIR)/usr/include/natalie-dect/ \
		-I$(STAGING_DIR)/usr/include/natalie-dect/Phoenix  \


all:
	gcc $(INCLUDE_PATHS) -o prog boot.c tty.c error.c main.c state.c util.c preloader.c flashloader.c Crc.c prog.c buffer.c packet.c





