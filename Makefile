#!/usr/bin/make

CFLAGS ?= -O3 -Wall -W -pedantic -ansi

CFLAGS += $(shell pkg-config --cflags sdl2)
LDLIBS += $(shell pkg-config --libs sdl2)

OBJS = gol.o crc32.o parse.o

gol: $(OBJS)

clean:
	$(RM) gol $(OBJS)
