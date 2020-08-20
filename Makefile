.POSIX:

PREFIX  = /usr/local
CFLAGS  = -Wall -Wextra -std=c99 -pedantic -Os -D_GNU_SOURCE
LDLIBS  = -llo
LDFLAGS =

OBJ = oscr.o util.o config.o

oscr: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJ) $(LDLIBS)

clean:
	rm -f oscr *.o

install: oscr
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	install -m 755 oscr $(DESTDIR)$(PREFIX)/bin

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/oscr

.PHONY: clean install uninstall
