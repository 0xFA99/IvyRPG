#ifndef IVY_CORE_INPUT_H
#define IVY_CORE_INPUT_H

#include "ivy/core/types.h"
#include "ivy/utils/forward.h"

#ifdef __cplusplus
extern "C" {
#endif

struct IvyKeybindInfo
{
    IvyKeybind  key;            // 4
    int         defaultKey;     // 4
    int         currentKey;     // 4
    int         padding;        // 4
    const char *name;           // 8
};                              // 24
IVY_ASSERT_STATIC(sizeof(IvyKeybindInfo) == 24, "[IvyKeybindInfo] Size must be 24 bytes!");

const IvyKeybindInfo *Ivy_Keybind_GetKeybindInfo(void);
void                  Ivy_Keybind_Load(const IvySaveManager *saveManager);
void                  Ivy_Keybind_Update(const IvySaveManager *saveManager, IvyKeybind keybind, int key);
void                  Ivy_Keybind_Reset(const IvySaveManager *saveManager);

const char           *Ivy_Keybind_GetKeyName(int key);

IVY_INLINE int Ivy_Keybind_GetCurrentKey(const IvyKeybindInfo *keybind, const u32 key)
{
    IVY_ASSERT(keybind != NULL, "[IvyKeybind] Keybind is NULL!");
    return keybind[key].currentKey;
}

#ifdef __cplusplus
}
#endif

#endif