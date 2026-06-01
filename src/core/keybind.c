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

const char *Ivy_Keybind_GetKeyName(const int key)
{
    switch (key)
    {
        case KEY_A: return "A";
        case KEY_B: return "B";
        case KEY_C: return "C";
        case KEY_D: return "D";
        case KEY_E: return "E";
        case KEY_F: return "F";
        case KEY_G: return "G";
        case KEY_H: return "H";
        case KEY_I: return "I";
        case KEY_J: return "J";
        case KEY_K: return "K";
        case KEY_L: return "L";
        case KEY_M: return "M";
        case KEY_N: return "N";
        case KEY_O: return "O";
        case KEY_P: return "P";
        case KEY_Q: return "Q";
        case KEY_R: return "R";
        case KEY_S: return "S";
        case KEY_T: return "T";
        case KEY_U: return "U";
        case KEY_V: return "V";
        case KEY_W: return "W";
        case KEY_X: return "X";
        case KEY_Y: return "Y";
        case KEY_Z: return "Z";

        case KEY_F1: return "F1";
        case KEY_F2: return "F2";
        case KEY_F3: return "F3";
        case KEY_F4: return "F4";
        case KEY_F5: return "F5";
        case KEY_F6: return "F6";
        case KEY_F7: return "F7";
        case KEY_F8: return "F8";
        case KEY_F9: return "F9";
        case KEY_F10: return "F10";
        case KEY_F11: return "F11";
        case KEY_F12: return "F12";

        case KEY_UP: return "UP";
        case KEY_DOWN: return "DOWN";
        case KEY_LEFT: return "LEFT";
        case KEY_RIGHT: return "RIGHT";

        case KEY_SPACE: return "SPACE";
        case KEY_ENTER: return "ENTER";
        case KEY_ESCAPE: return "ESC";
        case KEY_TAB: return "TAB";
        case KEY_BACKSPACE: return "BACKSPACE";
        case KEY_DELETE: return "DELETE";
        case KEY_INSERT: return "INSERT";
        case KEY_HOME: return "HOME";
        case KEY_END: return "END";
        case KEY_PAGE_UP: return "PAGE_UP";
        case KEY_PAGE_DOWN: return "PAGE_DOWN";
        case KEY_CAPS_LOCK: return "CAPS_LOCK";
        case KEY_NUM_LOCK: return "NUM_LOCK";
        case KEY_SCROLL_LOCK: return "SCROLL_LOCK";
        case KEY_PAUSE: return "PAUSE";
        case KEY_PRINT_SCREEN: return "PRT_SCREEN";

        case KEY_KP_0: return "NUM0";
        case KEY_KP_1: return "NUM1";
        case KEY_KP_2: return "NUM2";
        case KEY_KP_3: return "NUM3";
        case KEY_KP_4: return "NUM4";
        case KEY_KP_5: return "NUM5";
        case KEY_KP_6: return "NUM6";
        case KEY_KP_7: return "NUM7";
        case KEY_KP_8: return "NUM8";
        case KEY_KP_9: return "NUM9";
        case KEY_KP_DIVIDE: return "NUM/";
        case KEY_KP_MULTIPLY: return "NUM*";
        case KEY_KP_ENTER: return "NUM_ENTER";
        case KEY_KP_DECIMAL: return "NUM.";

        case KEY_GRAVE: return "`";
        case KEY_MINUS: return "-";
        case KEY_LEFT_BRACKET: return "[";
        case KEY_RIGHT_BRACKET: return "]";
        case KEY_BACKSLASH: return "\\";
        case KEY_SEMICOLON: return ";";
        case KEY_COMMA: return ",";
        case KEY_PERIOD: return ".";
        case KEY_SLASH: return "/";

        default: return "UNKNOWN";
    }
}
