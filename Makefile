include .env

# base c flags
CFLAGS	 := -std=c11 -Werror -Wall -MMD -MP -I./main 
# optimize c flags (no use)
OFLAGS   := -O3
# test c flags
TFLAGS	 := -O0 -ggdb -I./test
# linker flags
LDFLAGS  := -lc -lm
# OS detection
ifeq ($(OS),Windows_NT)
	$(error Unsupported Windows)
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		UNAME_M  := $(shell uname -m)
		ASMFLAGS := --warn --fatal-warnings
		ifeq ($(UNAME_M),aarch64)
			ARCH     := arm64
			ASMFLAGS += -march=armv8-a
			CFLAGS   += -DASM
		else ifeq ($(UNAME_M),armv7l)
			ARCH     := arm
			ASMFLAGS += -march=armv7-a
			CFLAGS   += -DASM -mfloat-abi=hard -mfpu=neon
		else ifeq ($(UNAME_M),x86_64)
			ARCH     := x64
			ASMFLAGS += -march=x86-64
			CFLAGS   += -DASM
		else ifneq (,$(filter i386 i486 i586 i686,$(UNAME_M)))
			ARCH     := x86
			ASMFLAGS += -march=i386 -m32
			CFLAGS   += -DASM -m32
		else
			$(error Unknown Machine: $(UNAME_M))
		endif
	else ifeq ($(UNAME_S),Darwin)
		$(error Unsupported MacOS)
	else
		$(error Unknown OS: $(UNAME_S))
	endif
endif

CMN_FILES := $(OBJ_DIR)/main/common.c.o
ifdef ARCH
CMN_FILES += $(OBJ_DIR)/$(ARCH)/common.s.o
endif


ALL_TESTS := bigInteger_test complex_test matrix_test sort_test hash_test
TEST_SRC := $(OBJ_DIR)/test/util/profiling.c.o
# information
define INF =
	@if !($(1)); then \
		echo "\033[31m$(2)\033[0m"; \
		exit 1; \
	fi
endef
# program linker template
define PROG =
$(1): CFLAGS += $(2)
$(1): $(CMN_FILES) $(3)
	@mkdir -p $$(@D)
	$$(call INF, $(CC) $$(filter %.o, $$^) -o $$@ $(LDFLAGS), build $$@ failed)
	@chmod +x $$@
	$$(call INF, ./$$@, running $$@ failed)
	@echo "running $$@ \033[32msuccess!\033[0m"
endef

.PHONY: t c h s
.PHONY: tests clean help status

help: h
h:
	@printf "\n"\
	  " COMMAND          Description\n" \
		"h[elp]      show this message.\n" \
		"t[ests]     make and run all test.\n" \
		"c[lean]     clean generated binary files/folders.\n" \
		"s[tatus]    show current device stat.\n" \
 "$(TEST_DIR)/*  run spesific test, that is:\n" \
 $(addprefix "\t\t",$(addsuffix "\n", $(ALL_TESTS))) \
		"" >&2


status: s
s:
ifdef ARCH
	@echo "Use Assembly: $(ARCH)"
endif
	@for t in $(ALL_TESTS); do \
		if [ -e "$(TEST_DIR)/$$t" ]; then \
			echo "test of $$t created!"; \
		fi \
	done


$(TEST_DIR)/qtest: LDFLAGS += -lm
$(eval $(call PROG,$(TEST_DIR)/qtest, $(TFLAGS), \
	$(TEST_SRC) \
	$(OBJ_DIR)/test/qtest.c.o \
))

tests: t
t: $(addprefix $(TEST_DIR)/, $(ALL_TESTS))
	@echo "All Tests Created!"

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
	$(call INF, $(CC) -c $< -o $@ $(CFLAGS), build $@ failed)

ifdef ARCH
# generate asm to o file
$(OBJ_DIR)/%.s.o: %.s
	@mkdir -p $(@D)
	$(call INF, as $(ASMFLAGS) -o $@ $<, build $@ failed)
endif

clean: c
c:
	@echo "cleanup binary files ..."
	@rm -rf $(BIN_DIR)

-include $(shell find $(OBJ_DIR) -type f -name "*.d") \
