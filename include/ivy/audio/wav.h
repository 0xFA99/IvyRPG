#ifndef IVY_AUDIO_WAV_H
#define IVY_AUDIO_WAV_H

#include "ivy/core/types.h"
#include "ivy/arena/linear.h"
#include "ivy/utils/forward.h"

#include "raylib/raylib.h"

#ifdef __cplusplus
extern "C" {
#endif

struct IvySound {
    IvyArenaLinearSnapshot snap;
    Sound data;
};

IvySound    Ivy_Audio_LoadSoundWav(IvyArenaLinear *restrict arena, IvyAssetManager *restrict manager, u32 id);
void        Ivy_Audio_UnloadSound(const IvySound *sound);

#ifdef __cplusplus
}
#endif

#endif