
INCLUDE_PATHS += -I$(STAGING_DIR)/usr/include/natalie-dect/ \
		-I$(STAGING_DIR)/usr/include/natalie-dect/Phoenix


%.o: %.c
	$(CC) -c $(CFLAGS) $(INCLUDE_PATHS) -o $@ $<




dect-prog: boot.o tty.o error.o main.o state.o util.o preloader.o flashloader.o Crc.o app.o buffer.o busmail.o nvs.o
	$(GCC) $(INCLUDE_PATHS) -o $@ $^





