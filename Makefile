CC = clang
CXX = clang++
AS = nasm


.PHONY: tests

STRUCTURE := $(shell find src -type d)
CODEFILES := $(addsuffix /*,$(STRUCTURE))
CODEFILES := $(wildcard $(CODEFILES))


CPPSOURCES := $(filter %.cpp,$(CODEFILES))
COBJECTS := $(CPPSOURCES:%.cpp=build/%.cpp.o)

# ASOURCES:=$(wildcard kernel/src/*.asm)
ASOURCES:=$(filter %.asm,$(CODEFILES))
AOBJECTS:=$(ASOURCES:%.asm=build/%.asm.o)


FINAL_BIN = build/mobo


CINCLUDES=-I./include/

COMMON_FLAGS := $(CINCLUDES) -O3 -DGIT_REVISION=\"$(shell git rev-parse HEAD)\"


CFLAGS:=$(COMMON_FLAGS)

CPPFLAGS:=$(CFLAGS) -std=c++17

default: $(FINAL_BIN)

all: $(FINAL_BIN) kern tests

build/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	@echo " CXX " $<
	@$(CXX) $(CPPFLAGS) -o $@ -c $<

$(FINAL_BIN):
	mkdir -p build; cd build; cmake ..; make -j

build:
	mkdir -p build

clean:
	$(MAKE) -s -f Makefile.kernel clean
	@rm -rf build


kern: build
	$(MAKE) -f Makefile.kernel


tests:
	@./build-tests.sh
