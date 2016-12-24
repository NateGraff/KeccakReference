build: *.c *.h
	gcc *.c -o mainReference -Wall -Wextra -std=c99 -pedantic

clean:
	rm mainReference

run: mainReference
	./mainReference
