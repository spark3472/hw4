main: userthread.c
	
	gcc -g -Wall -fpic -c userthread.c
	gcc -g -o libuserthread.so userthread.o -shared
	gcc -g -o userthread userthread.c -L. -luserthread -lpthread


clean:
	rm userthread




