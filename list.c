
#include "list.h"
#include <stdlib.h>

struct t_list {
    pthread_t pid;
    struct t_list *nxt;
};


void t_list_init(struct t_list **lst)
{
    *lst = malloc(sizeof(**lst));
    (*lst)->nxt = NULL;
}

void t_list_push_front(struct t_list *t_lst, pthread_t pid)
{
    struct t_list *nlst;
    
    nlst = malloc(sizeof(*nlst));
    nlst->pid = pid;
    nlst->nxt = t_lst->nxt;
    t_lst->nxt = nlst;
}

void t_list_destroy(struct t_list **lst, void (*stop)(pthread_t))
{

    struct t_list *iter;

    iter = (*lst)->nxt;
    while (iter != NULL) {
        struct t_list *n;

        stop(iter->pid);

        n = iter->nxt;
        free(iter);
        iter = n;
    }

    free((*lst));
    *lst = NULL;
}
