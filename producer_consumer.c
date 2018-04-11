#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <pthread.h>

#include "shm_com.h"

static int set_semvalue(int sem_id, int value);
static void del_semvalue(int sem_id);
static int semaphore_p(int sem_id);
static int semaphore_v(int sem_id);
void* consumer(void* arg);
void* producer(void* arg);
static int producerItem(void);
static void putItemIntoBuffer(int item);
static int removeItemFromBuffer(void);
static void consumerItem(int item);

static int sem_empty;
static int sem_full;
static int sem_mutex;


void *shared_memory = (void*)0;
struct shared_use_st *shared_stuff;

int main(int argc, char const *argv[])
{
	int p_num=5;
	pid_t ret;
	printf("producer-consumer program starting...\n");

	/*建立信号量*/
	sem_empty = semget((key_t)1234, 1, 0666|IPC_CREAT);
	if (!set_semvalue(sem_empty, TEXT_SZ)) 												//初始化空信号量为TEXT_SZ
		{
			fprintf(stderr, "Failed to initialize semaphore\n");
			exit(EXIT_FAILURE);
		}

	sem_full = semget((key_t)1233, 1, 0666|IPC_CREAT);
	if (!set_semvalue(sem_full, 0)) 															//初始化满信号量为0
		{
			fprintf(stderr, "Failed to initialize semaphore\n");
			exit(EXIT_FAILURE);
		}

	sem_mutex = semget((key_t)1232, 1, 0666|IPC_CREAT);
	if (!set_semvalue(sem_mutex, 1)) 															//初始化缓冲区信号量为1;
		{
			fprintf(stderr, "Failed to initialize semaphore\n");
			exit(EXIT_FAILURE);
		}

	/*建立共享内存*/
	int shmid;
	shmid = shmget((key_t)1231, sizeof(struct shared_use_st), 0666 | IPC_CREAT);
	if (shmid == -1)
	{
		fprintf(stderr, "shmget failed\n");
		exit(EXIT_FAILURE);
	}

	shared_memory = shmat(shmid, (void*)0, 0);
	if (shared_memory == (void*)-1)
	{
		fprintf(stderr, "shmat failed\n");
		exit(EXIT_FAILURE);	
	}

	printf("Memory attached at %X\n", (int)shared_memory);

	shared_stuff = (struct shared_use_st *)shared_memory;
	shared_stuff->in = 0;
	shared_stuff->out = 0;

  /* 使用循环创建多个进程 */  
  for ( int i=0; i < p_num; i++ )  
  {  
      ret = 0;  
      if ( (ret = fork()) < 0 )  
      {  
        fprintf(stderr,"create child process error\n");  
      }  
      else if ( 0 == ret )  
      {  
        printf("This is child process [%d], pid[%4d] ppid[%4d]\n",i,getpid(),getppid());  
        sleep(i);
        break; 		//在子进程中先break，避免fork逻辑混乱
      }  
  } 

  /*-----子进程逻辑-----*/
  if(ret==0) {
  	int res;
  	pthread_t consumer_thread;
  	/****新线程执行消费者程序*******/
  	res = pthread_create(&consumer_thread, NULL, consumer, (void*)0);
  	if (res != 0)
  	{
  		perror("Thread creation failed");
  		exit(EXIT_FAILURE);
  	}
  	/****主线程执行生产者程序*******/
  	producer((void*)0);
  }

	/*父进程逻辑*/
  if (ret > 0)
  {
  	while(true);
  }
}


void* consumer(void* arg)
{
	int item;
	while(true) {
		semaphore_p(sem_full);
		semaphore_p(sem_mutex);
		item = removeItemFromBuffer();
		shared_stuff->out = (shared_stuff->out + 1) % TEXT_SZ;
		consumerItem(item);
		semaphore_v(sem_mutex);
		semaphore_v(sem_empty);
		sleep(2);
	}
}

void* producer(void *arg)
{
	int item;
	while(true) {
			item = producerItem();
			semaphore_p(sem_empty);
			semaphore_p(sem_mutex);
			putItemIntoBuffer(item);
			shared_stuff->in = (shared_stuff->in + 1) % TEXT_SZ;
			semaphore_v(sem_mutex);
			semaphore_v(sem_full);
			sleep(2);
	}
}

static int producerItem(void) {
	return getpid();
}

static void putItemIntoBuffer(int item) {
	
	shared_stuff->some_text[shared_stuff->in] = item;	

}

static int removeItemFromBuffer(void) {
	int tmp;
	tmp = shared_stuff->some_text[shared_stuff->out];
	shared_stuff->some_text[shared_stuff->out] = 0;
	return tmp;

}

static void consumerItem(int item) {

	printf("pid[%4d] consumed %d: %d\n", getpid(), shared_stuff->out, item);

}




static int set_semvalue(int sem_id, int value)
{
	union semun sem_union;

	sem_union.val = value;
	if (semctl(sem_id, 0, SETVAL, sem_union) == -1) return 0;
	return 1;
}

static void del_semvalue(int sem_id)
{
	union semun sem_union;
	if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1)
		fprintf(stderr, "Failed to delete semaphore\n");
}

static int semaphore_p(int sem_id)
{
	struct sembuf sem_b;

	sem_b.sem_num = 0;
	sem_b.sem_op = -1;
	sem_b.sem_flg = SEM_UNDO;
	if (semop(sem_id, &sem_b, 1) == -1)
	{
		fprintf(stderr, "semaphore_p failed.\n");
		return 0;
	}
	return 1;
}

static int semaphore_v(int sem_id)
{
		struct sembuf sem_b;

	sem_b.sem_num = 0;
	sem_b.sem_op = 1;
	sem_b.sem_flg = SEM_UNDO;
	if (semop(sem_id, &sem_b, 1) == -1)
	{
		fprintf(stderr, "semaphore_v failed.\n");
		return 0;
	}
	return 1;
}