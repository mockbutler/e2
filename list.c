#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct list {
    struct list* prev;
    struct list* next;
};

#define LIST_INIT_HEAD(hd) \
    ((hd)->prev = (hd), (hd)->next = (hd))

struct num {
    int i;
    struct list l;
};

#define container_of(p, t, m) \
    ((t*)((void*)p - offsetof(t, m)))

void list_append(struct list* head, struct list* elem)
{
    elem->next = head->next;
    elem->prev = head;
    head->prev->next = elem;
    head->next = elem;
}

struct num* new_num(int i)
{
    struct num* n;
    n = malloc(sizeof(*n));
    n->i = i;
    LIST_INIT_HEAD(&n->l);
    return n;
}

int main(void)
{
    struct num *hd, *tmp;
    struct list* i;
    hd = new_num(1);
    tmp = new_num(2);
    list_append(&hd->l, &tmp->l);
    for (i = &hd->l; i != &hd->l; i = i->next) {
        tmp = container_of(i, struct num, l);
        printf("%d\n", tmp->i);
    }
    exit(0);
}
