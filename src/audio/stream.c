#include "ivy/audio/buffer.h"
#include "ivy/audio/stream.h"
#include "ivy/audio/wav.h"
#include "ivy/systems/scene_manager.h"
#include "ivy/utils/forward.h"

#define STB_VORBIS_HEADER_ONLY
#include "external/stb_vorbis.c"

AudioStream Ivy_Audio_LoadStream(IvyArenaLinear *arena, const unsigned int sampleRate,
                                 const unsigned int sampleSize, const unsigned int channels)
{
    AudioStream stream = {0};
    stream.sampleRate  = sampleRate;
    stream.sampleSize  = sampleSize;
    stream.channels    = channels;

    const IvyAudioData *audioData = Ivy_Audio_GetAudioData();

    const ma_format formatIn = (stream.sampleSize == 8)  ? ma_format_u8  :
                               (stream.sampleSize == 16) ? ma_format_s16 : ma_format_f32;

    const unsigned int periodSize = audioData->System.device.playback.internalPeriodSizeInFrames;

    int deviceBitsPerSample = audioData->System.device.playback.format;
    if (deviceBitsPerSample > 4) deviceBitsPerSample = 4;
    deviceBitsPerSample *= (int)audioData->System.device.playback.channels;

    unsigned int subBufferSize = (audioData->Buffer.defaultSize == 0)
                               ? (audioData->System.device.sampleRate / 30 * deviceBitsPerSample)
                               : audioData->Buffer.defaultSize;

    if (subBufferSize < periodSize) subBufferSize = periodSize;

    stream.buffer = Ivy_Audio_LoadBuffer(arena, formatIn, stream.channels,
                                         stream.sampleRate, subBufferSize * 2,
                                         AUDIO_BUFFER_USAGE_STREAM);
    stream.buffer->looping = true;

    return stream;
}

void Ivy_Audio_UnloadStream(const Music *music)
{
    IvyAudioData *audioData = Ivy_Audio_GetAudioData();

    IvyAudioBuffer *buffer = music->stream.buffer;

    ma_mutex_lock(&audioData->System.lock);
        Ivy_Audio_StopAudioBuffer(buffer);
    ma_mutex_unlock(&audioData->System.lock);

    Ivy_Audio_UnloadBuffer(buffer);
    stb_vorbis_close((stb_vorbis *)music->ctxData);
}
