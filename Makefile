#
#  Gronda Edit Makefile for Linux based systems
#

BINARY_NAME     = gronda
EDITOR_NAME     = "Gronda"
EDITOR_VERSION  = "0.3.9b" 

CC      = gcc
CFLAGS  = -rdynamic -ggdb3 -Wall -Isrc/include -DLINUX -c -DEDITOR_NAME='$(EDITOR_NAME)' -DEDITOR_VERSION='$(EDITOR_VERSION)' -DYY_NO_UNPUT
#CFLAGS  = -rdynamic -Wall -Iinclude -DLINUX -c -DINSTALL_DIR='"$(BASE_DIR)"' -DEDITOR_NAME='$(EDITOR_NAME)' -DEDITOR_VERSION='$(EDITOR_VERSION)' -DBUG_WEB='$(BUG_WEB)' -DBUG_EMAIL='$(BUG_EMAIL)'
CMOD    = -fPIC
LD      = gcc
LDFLAGS = -ldl -rdynamic
LDMOD   = -export-dynamic -shared -ggdb3 -Wall

# flags required by display library
DISPLAYFLAGS = -lncurses

all : $(BINARY_NAME)
	@(echo "Done") 

OBJS = src/buffer.o \
	src/cmd_buffer.o \
	src/cmd_command.o \
	src/cmd_cursor.o \
	src/cmd_file.o \
	src/cmd_key.o \
	src/cmd_pad.o \
	src/cmd_system.o \
	src/cmd_text.o \
	src/cmd_window.o \
	src/command.o \
	src/display.o \
	src/editor.o \
	src/keydefs.o \
	src/lex.o \
	src/line.o \
	src/main.o \
	src/menu.o \
	src/recovery.o \
	src/redraw.o \
	src/signals.o \
	src/str.o \
	src/tools.o

$(BINARY_NAME) : $(OBJS)
	@(if ( test -x $(BINARY_NAME) ) then (echo "$(BINARY_NAME) exists - overwriting"; rm -f $(BINARY_NAME)) fi)
	@( echo "Linking $(BINARY_NAME)" )
	@( $(LD) $(LDFLAGS) $(DISPLAYFLAGS) *.o -o $(BINARY_NAME) )
                
clean :            
	@( echo "Cleaning..." )
	@( rm -f $(BINARY_NAME) )
	@( rm -f *.o )       
	@( echo "Done" )

.c.o:
	@( echo "Compiling $<" )
	@( $(CC) $(CFLAGS) $< )

