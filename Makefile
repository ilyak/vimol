CC= cc
PREFIX= /usr/local
CFLAGS= -g -Wall -I/usr/include -I/usr/local/include -I/usr/include/cairo -I/usr/local/include/cairo -I/usr/include/SDL2 -I/usr/local/include/SDL2
LDFLAGS= -L/usr/lib -L/usr/local/lib -L/usr/X11R6/lib
LIBS= -lcairo -lSDL2 -lm -lc
PROG= vimol

ALL_C= atoms.c bind.c camera.c cmd.c edit.c error.c exec.c formats.c graph.c \
       history.c main.c pair.c rec.c sel.c settings.c spi.c state.c \
       statusbar.c sys.c tok.c undo.c util.c vec.c view.c wnd.c xmalloc.c yank.c
ALL_O= atoms.o bind.o camera.o cmd.o edit.o error.o exec.o formats.o graph.o \
       history.o main.o pair.o rec.o sel.o settings.o spi.o state.o \
       statusbar.o sys.o tok.o undo.o util.o vec.o view.o wnd.o xmalloc.o yank.o

$(PROG): $(ALL_O)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(PROG) $(ALL_O) $(LIBS)

install:
	install -m 0755 $(PROG) $(PREFIX)/bin

uninstall:
	rm -f $(PREFIX)/bin/$(PROG)

clean:
	rm -f $(PROG) $(PROG).core gmon.out $(ALL_O)

.PHONY: install uninstall clean
