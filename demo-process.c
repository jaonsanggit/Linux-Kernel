#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h> 

#define NUM_THREADS   2

void *PrintHello(void *args)
{
    int thread_arg;
    sleep(1);
    thread_arg = (int)(*((int*)args));
    printf("Hello from thread %d\n", thread_arg);
    return NULL;
}

void creat_thread(void)
{
  int rc,t;
  pthread_t thread[NUM_THREADS];

  for( t = 0; t < NUM_THREADS; t++)
  {
    printf("Creating thread %d\n", t);
    rc = pthread_create(&thread[t], NULL, PrintHello, &t);
    if (rc)
    {
      printf("ERROR; return code is %d\n", rc);
      return;
     }
  }
  sleep(5);
  for( t = 0; t < NUM_THREADS; t++)
    pthread_join(thread[t], NULL);
}

int main()
{
	pid_t pid;
  if((pid=fork())==0)
  {
  //子进程代码
    creat_thread();
 	  printf("子进程%d\n", pid);
 	  exit(0);
  }
  else if(pid>0)
  {
  	//父进程代码
    creat_thread();
    printf("父进程%d\n", pid);
    exit(0); 
  }
  else {
    printf("Error");
    exit(1);
  }
}
