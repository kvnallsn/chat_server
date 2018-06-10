/**
 * file: test.c
 *
 * Test driver for generic_list.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "generic_list.h"

void print_long_item(void *data, void *param);
void long_test(void);
void deallocate_user(void *p);
int compare_user(void *lhs, void *rhs);
void print_user(void *data, void *param);
void struct_test(void);

void print_long_item(void *data, void *param)
{
    unsigned long v;

    v = (unsigned long)data;
    printf("%lu\n", v);
}

void long_test()
{
    struct list *lst;
    unsigned long value;

    list_init(&lst, NULL, NULL);

    list_add(lst, (void*)1L);
    list_add(lst, (void*)2L);
    list_add(lst, (void*)4L);

    printf("Getting 3rd item...");
    value = (unsigned long) list_get(lst, 2);
    printf("%s\n", value == 1 ? "Pass" : "Fail");

    list_remove(lst, (void*)2L);

    printf("Getting 3rd item...");
    value = (unsigned long) list_get(lst, 2);
    printf("%s\n", value == 0 ? "Pass" : "Fail");

    list_destroy(&lst);
}

struct user {
    int s;
    char name[28];
};

void deallocate_user(void *p)
{
    struct user *u;
    u = (struct user*)p;

    free(u);
}

int compare_user(void *lhs, void *rhs)
{
    int urhs;
    struct user *ulhs;
    ulhs = (struct user*)lhs;
    urhs = (int)rhs;

    return (ulhs->s < urhs ? -1 :
            (ulhs->s == urhs ? 0 : 1));
}

void print_user(void *data, void *param)
{

    struct user *u;
    u = (struct user*)data;

    printf("%s: %d\n", u->name, u->s);
}

void struct_test()
{
    struct list *lst;
    struct user *u1, *u2, *u3, *s1, *s2;

    list_init(&lst, deallocate_user, compare_user);

    u1 = malloc(sizeof(*u1));
    u2 = malloc(sizeof(*u2));
    u3 = malloc(sizeof(*u3));

    u1->s = 1; strcpy(u1->name, "Mark");
    u2->s = 2; strcpy(u2->name, "Ron");
    u3->s = 4; strcpy(u3->name, "Trike");

    list_add(lst, u1);
    list_add(lst, u2);
    list_add(lst, u3);

    printf("\n");
    list_for_each(lst, NULL, print_user);

    printf("Searching for user-id 2...");
    s1 = list_find(lst, (void*)2); 
    printf("%s\n", s1 ? "Found" : "Not Found");

    printf("Searching for user-id 3...");
    s2 = list_find(lst, (void*)3);
    printf("%s\n", s2 ? "Found" : "Not Found");

    list_remove(lst, s1);

    printf("\n");
    list_for_each(lst, NULL, print_user);

    list_destroy(&lst);
}

int main()
{   
    printf("=== Unsigned Long Test Start ===\n");
    long_test();
    printf("=== Unsigned Long Test End   ===\n\n");

    printf("=== Struct Test Start ===\n");
    struct_test();
    printf("=== Struct Test End   ===\n\n");

    return 0;
}
