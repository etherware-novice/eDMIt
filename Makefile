CC=gcc
CFLAGS=-ggdb -Wpedantic -Wall -Wextra -Fanalyzer -fsanitize=leak -fsanitize=undefined -fsanitize=address -I$(.CURDIR)/include
LDLIBS=-lz

CFLAGS+=$(:!MagickWand-config --cflags --libs!)

#.SUFFIXES:
#.SUFFIXES: .c .o .h  

.PATH.c: $(.CURDIR)/src
.PATH.h: $(.CURDIR)/include


main: main.c parser.o fileman.o export.o buffer.o imagewand.o
	${LINK.c} -o ${.TARGET} ${.ALLSRC} ${LDLIBS}
