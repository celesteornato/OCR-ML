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
    --std=gnu99 \
    -I$(INCLUDE)

ifdef DEBUGPRINT
	CFLAGS += -DDEBUGPRINT
endif

ifdef DEBUG
	CFLAGS+= \
	    -DDEBUG\
	    -g3 \
	    -fsanitize=address,undefined 
else
	CFLAGS += \
	    -O3 \
	    -ffast-math # Living on the edge
endif


SRC_SOLVER:=$(shell find ./src/gridsolver -name '*.c')
SRC_NEURAL:=$(shell find ./src/neural_network -name '*.c')
SRC_INTERFACE:=$(shell find ./src/interface -name '*.c')
SRC_IMG:=$(shell find ./src/image_processing -name '*.c')

OBJ_SOLVER:=$(SRC_SOLVER:.c=.o)
OBJ_NEURAL:=$(SRC_NEURAL:.c=.o)
OBJ_INTERFACE:=$(SRC_INTERFACE:.c=.o)
OBJ_IMG:=$(SRC_IMG:.c=.o)

LIBS:=-lm -lSDL2 -lSDL2_image -lSDL2_ttf

all: neural solver interface image_processing
clean:
	rm -f $(OBJ_SOLVER) $(OBJ_NEURAL) $(OBJ_INTERFACE) $(OBJ_IMG)

neural: $(OBJ_NEURAL)
	$(CC) $(CFLAGS) $(WARNS) -o neural.out $(OBJ_NEURAL) $(LIBS)
solver: $(OBJ_SOLVER)
	$(CC) $(CFLAGS) $(WARNS) -o solver.out $(OBJ_SOLVER) $(LIBS)
interface: $(OBJ_INTERFACE)
	$(CC) $(CFLAGS) $(WARNS) -o interface.out $(OBJ_INTERFACE) $(LIBS)
image_processing: $(OBJ_IMG)
	$(CC) $(CFLAGS) $(WARNS) -o image_processing.out $(OBJ_IMG) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(WARNS) -c -o $@ $<

.PHONY: all clean interface neural solver image_processing
.SUFFIXES: .o .c .h

