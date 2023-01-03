all: object lib

lib: object
	gcc -shared -o libFIRST.so 1st_implementation.o -lm
	gcc -shared -o libSECOND.so 2nd_implementation.o -lm

object:
	gcc -fPIC -c src_files/*.c -lm

clean:
	rm -rf *.o