all:
	gcc -c test.c -o test.o -I ../
	gcc -c ../mmalcr.c -o mmalcr.o -I ../
	gcc test.o mmalcr.o -o test

clean:
	rm -rf test.o mmalcr.o test
