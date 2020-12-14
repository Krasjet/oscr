.POSIX:

PREFIX  = /usr/local

CC = cc
CFLAGS  = -Wall -Wextra -std=c99 -pedantic -Os -D_GNU_SOURCE
LDLIBS  = -llo
LDFLAGS =

OBJ = oscr.o util.o config.o

oscr: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJ) $(LDLIBS)

oscr.o: config.h util.h
config.o: util.h

clean:
	rm -f oscr *.o

install: oscr oscr.1
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/share/man/man1
	install -m 755 oscr $(DESTDIR)$(PREFIX)/bin
	gzip < oscr.1 > $(DESTDIR)$(PREFIX)/share/man/man1/oscr.1.gz

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/oscr
	rm -f $(DESTDIR)$(PREFIX)/share/man/man1/oscr.1.gz

.PHONY: clean install uninstall
