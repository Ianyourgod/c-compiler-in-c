#ifndef VEC_H
#define VEC_H

#include <stdlib.h>
#include <stdio.h>

#define Option(T) struct {T data;int is_some;}

#define panic(...) {fprintf(stderr, __VA_ARGS__);exit(1);}

#define false 0
#define true 1

#define VEC(T) \
    struct { \
        T* data; \
        int length; \
        int capacity; \
    }

#define vec_push(vec, value) \
    if ((vec).length == (vec).capacity) { \
        (vec).capacity = (vec).capacity == 0 ? 1 : (vec).capacity * 2; \
        (vec).data = realloc((vec).data, sizeof(*(vec).data) * (vec).capacity); \
    } \
    (vec).data[(vec).length++] = value; \

#define vecptr_push(vec, value) \
    if ((vec)->length == (vec)->capacity) { \
        (vec)->capacity = (vec)->capacity == 0 ? 1 : (vec)->capacity * 2; \
        (vec)->data = realloc((vec)->data, sizeof(*(vec)->data) * (vec)->capacity); \
    } \
    (vec)->data[(vec)->length++] = value; \

#define vec_pop(vec) \
    (vec).data[(vec).length--]

#define vecptr_pop(vec) \
    (vec)->data[(vec)->length--]

#define vec_free(vec) \
    free((vec).data)

int quick_log10(int n);

#define malloc_type(T) (T*)malloc(sizeof(T))
#define malloc_n_type(T, n) (T*)malloc(sizeof(T) * n)

#endif