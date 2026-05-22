#ifndef IVY_ARENA_LINEAR_H
#define IVY_ARENA_LINEAR_H

#include "ivy/core/types.h"
#include "ivy/arena/types.h"
#include "ivy/utils/forward.h"

#include <string.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef IVY_DEBUG
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct IvyArenaLinear {
    u8      *buffer;        // 8
    usize    capacity;      // 8
    usize    offset;        // 8
    bool     owned;         // 1
    u8       padding[7];    // 7
};                          // 32
IVY_ASSERT_STATIC(sizeof(IvyArenaLinear) == 32, "[Arena](Linear) Size must be 32 bytes!");

struct IvyArenaLinearSnapshot {
    usize    offset;        // 8
};                          // 8
IVY_ASSERT_STATIC(sizeof(IvyArenaLinearSnapshot) == 8, "[Arena](Linear) Snapshot size must be 8 bytes!");

void                    Ivy_Arena_LinearInit(IvyArenaLinear *arena, usize capacity);
void                    Ivy_Arena_LinearDestroy(IvyArenaLinear *arena);
void                    Ivy_Arena_LinearInitStatic(IvyArenaLinear *restrict arena, void *restrict buffer, usize size);
void                    Ivy_Arena_LinearReset(IvyArenaLinear *arena);

IvyArenaLinearSnapshot  Ivy_Arena_LinearGetSnapshot(const IvyArenaLinear *arena);
void                    Ivy_Arena_LinearRestore(IvyArenaLinear *arena, IvyArenaLinearSnapshot snap);

IVY_INLINE usize Ivy_Arena_LinearAvailable(const IvyArenaLinear *arena)
{
    return arena->capacity - arena->offset;
}

IVY_INLINE void *Ivy_Arena_LinearAllocAlign(IvyArenaLinear *arena, const usize size, const usize align)
{
    IVY_ASSERT(arena != NULL, "[Arena] Instance is NULL!");

    if (IVY_UNLIKELY(size == 0)) return NULL;

    // bump the offset forward to meet alignment.
    const usize aligned_offset = Ivy_Arena_AlignForward(arena->offset, align);

    // OOM: not enough contiguous space left.
    if (IVY_UNLIKELY(aligned_offset + size > arena->capacity)) {
        IVY_ASSERT(false, "[Arena] OOM! Req: %zu, Cap: %zu", aligned_offset + size, arena->capacity);
        return NULL;
    }

    void *ptr = arena->buffer + aligned_offset;
    arena->offset = aligned_offset + size;  // commit the bump.

    return ptr;
}

IVY_INLINE void *Ivy_Arena_LinearAlloc(IvyArenaLinear *arena, const usize size)
{
    return Ivy_Arena_LinearAllocAlign(arena, size, LINEAR_DEFAULT_ALIGNMENT);
}

IVY_INLINE void *Ivy_Arena_LinearAllocZero(IvyArenaLinear *arena, const usize size)
{
    void *ptr = Ivy_Arena_LinearAlloc(arena, size);
    if (IVY_LIKELY(ptr)) memset(ptr, 0, size);
    return ptr;
}

IVY_INLINE void Ivy_Arena_LinearDebugPrint(const IvyArenaLinear *restrict arena, const char *restrict label)
{
#ifdef IVY_DEBUG
    const usize used      = arena->offset;
    const usize remaining = arena->capacity - arena->offset;
    const float  pct       = (float)used / (float)arena->capacity * 100.0f;

    fprintf(stderr, "[Arena] %s | used: %zu B / %zu B (%.1f%%) | remaining: %zu B\n",
        label ? label : "?", used, arena->capacity, pct, remaining);
#else
    (void)arena; (void)label;
#endif
}

#ifdef __cplusplus
}
#endif

#endif