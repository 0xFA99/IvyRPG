#include "ivy/arena/linear.h"
#include "ivy/audio/buffer.h"
#include "ivy/audio/wav.h"
#include "ivy/core/types.h"
#include "ivy/systems/asset_manager.h"
#include "ivy/utils/forward.h"

#include "external/dr_wav.h"
#include "external/miniaudio.h"

#include "raylib/raylib.h"

#include <stdbool.h>
#include <stddef.h>

static Wave load_wave(IvyArenaLinear *restrict arena, IvyAssetManager *restrict manager, const u32 id)
{
    Wave wave = {0};
    drwav wav  = {0};

    usize wav_size;
    const u8 *data = Ivy_Asset_Get(manager, id, &wav_size);
    IVY_ENSURE(data != NULL);

    const bool ok = drwav_init_memory(&wav, data, wav_size, NULL);
    IVY_CHECK(ok, "[IvyAudio] Failed to init WAV from memory (ID: %u)", id);

    wave.frameCount = (u32)wav.totalPCMFrameCount;
    wave.sampleRate = wav.sampleRate;
    wave.sampleSize = 16;
    wave.channels   = wav.channels;

    const size_t dataSize = wave.frameCount * wave.channels * sizeof(short);
    wave.data = Ivy_Arena_LinearAlloc(arena, dataSize);
    IVY_ENSURE(wave.data != NULL);

    const drwav_uint64 framesRead = drwav_read_pcm_frames_s16(&wav, wave.frameCount, wave.data);
    IVY_ASSERT(framesRead == wave.frameCount, "WAV frame read mismatch: expected %u, got %llu",
               wave.frameCount, framesRead);

    drwav_uninit(&wav);
    return wave;
}

IvySound Ivy_Audio_LoadSoundWav(IvyArenaLinear *restrict arena,
                                IvyAssetManager *restrict manager, const u32 id)
{
    IvySound sound = {0};
    sound.snap = Ivy_Arena_LinearGetSnapshot(arena);

    const Wave wave = load_wave(arena, manager, id);

    const ma_format formatIn = (wave.sampleSize == 8)  ? ma_format_u8  :
                               (wave.sampleSize == 16) ? ma_format_s16 : ma_format_f32;

    const IvyAudioData *audioData = Ivy_Audio_GetAudioData();
    IVY_ENSURE(audioData != NULL);
    IVY_ENSURE(audioData->System.isReady);

    ma_uint32 frameCount = (ma_uint32)ma_convert_frames(
        NULL, 0,
        AUDIO_DEVICE_FORMAT, AUDIO_DEVICE_CHANNELS, audioData->System.device.sampleRate,
        NULL, wave.frameCount, formatIn, wave.channels, wave.sampleRate
    );
    IVY_ASSERT(frameCount > 0, "Invalid frame count for sound ID: %u", id);

    IvyAudioBuffer *buf = Ivy_Audio_LoadBuffer(
        arena, AUDIO_DEVICE_FORMAT, AUDIO_DEVICE_CHANNELS,
        audioData->System.device.sampleRate, frameCount, AUDIO_BUFFER_USAGE_STATIC
    );
    IVY_ENSURE(buf != NULL);

    frameCount = (ma_uint32)ma_convert_frames(
        buf->data, frameCount,
        AUDIO_DEVICE_FORMAT, AUDIO_DEVICE_CHANNELS, audioData->System.device.sampleRate,
        wave.data, wave.frameCount, formatIn, wave.channels, wave.sampleRate
    );

    sound.data.frameCount        = frameCount;
    sound.data.stream.sampleRate = audioData->System.device.sampleRate;
    sound.data.stream.sampleSize = 32;
    sound.data.stream.channels   = AUDIO_DEVICE_CHANNELS;
    sound.data.stream.buffer     = buf;

    return sound;
}

void Ivy_Audio_UnloadSound(const IvySound *sound)
{
    IVY_ENSURE(sound != NULL);
    IVY_ENSURE(sound->data.stream.buffer != NULL);

    Ivy_Audio_UnloadBuffer(sound->data.stream.buffer);
}
