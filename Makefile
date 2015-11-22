# Just a helper makefile to go to build directory

.PHONY: all clean test install

all:
	@$(MAKE) -C build all

clean:
	@$(MAKE) -C build clean

test:
	@$(MAKE) -C build test

install:
	@$(MAKE) -C build install

