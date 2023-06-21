# This Makefile is a modified version of the one found on
# https://makefiletutorial.com.

MAIN_BINARY := albatross
TEST_BINARY := run_tests
CXX         := g++

TEST_DIR    := ./tests
BUILD_DIR   := ./build
SRC_DIR     := ./src

MAIN_SRC    := $(shell find $(SRC_DIR) -name '*.cpp')
TESTS_SRC   := $(shell find $(TEST_DIR) -name '*.cpp')

# String substitution for every C/C++ file.
# As an example, hello.cpp turns into ./build/hello.cpp.o
MAIN_OBJS   := $(MAIN_SRC:%=$(BUILD_DIR)/%.o)

# Exclude the cpp file with main() so that Catch2 can provide its own.
# This also assumes that the file is named after the binary that is eventually
# produced.
TEST_OBJS   := $(filter-out $(BUILD_DIR)/$(SRC_DIR)/$(MAIN_BINARY).cpp.o, $(MAIN_OBJS))
TEST_OBJS   := $(TEST_OBJS) $(TESTS_SRC:%=$(BUILD_DIR)/%.o)

# String substitution (suffix version without %).
# As an example, ./build/hello.cpp.o turns into ./build/hello.cpp.d
DEPS := $(MAIN_OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIR) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# The -MMD and -MP flags together generate Makefiles for us!
# These files will have .d instead of .o as the output.
CPPFLAGS := $(INC_FLAGS) -MMD -Wall -Wextra -Werror -MP -std=c++17 -g

all: $(BUILD_DIR)/$(MAIN_BINARY)

# Build the main binary.
$(BUILD_DIR)/$(MAIN_BINARY): $(MAIN_OBJS)
	mkdir -p $(dir $@)
	$(CXX) $(MAIN_OBJS) -o $@ $(LDFLAGS)

# Run the test binary, then delete it.
test: $(BUILD_DIR)/$(TEST_BINARY)
	$(BUILD_DIR)/$(TEST_BINARY)
	rm $(BUILD_DIR)/$(TEST_BINARY)

$(BUILD_DIR)/$(TEST_BINARY): $(TEST_OBJS) $(MAIN_OBJS)
	$(CXX) $(TEST_OBJS) -o $@ $(LDFLAGS)

# Build step for C++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -r $(BUILD_DIR)

# Include the .d makefiles. The - at the front suppresses the errors of missing
# Makefiles. Initially, all the .d files will be missing, and we don't want those
# errors to show up.
-include $(DEPS)
