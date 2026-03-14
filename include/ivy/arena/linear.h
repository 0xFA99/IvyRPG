#ifndef IVY_ARENA_LINEAR_H
#define IVY_ARENA_LINEAR_H

#define LINEAR_DEFAULT_ALIGNMENT    (sizeof(void *))
#define LINEAR_DEFAULT_CAPACITY     (1024 * 1024)

#include "ivy/types.h"

#include <stdbool.h>
#include <stddef.h>


typedef struct {
    u8      *buffer;
    size_t   capacity;
    size_t   offset;
    bool     owned;
} ArenaLinear;

typedef struct {
    size_t  offset;
} ArenaLinearSnapshot;

bool     ArenaLinearInit(ArenaLinear *arena, size_t capacity);
void    *ArenaLinearAlloc(ArenaLinear *arena, size_t size);
void    *ArenaLinearInitAlign(ArenaLinear *arena, size_t size, size_t align);
void     ArenaLinearDestroy(ArenaLinear *arena);

ArenaLinearSnapshot ArenaLinearGetSnapshot(const ArenaLinear *arena);

void     ArenaLinearInitStatic(ArenaLinear *arena, void *buffer, size_t size);
void     ArenaLinearReset(ArenaLinear *arena);


#endif