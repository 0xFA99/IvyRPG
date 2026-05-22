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
void                  Ivy_Keybind_Update(IvyKeybind keybind, int key);
void                  Ivy_Keybind_Reset(void);

// TODO: Add keybind loader and save!
// void            Ivy_Keybind_Load(void);
// void            Ivy_Keybind_Save(void);

#ifdef __cplusplus
}
#endif

#endif