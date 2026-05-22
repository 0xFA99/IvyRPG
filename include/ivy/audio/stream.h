#ifndef IVY_AUDIO_STREAM_H
#define IVY_AUDIO_STREAM_H

#include "ivy/core/types.h"
#include "ivy/arena/linear.h"

#include "raylib/raylib.h"

#ifdef __cplusplus
extern "C" {
#endif

AudioStream Ivy_Audio_LoadStream(IvyArenaLinear *arena, u32 sampleRate, u32 sampleSize, u32 channels);

#ifdef __cplusplus
}
#endif

#endif