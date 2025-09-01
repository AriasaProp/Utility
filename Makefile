BUILD_DIR := build
MAIN_DIR := main
TEST_DIR := test

# flags
MAKEFLAGS += -j4
FLAG_SRC := -Werror -Wall -MMD -MP
CFLAGS := -std=c11
CXXFLAGS := -std=c++20
LDFLAGS := -lc

# sources
MAIN_SRCS := $(shell find $(MAIN_DIR) -type f \( -name "*.c" -o -name "*.cpp" \))
TEST_SRCS := $(shell find $(TEST_DIR) -type f \( -name "*.c" -o -name "*.cpp" \))
MAIN_OBJS := $(MAIN_SRCS:%=$(BUILD_DIR)/%.o)
TEST_OBJS := $(TEST_SRCS:%=$(BUILD_DIR)/%.o)
MAIN_DEPS := $(MAIN_OBJS:.o=.d)
TEST_DEPS := $(TEST_OBJS:.o=.d)

all:
	@echo "nothing"
	@echo $(TEST_SRCS) # $(MAIN_SRCS)

# executables
Test: FLAG_SRC += -O0 -g $(addprefix -D,$(ARGS)) $(addprefix -I./,$(MAIN_DIR) $(TEST_DIR))
Test: $(TEST_OBJS) $(MAIN_OBJS)
	@echo "build test ... "
	@$(CXX) $^ -o $@ $(LDFLAGS)
	@chmod +x Test
	@./Test

# generate cpp to o file
$(BUILD_DIR)/%.cpp.o: %.cpp
	@mkdir -p $(@D)
	@$(CXX) -c $< -o $@ $(CXXFLAGS) $(FLAG_SRC)

# generate c to o file
$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(@D)
	@$(CC) -c $< -o $@ $(CFLAGS) $(FLAG_SRC)

clean:
	@echo "cleanup build files ..."
	@rm -rf $(BUILD_DIR) Test


-include $(TEST_DEPS) $(MAIN_DEPS)