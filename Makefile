prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin

INSTALL = install
INSTALL_PROGRAM = $(INSTALL)
MKDIR = $(INSTALL) -d
RM = rm -f

CC=gcc
CFLAGS=-Wall -Wextra -Werror $(shell pkg-config --cflags x11 xft)
LDFLAGS=-lpthread $(shell pkg-config --libs x11 xft)

OBJS=xmailmon.o utils.o
TARG=xmailmon

all:	$(TARG)

$(OBJS):	config.h

config.h:
	cp config.def.h config.h

$(TARG):	$(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

%.o:	%.c
	$(CC) $< -o $@ -c $(CFLAGS)

clean:
	-$(RM) $(TARG)
	-$(RM) *.o

install:	all
	$(MKDIR) $(bindir)
	$(INSTALL_PROGRAM) $(TARG) $(bindir)/

uninstall:
	-$(RM) $(bindir)/$(TARG)

.PHONY:	all clean install uninstall

