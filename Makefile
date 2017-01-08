
COMPILER_FLAGS = -Wall -Wextra -std=c99 -pedantic

KECCAK_LIB_C = KeccakF-1600-reference.c KeccakSponge.c KeccakNISTInterface.c
KECCAK_LIB_H = KeccakF-1600-reference.h KeccakSponge.h KeccakNISTInterface.h
KECCAK_LIB = $(KECCAK_LIB_C) $(KECCAK_LIB_H)

all: build run

build: mainReference.c $(KECCAK_LIB_C) $(KECCAK_LIB_H)
	gcc mainReference.c $(KECCAK_LIB_C) -o mainReference $(COMPILER_FLAGS)

clean:
	rm mainReference

run: mainReference
	./mainReference

valgrind:
	gcc mainReference.c $(KECCAK_LIB_C) -o mainReference -g -O0 $(COMPILER_FLAGS)
	valgrind --leak-check=yes ./mainReference
	rm mainReference
