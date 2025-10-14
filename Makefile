INCLUDE=./include
CC=cc
WARNS=\
    -Wall\
    -Wextra\
    -Wpedantic\
    -Wunused\
    -Wfloat-equal\
    -Wundef\
    -Wshadow\
    -Wpointer-arith\
    -Wcast-align\
    -Wstrict-prototypes\
    -Wstrict-overflow=5\
    -Wwrite-strings\
    -Wcast-qual\
    -Wswitch-default\
    -Wswitch-enum\
    -Wconversion\
    -Wvla\
    -Wunreachable-code

CFLAGS+= \
	--std=c99 \
	-O3 \
	-I$(INCLUDE)

SRCS+=    $(shell find src -name '*.c')
OBJ+=$(patsubst %.c,%.o,$(SRCS))

all: solver.out

solver.out:
	$(CC) $(CFLAGS) $(WARNS) -o solver.out src/solver.c

.c.o:
	$(CC) $(CFLAGS) $(WARNS) -c $< -o $@

.PHONY: all debug clean
.SUFFIXES: .o .c .h

