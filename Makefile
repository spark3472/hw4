main: userthread.c
	gcc -o userthread userthread.c -L. -luserthread

clean:
	rm userthread




