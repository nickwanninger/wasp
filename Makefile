CC = clang
CXX = clang++
AS = nasm


.PHONY: tests

FINAL_BIN = build/wasp
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
	mkdir -p build; cd build; cmake ..; make -w -j



install: $(FINAL_BIN)
	cd build; make install

do:
	mkdir -p build; cd build; cmake ..; make -w -j

build:
	mkdir -p build

clean:
	@rm -rf build

tests:
	@./build-tests.sh
