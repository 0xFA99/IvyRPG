#ifndef IVY_SCENES_OPTIONS_PRIVATE_H
#define IVY_SCENES_OPTIONS_PRIVATE_H

#include "ivy/core/types.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    OPTIONS_LOCALE_SCREEN_SIZE = 0,
    OPTIONS_LOCALE_FULLSCREEN,
    OPTIONS_LOCALE_LANGUAGE,
    OPTIONS_LOCALE_KEYBIND,
    OPTIONS_LOCALE_BACK,
    OPTIONS_LOCALE_COUNT
} _IvyOptionsLocaleList;

typedef enum {
    OPTIONS_LOCALE_INDEX_SCREEN_SIZE_KEY = 9,
    OPTIONS_LOCALE_INDEX_FULLSCREEN      = 10,
    OPTIONS_LOCALE_INDEX_LANGUAGE        = 13,
    OPTIONS_LOCALE_INDEX_KEYBIND         = 11,
    OPTIONS_LOCALE_INDEX_BACK            = 2,
} _IvyOptionsLocaleIndexKey;

typedef enum {
    OPTIONS_CONFIG_KEY_CLOSED = 0,
    OPTIONS_CONFIG_KEY_OPENED,
    OPTIONS_CONFIG_KEY_SELECTING,
    OPTIONS_CONFIG_KEY_WAITING,
} _IvyOptionsKeybindState;

typedef struct {
    const char *strings[OPTIONS_LOCALE_COUNT];  // 40
    u32         lengths[OPTIONS_LOCALE_COUNT];  // 20
    u32         _padding;                       // 4
} _IvyOptionsLocale;                            // (64)

typedef struct {
    _IvyOptionsKeybindState state;              // 4
    u16                     cursorY;            // 2
    u16                     targetY;            // 2
    bool                    isWaiting;          // 1
    u8                      selected;           // 1
    u8                      scrollOffset;       // 1
    u8                      _padding;           // 1
} _IvyKeybindConfig;                            // (12)

#ifdef __cplusplus
}
#endif

#endif