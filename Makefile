CC = gcc
CXX = g++
AS = nasm

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

all: $(FINAL_BIN) kern

build/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	@echo " CXX " $<
	@$(CXX) $(CPPFLAGS) -o $@ -c $<

$(FINAL_BIN): build $(COBJECTS)
	@echo " CXX " $<
	@$(CXX) -o $(FINAL_BIN) -lcapstone -pthread $(COBJECTS)

build:
	mkdir -p build

clean:
	rm -rf build



kern: build
	$(MAKE) -f Makefile.kernel
