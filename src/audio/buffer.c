#include "ivy/arena/linear.h"
#include "ivy/audio/buffer.h"
#include "ivy/core/types.h"
#include "ivy/graphics/collusion.h"
#include "ivy/graphics/gfx.h"
#include "ivy/utils/forward.h"
#include "ivy/utils/io.h"

#include "external/miniaudio.h"

IvyAudioBuffer *Ivy_Audio_LoadBuffer(IvyArenaLinear *arena, const int format, const u32 channels,
                                     const u32 sampleRate, const u32 sizeInFrames, const int usage)
{
    const IvyArenaLinearSnapshot snap = Ivy_Arena_LinearGetSnapshot(arena);
    IvyAudioBuffer *buf = Ivy_Arena_LinearAllocZero(arena, sizeof(IvyAudioBuffer));
    IVY_ENSURE(buf != NULL);

    if (IVY_LIKELY(sizeInFrames > 0)) {
        const size_t dataSize = sizeInFrames * channels * ma_get_bytes_per_sample(format);
        buf->data = Ivy_Arena_LinearAllocZero(arena, dataSize);
        IVY_ENSURE(buf->data != NULL);
    }

    ma_data_converter_config cfg = ma_data_converter_config_init(
        format, AUDIO_DEVICE_FORMAT,
        channels, AUDIO_DEVICE_CHANNELS,
        sampleRate, AUDIO_SAMPLE_RATE
    );
    cfg.allowDynamicSampleRate = true;

    if (IVY_UNLIKELY(ma_data_converter_init(&cfg, NULL, &buf->converter) != MA_SUCCESS)) {
        Ivy_Arena_LinearRestore(arena, snap);
        return NULL;
    }

    const size_t residualSize = AUDIO_BUFFER_RESIDUAL_CAP * ma_get_bytes_per_frame(format, channels);
    buf->converterResidual = Ivy_Arena_LinearAllocZero(arena, residualSize);
    IVY_ENSURE(buf->converterResidual != NULL);

    buf->volume  = 1.0f;
    buf->pitch   = 1.0f;
    buf->pan     = 0.0f;
    buf->usage   = usage;
    buf->isSubBufferProcessed[0] = true;
    buf->isSubBufferProcessed[1] = true;
    buf->sizeInFrames = sizeInFrames;

    IvyAudioData *audioData = Ivy_Audio_GetAudioData();
    ma_mutex_lock(&audioData->System.lock);
    {
        if (audioData->Buffer.first == NULL) {
            audioData->Buffer.first = buf;
        } else {
            audioData->Buffer.last->next = buf;
            buf->prev = audioData->Buffer.last;
        }
        audioData->Buffer.last = buf;
    }
    ma_mutex_unlock(&audioData->System.lock);

    return buf;
}

void Ivy_Audio_PlayAudioBuffer(IvyAudioBuffer *buffer)
{
    IVY_ENSURE(buffer != NULL);

    IvyAudioData *data = Ivy_Audio_GetAudioData();
    IVY_ENSURE(data != NULL);

    ma_mutex_lock(&data->System.lock);
    {
        buffer->playing                 = true;
        buffer->paused                  = false;
        buffer->frameCursorPos          = 0;
        buffer->framesProcessed         = 0;
        buffer->isSubBufferProcessed[0] = true;
        buffer->isSubBufferProcessed[1] = true;
    }
    ma_mutex_unlock(&data->System.lock);
}

void Ivy_Audio_StopAudioBuffer(IvyAudioBuffer *buffer)
{
    IVY_ENSURE(buffer != NULL);

    if (!buffer->playing || buffer->paused) return;

    buffer->playing                 = false;
    buffer->paused                  = false;
    buffer->frameCursorPos          = 0;
    buffer->framesProcessed         = 0;
    buffer->isSubBufferProcessed[0] = true;
    buffer->isSubBufferProcessed[1] = true;
}

void Ivy_Audio_StopAudioBufferSafe(IvyAudioBuffer *buffer)
{
    IVY_ENSURE(buffer != NULL);

    IvyAudioData *data = Ivy_Audio_GetAudioData();
    IVY_ENSURE(data != NULL);

    ma_mutex_lock(&data->System.lock);
        Ivy_Audio_StopAudioBuffer(buffer);
    ma_mutex_unlock(&data->System.lock);
}

void Ivy_Audio_UnloadBuffer(IvyAudioBuffer *buffer)
{
    IVY_ENSURE(buffer != NULL);

    IvyAudioData *audioData = Ivy_Audio_GetAudioData();
    IVY_ENSURE(audioData != NULL);

    ma_mutex_lock(&audioData->System.lock);
    {
        if (buffer->prev == NULL) audioData->Buffer.first = buffer->next;
        else buffer->prev->next = buffer->next;

        if (buffer->next == NULL) audioData->Buffer.last = buffer->prev;
        else buffer->next->prev = buffer->prev;

        buffer->prev = NULL;
        buffer->next = NULL;
    }
    ma_mutex_unlock(&audioData->System.lock);

    ma_data_converter_uninit(&buffer->converter, NULL);
    /* Memory reclaimed by arena snapshot restore at call site */
}

void Ivy_Audio_ResetSystemBuffers(void)
{
    IvyAudioData *audioData = Ivy_Audio_GetAudioData();

    ma_mutex_lock(&audioData->System.lock);
    {
        audioData->Buffer.first = NULL;
        audioData->Buffer.last  = NULL;
    }
    ma_mutex_unlock(&audioData->System.lock);
}

void Ivy_Audio_InitPcmScratch(IvyArenaLinear *arena)
{
    IvyAudioData *audioData = Ivy_Audio_GetAudioData();
    IVY_ENSURE(audioData != NULL);
    IVY_ENSURE(audioData->System.isReady);

    const ma_device *dev = &audioData->System.device;
    const unsigned int bytesPerDeviceFrame = (unsigned int)ma_get_bytes_per_frame(
        dev->playback.format,
        dev->playback.channels
    );

    const unsigned int subBufferSize      = dev->sampleRate / 30 * bytesPerDeviceFrame;
    const unsigned int periodSize         = dev->playback.internalPeriodSizeInFrames;
    const unsigned int effectiveSubBuffer = (subBufferSize < periodSize) ? periodSize : subBufferSize;
    const size_t scratchSize              = (size_t)(effectiveSubBuffer * 2) * bytesPerDeviceFrame;

    audioData->System.pcmBuffer     = Ivy_Arena_LinearAllocZero(arena, scratchSize);
    audioData->System.pcmBufferSize = scratchSize;

    IVY_ENSURE(audioData->System.pcmBuffer != NULL);
}
