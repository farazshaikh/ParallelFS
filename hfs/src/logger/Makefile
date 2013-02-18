LOG_TGT = $(LOG_TGT_PATH)/liblogger.a
INCLUDES += -I ./logger/inc/
LOG_OBJFILES = $(patsubst %.c,%.o,$(wildcard logger/*.c))

$(LOG_TGT): $(LOG_OBJFILES)
	$(STATIC_LIB) $@ $^

LOG_CLEAN_TGT:
	rm -f logger/*.o
