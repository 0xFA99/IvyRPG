#include "ivy/arena/pool.h"

#include <stdlib.h>
#include <assert.h>

#define MIN_CHUNK_SIZE  sizeof(PoolFreeNode)

static size_t AlignUp(const size_t size, const size_t align)
{
    return size + align - 1 & ~(align - 1);
}

static void pool_build_freelist(ArenaPool *pool)
{
    pool->free_head = NULL;
    pool->num_free  = 0;

    for (size_t i = pool->count; i > 0; --i)
    {
        u8 *slot        = pool->buffer + (i - 1) * pool->chunk_size;
        PoolFreeNode *node = (PoolFreeNode *)slot;
        node->next        = pool->free_head;
        pool->free_head   = node;
        pool->num_free++;
    }
}

int ArenaPoolInit(ArenaPool *pool, size_t const chunk_size, const size_t chunk_count)
{
    assert(pool        != NULL);
    assert(chunk_size  >  0);
    assert(chunk_count >  0);

    size_t real_chunk = chunk_size < MIN_CHUNK_SIZE ? MIN_CHUNK_SIZE : chunk_size;

    real_chunk = AlignUp(real_chunk, sizeof(void *));

    const size_t total = real_chunk * chunk_count;
    pool->buffer = (u8 *)malloc(total);
    if (!pool->buffer) return -1;

    pool->capacity   = total;
    pool->chunk_size = real_chunk;
    pool->count      = chunk_count;
    pool->owned      = true;

    pool_build_freelist(pool);
    return 0;
}

int ArenaPoolInitStatic(ArenaPool *pool, void *buf, const size_t buf_size, const size_t chunk_size)
{
    assert(pool       != NULL);
    assert(buf        != NULL);
    assert(buf_size   >  0);
    assert(chunk_size >  0);

    size_t real_chunk = chunk_size < MIN_CHUNK_SIZE ? MIN_CHUNK_SIZE : chunk_size;
    real_chunk = AlignUp(real_chunk, sizeof(void *));

    const size_t chunk_count = buf_size / real_chunk;
    if (chunk_count == 0) return -1;

    pool->buffer     = (u8 *)buf;
    pool->capacity   = chunk_count * real_chunk;
    pool->chunk_size = real_chunk;
    pool->count      = chunk_count;
    pool->owned      = false;

    pool_build_freelist(pool);
    return 0;
}

void ArenaPoolDestroy(ArenaPool *pool)
{
    assert(pool != NULL);
    if (pool->owned && pool->buffer) free(pool->buffer);

    pool->buffer     = NULL;
    pool->capacity   = 0;
    pool->chunk_size = 0;
    pool->count      = 0;
    pool->free_head  = NULL;
    pool->num_free   = 0;
    pool->owned      = false;
}

void *ArenaPoolAlloc(ArenaPool *pool)
{
    assert(pool != NULL);

    if (!pool->free_head) return NULL;

    PoolFreeNode *node  = pool->free_head;
    pool->free_head     = node->next;
    pool->num_free--;

    return node;
}

void ArenaPoolFree(ArenaPool *pool, void *ptr)
{
    assert(pool != NULL);
    assert(ptr  != NULL);
    assert(IsArenaPoolOwned(pool, ptr));

    PoolFreeNode *node = ptr;
    node->next         = pool->free_head;
    pool->free_head    = node;
    pool->num_free++;
}

void ArenaPoolReset(ArenaPool *pool)
{
    assert(pool != NULL);
    pool_build_freelist(pool);
}

bool IsArenaPoolOwned(const ArenaPool *pool, const void *ptr)
{
    if (!pool || !ptr || !pool->buffer) return false;

    const u8 *p     = ptr;
    const u8 *start = pool->buffer;
    const u8 *end   = pool->buffer + pool->capacity;

    if (p < start || p >= end) return false;

    const size_t offset = (size_t)(p - start);
    return offset % pool->chunk_size == 0;
}