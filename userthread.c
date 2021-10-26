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

#define THREAD_NUMBER 100
#define FAIL -1

#include "userthread.h"
#include "thread.h"

//#define STACKSIZE 4096
#define INTERVAL_SECS           1
#define INTERVAL_MICROSECS      0
#define VALUE_SECS              1
#define VALUE_MICROSECS         0
#define MICROS                  1000000


int id = 0;
ucontext_t ma; 
int is_libterm = 1;
struct thread_queue* global;
struct itimerval timer;

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



/* TIMER THINGS */
void print_thread()
{

  mythread *pointer = global->head;
  printf("Length of Queue: %d\n", global->length);
  for (int i = 0; i < global->length; i++)
  {
    printf("Thread id : %d  Priority: %d\n", pointer->id, pointer->priority);
    pointer = pointer->next;
  }

}
/* prints to userthread_log.txt
 *
 */
int milliseconds_passed = 0;
void write_to_log(mythread* chosen_thread)
{
  milliseconds_passed += 100;
  char* state;
  if (chosen_thread->state == CREATED)
  {
    state = "CREATED";

  }else if (chosen_thread->state == SCHEDULED)
  {
    state = "SCHEDULED";

  }else if(chosen_thread->state == STOPPED)
  {
    state = "STOPPED";

  }else if(chosen_thread->state == FINISHED)
  {
    state = "FINISHED";
  }

  printf("%d    %s    %d    %d\n", milliseconds_passed, state, chosen_thread->id, chosen_thread->priority);
}

/* scheduler
 * called by sigvalarm handler
 */
mythread *current;
void FIFO_scheduler() {
  //schedule a thread
  if (global->length > 0)
  {
    current = global->head;
    while (current != NULL && current->next != NULL)
    {
      if (current->state == FINISHED)
      {
        current = current->next;
      }else
      {
        break;
      }
    }
  }
  current->state = SCHEDULED;
  write_to_log(current);
  swapcontext(&ma, &current->uc);
}



void FIFO_handler(int sig) 
{
  /* check if the thread is in a critical section */
  FIFO_scheduler();
}

mythread* swap(mythread *ptr1, mythread *ptr2)
{
    mythread* tmp = ptr2->next;
    ptr2->next = ptr1;
    ptr1->next = tmp;
    return ptr2;

}
sem_t sjf_mutex;
int sjf_once = 0;
//Bubble Sort
void SJF_scheduler()
{
    mythread** h;
    int swapped;
  
    for (int i = 0; i <= global->length; i++)
    {
  
        h = &global->head;
        swapped = 0;
  
        for (int j = 0; j < global->length - i - 1; j++) 
        {
  
            mythread* p1 = *h;
            mythread* p2 = p1->next;
  
            if (p1->milliseconds_to_run > p2->milliseconds_to_run)
            {
                *h = swap(p1, p2);
                swapped = 1;
            }
  
            h = &(*h)->next;
        }
  
        /* break if the loop ended without any swap */
        if (swapped == 0)
            break;
    }







    if (global->length > 0)
      {
      current = global->head;
      while (current != NULL && current->next != NULL)
        {
        if (current->state == FINISHED)
        {
          current = current->next;
        }else
        {
          break;
        } 
      }
    }
    current->state = SCHEDULED;
    write_to_log(current);
    swapcontext(&ma, &current->uc);
  
}


void SJF_handler()
{
  SJF_scheduler();
}

void PRIORITY_scheduler()
{
  //sort the queue by priority (doesn't matter the order after)
  //switch the context using thread yield

  //switches head and "tail"
  if (global->length > 0)
  {
    if (global->head->state != FINISHED)
    {
      global->head->state = SCHEDULED;
    }
    
      mythread* ptr = global->head;
      mythread* tmp = global->head;
      
      write_to_log(ptr);
      swapcontext(&ma, &global->head->uc);


      while(ptr != NULL && ptr->next != NULL){
      ptr = ptr->next;
      }
      if (global->length > 1)
      {
        global->head = global->head->next;
        tmp->next = NULL;
        ptr->next = tmp;
      }

  }
}

void PRIORITY_handler()
{
  PRIORITY_scheduler();
}

sem_t global_mutex;
int thread_libinit(int policy) 
{
    sem_init(&global_mutex, 0, 1);
    sem_init(&sjf_mutex, 0, 1);


    global = malloc(1 * sizeof(struct thread_queue));
    global->length = 0;
    global->head = NULL;

     
    /* Install timer_handler as the signal handler for SIGVTALRM. */
    //sigaction(SIGVTALRM, &interrupt_handler, NULL);
     
     
    /* Configure the timer to expire after 250 msec... */
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 100000;
    /* ... and every 250 msec after that. */
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 100000;
    /* Start a virtual timer. It counts down whenever this process is executing. */
    setitimer(ITIMER_VIRTUAL, &timer, NULL);

    
    if (policy == FIFO)
    {
      signal(SIGVTALRM, FIFO_handler);
      //threads on the ready queue are to be scheduled as they arrive


      return 0;

    }else if (policy == SJF)
    {
      //handler for SJF
      signal(SIGVTALRM, SJF_handler);

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
      //handler for PRIORITY
      signal(SIGVTALRM, PRIORITY_handler);

      //level -1: 1.5x more than level 0
      //level 0: 1.5x more than level 1

      //schedule a SIGARLM to send to scheduler every 100 milliseconds
      //see lab ucontext-hw.c
      //refer to setitimer(2)

      return 0;

    }else{
      

      return -1;
    }

}

/* Frees a thread
 * @param thread to free
*/
void free_thread(mythread* thread) 
{
    free(thread->uc.uc_stack.ss_sp);
    free(thread);
}

/* Goes through the queue and frees each node
 * @param node The node of the queue to free
 */
void free_next(mythread* node) 
{
    if(node == NULL) 
    {
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
    mythread* tmp;
    int count = 0;
    while(global->head != NULL)
    {
      tmp = global->head;
      global->head = global->head->next;
      free(tmp->uc.uc_stack.ss_sp);
      free(tmp);
      count++;

    }
    free(global);
    //printf("count %d\n", count);
}


int thread_libterminate(void)
{
  free_thread_queue(global);

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
    current->state = FINISHED;
    setcontext(&ma);

}

/* Adds recently initialized thread to queue
 * @param thread to add
 */
int add_thread(mythread *new_thread)
{

    if (global->head == NULL)
    {

      sem_wait(&global_mutex);

      global->head = new_thread;
      global->length++;

      sem_post(&global_mutex);

      if (new_thread->id < 0)
      {
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

      if (traverse->next->id < 0)
      {
        return -1;
      }else{
        return traverse->next->id;
      }

    }

}

/* initializes thread and calls add_thread to add it to queue
 * @param function for thread to execute
 * @param argument to pass into function for thread to execute
 * @param priority number (-1, 0, or 1) not applicable if policy = FIFO or SJF
 */
int count = 0;
int thread_create(void (*func)(void *), void *arg, int priority)
{
    //initializing a context
    ucontext_t new;
    int context = -1;
    if ((context = getcontext(&new)) == -1)
    {
      printf("failed to get context\n");
      return -1;
    }
  
    new.uc_stack.ss_sp = malloc(STACKSIZE);
    new.uc_stack.ss_size = STACKSIZE;
    makecontext(&new, (ucfunc_t)helper, 2 , func, arg);
    new.uc_stack.ss_flags = 0;
    sigemptyset(&(new.uc_sigmask));
    sigaddset(&(new.uc_sigmask), SIGALRM);


    //initializing a mythread
    mythread *new_thread = malloc(sizeof(mythread));
    new_thread->uc = new;
    new_thread->start_func = func;
    new_thread->args = arg;
    new_thread->id = id++;
    
    
    new_thread->state = CREATED;
    new_thread->priority = priority;
    
    /*if (count%2 == 0)
    {
      new_thread->milliseconds_to_run = 50;
    }else
    {
      new_thread->milliseconds_to_run = 0;
    }*/
    //half of quanta
    new_thread->milliseconds_to_run = 50;

    count++;

    
    new_thread->returnValue = NULL;
    new_thread->blockedForJoin = NULL;
    new_thread->next = NULL;

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
}

int thread_yield(void)
{
  swapcontext(&ma, &global->head->uc);

  
  //swapcontext from current thread to next thread scheduled
  //if policy is PRIORITY
  //  switch thread at head to the end of the queue for the respective priority level
  //else
  //  switch the head thread with the last thread in the queue
  return 0;
}

int thread_join(int tid)
{
  if (global->length == 0)
  {
    return -1;
  }
  while(1)
  {
    mythread* ptr = global->head;
    while(ptr->id != tid){
      ptr = ptr->next;
    }
    if (ptr->id != tid)
    {
      return -1;
    }else if (ptr->state == FINISHED)
    {
      return 0;
    }
  }
  //search for thread id in list
  //if thread is not FINISHED, then while loop
  //if thread does not exist assume it is FINISHED

  return -1;
}



int main()
{
        int thread_lib = thread_libinit(FIFO);
        if (thread_lib < 0)
        {
          printf("Policy not chosen!\n");
          exit(0);
        }

        void *arg = "hello";
        int create = 0;
        for (int i = 0; i < 10; i++)
        {
          create = thread_create(doThing, arg, -1);

          //printf("Thread id: %d\n", create);
        }

        for (int i = 0; i < create + 1; i++)
        {
          thread_join(i);
        }

        thread_libterminate();

}
