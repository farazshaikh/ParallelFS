CLIENT_TGT = $(CLIENT_TGT_PATH)/hfs_client
INCLUDES += -I ./client/inc/
CLIENT_OBJFILES = $(patsubst %.c,%.o,$(wildcard client/*.c))
CLIENT_LIBRARIES = $(LIBRARIES)
CLIENT_LIBRARIES += -lfuse
CFLAGS += -DFUSE_USE_VERSION=26

$(CLIENT_TGT): $(CLIENT_OBJFILES)
	$(BUILD) -o $@ $^ $(CLIENT_LIBRARIES)

CLIENT_CLEAN_TGT:
	rm -f client/*.o
