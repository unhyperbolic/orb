SHELL=/bin/sh

CFLAGS= -c -g -Wall -I../headers
SRCS = ${wildcard *.c}
OBJS= ${SRCS:%.c=%.o}

all: graph2manifold.o graph_complement.o test_parse_util

test_parse_util: test_parse_util.o parse_util.o
	gcc test_parse_util.o parse_util.o -o test_parse_util

%.o: %.c
	gcc ${CFLAGS} $*.c -o $@

depend:
	@echo making dependency file Makefile.dependencies ...
	gcc -MM -I../headers *.c > Makefile.dependencies

include Makefile.dependencies
