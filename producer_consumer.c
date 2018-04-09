#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include "shm_com.h"

static int set_semvalue(void);
static void del_semvalue(void);
static int semaphore_p(void);
static int semaphore_v(void);
static int sem_id;

int main(int argc, char const *argv[])
{
	int p_num=5;
	pid_t ret;
	printf("producer-consumer program starting...\n");

	/*建立信号量*/
	sem_id = semget((key_t)1234, 1, 0666|IPC_CREAT);
	if (!set_semvalue()) //初始化信号量
		{
			fprintf(stderr, "Failed to initialize semaphore\n");
			exit(EXIT_FAILURE);
		}

	/*建立共享内存*/
	void *shared_memory = (void*)0;
	struct shared_use_st *shared_stuff;
	int shmid;
	shmid = shmget((key_t)1234, sizeof(struct shared_use_st), 0666 | IPC_CREAT);
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

  // 子进程逻辑
  if(ret==0) {
  	/*连接到共享内存*/
  // 	shared_memory = shmat(shmid, (void*)0, 0);
		// if (shared_memory == (void*)-1) {
		// 	fprintf(stderr, "shmat failed\n");
		// 	exit(EXIT_FAILURE);	
		// }
		// shared_stuff = (struct shared_use_st *)shared_memory;

		while(1) {
			if(!semaphore_p()) exit(EXIT_FAILURE);
			/*临界操作*/
				shared_stuff->in++;
				printf("child[%d], parent[%d], %d\n", getpid(), getppid(), shared_stuff->in);
			if(!semaphore_v()) exit(EXIT_FAILURE);
		}
  }

	/*父进程逻辑*/
  if (ret > 0)
  {
  	
  }
}

static int set_semvalue(void)
{
	union semun sem_union;

	sem_union.val = 1;
	if (semctl(sem_id, 0, SETVAL, sem_union) == -1) return 0;
	return 1;
}

static void del_semvalue(void)
{
	union semun sem_union;
	if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1)
		fprintf(stderr, "Failed to delete semaphore\n");
}

static int semaphore_p(void)
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

static int semaphore_v(void)
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





