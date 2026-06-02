#include "ivy/arena/linear.h"
#include "ivy/audio/buffer.h"
#include "ivy/audio/ogg.h"
#include "ivy/audio/stream.h"
#include "ivy/core/types.h"
#include "ivy/systems/asset_manager.h"
#include "ivy/utils/forward.h"

#define STB_VORBIS_HEADER_ONLY
#include "external/stb_vorbis.c"

#include "external/miniaudio.h"
#include "raylib/raylib.h"

#include <string.h>

#define MUSIC_AUDIO_OGG     2
#define DEFAULT_BITRATE_OGG 16

Music Ivy_Audio_LoadMusicOGG(IvyArenaLinear *restrict arena, IvyAssetManager *restrict assetManager,
                             const u32 id, const usize vorbisArenaSize)
{
    Music music = {0};

    usize musicSize;
    const u8 *musicData = Ivy_Asset_Get(assetManager, id, &musicSize);

    stb_vorbis_alloc vorbisAlloc = {0};
    vorbisAlloc.alloc_buffer = Ivy_Arena_LinearAllocZero(arena, vorbisArenaSize);
    vorbisAlloc.alloc_buffer_length_in_bytes = (i32)vorbisArenaSize;
    IVY_ENSURE(vorbisAlloc.alloc_buffer != NULL);

    i32 error;
    stb_vorbis *ctx = stb_vorbis_open_memory(musicData, (i32)musicSize, &error, &vorbisAlloc);
    if (IVY_UNLIKELY(ctx == NULL)) return music;

    music.ctxType = MUSIC_AUDIO_OGG;
    music.ctxData = ctx;

    const stb_vorbis_info info = stb_vorbis_get_info(ctx);
    music.stream      = Ivy_Audio_LoadStream(arena, info.sample_rate, DEFAULT_BITRATE_OGG, info.channels);
    music.frameCount  = (u32)stb_vorbis_stream_length_in_samples(ctx);
    music.looping     = true;

    return music;
}

void Ivy_Audio_UpdateMusicOGG(const Music *music)
{
    IVY_ENSURE(music != NULL);
    if (IVY_UNLIKELY(!music->stream.buffer || !music->stream.buffer->playing)) return;

    IvyAudioData *audioData = Ivy_Audio_GetAudioData();

    ma_mutex_lock(&audioData->System.lock);

    const u32 subBufferFrames = music->stream.buffer->sizeInFrames / 2;
    const u32 frameSize       = music->stream.channels * (music->stream.sampleSize / 8);
    const u32 pcmSize         = subBufferFrames * frameSize;

    IVY_ASSERT(audioData->System.pcmBufferSize >= pcmSize,
               "PCM scratch too small: need %u, have %zu",
               pcmSize, audioData->System.pcmBufferSize);

    const bool bothProcessed = music->stream.buffer->isSubBufferProcessed[0] &&
                               music->stream.buffer->isSubBufferProcessed[1];

    if (IVY_UNLIKELY(bothProcessed)) {
        music->stream.buffer->frameCursorPos  = 0;
        music->stream.buffer->framesProcessed = 0;
    }

    for (u32 subBufferToUpdate = 0; subBufferToUpdate < 2; subBufferToUpdate++)
    {
        const u32 framesLeft = music->frameCount - music->stream.buffer->framesProcessed;
        const u32 framesToStream = (framesLeft >= subBufferFrames || music->looping) ? subBufferFrames : framesLeft;

        if (IVY_UNLIKELY(framesToStream == 0))
        {
            if (music->stream.buffer->isSubBufferProcessed[0] && music->stream.buffer->isSubBufferProcessed[1]) {
                Ivy_Audio_StopAudioBuffer(music->stream.buffer);
                stb_vorbis_seek_start((stb_vorbis *)music->ctxData);
            }

            break;
        }

        if (!music->stream.buffer->isSubBufferProcessed[subBufferToUpdate]) continue;

        i32 remaining = (i32)framesToStream;
        i32 readTotal = 0;
        while (remaining > 0) {
            const i32 read = stb_vorbis_get_samples_short_interleaved(
                (stb_vorbis *)music->ctxData,
                (i32)music->stream.channels,
                (short *)((u8 *)audioData->System.pcmBuffer + readTotal * frameSize),
                remaining * (i32)music->stream.channels
            );

            readTotal += read;
            remaining -= read;

            if (remaining > 0) stb_vorbis_seek_start((stb_vorbis *)music->ctxData);
        }

        u8 *subBuffer = music->stream.buffer->data + (subBufferFrames * frameSize * subBufferToUpdate);
        music->stream.buffer->framesProcessed += framesToStream;

        const u32 bytesToWrite = framesToStream * frameSize;
        memcpy(subBuffer, audioData->System.pcmBuffer, bytesToWrite);

        const u32 leftover = subBufferFrames - framesToStream;
        if (leftover > 0) {
            memset(subBuffer + bytesToWrite, 0, leftover * frameSize);
        }

        music->stream.buffer->isSubBufferProcessed[subBufferToUpdate] = false;
    }

    ma_mutex_unlock(&audioData->System.lock);
}
