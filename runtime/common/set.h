#ifndef COMMON_SET
#define COMMON_SET
#include "stdbool.h"
#include "stdio.h"
#include "stdlib.h"

struct Set
{
    short element[100];
    unsigned num;
    int max_size;
};

void set_init(struct Set *set);
struct Set *set_create();
struct Set *set_copy(struct Set *set);

void set_insert(struct Set *set, unsigned i);

void set_delete(struct Set *set, unsigned i);
void set_print(struct Set *set);
int set_exist(struct Set *set, unsigned i);
short set_pop(struct Set *set);
struct Set_iterator
{
    struct Set *set;
    int index;
};

struct Set_iterator *set_iterator_create(struct Set *set);
void set_iterator_next(struct Set_iterator *set_iterator);
void reset_iterator(struct Set_iterator *set_iterator);
short set_iterator_get(struct Set_iterator *set_iterator);

struct Set **set_split(struct Set *set, int num);
void set_reset(struct Set *set);
#endif /* COMMON_SET */
