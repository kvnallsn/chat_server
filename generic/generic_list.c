/**
 * file: list.c
 *
 * Represents a list
 */

#include <stdlib.h>
#include <string.h>

#include "generic_list.h"

/** Type defintions **/

struct node {
    struct node *prev;
    struct node *next;
    void *data;
};

struct list {
    struct node *head;
    void (*deallocate)(void *p);
    int (*compare)(void *lhs, void *rhs);
};

/** Default list functions **/

static void default_deallocate(void *p)
{
    /* Default deallocate does nothing */
}

/**
 * Comparator function for our list.
 * 
 * \returns
 *      -1: If lhs < rhs
 *       0: If lhs == rhs
 *       1: if lhs > rhs
 */
static int default_compare(void *lhs, void *rhs)
{
    return (lhs < rhs ? -1 :
            (lhs == rhs ? 0 : 1));
}


/** List functions **/

void list_init(struct list **lst, void (*deallocate)(void *p),
        int (compare)(void *lhs, void *rhs))
{
    struct list *l;
    if (lst == NULL)
        return;

    l = calloc(1, sizeof(struct list));
    if (l == NULL)
        return;

    l->deallocate = deallocate ? deallocate : default_deallocate;
    l->compare = compare ? compare : default_compare;
    *lst = l;
}

void list_add(struct list *lst, void *data)
{
    struct node *new_user;

    if (lst == NULL)
        return;

    new_user = calloc(1, sizeof(*new_user));
    if (new_user == NULL)
        return;

    new_user->data = data;
    new_user->prev = NULL;
    new_user->next = lst->head;
    if (lst->head != NULL) {
        lst->head->prev = new_user;
    }
    lst->head = new_user;
}

void* list_get(struct list *lst, int idx)
{
    int i;
    struct node *iter;
    
    if (lst == NULL)
        return NULL;

    iter = lst->head;

    for (i = 0; i++ < idx && iter != NULL; iter = iter->next)
        ;

    return iter ? iter->data : NULL;
}

void *list_find(struct list *lst, void *item)
{
    struct node *iter;

    if (lst == NULL)
        return NULL;
   
    for (iter = lst->head; iter != NULL; iter = iter->next) {
        if (lst->compare(iter->data, item) == 0)
            return iter->data;
    }

    return NULL;
}

void list_for_each(struct list *lst,
        void *param,
        void (*fn)(void *data, void *param))
{
    struct node *iter;

    if (lst == NULL)
        return;

    iter = lst->head;
    while (iter != NULL) {
        fn(iter->data, param);
        iter = iter->next;
    }
}

void list_remove(struct list *lst, void *p)
{
    struct node *iter;

    if (lst == NULL)
        return;

    for (iter = lst->head; iter != NULL; iter = iter->next) {
        if (iter->data == p) {
            /* remove this node */

            if (iter->prev)
                iter->prev->next = iter->next;

            if (iter->next)
                iter->next->prev = iter->prev;

            if (lst->head)
                lst->head = iter->next;

            lst->deallocate(iter->data);
            free(iter);
            break;
        }
    }
}

void list_destroy(struct list **lst)
{
    struct node *iter;

    if (lst == NULL || *lst == NULL)
        return;

    for (iter = (*lst)->head; iter != NULL; ) {
        struct node *tmp;
        tmp = iter->next;
        (*lst)->deallocate(iter->data);
        free(iter);
        iter = tmp;
    }

    free(*lst);
    *lst = NULL;
}
