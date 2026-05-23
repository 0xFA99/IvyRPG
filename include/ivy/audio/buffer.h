#ifndef IVY_AUDIO_BUFFER_H
#define IVY_AUDIO_BUFFER_H

#include "ivy/core/types.h"
#include "ivy/utils/forward.h"

#include "raylib/raylib.h"
#include "external/miniaudio.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AUDIO_DEVICE_FORMAT       ma_format_f32
#define AUDIO_DEVICE_CHANNELS     2
#define AUDIO_SAMPLE_RATE         44100
#define AUDIO_BUFFER_RESIDUAL_CAP 8
#define AUDIO_BUFFER_USAGE_STATIC 0
#define AUDIO_BUFFER_USAGE_STREAM 1

struct IvyAudioBuffer {
    ma_data_converter converter;
    unsigned char    *converterResidual;
    unsigned int      converterResidualCount;

    AudioCallback     callback;
    rAudioProcessor  *processor;

    float volume;
    float pitch;
    float pan;

    bool playing;
    bool paused;
    bool looping;
    int  usage;

    bool         isSubBufferProcessed[2];
    unsigned int sizeInFrames;
    unsigned int frameCursorPos;
    unsigned int framesProcessed;

    unsigned char  *data;
    rAudioBuffer   *next;
    rAudioBuffer   *prev;
};

// raudio.c
typedef struct {
    struct {
        ma_context context;
        ma_device  device;
        ma_mutex   lock;
        bool       isReady;
        size_t     pcmBufferSize;
        void      *pcmBuffer;
    } System;
    struct {
        rAudioBuffer *first;
        rAudioBuffer *last;
        int           defaultSize;
    } Buffer;
    rAudioProcessor *mixedProcessor;
} IvyAudioData;

IvyAudioBuffer *Ivy_Audio_LoadBuffer(IvyArenaLinear *arena, int format, u32 channels, u32 sampleRate, u32 sizeInFrames, int usage);
void            Ivy_Audio_PlayAudioBuffer(IvyAudioBuffer *buffer);
void            Ivy_Audio_StopAudioBuffer(IvyAudioBuffer *buffer);
void            Ivy_Audio_UnloadBuffer(IvyAudioBuffer *buffer);

#ifdef __cplusplus
}
#endif

#endif