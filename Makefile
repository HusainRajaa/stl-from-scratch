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
DEMO_SOURCES := $(wildcard src/demo_*.cpp)
DEMO_NAMES := $(basename $(notdir $(DEMO_SOURCES)))
DEMO_BINS := $(addprefix $(BUILD_DIR)/,$(addsuffix .exe,$(DEMO_NAMES)))
TEST_RUN_TARGETS := $(addprefix run-,$(TEST_NAMES))
DEMO_RUN_TARGETS := $(addprefix run-,$(DEMO_NAMES))
RUN_TARGETS := $(TEST_RUN_TARGETS) $(DEMO_RUN_TARGETS)

.PHONY: all build test run cpp debug clean $(RUN_TARGETS)

all: test run

build: $(TEST_BINS) $(DEMO_BINS)

test: $(TEST_RUN_TARGETS)

cpp: run

run: $(DEMO_RUN_TARGETS)

$(RUN_TARGETS): run-%: $(BUILD_DIR)/%.exe
	@echo Running $<
	@./$<

$(TEST_BINS): $(BUILD_DIR)/%.exe: tests/%.cpp Makefile | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -std=c++20 -Iinclude -MMD -MP -MF $(@:.exe=.d) -MT $@ $< $(LDFLAGS) $(LDLIBS) -o $@

$(DEMO_BINS): $(BUILD_DIR)/%.exe: src/%.cpp Makefile | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -std=c++20 -Iinclude -MMD -MP -MF $(@:.exe=.d) -MT $@ $< $(LDFLAGS) $(LDLIBS) -o $@

$(BUILD_DIR):
	mkdir -p $@

debug:
	$(MAKE) BUILD_DIR=build/debug CXXFLAGS="-O0 -g -D_GLIBCXX_DEBUG -Wall -Wextra -Wpedantic" all

clean:
	rm -rf build

-include $(TEST_BINS:.exe=.d) $(DEMO_BINS:.exe=.d)
