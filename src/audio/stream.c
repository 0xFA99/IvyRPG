#include "ivy/audio/buffer.h"
#include "ivy/audio/stream.h"
#include "ivy/utils/forward.h"
#include "ivy/core/types.h"

#define STB_VORBIS_HEADER_ONLY
#include "external/stb_vorbis.c"
#include "external/miniaudio.h"

#include "raylib/raylib.h"

#include <stdbool.h>

AudioStream Ivy_Audio_LoadStream(IvyArenaLinear *arena, const u32 sampleRate,
                                 const u32 sampleSize, const u32 channels)
{
    AudioStream stream = {0};
    stream.sampleRate  = sampleRate;
    stream.sampleSize  = sampleSize;
    stream.channels    = channels;

    const IvyAudioData *audioData = Ivy_Audio_GetAudioData();

    const ma_format formatIn = (stream.sampleSize == 8)  ? ma_format_u8  :
                               (stream.sampleSize == 16) ? ma_format_s16 : ma_format_f32;

    const u32 periodSize           = audioData->System.device.playback.internalPeriodSizeInFrames;
    const ma_format deviceFormat   = audioData->System.device.playback.format;
    const u32 deviceChannels       = audioData->System.device.playback.channels;
    const u32 bytesPerDeviceFrame  = (u32)ma_get_bytes_per_frame(deviceFormat, deviceChannels);

    u32 subBufferSize = (audioData->Buffer.defaultSize == 0)
                      ? (audioData->System.device.sampleRate / 30 * bytesPerDeviceFrame)
                      : (u32)audioData->Buffer.defaultSize;

    if (IVY_UNLIKELY(subBufferSize < periodSize)) subBufferSize = periodSize;

    stream.buffer = Ivy_Audio_LoadBuffer(arena, formatIn, stream.channels,
                                         stream.sampleRate, subBufferSize * 2,
                                         AUDIO_BUFFER_USAGE_STREAM);

    if (IVY_LIKELY(stream.buffer != NULL)) {
        stream.buffer->looping = true;
    }

    return stream;
}

void Ivy_Audio_UnloadStream(const Music *music)
{
    if (IVY_UNLIKELY(!music)) return;

    IvyAudioData *audioData = Ivy_Audio_GetAudioData();
    IvyAudioBuffer *buffer  = music->stream.buffer;

    if (IVY_LIKELY(buffer != NULL)) {
        ma_mutex_lock(&audioData->System.lock);
            Ivy_Audio_StopAudioBuffer(buffer);
        ma_mutex_unlock(&audioData->System.lock);

        Ivy_Audio_UnloadBuffer(buffer);
    }

    if (music->ctxData) {
        stb_vorbis_close((stb_vorbis *)music->ctxData);
    }
}
