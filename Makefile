#!/usr/bin/make

CFLAGS ?= -O3 -Wall

CFLAGS += $(shell pkg-config --cflags sdl2)
LDLIBS += $(shell pkg-config --libs sdl2)

OBJS = gol.o crc32.o

gol: $(OBJS)

clean:
	$(RM) gol $(OBJS)
