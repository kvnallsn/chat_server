#ifndef THREAD_LIST
#define THREAD_LIST

#include <pthread.h>

struct t_list;

void t_list_init(struct t_list **lst);
void t_list_push_front(struct t_list *lst, pthread_t pid);
void t_list_destroy(struct t_list **lst, void (*stop)(pthread_t));
#endif
