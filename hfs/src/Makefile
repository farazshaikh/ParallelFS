ECHO = @echo
CC = gcc
CFLAGS = -Wall -g -D_FILE_OFFSET_BITS=64 -D_XOPEN_SOURCE=500
INCLUDES = -I ./common/inc/ -I /usr/local/mysql/include/mysql/ -I /usr/include/mysql -I /usr/local/include/mysql
LIB_PATH = ../lib
BIN_PATH = ../bin
TEST_PATH = ./test
MYSQL_LIBPATH = -L /usr/local/lib/mysql -L /usr/local/mysql/lib/mysql -L /usr/lib/mysql
LIBRARIES     = $(subst .a, ,$(subst ../lib/lib,-l,$(wildcard ../lib/*.a)))
LIBRARIES     += -lpthread -lm -lcrypt

COMPILE = $(ECHO) [COMPILE] $@;$(CC) $(CFLAGS) -c
BUILD =   $(ECHO) [BUILD   ] $@;$(CC) $(CFLAGS) $(MYSQL_LIBPATH) -L$(LIB_PATH)

ARCHIVER =$(ECHO) [ARCHIVE] $@; ar
AFLAGS = rcs
STATIC_LIB = $(ARCHIVER) $(AFLAGS)

all: TARGETS

LOG_TGT_PATH = $(LIB_PATH)
include logger/Makefile

UTIL_TGT_PATH = $(LIB_PATH)
include utils/Makefile

DB_TGT_PATH = $(LIB_PATH)
include db/Makefile

IO_TGT_PATH = $(LIB_PATH)
include io/Makefile

PROTO_TGT_PATH = $(LIB_PATH)
include proto/Makefile

ADMIN_TGT_PATH = $(BIN_PATH)
include admin/Makefile

CLIENT_TGT_PATH = ../bin
include client/Makefile

SRVR_TGT_PATH = $(BIN_PATH)
include server/Makefile

TEST_TGT_PATH = $(TEST_PATH)
include test/Makefile

ALL_TGTS = $(LOG_TGT) $(UTIL_TGT) $(DB_TGT) $(IO_TGT) $(PROTO_TGT) $(ADMIN_TGT) $(ADMIN_TGT2) $(SRVR_TGT) $(TEST_TGT) $(CLIENT_TGT)

CLEAN_TGTS = LOG_CLEAN_TGT UTIL_CLEAN_TGT DB_CLEAN_TGT IO_CLEAN_TGT PROTO_CLEAN_TGT ADMIN_CLEAN_TGT SRVR_CLEAN_TGT TEST_CLEAN_TGT CLIENT_CLEAN_TGT

TARGETS: $(ALL_TGTS)

client: $(CLIENT_TGT)

%.o: %.c
	$(COMPILE) $(INCLUDES) -o $@ $^

clean: $(CLEAN_TGTS)
	rm -f $(LOG_TGT)
	rm -f $(UTIL_TGT)
	rm -f $(DB_TGT)
	rm -f $(IO_TGT)
	rm -f $(PROTO_TGT)
	rm -f $(ADMIN_TGT)
	rm -f $(CLIENT_TGT)
	rm -f $(SRVR_TGT)
	rm -f $(TEST_TGT)
	rm -f $(CLIENT_TGT)


ETAGS=etags	# Exuberant ctags

tags:
	find . -name '*.c' -o -name '*.h' \
| xargs $(ETAGS) -o ETAGS  --append --declarations --globals -I --language=auto --members
