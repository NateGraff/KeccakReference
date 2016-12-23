build: *.c *.h
	gcc *.c -o mainReference -Wall -Wextra -std=c99 -pedantic

clean:
	rm mainReference KeccakSpongeIntermediateValues* KeccakDuplexIntermediateValues* KeccakPermutationIntermediateValues*

run: mainReference
	./mainReference
