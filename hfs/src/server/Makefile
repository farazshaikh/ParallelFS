SRVR_TGT = $(SRVR_TGT_PATH)/hfs_server
INCLUDES += -I ./server/inc/
SRVR_OBJFILES = $(patsubst %.c,%.o,$(wildcard server/*.c))

$(SRVR_TGT): $(SRVR_OBJFILES)
	$(BUILD) -o $@ $^ $(LIBRARIES) -lmysqlclient

SRVR_CLEAN_TGT:
	rm -f server/*.o
