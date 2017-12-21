PREFIX= /usr
CFLAGS= -g -Wall -Wextra -I/usr/include -I/usr/local/include -I/usr/include/cairo -I/usr/local/include/cairo -I/usr/include/SDL2 -I/usr/local/include/SDL2
LDFLAGS= -L/usr/lib -L/usr/local/lib -L/usr/X11R6/lib
LIBS= -lcairo -lSDL2 -lm -lc
PROG= vimol

ALL_O= atoms.o bind.o camera.o cmd.o edit.o error.o exec.o formats.o graph.o \
       history.o main.o pair.o rec.o sel.o settings.o spi.o state.o \
       statusbar.o sys.o tabs.o tok.o undo.o util.o vec.o view.o xmalloc.o \
       yank.o

all: $(PROG)

$(PROG): $(ALL_O)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(PROG) $(ALL_O) $(LIBS)

install:
	install -d $(PREFIX)/bin
	install -d $(PREFIX)/share/man/man1
	install -m 0755 $(PROG) $(PREFIX)/bin
	install -m 0644 $(PROG).1 $(PREFIX)/share/man/man1

uninstall:
	rm -f $(PREFIX)/bin/$(PROG)
	rm -f $(PREFIX)/share/man/man1/$(PROG).1

clean:
	rm -f $(PROG) $(PROG).core gmon.out $(ALL_O)

.PHONY: all install uninstall clean
