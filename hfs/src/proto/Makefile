PROTO_TGT = $(PROTO_TGT_PATH)/libproto.a
INCLUDES += -I ./proto/inc/
PROTO_OBJFILES = $(patsubst %.c,%.o,$(wildcard proto/*.c))

$(PROTO_TGT): $(PROTO_OBJFILES)
	$(STATIC_LIB) $@ $^

PROTO_CLEAN_TGT:
	rm -f proto/*.o
