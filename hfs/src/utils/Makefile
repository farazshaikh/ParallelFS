UTIL_TGT = $(UTIL_TGT_PATH)/libutils.a
INCLUDES += -I ./utils/inc/
UTIL_OBJFILES = $(patsubst %.c,%.o,$(wildcard utils/*.c))

$(UTIL_TGT): $(UTIL_OBJFILES)
	$(STATIC_LIB) $@ $^

UTIL_CLEAN_TGT:
	rm -f utils/*.o
