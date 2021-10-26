#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <ucontext.h>

#define FAlSE 0
#define TRUE 1

#define CREATED 0
#define SCHEDULED 1
#define STOPPED 2
#define FINISHED 3

typedef struct thread
{
    ucontext_t uc;			
    int state;
    int id;	
    int priority;	
    int milliseconds_to_run;	
    void (*start_func)(void *); 	
    void *args; 				
    void *returnValue; 		
    struct thread *blockedForJoin; 	
    sem_t thread_semaphore;		
    struct thread  *next; 

}mythread;





