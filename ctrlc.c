#include <signal.h>
#include <stdio.h>
#include <unistd.h>

void ouch(int sig)
{
	printf("OUCH!! I got a signal %d\n", sig);
}

int main(int argc, char const *argv[])
{
	struct sigaction act;

	act.sa_handler = ouch;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	sigaction(SIGINT, &act, 0);

	while(1)
	{
		printf("Hello World!\n");
		sleep(1);
	}
	return 0;
}