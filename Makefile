
COMPILER_FLAGS = -Wall -Wextra -std=c99 -pedantic

KECCAK_LIB_C = KeccakF-1600-reference.c KeccakSponge.c KeccakNISTInterface.c
KECCAK_LIB_H = KeccakF-1600-reference.h KeccakSponge.h KeccakNISTInterface.h
KECCAK_LIB = $(KECCAK_LIB_C) $(KECCAK_LIB_H)

all: mainReference run

build: mainReference mainFuzz

mainReference: mainReference.c $(KECCAK_LIB)
	gcc mainReference.c $(KECCAK_LIB_C) -o mainReference $(COMPILER_FLAGS)

mainFuzz: mainFuzz.c $(KECCAK_LIB)
	afl-clang mainFuzz.c $(KECCAK_LIB_C) -o mainFuzz $(COMPILER_FLAGS)

fuzz: mainFuzz afl-input
	afl-fuzz -i afl-input -o afl-findings -t 2000 ./mainFuzz @@

clean:
	rm mainReference mainFuzz

run: mainReference
	./mainReference

