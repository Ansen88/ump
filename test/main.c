#include "ump.h"
#include <sys/time.h>
#include <stdio.h>

#define DELAY	10000000
#define TIMES	16

static long number[TIMES];

long routine0(void)
{
	long i;

	for(i = 0; i <  ((long)TIMES * DELAY); i++){
	}

	return i;
}

void* func(void *data, int nr_data, int id)
{
	int i;
	int nr = *((int*)data + 1);

	for(i = 0; i < nr; i++){
	}

	number[id] = i;
	
	return NULL;
}

long routine1(void)
{
	long sum = 0;
	
	int arg[2];
	arg[1] = DELAY;

	ump_init();

	for(int i = 0; i < TIMES; i++){
		number[i] = -1;
		arg[0] = i;
		ump_set_func(func, &arg, 1, i, NULL);
	}

	for(int i = 0; i < TIMES; i++){
		while(number[i] == -1);

		sum += number[i];
	}

	return sum;
}

int main(int argc, char** argv)
{
	struct timeval tv;
	unsigned long time;

	gettimeofday (&tv , NULL);
	time = tv.tv_sec * 1000 + tv.tv_usec;

	if(argc == 1){
		printf("routine0 is %lu\n", routine0());
	}else{
		printf("routine1 is %lu\n", routine1());
	}
	
	gettimeofday (&tv , NULL);
	time = tv.tv_sec * 1000 + tv.tv_usec - time;

	printf("delay is %lu\n", time);
	return 0;
}

