CC=gcc
CFLAGS=-ggdb -Wpedantic -Wall -Wextra -Fanalyzer -fsanitize=leak -fsanitize=undefined -fsanitize=address -I$(.CURDIR)/include
LDLIBS=-lz

CFLAGS+=$(:!MagickWand-config --cflags --libs!)

INSTALLED=main
INSTALLDIR=~/.local/bin

.PATH.c: $(.CURDIR)/src
.PATH.h: $(.CURDIR)/include

.PHONY: install

main: main.c parser.o fileman.o export.o buffer.o imagewand.o menu.o
	${LINK.c} -o ${.TARGET} ${.ALLSRC} ${LDLIBS}

install: $(INSTALLED)
	install -Cm 755 $(.ALLSRC) $(INSTALLDIR)/edmit
