binary: main.o libworld.a
	gcc -o main main.o -L. -lmain -I.
main.o: main.c
	gcc -c main.c
libmain.a: RevertString.o
	ar cr libmain.a RevertString.o
RevertString.o: revert_string.c
	gcc -c revert_string.c
clean:
	rm   -f   *.o  *.a main