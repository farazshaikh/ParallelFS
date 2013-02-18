DB_TGT = $(DB_TGT_PATH)/libdb.a
INCLUDES += -I ./db/inc/
DB_OBJFILES = $(patsubst %.c,%.o,$(wildcard db/*.c))

$(DB_TGT): $(DB_OBJFILES)
	$(STATIC_LIB) $@ $^

DB_CLEAN_TGT:
	rm -f db/*.o
