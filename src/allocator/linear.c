#include "ivy/arena/linear.h"

#include <assert.h>
#include <stdlib.h>


static int IsPowerOfTwo(const size_t x)
{
    return x > 0 && (x & x - 1) == 0;
}

static size_t AlignForward(const size_t offset, const size_t align)
{
    assert(IsPowerOfTwo(align));
    return offset + align - 1 & ~(align - 1);
}

bool ArenaLinearInit(ArenaLinear *arena, size_t capacity)
{
    assert(arena != NULL);
    if (capacity == 0) {
        capacity = LINEAR_DEFAULT_CAPACITY;
    }

    arena->buffer = (u8 *)malloc(capacity);
    if (arena->buffer == NULL) return false;

    arena->capacity = capacity;
    arena->offset   = 0;
    arena->owned    = true;
    return true;
}

void *ArenaLinearAlloc(ArenaLinear *arena, const size_t size)
{
    return ArenaLinearInitAlign(arena, size, LINEAR_DEFAULT_ALIGNMENT);
}

void *ArenaLinearInitAlign(ArenaLinear *arena, const size_t size, const size_t align)
{
    assert(arena != NULL);
    assert(IsPowerOfTwo(align));

    if (arena->buffer == NULL) return NULL;
    if (size == 0) return NULL;

    const size_t aligned_offset = AlignForward(arena->offset, align);

    if (aligned_offset > arena->capacity || size > arena->capacity - aligned_offset) {
        return NULL;
    }

    const size_t new_offset = aligned_offset + size;
    arena->offset = new_offset;

    return arena->buffer + aligned_offset;
}

void ArenaLinearDestroy(ArenaLinear *arena)
{
    assert(arena != NULL);
    if (arena->owned && arena->buffer != NULL) free(arena->buffer);

    arena->buffer   = NULL;
    arena->capacity = 0;
    arena->offset   = 0;
    arena->owned    = false;
}

ArenaLinearSnapshot ArenaLinearGetSnapshot(const ArenaLinear *arena)
{
    assert(arena != NULL);

    ArenaLinearSnapshot snap;
    snap.offset = arena->offset;

    return snap;
}

void ArenaLinearInitStatic(ArenaLinear *arena, void *buffer, const size_t size)
{
    assert(arena != NULL);
    assert(buffer != NULL);
    assert(size > 0);

    arena->buffer   = (u8 *)buffer;
    arena->capacity = size;
    arena->offset   = 0;
    arena->owned    = false;
}

void ArenaLinearReset(ArenaLinear *arena)
{
    assert(arena != NULL);
    arena->offset = 0;
}