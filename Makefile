
COMPILER_FLAGS = -Wall -Wextra -std=c99 -pedantic

KECCAK_LIB_C = KeccakF-1600-reference.c KeccakSponge.c KeccakNISTInterface.c
KECCAK_LIB_H = KeccakF-1600-reference.h KeccakSponge.h KeccakNISTInterface.h
KECCAK_LIB = $(KECCAK_LIB_C) $(KECCAK_LIB_H)

TEST_VECTOR_FILES = keccak_in.txt keccak_code_in.txt keccak_ref_out.txt keccak_code_out.txt

all: mainReference run

mainReference: mainReference.c $(KECCAK_LIB_C) $(KECCAK_LIB_H)
	gcc mainReference.c $(KECCAK_LIB_C) -o mainReference $(COMPILER_FLAGS)

genTestVectors: genTestVectors.c $(KECCAK_LIB_C) $(KECCAK_LIB_H)
	gcc genTestVectors.c $(KECCAK_LIB_C) -o genTestVectors $(COMPILER_FLAGS)
	./genTestVectors

cleanTestVectors:
	rm $(TEST_VECTOR_FILES) genTestVectors

clean:
	rm mainReference

run: mainReference
	./mainReference
