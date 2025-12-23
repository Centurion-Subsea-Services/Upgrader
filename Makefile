CXX = g++
CXXFLAGS = -std=c++17 -Wall -pthread \
           $(shell pkg-config --cflags redis++ hiredis)
LDFLAGS  = $(shell pkg-config --libs redis++ hiredis)


TARGET_EXEC ?= upgrader

BUILD_DIR ?= build
SRC_DIRS ?= src
COMMIT := $(shell git describe --always --dirty)

SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CFLAGS   ?= $(INC_FLAGS) -g -Wall -DCOMMIT=$(COMMIT)
CXXFLAGS ?= $(INC_FLAGS) -g -Wfatal-errors -DCOMMIT=$(COMMIT)

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p