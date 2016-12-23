build: *.c *.h
	gcc *.c -o mainReference

clean: mainReference KeccakSpongeIntermediateValues* KeccakDuplexIntermediateValues* KeccakPermutationIntermediateValues*
	rm mainReference KeccakSpongeIntermediateValues* KeccakDuplexIntermediateValues* KeccakPermutationIntermediateValues*

run: mainReference
	./mainReference
