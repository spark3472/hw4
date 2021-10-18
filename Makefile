main: userthread.c
	gcc -Wall -fpic -c userthread.c
	gcc -o libuserthread.so userthread.o -shared
	gcc -o userthread userthread.c -L. -luserthread


clean:
	rm userthread




