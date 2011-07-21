#
#  Gronda Edit Makefile for Linux based systems
#

BINARY_NAME     = gronda
EDITOR_NAME     = "Gronda"
EDITOR_VERSION  = "0.3.9b" 

CC      = gcc
CFLAGS  = -ggdb3 -Wall -Isrc/include -DLINUX -DEDITOR_NAME='$(EDITOR_NAME)' -DEDITOR_VERSION='$(EDITOR_VERSION)' -DYY_NO_UNPUT

all : gronda-terminal

gronda-terminal : clean
	$(CC) $(CFLAGS) src/*.c src/terminal/*.c -lncurses -o $(BINARY_NAME)-terminal
                
clean :            
	@( rm -f $(BINARY_NAME)-terminal )
