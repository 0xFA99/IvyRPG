#include "ivy/audio/device.h"
#include "ivy/audio/buffer.h"
#include "ivy/core/types.h"

#include <string.h>

#define AUDIO_DEVICE_SAMPLE_RATE           0
#define AUDIO_DEVICE_PERIOD_SIZE_IN_FRAMES 0
#define AUDIO_BUFFER_RESIDUAL_CAPACITY     8

struct rAudioProcessor {
    AudioCallback   process;
    rAudioProcessor *next;
    rAudioProcessor *prev;
};

static void MixAudioFrames(float *restrict framesOut, const float *restrict framesIn, const IvyAudioBuffer *restrict buffer, const u32 frameCount)
{
    const IvyAudioData *audioData = Ivy_Audio_GetAudioData();
    const float localVolume       = buffer->volume;
    const u32 channels            = audioData->System.device.playback.channels;

    if (channels == 2)
    {
        const float right      = (buffer->pan + 1.0f) / 2.0f;
        const float left       = 1.0f - right;
        const float levels[2]  = {
            localVolume * 0.5f * left  * (3.0f - left  * left),
            localVolume * 0.5f * right * (3.0f - right * right)
        };

        float *fo       = framesOut;
        const float *fi = framesIn;

        for (u32 i = 0; i < frameCount; i++, fo += 2, fi += 2) {
            fo[0] += fi[0] * levels[0];
            fo[1] += fi[1] * levels[1];
        }
    }
    else
    {
        for (u32 frame = 0; frame < frameCount; frame++)
        {
            float *fo       = framesOut + frame * channels;
            const float *fi = framesIn  + frame * channels;

            for (u32 c = 0; c < channels; c++) {
                fo[c] += fi[c] * localVolume;
            }
        }
    }
}

static bool IsAudioBufferPlayingInLockedState(const IvyAudioBuffer *buffer)
{
    return buffer->playing && !buffer->paused;
}

static void StopAudioBufferInLockedState(IvyAudioBuffer *buffer)
{
    if (IVY_UNLIKELY(!buffer || !IsAudioBufferPlayingInLockedState(buffer))) return;

    buffer->playing                 = false;
    buffer->paused                  = false;
    buffer->frameCursorPos          = 0;
    buffer->framesProcessed         = 0;
    buffer->isSubBufferProcessed[0] = true;
    buffer->isSubBufferProcessed[1] = true;
}

static u32 ReadAudioBufferFramesInInternalFormat(IvyAudioBuffer *restrict audioBuffer, void *restrict framesOut, const u32 frameCount)
{
    if (!audioBuffer->playing) return 0;

    if (audioBuffer->callback) {
        audioBuffer->callback(framesOut, frameCount);
        audioBuffer->framesProcessed += frameCount;
        return frameCount;
    }

    const u32 subBufferSizeInFrames = (audioBuffer->sizeInFrames > 1)
                                    ? audioBuffer->sizeInFrames / 2
                                    : audioBuffer->sizeInFrames;

    u32 currentSubBufferIndex = audioBuffer->frameCursorPos / subBufferSizeInFrames;

    if (IVY_UNLIKELY(currentSubBufferIndex > 1)) return 0;

    // Snapshot processed flags to avoid mid-loop races with the streaming thread
    bool isSubBufferProcessed[2] = {
        audioBuffer->isSubBufferProcessed[0],
        audioBuffer->isSubBufferProcessed[1]
    };

    const u32 frameSizeInBytes = ma_get_bytes_per_frame(audioBuffer->converter.formatIn, audioBuffer->converter.channelsIn);
    u32 framesRead = 0;

    while (true)
    {
        if (audioBuffer->usage == AUDIO_BUFFER_USAGE_STATIC) {
            if (framesRead >= frameCount) break;
        }
        else {
            if (isSubBufferProcessed[currentSubBufferIndex]) break;
        }

        const u32 totalFramesRemaining = frameCount - framesRead;
        if (totalFramesRemaining == 0) break;

        u32 framesRemainingInOutputBuffer;
        if (audioBuffer->usage == AUDIO_BUFFER_USAGE_STATIC) {
            framesRemainingInOutputBuffer = audioBuffer->sizeInFrames - audioBuffer->frameCursorPos;
        }
        else {
            const u32 firstFrame = subBufferSizeInFrames * currentSubBufferIndex;
            framesRemainingInOutputBuffer = subBufferSizeInFrames - (audioBuffer->frameCursorPos - firstFrame);
        }

        const u32 framesToRead = (totalFramesRemaining < framesRemainingInOutputBuffer)
                               ? totalFramesRemaining
                               : framesRemainingInOutputBuffer;

        memcpy(
            (u8 *)framesOut + framesRead * frameSizeInBytes,
            audioBuffer->data + audioBuffer->frameCursorPos * frameSizeInBytes,
            framesToRead * frameSizeInBytes
        );

        audioBuffer->frameCursorPos = (audioBuffer->frameCursorPos + framesToRead) % audioBuffer->sizeInFrames;
        framesRead += framesToRead;

        if (framesToRead == framesRemainingInOutputBuffer)
        {
            audioBuffer->isSubBufferProcessed[currentSubBufferIndex] = true;
            isSubBufferProcessed[currentSubBufferIndex] = true;
            currentSubBufferIndex = (currentSubBufferIndex + 1) % 2;

            if (!audioBuffer->looping) {
                StopAudioBufferInLockedState(audioBuffer);
                break;
            }
        }
    }

    const u32 remaining = frameCount - framesRead;
    if (remaining > 0)
    {
        memset((u8 *)framesOut + framesRead * frameSizeInBytes, 0, remaining * frameSizeInBytes);
        if (audioBuffer->usage != AUDIO_BUFFER_USAGE_STATIC) framesRead += remaining;
    }

    return framesRead;
}

static u32 ReadAudioBufferFramesInMixingFormat(IvyAudioBuffer *restrict audioBuffer, float *restrict framesOut, const u32 frameCount)
{
    u32 totalOutputFramesProcessed = 0;
    u8 inputBuffer[4096]           = { 0 };
    const u32 bpf                  = ma_get_bytes_per_frame(audioBuffer->converter.formatIn, audioBuffer->converter.channelsIn);
    const u32 inputBufferFrameCap  = sizeof(inputBuffer) / bpf;

    while (totalOutputFramesProcessed < frameCount)
    {
        float *runningFramesOut = framesOut + totalOutputFramesProcessed * audioBuffer->converter.channelsOut;
        u64 outputFramesToProcessThisIteration = frameCount - totalOutputFramesProcessed;

        if (audioBuffer->converterResidualCount > 0)
        {
            ma_uint64 inputFramesProcessed  = audioBuffer->converterResidualCount;
            ma_uint64 outputFramesProcessed = outputFramesToProcessThisIteration;

            ma_data_converter_process_pcm_frames(&audioBuffer->converter, audioBuffer->converterResidual, &inputFramesProcessed, runningFramesOut, &outputFramesProcessed);

            memmove(
                audioBuffer->converterResidual,
                audioBuffer->converterResidual + inputFramesProcessed * bpf,
                (size_t)(AUDIO_BUFFER_RESIDUAL_CAPACITY - inputFramesProcessed) * bpf
            );
            audioBuffer->converterResidualCount -= (u32)inputFramesProcessed;
            totalOutputFramesProcessed          += (u32)outputFramesProcessed;
        }
        else
        {
            u32 estimatedInputFrameCount = (u32)(
                ((float)audioBuffer->converter.resampler.sampleRateIn / audioBuffer->converter.resampler.sampleRateOut)
                * outputFramesToProcessThisIteration
            );

            if (estimatedInputFrameCount == 0) estimatedInputFrameCount = 1;
            if (estimatedInputFrameCount > inputBufferFrameCap) estimatedInputFrameCount = inputBufferFrameCap;

            const u32 inputFramesInInternalFormatCount = ReadAudioBufferFramesInInternalFormat(audioBuffer, inputBuffer, estimatedInputFrameCount);

            ma_uint64 inputFramesProcessed  = inputFramesInInternalFormatCount;
            ma_uint64 outputFramesProcessed = outputFramesToProcessThisIteration;

            ma_data_converter_process_pcm_frames(&audioBuffer->converter, inputBuffer, &inputFramesProcessed, runningFramesOut, &outputFramesProcessed);

            totalOutputFramesProcessed += (u32)outputFramesProcessed;

            if (inputFramesInInternalFormatCount > inputFramesProcessed)
            {
                u64 residualFrameCount = inputFramesInInternalFormatCount - inputFramesProcessed;
                if (residualFrameCount > AUDIO_BUFFER_RESIDUAL_CAPACITY)
                    residualFrameCount = AUDIO_BUFFER_RESIDUAL_CAPACITY;

                memcpy(
                    audioBuffer->converterResidual,
                    inputBuffer + inputFramesProcessed * bpf,
                    (size_t)(residualFrameCount * bpf)
                );

                audioBuffer->converterResidualCount = (unsigned int)residualFrameCount;
            }

            if (inputFramesInInternalFormatCount < estimatedInputFrameCount) break;
        }
    }

    return totalOutputFramesProcessed;
}

static void OnSendAudioDataToDevice(ma_device *pDevice, void *pFramesOut, const void *pFramesInput, const u32 frameCount)
{
    // Menggunakan macro void-casting/unused jika ada di types.h, atau tetap standar C
    (void)pDevice;
    (void)pFramesInput;

    IvyAudioData *audioData = Ivy_Audio_GetAudioData();

    memset(pFramesOut, 0, frameCount * pDevice->playback.channels * ma_get_bytes_per_sample(pDevice->playback.format));

    ma_mutex_lock(&audioData->System.lock);
    {
        for (IvyAudioBuffer *audioBuffer = audioData->Buffer.first; audioBuffer != NULL; audioBuffer = audioBuffer->next) {
            if (!audioBuffer->playing || audioBuffer->paused) continue;

            u32 framesRead = 0;

            while (framesRead < frameCount)
            {
                u32 framesToRead = frameCount - framesRead;

                while (framesToRead > 0)
                {
                    float tempBuffer[1024] = { 0 };
                    u32 framesToReadNow = framesToRead;

                    if (framesToReadNow > sizeof(tempBuffer) / sizeof(tempBuffer[0]) / AUDIO_DEVICE_CHANNELS)
                        framesToReadNow = sizeof(tempBuffer) / sizeof(tempBuffer[0]) / AUDIO_DEVICE_CHANNELS;

                    const u32 framesJustRead = ReadAudioBufferFramesInMixingFormat(audioBuffer, tempBuffer, framesToReadNow);

                    if (framesJustRead > 0) {
                        float *fo = (float *)pFramesOut + framesRead * audioData->System.device.playback.channels;

                        for (rAudioProcessor *p = audioBuffer->processor; p; p = p->next) {
                            p->process(tempBuffer, framesJustRead);
                        }

                        MixAudioFrames(fo, tempBuffer, audioBuffer, framesJustRead);

                        framesToRead -= framesJustRead;
                        framesRead   += framesJustRead;
                    }

                    if (!audioBuffer->playing) { framesRead = frameCount; break; }

                    if (framesJustRead < framesToReadNow) {
                        if (!audioBuffer->looping) {
                            StopAudioBufferInLockedState(audioBuffer);
                            break;
                        }

                        audioBuffer->frameCursorPos = 0;
                    }
                }

                if (framesToRead > 0) break;
            }
        }
    }

    for (rAudioProcessor *p = audioData->mixedProcessor; p; p = p->next) {
        p->process(pFramesOut, frameCount);
    }

    ma_mutex_unlock(&audioData->System.lock);
}

bool Ivy_Audio_InitDevice(void)
{
    IvyAudioData *audioData = Ivy_Audio_GetAudioData();

    ma_context_config ctxConfig = ma_context_config_init();
    if (ma_context_init(NULL, 0, &ctxConfig, &audioData->System.context) != MA_SUCCESS) {
        return false;
    }

    ma_device_config config          = ma_device_config_init(ma_device_type_playback);
    config.playback.pDeviceID        = NULL;
    config.playback.format           = AUDIO_DEVICE_FORMAT;
    config.playback.channels         = AUDIO_DEVICE_CHANNELS;
    config.sampleRate                = AUDIO_DEVICE_SAMPLE_RATE;
    config.periodSizeInFrames        = AUDIO_DEVICE_PERIOD_SIZE_IN_FRAMES;
    config.dataCallback              = OnSendAudioDataToDevice;
    config.pUserData                 = NULL;
    config.noPreSilencedOutputBuffer = true;
    config.noFixedSizedCallback      = true;

    if (ma_device_init(&audioData->System.context, &config, &audioData->System.device) != MA_SUCCESS)
        goto fail_context;

    if (ma_mutex_init(&audioData->System.lock) != MA_SUCCESS)
        goto fail_device;

    if (ma_device_start(&audioData->System.device) != MA_SUCCESS)
        goto fail_mutex;

    audioData->System.isReady = true;
    return true;

fail_mutex:   ma_mutex_uninit(&audioData->System.lock);
fail_device:  ma_device_uninit(&audioData->System.device);
fail_context: ma_context_uninit(&audioData->System.context);
    return false;
}

void Ivy_Audio_CloseDevice(void)
{
    IvyAudioData *audioData = Ivy_Audio_GetAudioData();

    if (!audioData->System.isReady) return;

    ma_device_uninit(&audioData->System.device);
    ma_mutex_uninit(&audioData->System.lock);
    ma_context_uninit(&audioData->System.context);

    audioData->System.isReady       = false;
    audioData->System.pcmBuffer     = NULL;
    audioData->System.pcmBufferSize = 0;
}
