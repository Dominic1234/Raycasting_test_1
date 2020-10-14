raycaster: main.o
	gcc main.o -o raycaster

main.o: main.c
	gcc -c -Wall main.c

clean:
	rm -f main.o
