#ifndef IVY_ALLOCATOR_POOL_H
#define IVY_ALLOCATOR_POOL_H

#include "ivy/types.h"

#include <stddef.h>
#include <stdbool.h>


typedef struct PoolFreeNode {
    struct PoolFreeNode *next;
} PoolFreeNode;

typedef struct PoolArena {
    u8              *buffer;
    size_t           capacity;
    size_t           chunk_size;
    size_t           count;
    PoolFreeNode    *free_head;
    size_t           num_free;
    bool             owned;
} ArenaPool;


int      ArenaPoolInit(ArenaPool *pool, size_t chunk_size, size_t chunk_count);
int      ArenaPoolInitStatic(ArenaPool *pool, void *buf, size_t buf_size, size_t chunk_size);
void     ArenaPoolDestroy(ArenaPool *pool);
void    *ArenaPoolAlloc(ArenaPool *pool);
void     ArenaPoolFree(ArenaPool *pool, void *ptr);
void     ArenaPoolReset(ArenaPool *pool);

bool     IsArenaPoolOwned(const ArenaPool *pool, const void *ptr);


#endif