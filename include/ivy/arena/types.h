#ifndef IVY_ARENA_TYPES_H
#define IVY_ARENA_TYPES_H

#include "ivy/core/types.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LINEAR_DEFAULT_ALIGNMENT    16 // 8 / 16 / 32 / 64 Sheesshhhhh!!!!
#define LINEAR_DEFAULT_CAPACITY     (1024 * 1024)

IVY_INLINE bool Ivy_Arena_IsPowerOfTwo(const usize x)
{
    return x > 0 && (x & (x - 1)) == 0;
}

IVY_INLINE usize Ivy_Arena_AlignForward(const usize offset, const usize align)
{
    IVY_ASSERT(Ivy_Arena_IsPowerOfTwo(align), "[Arena] Alignment must be power of two.");
    return (offset + align - 1) & ~(align - 1);
}

#ifdef __cplusplus
}
#endif

#endif