#include "ivy/core/types.h"
#include "ivy/utils/utils.h"

#define IVY_RANDOM_SEED 0x243F6A88

static u32 _ivyUtils_randomU32(void)
{
    static u32 state = IVY_RANDOM_SEED;

    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;

    return state;
}

u32 Ivy_Utils_RandomRange(const u32 min, const u32 max)
{
    return min + (_ivyUtils_randomU32() % (max - min + 1));
}
