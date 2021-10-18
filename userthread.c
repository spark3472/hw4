#include <stdio.h>
#include "userthread.h"

#define STACKSIZE (256*1024)

int thread_libinit(int policy)
{
  if (policy == FIFO)
    {

    }else if (policy == SJF)
    {

    }else if (policy == PRIORITY)
    {

    }else
    {

    }
  return -1;
}

int thread_libterminate(void)
{

  return -1;
}

int thread_create(void (*func)(void *), void *arg, int priority)
{

  return -1;
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
  
  printf("run\n");
}
