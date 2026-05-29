#include "ivy/audio/ogg.h"
#include "ivy/audio/buffer.h"
#include "ivy/audio/stream.h"
#include "ivy/systems/asset_manager.h"

#define STB_VORBIS_HEADER_ONLY
#include "external/stb_vorbis.c"

#include "external/miniaudio.h"
#include "raylib/raylib.h"

#include <string.h>
#include <stdlib.h>

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
    vorbisAlloc.alloc_buffer_length_in_bytes = (int)vorbisArenaSize;
    IVY_ENSURE(vorbisAlloc.alloc_buffer != NULL);

    int error;
    stb_vorbis *ctx = stb_vorbis_open_memory(musicData, (int)musicSize, &error, &vorbisAlloc);
    if (ctx == NULL) return music;

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
    if (!music->stream.buffer || !music->stream.buffer->playing) return;

    IvyAudioData *audioData = Ivy_Audio_GetAudioData();

    ma_mutex_lock(&audioData->System.lock);

    const u32 subBufferFrames = music->stream.buffer->sizeInFrames / 2;
    const int frameSize       = (int)(music->stream.channels * music->stream.sampleSize / 8);
    const u32 pcmSize         = subBufferFrames * frameSize;

    /* TODO: replace with arena allocation to avoid heap alloc on audio thread */
    if (audioData->System.pcmBufferSize < pcmSize) {
        RL_FREE(audioData->System.pcmBuffer);
        audioData->System.pcmBuffer     = RL_CALLOC(1, pcmSize);
        audioData->System.pcmBufferSize = pcmSize;
    }

    const bool bothProcessed = music->stream.buffer->isSubBufferProcessed[0] &&
                               music->stream.buffer->isSubBufferProcessed[1];

    if (bothProcessed) {
        music->stream.buffer->frameCursorPos  = 0;
        music->stream.buffer->framesProcessed = 0;
    }

    for (int i = 0; i < 2; i++) {
        const u32 framesLeft = music->frameCount - music->stream.buffer->framesProcessed;
        const u32 framesToStream = (framesLeft >= subBufferFrames || music->looping)
            ? subBufferFrames : framesLeft;

        if (framesToStream == 0) {
            if (music->stream.buffer->isSubBufferProcessed[0] &&
                music->stream.buffer->isSubBufferProcessed[1])
            {
                Ivy_Audio_StopAudioBuffer(music->stream.buffer);
                stb_vorbis_seek_start((stb_vorbis *)music->ctxData);
            }
            break;
        }

        const u32 subBufferToUpdate = (u32)i;
        if (!music->stream.buffer->isSubBufferProcessed[subBufferToUpdate]) continue;

        int remaining = (int)framesToStream;
        int readTotal = 0;
        while (remaining > 0) {
            const int read = stb_vorbis_get_samples_short_interleaved(
                (stb_vorbis *)music->ctxData,
                (int)music->stream.channels,
                (short *)((char *)audioData->System.pcmBuffer + readTotal * frameSize),
                (int)(remaining * music->stream.channels)
            );
            readTotal += read;
            remaining -= read;
            if (remaining > 0) stb_vorbis_seek_start((stb_vorbis *)music->ctxData);
        }

        u8 *subBuffer = music->stream.buffer->data + (subBufferFrames * frameSize * subBufferToUpdate);
        music->stream.buffer->framesProcessed += framesToStream;

        const ma_uint32 bytesToWrite = framesToStream * frameSize;
        memcpy(subBuffer, audioData->System.pcmBuffer, bytesToWrite);

        const u32 leftover = subBufferFrames - framesToStream;
        if (leftover > 0) memset(subBuffer + bytesToWrite, 0, leftover * frameSize);

        music->stream.buffer->isSubBufferProcessed[subBufferToUpdate] = false;
    }

    ma_mutex_unlock(&audioData->System.lock);
}
