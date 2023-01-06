all: on_run linked
	rm -rf *.o

on_run:
	gcc ../main_2.c -ldl -o lab5

linked: object
	gcc -o lab5_1 main.o interface.o -L../lib -lFIRST -Wl,-rpath,../lib
	gcc -o lab5_2 main.o interface.o -L../lib -lSECOND -Wl,-rpath,../lib

object:
	gcc ../*.c -c 

clean:
	rm lab5*