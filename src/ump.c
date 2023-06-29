#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

#include <stdio.h>

#include "stdtype.h"

typedef struct{
	struct list_head work;
	int id;
	pthread_mutex_t mutex;
	pthread_cond_t empty;
	pthread_t pid;
} worker_t;

typedef struct{
	struct list_head node;
	int seq_number;
	func_t func;
	callback_t cb;
	void *data;
	int nr_data;
	int arg_id;
} work_t;

static struct{
	long ncpus;
	int idx;
	int seq_id;
	worker_t *worker; 
	struct list_head wList;
	pthread_mutex_t mutex;
}global;

static int alloc_work(void);

static void* take(void *arg)
{
	int count = 0;

	int i = (int)(long)arg;
	worker_t *worker = global.worker + i;

	while(1){
		struct list_head *node;

		count++;

		pthread_testcancel();

		pthread_mutex_t *mutex = &worker->mutex;
		pthread_cond_t *empty = &worker->empty;


		pthread_mutex_lock(mutex);
		if(list_is_head(worker->work.next, &worker->work)){
			pthread_cond_wait(empty, mutex);
		}

		node = worker->work.next;
		list_del(node);

		pthread_mutex_unlock(mutex);

		if(node == &worker->work)
			continue;

		work_t *work = container_of(node, work_t, node);
		if(work->cb)
			work->cb(work->func(work->data, work->nr_data, work->arg_id),work->arg_id);
		else
			work->func(work->data, work->nr_data, work->arg_id);

		pthread_mutex_lock(&global.mutex);
		list_add(node, &global.wList);
		pthread_mutex_unlock(&global.mutex);
	}

	return NULL;
}

int ump_init(void)
{
	int i, err;
	long ncpus;

	pthread_mutex_init(&global.mutex, NULL);

	ncpus = sysconf(_SC_NPROCESSORS_CONF);

	global.wList.prev = &global.wList;
	global.wList.next = &global.wList;
	global.idx = 0;
	global.ncpus = ncpus;

	err = alloc_work();
	if(err){
		return -ENOMEM;
	}

	worker_t *worker = malloc(sizeof(worker_t) * ncpus);
	if(!worker){
		err = -ENOMEM;
		goto err_worker;
	}

	global.worker = worker;
	for(i = 0; i < ncpus; i++){
		worker_t *pworker = worker + i;

		pthread_mutex_init(&pworker->mutex, NULL);
		pthread_cond_init(&pworker->empty, NULL);

		pworker->work.prev = &pworker->work;
		pworker->work.next = &pworker->work;

		err = pthread_create(&pworker->pid, NULL, take, (void*)(long)i);
		if(err)
			goto err_pcreate;
	}

	return 0;

err_pcreate:
	for(int j = 0; j < i; j++){
		worker_t *pworker = worker + j;
		pthread_cancel(pworker->pid);
	}

	for(int j = 0; j < i; j++){
		worker_t *pworker = worker + j;
		pthread_cond_signal(&worker->empty);
	}

	global.worker = NULL;

err_worker:
	return err;
}

int ump_exit(void)
{
	pthread_mutex_lock(&global.mutex);

	worker_t *worker = global.worker;
	for(int	i = 0; i < global.ncpus; i++){
		worker_t *pworker = worker + i;
		pthread_cancel(pworker->pid);
	}

	for(int i = 0; i < global.ncpus; i++){
		worker_t *pworker = worker + i;
		pthread_cancel(pworker->pid);
	}

	pthread_mutex_unlock(&global.mutex);
}

static int alloc_work(void)
{
	work_t *work = malloc(sizeof(work_t) * global.ncpus);
	if(!work)
		return -ENOMEM;
	
	pthread_mutex_lock(&global.mutex);

	for( int i = 0; i < global.ncpus; i++){
		work[i].node.prev = &work[i].node;
		work[i].node.next = &work[i].node;

		list_add(&work[i].node, &global.wList);
	}

	pthread_mutex_unlock(&global.mutex);

	return 0;
}

#define IS_NO_WORK()	list_is_head(global.wList.next, &global.wList)

static work_t* take_work(void)
{
	struct list_head *node;

	if(IS_NO_WORK()){
		if(alloc_work()){
			return NULL;
		}
	}

	pthread_mutex_lock(&global.mutex);
	node = global.wList.next;
	list_del(node);
	pthread_mutex_unlock(&global.mutex);

	work_t *work = container_of(node, work_t, node);

	return work;
}

int ump_set_func(func_t func, void *data, int nr_data, int id, callback_t cb)
{
	int idx;
	
	pthread_mutex_lock(&global.mutex);
	idx = global.idx++;
	pthread_mutex_unlock(&global.mutex);

	idx = idx % global.ncpus;
	worker_t *worker = global.worker + idx;
	work_t *work = take_work();	

	if(!work){
		return -ENOMEM;
	}

	work->func = func;
	work->data = data;
	work->nr_data = nr_data;
	work->arg_id = id;
	work->cb = cb;

	pthread_mutex_lock(&worker->mutex);
	list_add_tail(&work->node, &worker->work);
	pthread_cond_signal(&worker->empty);
	pthread_mutex_unlock(&worker->mutex);
	
	if(IS_NO_WORK()){
		if(alloc_work())
			printf("alloc_work error\n");
	}

	return 0;
}

/*
//test
void* func(void *data, int nr_data, int id)
{
	sleep(1);
	printf("this fun is %d\n", id);
	sleep(1);
	return NULL;
}

//test
int main(void)
{
	int i;

	ump_init();
	sleep(1);

	for(i = 0; i < 128; i++)
		ump_set_func(func, NULL, 0, i, NULL);

	sleep(100);

	ump_exit();
	return 0;
}
*/
