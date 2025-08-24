SHELL=/bin/sh

CFLAGS= -c -g -Wall -I../headers
SRCS = ${wildcard *.c}
OBJS= ${SRCS:%.c=%.o}

all: test_parse_orb

test_parse_orb: test_parse_orb.o orb_io.o diagram.o graph.o casson_io.o diagram_io.o ostream.o ../code/liborb.a
	gcc test_parse_orb.o orb_io.o diagram.o ../code/*.o graph.o casson_io.o diagram_io.o ostream.o -lm -o test_parse_orb

%.o: %.c
	gcc ${CFLAGS} $*.c -o $@

depend:
	@echo making dependency file Makefile.dependencies ...
	gcc -MM -I../headers *.c > Makefile.dependencies

include Makefile.dependencies
