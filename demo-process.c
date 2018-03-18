#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
	pid_t pid;
if((pid=fork())==0)
{
//子进程代码
  	char command[81]; 
  	int i;
  	for (i=1;i<8;i++)
 	{
  		sprintf(command,"echo $USER");
  		
  		system(command);
  	}
 	printf("子进程%d\n", pid);
 	exit(0);
}
else if(pid>0)
{
  	//父进程代码
  		printf("父进程%d\n", pid);
  		exit(0); 
}
else {
  printf("Error");
  exit(1);
}
}
