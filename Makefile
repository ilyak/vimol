CC= cc
PREFIX= /usr/local
CFLAGS= -Wall -I/usr/include -I/usr/local/include -I/usr/include/cairo -I/usr/local/include/cairo -I/usr/include/SDL2 -I/usr/local/include/SDL2
LDFLAGS= -L/usr/lib -L/usr/local/lib -L/usr/X11R6/lib
LIBS= -lcairo -lSDL2
PROG= vimol

ALL_C= alias.c atoms.c camera.c cmd.c color.c edit.c error.c exec.c graph.c \
       history.c log.c main.c pair.c rec.c sel.c settings.c spi.c state.c \
       status.c sys.c tok.c undo.c util.c vec.c view.c wnd.c xmalloc.c yank.c
ALL_O= alias.o atoms.o camera.o cmd.o color.o edit.o error.o exec.o graph.o \
       history.o log.o main.o pair.o rec.o sel.o settings.o spi.o state.o \
       status.o sys.o tok.o undo.o util.o vec.o view.o wnd.o xmalloc.o yank.o

$(PROG): $(ALL_O)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(PROG) $(ALL_O) $(LIBS)

install:
	install -m 0755 $(PROG) $(PREFIX)/bin

uninstall:
	rm -f $(PREFIX)/bin/$(PROG)

clean:
	rm -f $(PROG) $(PROG).core $(PROG).gmon $(ALL_O)

.PHONY: install uninstall clean
