#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <ucontext.h>

#define FAlSE 0
#define TRUE 1

#define RUNNING 0
#define READY 1
#define BLOCKED 2
#define FINISHED 3

typedef struct thread
{
    ucontext_t uc;			
    int state;
    int id;			
    void (*start_func)(void *); 	
    void *args; 				
    void *returnValue; 		
    struct thread *blockedForJoin; 	
    sem_t thread_semaphore;		
    struct thread  *next; 

}mythread;





