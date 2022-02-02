#include "set.h"

struct Set_iterator *set_iterator_create(struct Set *set)
{
    struct Set_iterator *set_iterator = malloc(sizeof(struct Set_iterator));
    set_iterator->index = 0;
    set_iterator->set = set_copy(set);
    while (set_iterator_get(set_iterator) == false)
    {
        set_iterator->index += 1;
    }
    return set_iterator;
}

void set_iterator_next(struct Set_iterator *set_iterator)
{
    int result = set_iterator->index;
    if (result == 100)
    {
        return;
    }
    else
    {
        set_iterator->index += 1;
        while (set_iterator_get(set_iterator) == false)
        {
            set_iterator->index += 1;
        }
        return;
    }
}

short set_iterator_get(struct Set_iterator *set_iterator)
{
    return set_iterator->set->element[set_iterator->index];
}

void reset_iterator(struct Set_iterator *set_iterator)
{
    set_iterator->index = 0;
}

void set_init(struct Set *set)
{
    for (int i = 0; i < 100; i++)
    {
        set->element[i] = false;
    }
    set->max_size = 100;
    set->num = 0;
}

struct Set *set_create()
{
    struct Set *set = malloc(sizeof(struct Set));
    set_init(set);
    return set;
}

struct Set *set_copy(struct Set *set)
{
    // printf("set_copy started \n");
    struct Set *set_copied = malloc(sizeof(struct Set));
    set_copied->max_size = set->max_size;
    set_copied->num = set->num;
    for (int i = 0; i < set->max_size; i++)
    {
        set_copied->element[i] = set->element[i];
    }
    // printf("set_copy done \n");
    return set_copied;
}

void set_insert(struct Set *set, unsigned i)
{
    if (set->element[i] == false)
    {
        set->element[i] = true;
        set->num += 1;
    }
}

void set_delete(struct Set *set, unsigned i)
{
    if (set->element[i] == true)
    {
        set->element[i] = false;
        set->num -= 1;
    }
    return;
}

void set_print(struct Set *set)
{
    struct Set_iterator *set_iterator = set_iterator_create(set);
    while (set_iterator->index != 100)
    {
        if (set_iterator_get(set_iterator) == true)
            printf("%d ", set_iterator->index);
        set_iterator_next(set_iterator);
    }
    printf("\b");
    printf("\n");
}

short set_pop(struct Set *set)
{
    // printf("set pop begin\n");
    struct Set_iterator *set_iterator = set_iterator_create(set);
    while (set_iterator->index != 100)
    {
        // printf("%d\n", set_iterator->index);
        if (set_iterator_get(set_iterator) == true)
        {
            set_delete(set, set_iterator->index);
            return set_iterator->index;
        }
        set_iterator_next(set_iterator);
    }
    // printf("set pop done\n");
}

struct Set **set_split(struct Set *set, int num)
{
    // printf("split begin\n");
    int element_num = set->num;
    int num_per_subset = element_num / num;
    printf("num_per_subset %d\n", num_per_subset);
    int remainder = element_num - num * num_per_subset;
    int *set_size_list = malloc(num * sizeof(int));
    for (int i = 0; i < num; i++)
    {
        set_size_list[i] = num_per_subset;
    }
    for (int i = 0; i < remainder; i++)
    {
        set_size_list[i] += 1;
    }
    struct Set **set_list = malloc(num * sizeof(struct Set *));
    struct Set *set_copied = set_copy(set);
    for (int i = 0; i < num; i++)
    {
        // printf("i\n");
        set_list[i] = set_create();
        for (int j = 0; j < set_size_list[i]; j++)
        {
            // printf("%d\n", set_size_list[i]);
            // printf("%d\n", set->num);
            // printf("j\n");
            set_insert(set_list[i], set_pop(set_copied));
        }
    }
    // printf("%d\n", set->max_size);
    // printf("split done\n");
    return set_list;
}

void set_reset(struct Set *set)
{
    for (int i = 0; i < 100; i++)
    {
        set->element[i] = false;
    }
    set->num = 0;
}