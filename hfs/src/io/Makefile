IO_TGT = $(IO_TGT_PATH)/libio.a
INCLUDES += -I ./io/inc/
IO_OBJFILES = $(patsubst %.c,%.o,$(wildcard io/*.c))

$(IO_TGT): $(IO_OBJFILES)
	$(STATIC_LIB) $@ $^

IO_CLEAN_TGT:
	rm -f io/*.o
