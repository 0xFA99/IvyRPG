#include "ivy/core/keybind.h"

#include "ivy/systems/profile_manager.h"
#include "raylib/raylib.h"

#ifndef _WIN32
#include <stddef.h>
#endif

static IvyKeybindInfo keyBindInfo[IVY_KEY_MAX] = {
    // Action - Default Key - Current Key - Padding - Name Key
    { IVY_KEY_UP, KEY_UP, KEY_UP, 0, "Move Up" },
    { IVY_KEY_DOWN, KEY_DOWN, KEY_DOWN, 0, "Move Down" },
    { IVY_KEY_LEFT, KEY_LEFT, KEY_LEFT, 0, "Move Left" },
    { IVY_KEY_RIGHT, KEY_RIGHT, KEY_RIGHT, 0, "Move Right" },
    { IVY_KEY_CONFIRM, KEY_ENTER, KEY_ENTER, 0, "Confirm" },
    { IVY_KEY_CANCEL, KEY_ESCAPE, KEY_ESCAPE, 0, "Cancel" },
};

const IvyKeybindInfo *Ivy_Keybind_GetKeybindInfo(void)
{
    return keyBindInfo;
}

void Ivy_Keybind_Load(const IvySaveManager *saveManager)
{
    IVY_ENSURE(saveManager != NULL);

    for (int i = 0; i < IVY_KEY_MAX; i++) {
        keyBindInfo[i].currentKey = saveManager->save->profile.keybind[i];
    }
}

void Ivy_Keybind_Update(const IvySaveManager *saveManager, const IvyKeybind keybind, const int key)
{
    keyBindInfo[keybind].currentKey = key;

    saveManager->save->profile.keybind[keybind] = key;
}

void Ivy_Keybind_Reset(const IvySaveManager *saveManager)
{
    for (int i = 0; i < IVY_KEY_MAX; i++) {
        const int defaultKey = keyBindInfo[i].defaultKey;

        keyBindInfo[i].currentKey = defaultKey;
        saveManager->save->profile.keybind[i] = defaultKey;
    }
}

// void Ivy_Keybind_Load(void) {}
// void Ivy_Keybind_Save(void) {}
