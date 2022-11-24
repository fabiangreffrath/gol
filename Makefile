#!/usr/bin/make

CFLAGS ?= -O3

CFLAGS += $(shell pkg-config --cflags sdl2)
LDLIBS += $(shell pkg-config --libs sdl2)
