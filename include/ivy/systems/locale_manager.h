#ifndef IVY_SYSTEMS_LOCALE_MANAGER_H
#define IVY_SYSTEMS_LOCALE_MANAGER_H

#include "ivy/core/types.h"
#include "ivy/utils/forward.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LOCALE_LIST                         \
    X(COMMON_ON)                            \
    X(COMMON_OFF)                           \
    X(COMMON_BACK)                          \
    X(COMMON_CHANGE)                        \
    X(COMMON_CONFIRM)                       \
    \
    X(MAIN_MENU_NEW_GAME)                   \
    X(MAIN_MENU_CONTINUE)                   \
    X(MAIN_MENU_OPTIONS)                    \
    X(MAIN_MENU_EXIT)                       \
    \
    X(SETTINGS_GRAPHICS_RESOLUTION)         \
    X(SETTINGS_GRAPHICS_FULLSCREEN)         \
    X(SETTINGS_CONTROLS_LABEL)              \
    X(SETTINGS_CONTROLS_KEY_BINDINGS)       \
    \
    X(SETTINGS_LANGUAGE_LABEL)              \
    X(SETTINGS_LANGUAGE_VALUE)              \
    \
    X(MISC_PRESS_ANY_KEY)                   \
    \
    X(PAUSE_MENU_RESUME)                    \
    X(PAUSE_MENU_SAVE_GAME)                 \
    X(PAUSE_MENU_LOAD_GAME)                 \
    X(PAUSE_MENU_QUIT_TO_TITLE)

typedef enum {
#define X(name) name,
    LOCALE_LIST
#undef X
    IVY_LOCALE_MAX
} IvyLocaleKey;

struct IvyLocale {
    const u8       *buffer;         // 8
    const u32      *offsets;        // 8
    u32             num_entries;    // 4

    u32             hashID;
    IvyLocaleIndex  index;
};

IvyLocale  *Ivy_Locale_Load(IvyAssetManager *mgr, u32 id, IvyArenaLinear *arena);
void        Ivy_Locale_Update(IvyAssetManager *restrict assetManager, IvyLocale *restrict locale, u32 id);
void        Ivy_Locale_Next(IvyLocale *locale);
void        Ivy_Locale_DebugPrint(const IvyLocale *locale);

IVY_INLINE const char *Ivy_Locale_Tr(const IvyLocale *loc, const IvyLocaleKey key)
{
    return (const char *)(loc->buffer + loc->offsets[key]);
}

IVY_INLINE u32 Ivy_Locale_TrLength(const IvyLocale *loc, const IvyLocaleKey key)
{
    const char *s = Ivy_Locale_Tr(loc, key);
    u32 len = 0;

    while (s[len]) len++;

    return len;
}

#define IVY_TR(loc, key)     Ivy_Locale_Tr(loc, key)
#define IVY_TR_LEN(loc, key) Ivy_Locale_TrLength(loc, key)

#ifdef __cplusplus
}
#endif

#endif