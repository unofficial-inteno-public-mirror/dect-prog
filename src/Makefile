

ifeq ($(NATALIE_DIR),)
INCLUDE_PATHS += -I$(STAGING_DIR)/usr/include/natalie-dect/ \
		-I$(STAGING_DIR)/usr/include/natalie-dect/Phoenix

else
INCLUDE_PATHS += -I$(NATALIE_DIR)/SrcHeaders \
	-I$(NATALIE_DIR)/SrcHeaders/Phoenix
endif

%.o: %.c
	$(CC) -c $(CFLAGS) $(INCLUDE_PATHS) -o $@ $<




dect-prog: boot.o tty.o error.o main.o state.o util.o preloader.o flashloader.o Crc.o app.o buffer.o busmail.o nvs.o fifo.o
	$(CC) $(INCLUDE_PATHS) -o $@ $^


clean:
	rm -rf *.o
	rm -rf dect-prog



