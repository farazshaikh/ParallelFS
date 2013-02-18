ADMIN_TGT = $(ADMIN_TGT_PATH)/mkfs.hfs
ADMIN_TGT2 = $(ADMIN_TGT_PATH)/updatehfsconfig
INCLUDES += -I ./admin/inc/
ADMIN_OBJFILES = $(patsubst %.c,%.o,$(wildcard admin/mkfs_hercules.c))
ADMIN_OBJFILES2 = $(patsubst %.c,%.o,$(wildcard admin/updateFSConfig.c))


$(ADMIN_TGT): $(ADMIN_OBJFILES)
	$(BUILD) -o $@ $^ $(LIBRARIES) -lmysqlclient

$(ADMIN_TGT2): $(ADMIN_OBJFILES2)
	$(BUILD) -o $@ $^ $(LIBRARIES) -lmysqlclient


ADMIN_CLEAN_TGT:
	rm -f admin/*.o
