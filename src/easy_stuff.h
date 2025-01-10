#ifndef VEC_H
#define VEC_H

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

#define vec_free(vec) \
    free((vec).data)

int quick_log10(int n);

#endif