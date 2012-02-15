#
#  Gronda Text Editor Makefile for Unix based systems
#

BINARY_NAME     = gronda
EDITOR_NAME     = "Gronda"
EDITOR_VERSION  = "0.5"

CC      = gcc
CFLAGS  = -ggdb3 -Wall -Isrc/include -DEDITOR_NAME='$(EDITOR_NAME)' -DEDITOR_VERSION='$(EDITOR_VERSION)' -DYY_NO_UNPUT

libcfiles := $(wildcard src/*.c)
libobjects := $(notdir $(patsubst %.c,%.o,$(libcfiles)))

all: gronda

libgronda : $(libcfiles)
	$(CC) $(CFLAGS) $(libcfiles) -c
	ld -r -o libgronda.o $(libobjects)

gronda: libgronda
	`fltk-config --cxx` src/gui/main.cxx `fltk-config --cxxflags` `fltk-config --ldflags` libgronda.o -o $(BINARY_NAME)

gronda-terminal: libgronda
	$(CC) $(CFLAGS) src/terminal/*.c libgronda.o -lncurses -o $(BINARY_NAME)-terminal

clean:
	@( rm *.o )
	@( rm $(BINARY_NAME) )
	@( rm $(BINARY_NAME)-terminal )