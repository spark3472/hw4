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
ucontext_t ma; //for main function

//holds the thread_queue struct
struct thread_queue
{
  //first thread
  mythread *head; ;
  
  int length;
 
};

/* Makes a new thread_queue
 * @return 0 for success, -1 for failure
 */
//global queue
struct thread_queue* global;
int thread_libinit(int policy) 
{

    if (policy == FIFO)
    {
      //threads on the ready queue are to be scheduled as they arrive


      return 0;

    }else if (policy == SJF)
    {

      //no runtime history use half the quanta

      //threads on the ready queue are to be scheduled shortest job first with
      //according to estimated time (keep track of how long thread runs before 
      //calling thread_yield or thread_join) use average of the last three 
      //times it took for the thread ran to compute job time. When a job is
      //created, assume its an average job and set its runtime to the average
      //runtime of all threads so far

      
      return 0;

    }else if (policy == PRIORITY)
    {
      
      //level -1: 1.5x more than level 0
      //level 0: 1.5x more than level 1

      //schedule a SIGARLM to send to scheduler every 100 milliseconds
      //see lab ucontext-hw.c
      //refer to setitimer(2)

      return 0;

    }else{
      

      return -1;
    }

    global = malloc(1 * sizeof(struct thread_queue));
    global->length = 0;
    global->head = NULL;

}

/* Frees a thread
 * @param thread to free
*/
void free_thread(mythread* thread) 
{
    free(thread);
}

/* Goes through the queue and frees each node
 * @param node The node of the queue to free
 */
void free_next(mythread* node) 
{
    if(node == NULL) {
        return;
    }
    free_next(node->next);
    free_thread(node);
}

/* Frees the the queue
 * @param queue to free
 */
void free_thread_queue(struct thread_queue* global) 
{
    free_next(global->head);
    free(global);
}


int thread_libterminate(void)
{

  return -1;
}

/* https://stackoverflow.com/questions/60591497/how-can-i-mask-the-warning-from-makecontext-if-the-function-passed-in-has-parame
 * Work around to makecontext argument type error
 * @param function to execute
 * @param arguments to pass into function to execute
 */
typedef void (* ucfunc_t)(void);
void helper(void* (*selectFunction)(void*), void* selectArg) 
{
    // Calls select function with select argument
    selectFunction(selectArg);
}

/* Adds recently initialized thread to queue
 * @param thread to add
 */
sem_t global_mutex;
int add_thread(mythread *new_thread)
{

    if (global->head == NULL){


      sem_wait(&global_mutex);

      global->head = new_thread;
      global->length++;

      sem_post(&global_mutex);

      if (new_thread->id < 0){
        return -1;
      }else{
        return new_thread->id;
      }

    }else{

      sem_wait(&global_mutex);
      mythread *traverse = global->head;
      
      while(traverse != NULL && traverse->next != NULL)
      {
            traverse = traverse->next;
      }


      traverse->next = new_thread;
      global->length++;

      sem_post(&global_mutex);

      if (new_thread->id < 0){
        return -1;
      }else{
        return new_thread->id;
      }

    }

}

/* initializes thread and calls add_thread to add it to queue
 * @param function for thread to execute
 * @param argument to pass into function for thread to execute
 * @param priority number (-1, 0, or 1) not applicable if policy = FIFO or SJF
 */
int thread_create(void (*func)(void *), void *arg, int priority)
{
    //initializing a context
    ucontext_t new;
    void *stack;
    int context = -1;
    if ((context = getcontext(&new)) == -1)
    {
      printf("failed to get context\n");
      return -1;
    }
    stack = malloc(STACKSIZE);
    new.uc_stack.ss_sp = stack;
    new.uc_stack.ss_size = STACKSIZE;
    new.uc_stack.ss_flags = 0;
    sigemptyset(&(new.uc_sigmask));
    sigaddset(&(new.uc_sigmask), SIGALRM);


    //initializing a mythread
    mythread *new_thread = malloc(sizeof(mythread));
    new_thread->uc = new;
    new_thread->start_func = func;
    new_thread->args = arg;
    new_thread->id = ++id;
    
    makecontext(&new, (ucfunc_t)helper, 2 , func, arg);
    new_thread->state = READY;

    
    new_thread->returnValue = NULL;
    new_thread->blockedForJoin = NULL;
    new_thread->next = NULL;

    swapcontext(&ma, &new);
    return (add_thread(new_thread));

}

/* Test a function of makecontext
 * @param argument that is passed in from thread
 */
void doThing(void* arg)
{
        //work to do
        printf("%s\n", (char*)arg);

        //set context back to ma
        setcontext(&ma);
}

int thread_yield(void)
{

  return 0;
}

int thread_join(int tid)
{

  return -1;
}

void print_thread()
{
  
  mythread *pointer = global->head;
  printf("Length of Queue: %d\n", global->length);
  for (int i = 0; i < global->length; i++){
    printf("Thread id : %d\n", pointer->id);
    pointer = pointer->next;
  }
}


int main()
{
        sem_init(&global_mutex, 0, 1);
        

        int thread_lib = thread_libinit(FIFO);
        if (thread_lib < 0){
          printf("Policy not chosen!\n");
          exit(0);
        }

        void *arg = "hello";
        int create = 0;
        for (int i = 0; i < 10; i++){
          create = thread_create(doThing, arg, 0);

          printf("Thread id: %d\n", create);
        }
        print_thread();

}
