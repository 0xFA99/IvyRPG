#include "ivy/audio/stream.h"
#include "ivy/audio/buffer.h"

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