#ifndef _STDTYPE_H_
#define _STDTYPE_H_

#include "ump.h"
#include <stddef.h>

struct list_head{
	struct list_head *prev;
	struct list_head *next;
};

struct work_t{
    struct list_head node;
    int id;
    func_t func;
    void *arg;
    int argc;
	int arg_id;
    callback_t callback;
};

struct worker_t{
    pthread_t pid;
    struct list_head node;
    pthread_mutex_t mutex;
    pthread_cond_t empty;
};

//#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define container_of(ptr, type, member) ({              \
    void *__mptr = (void *)(ptr);                   \
    ((type *)(__mptr - offsetof(type, member))); })

#define LIST_HEAD_INIT(name) { &(name), &(name)

#define LIST_HEAD(name) \
    struct list_head name = LIST_HEAD_INIT(name)

static inline void __list_add(struct list_head *new,
                  struct list_head *prev,
                  struct list_head *next)
{   
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

static inline void list_add(struct list_head *new, struct list_head *head)
{
    __list_add(new, head, head->next);
}

static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
    __list_add(new, head->prev, head);
}

static inline void __list_del(struct list_head * prev, struct list_head * next)
{
    next->prev = prev;
    prev->next = next;
}

static inline void list_del(struct list_head *node)
{
	__list_del(node->prev, node->next);
}

static inline int list_is_head(const struct list_head *list, const struct list_head *head)
{
    return list == head;
}

#endif
