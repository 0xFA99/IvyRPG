#include "ivy/arena/linear.h"

#include <stdlib.h>
#include <string.h>

void Ivy_Arena_LinearInit(IvyArenaLinear *arena, usize capacity)
{
    IVY_ASSERT(arena != NULL, "[Arena](Linear) Instance is NULL!");

    if (IVY_UNLIKELY(capacity == 0)) {
        capacity = LINEAR_DEFAULT_CAPACITY;
    }

    arena->buffer = (u8 *)malloc(capacity);
    IVY_CHECK(arena->buffer != NULL, "[Arena](Linear) Failed to allocate %zu bytes", capacity);

    arena->capacity = capacity;
    arena->offset   = 0;
    arena->owned    = true;
}

void Ivy_Arena_LinearInitStatic(IvyArenaLinear *restrict arena, void *restrict buffer, const usize size)
{
    IVY_ASSERT(arena != NULL, "[Arena](Linear) Static init with NULL arena!");
    IVY_ASSERT(buffer != NULL, "[Arena](Linear) Static init with NULL buffer!");

    IVY_CHECK(size > 0, "[Arena](Linear) Static init with zero size!", NULL);

    arena->buffer   = (u8 *)buffer;
    arena->capacity = size;
    arena->offset   = 0;
    arena->owned    = false;
}

void Ivy_Arena_LinearDestroy(IvyArenaLinear *arena)
{
    IVY_ASSERT(arena != NULL, "[Arena](Linear) Destroy NULL instance!");

    // only free if we own the buffer
    if (IVY_LIKELY(arena->owned && arena->buffer != NULL)) {
        free(arena->buffer);
    }

    memset(arena, 0, sizeof(IvyArenaLinear));
}

void Ivy_Arena_LinearReset(IvyArenaLinear *arena)
{
    IVY_ASSERT(arena != NULL, "[Arena](Linear) Reset NULL instance!");
    arena->offset = 0;
}

IvyArenaLinearSnapshot Ivy_Arena_LinearGetSnapshot(const IvyArenaLinear *arena)
{
    IVY_ASSERT(arena != NULL, "[Arena](Linear) GetSnapshot NULL instance!");
    return (IvyArenaLinearSnapshot){ .offset = arena->offset };
}

void Ivy_Arena_LinearRestore(IvyArenaLinear *arena, const IvyArenaLinearSnapshot snap)
{
    IVY_ASSERT(arena != NULL, "[Arena](Linear) Restore NULL instance!");

    IVY_ENSURE(snap.offset <= arena->capacity);

    // roll back
    IVY_ASSERT(snap.offset <= arena->offset,
               "[Arena](Linear) Restore point (%zu) is ahead of current offset (%zu)!",
               snap.offset, arena->offset);

    arena->offset = snap.offset;
}
