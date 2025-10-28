INCLUDE=./include
CC:=clang
WARNS:=\
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
	-g3 \
	-fsanitize=address,undefined \
	-I$(INCLUDE) \
	-lm \
	-lSDL2

SRC:=$(shell find . -name '*.c')
OBJ:=$(SRC:.c=.o)
OUT:=gridsolver.out

all: $(OUT)

$(OUT): $(OBJ)
	$(CC) $(CFLAGS) $(WARNS) -o $(OUT) $(OBJ)

.c.o:
	$(CC) $(CFLAGS) $(WARNS) -c -o $@ $<

.PHONY: all debug clean
.SUFFIXES: .o .c .h

