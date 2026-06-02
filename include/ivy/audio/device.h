#ifndef IVY_AUDIO_DEVICE_H
#define IVY_AUDIO_DEVICE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool Ivy_Audio_InitDevice(void);
void Ivy_Audio_CloseDevice(void);

#ifdef __cplusplus
}
#endif

#endif