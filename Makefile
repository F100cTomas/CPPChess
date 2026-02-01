# Options: debug and release
BUILD ?= debug
ifeq ($(filter $(BUILD),debug release),)
$(error Unsupported BUILD option: $(BUILD))
endif
# Main options: clang++ and g++
# If other compiler is supplied it is no problem
COMPILER ?= clang++
CPP := $(COMPILER)
# Arguments passed to the compiler
CPPFLAGS_BASE := -Isrc
ifeq ($(BUILD),debug)
CPPFLAGS := -Wall -g3 -O0 -DDEBUG -fno-omit-frame-pointer $(CPPFLAGS_BASE)
else ifeq ($(BUILD),release)
CPPFLAGS := -Wall -O3 -DNDEBUG $(CPPFLAGS_BASE)
endif
# Arguments passed to the linker
LDFLAGS :=
# Files to be compiled
HPP := $(shell find src -name "*.hpp")
SRC := $(shell find src -name "*.cpp")
OBJ := $(SRC:src/%.cpp=.build/%.o)

# Compilation rules

all: chess

clean:
	$(RM) -r .build/*

chess: $(OBJ)
	$(CPP) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)
	@chmod +x $@

.build/%.o: src/%.cpp $(HPP)
	@mkdir -p $(dir $@)
	$(CPP) $(CPPFLAGS) -c $< -o $@

.PHONY: all clean
