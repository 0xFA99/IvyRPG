#ifndef IVY_AUDIO_OGG_H
#define IVY_AUDIO_OGG_H

#include "ivy/core/types.h"
#include "ivy/utils/forward.h"

#include "raylib/raylib.h"

#ifdef __cplusplus
extern "C" {
#endif

Music Ivy_Audio_LoadMusicOGG(IvyArenaLinear *restrict arena, IvyAssetManager *restrict assetManager, u32 id, usize vorbisArenaSize);
void  Ivy_Audio_UpdateMusicOGG(const Music *music);

#ifdef __cplusplus
}
#endif

#endif