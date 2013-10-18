CC = gcc
OBJECTS = relay.o
LIBS = -lftdi
CFLAGS = -Wall -O2 -g
NAME = relay
BINDIR = $(DESTDIR)/usr/bin

cbg: $(OBJECTS)
	$(CC) -o $(NAME) $(OBJECTS) $(LIBS)

%.o: %.c
	$(CC) -c $(CFLAGS) $<

clean:
	rm *.o $(NAME)

install:
	install --mode=755 $(NAME) $(BINDIR)/

uninstall:
	rm $(BINDIR)/$(NAME)
