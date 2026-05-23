#include "ivy/core/keybind.h"

#include "raylib/raylib.h"

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

void Ivy_Keybind_Update(const IvyKeybind keybind, const int key)
{
    keyBindInfo[keybind].currentKey = key;
}

void Ivy_Keybind_Reset(void)
{
    for (int i = 0; i < IVY_KEY_MAX; i++)
    {
        const int defaultKey = keyBindInfo[i].defaultKey;
        keyBindInfo[i].currentKey = defaultKey;
    }
}

// void Ivy_Keybind_Load(void) {}
// void Ivy_Keybind_Save(void) {}
