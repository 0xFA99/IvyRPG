#include "ivy/arena/linear.h"
#include "ivy/game.h"

#include <stdlib.h>


static int IsPowerOfTwo(const size_t x)
{
    return x > 0 && (x & x - 1) == 0;
}

static size_t AlignForward(const size_t offset, const size_t align)
{
    IVY_ASSERT(IsPowerOfTwo(align), "alignment must be power of two");
    return offset + align - 1 & ~(align - 1);
}

bool ArenaLinearInit(ArenaLinear *arena, size_t capacity)
{
    IVY_ASSERT(arena != NULL, "arena is NULL");
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
    IVY_ASSERT(arena != NULL, "arena is NULL");
    IVY_ASSERT(IsPowerOfTwo(align), "alignment must be power of two");

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
    IVY_ASSERT(arena != NULL, "arena is NULL");
    if (arena->owned && arena->buffer != NULL) free(arena->buffer);

    arena->buffer   = NULL;
    arena->capacity = 0;
    arena->offset   = 0;
    arena->owned    = false;
}

ArenaLinearSnapshot ArenaLinearGetSnapshot(const ArenaLinear *arena)
{
    IVY_ASSERT(arena != NULL, "arena is NULL");

    ArenaLinearSnapshot snap;
    snap.offset = arena->offset;

    return snap;
}

void ArenaLinearInitStatic(ArenaLinear *arena, void *buffer, const size_t size)
{
    IVY_ASSERT(arena != NULL, "arena is NULL");
    IVY_ASSERT(buffer != NULL, "buffer is NULL");
    IVY_ASSERT(size > 0, "size must be > 0");

    arena->buffer   = (u8 *)buffer;
    arena->capacity = size;
    arena->offset   = 0;
    arena->owned    = false;
}

void ArenaLinearReset(ArenaLinear *arena)
{
    IVY_ASSERT(arena != NULL, "arena is NULL");
    arena->offset = 0;
}