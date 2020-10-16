raycaster: main.o
	gcc main.o -o raycaster -lGL -lGLU -lglut -lm

main.o: main.c
	gcc -c main.c -lGL -lGLU -lglut -lm

clean:
	rm -f main.o
