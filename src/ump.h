#ifndef _UMP_H_
#define _UMP_H_

typedef void* (*func_t)(void *data, int nr_data, int id);
typedef void (*callback_t)(void *data, int id);

int ump_init(void);
int ump_exit(void);

int ump_set_func(func_t func, void *data, int nr_data, int id, callback_t cb);

#endif
