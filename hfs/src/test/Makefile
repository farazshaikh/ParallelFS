TEST_TGT = $(TEST_TGT_PATH)/hercules_test
INCLUDES += -I ./test/inc/
TEST_OBJFILES = $(patsubst %.c,%.o,$(wildcard test/*.c))

$(TEST_TGT): $(TEST_OBJFILES)
	$(BUILD) -o $@ $^ $(LIBRARIES)

TEST_CLEAN_TGT:
	rm -f test/*.o
