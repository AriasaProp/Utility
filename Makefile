BIN_DIR := bin
OBJ_DIR := $(BIN_DIR)/obj
TOOL_DIR := $(BIN_DIR)/tool
DATA_DIR := $(BIN_DIR)/data
TEST_DIR := $(BIN_DIR)/test

# flags
CFLAGS	 := -std=c11 -Werror -Wall -MMD -MP -I./main 
OFLAGS   := -O3
TFLAGS	 := -O0 -ggdb -I./test
LDFLAGS  := -lc -lm

ALL_TESTS := bigInteger_test complex_test matrix_test sort_test hash_test

TEST_SRC := $(OBJ_DIR)/test/util/profiling.c.o

# program create c template
define PROG =
$(1): CFLAGS += $(2)
$(1): $(OBJ_DIR)/main/common.c.o $(3)
	@echo "build $$@ "
	@mkdir -p $$(@D)
	@$(CC) $$(filter %.o, $$^) -o $$@ $$(LDFLAGS)
	@chmod +x $$@
	@./$$@
endef

.PHONY: tests clean h

h:
	@echo "\n"\
		"h[elp]      show this message.\n" \
 "$(TEST_DIR)/%  run spesific test.\n" \
 "that is : $(ALL_TESTS).\n" \
		"tests       make and run all test.\n" \
		"clean       clean generated binary files/folders.\n" \
		""

$(TEST_DIR)/qtest: LDFLAGS += -lm
$(eval $(call PROG,$(TEST_DIR)/qtest, $(TFLAGS), \
	$(TEST_SRC) \
	$(OBJ_DIR)/test/qtest.c.o \
))


tests: $(addprefix $(TEST_DIR)/, $(ALL_TESTS))
	@echo "All Tests Passed"

$(eval $(call PROG,$(TEST_DIR)/complex_test, $(TFLAGS), \
	$(TEST_SRC) \
	$(OBJ_DIR)/main/math/complex.c.o \
	$(OBJ_DIR)/test/math/complex_test.c.o \
))
$(eval $(call PROG,$(TEST_DIR)/bigInteger_test, $(TFLAGS), \
	$(TEST_SRC) \
	$(OBJ_DIR)/main/math/bigInteger.c.o \
	$(OBJ_DIR)/test/math/bigInteger_test.c.o \
))
$(eval $(call PROG,$(TEST_DIR)/matrix_test, $(TFLAGS), \
	$(TEST_SRC) \
	$(OBJ_DIR)/main/math/matrix.c.o \
	$(OBJ_DIR)/test/math/matrix_test.c.o \
))
$(eval $(call PROG,$(TEST_DIR)/sort_test, $(TFLAGS), \
	$(TEST_SRC) \
	$(OBJ_DIR)/main/algorithm/sort.c.o \
	$(OBJ_DIR)/test/algorithm/sort_test.c.o \
))
$(eval $(call PROG,$(TEST_DIR)/hash_test, $(TFLAGS), \
	$(TEST_SRC) \
	$(OBJ_DIR)/main/codec/hash.c.o \
	$(OBJ_DIR)/test/codec/hash_test.c.o \
))

# generate c to o file
$(OBJ_DIR)/%.c.o: %.c
	@mkdir -p $(@D)
	@$(CC) -c $< -o $@ $(CFLAGS)

clean:
	@echo "cleanup binary files ..."
	@rm -rf $(BIN_DIR)

-include $(shell find $(OBJ_DIR) -type f -name "*.d")
