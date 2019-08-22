CC=gcc
CFLAGS=-Wall -Werror $(shell pkg-config --cflags x11 xft)
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
	@rm -f $(TARG)
	@rm -f *.o

.phony:	all clean

