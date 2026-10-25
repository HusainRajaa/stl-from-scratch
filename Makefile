SHELL := sh
.DEFAULT_GOAL := all

CXX ?= g++
CPPFLAGS ?=
CXXFLAGS ?= -O3 -DNDEBUG -Wall -Wextra -Wpedantic
LDFLAGS ?=
LDLIBS ?=
BUILD_DIR ?= build/release

TEST_SOURCES := $(wildcard tests/*.cpp)
TEST_NAMES := $(basename $(notdir $(TEST_SOURCES)))
TEST_BINS := $(addprefix $(BUILD_DIR)/,$(addsuffix .exe,$(TEST_NAMES)))
RUN_TARGETS := $(addprefix run-,$(TEST_NAMES))

.PHONY: all build test run debug clean $(RUN_TARGETS)

all: test

build: $(TEST_BINS)

test run: $(RUN_TARGETS)

$(RUN_TARGETS): run-%: $(BUILD_DIR)/%.exe
	@echo Running $<
	@./$<

$(BUILD_DIR)/%.exe: tests/%.cpp Makefile | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -std=c++20 -Iinclude -MMD -MP -MF $(@:.exe=.d) -MT $@ $< $(LDFLAGS) $(LDLIBS) -o $@

$(BUILD_DIR):
	mkdir -p $@

debug:
	$(MAKE) BUILD_DIR=build/debug CXXFLAGS="-O0 -g -D_GLIBCXX_DEBUG -Wall -Wextra -Wpedantic" test

clean:
	rm -rf build

-include $(TEST_BINS:.exe=.d)
