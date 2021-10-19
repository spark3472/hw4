#include <stdio.h>
#include <ucontext.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>
#include <string.h>
#ifndef NULL
#define NULL (0)
#endif


#include "userthread.h"
#include "thread.h"


#define STACKSIZE (256*1024)

int id = 0;

//holds the thread_queue struct
struct thread_queue{
  //first thread
  mythread *head; ;
  
  int length;
 
};

/* Makes a new job list
 * @return the initiated joblist
 */
//global queue
struct thread_queue* global;
int thread_libinit(int policy) {
    global = malloc(1 * sizeof(struct thread_queue));
    global->length = 0;
    global->head = NULL;
    //newjobList->jobsTotal = 0;
    return 0;
}

/* Frees a process
 * @param process The process to free
*/
void free_thread(mythread* thread) {
  free(thread);
}

/* Goes through the joblist and frees each node
 * @param node The node of the joblist to free
 */
void free_next(mythread* node) {
    if(node == NULL) {
        return;
    }
    free_next(node->next);
    free_thread(node);
}

/* Frees the joblist, including calling freeHelper to free individual processes
 * @param jobList the jobList to free
 */
void free_thread_queue(struct thread_queue* global) {
    free_next(global->head);
    free(global);
}


int thread_libterminate(void)
{

  return -1;
}
typedef void (* ucfunc_t)(void);
void helper(void* (*selectFunction)(void*), void* selectArg) {
    // Calls select function with select argument
    selectFunction(selectArg);
}

sem_t global_mutex;
int add_thread(mythread *new_thread){

    sem_wait(&global_mutex);

    if (global->head == NULL){

      //printf("head is null\n");
      global->head = new_thread;
      global->length++;

      return 0;

    }else{

      //printf("head is not null\n");
      global->head->next = new_thread;
      global->length++;

      return 0;

    }

    sem_post(&global_mutex);
}


int thread_create(void (*func)(void *), void *arg, int priority)
{
    //initializing a context
    ucontext_t new;
    void *stack;
    int context = -1;
    if ((context = getcontext(&new)) == -1)
    {
      return -1;
    }
    stack = malloc(STACKSIZE);
    new.uc_stack.ss_sp = stack;
    new.uc_stack.ss_size = STACKSIZE;
    new.uc_stack.ss_flags = 0;
    sigemptyset(&(new.uc_sigmask));
    sigaddset(&(new.uc_sigmask), SIGALRM);
    //printf("ucontext made\n");

    //initializing a mythread
    mythread *new_thread = malloc(sizeof(mythread));
    //printf("malloc made\n");
    new_thread->uc = new;
    //printf("set uc = new made\n");
    new_thread->start_func = func;
    //printf("func made\n");
    new_thread->args = arg;
    //printf("args made\n");
    new_thread->id = ++id;
    //printf("id made\n");
    
    //Ready to be queued
    new_thread->state = READY;
    //printf("state made\n");
    
    new_thread->returnValue = NULL;
    //printf("returnValue made\n");
    new_thread->blockedForJoin = NULL;
    //printf("blocked made\n");
    new_thread->next = NULL;
    makecontext(&new, (ucfunc_t)helper, 2 , func, arg);
    setcontext(&new);


    return (add_thread(new_thread));

}

void doThing(void* arg)
{
 printf("%s thing done\n", (char*)arg);
}

int thread_yield(void)
{

  return 0;
}

int thread_join(int tid)
{

  return -1;
}


int main(){
  sem_init(&global_mutex, 0, 1);
  int thread_lib = thread_libinit(1);
  printf("%d thread library initiated\n", thread_lib);
  void *arg = "hello";
  int create = thread_create(doThing, arg, 0);
  printf("%d\n", create);
}
