#!/usr/bin/make

CFLAGS ?= -O3 -Wall -W -pedantic -ansi
CFLAGS += -std=c99

CFLAGS += $(shell pkg-config --cflags sdl3)
LDLIBS += $(shell pkg-config --libs sdl3)

OBJS = gol.o crc32.o parse.o

gol: $(OBJS)

clean:
	$(RM) gol $(OBJS)
