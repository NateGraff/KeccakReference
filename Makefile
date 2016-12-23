build: *.c *.h
	gcc *.c -o mainReference

clean: mainReference KeccakSpongeIntermediateValues*
	rm mainReference KeccakSpongeIntermediateValues*

run: mainReference
	./mainReference
