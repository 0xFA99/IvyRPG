#include "ivy/scenes/options.h"
#include "ivy/utils/file_ids.h"

enum {
    CURSOR_X        = 10,
    TEXT_X          = 28,
    VALUE_X         = 108,
    MENU_SPACING    = 16,
    TEXT_SIZE       = 14,
    MARGIN_BOTTOM   = 30,

    POPUP_WIDTH     = 200,
    POPUP_HEIGHT    = 160,
    POPUP_PADDING   = 10,
    KEYBIND_ITEM_H  = 14,
    KEYBIND_VISIBLE = 8,
    KEYBIND_VALUE_X = 120,
};

const float CURSOR_SPEED = 0.4f;
const float CURSOR_SCALE = 0.5f;
const float KEYBIND_CURSOR_SPEED = 0.5f;

const u32 LOCALE_ASSETS[] = {
    ASSET_LOCALES_EN_BIN,
    ASSET_LOCALES_ID_BIN,
};
const u32 LOCALE_COUNT = sizeof(LOCALE_ASSETS) / sizeof(LOCALE_ASSETS[0]);

const struct { u16 w, h; } SCREEN_SIZES[] = {
    { 640, 360 },
    { 960, 540 },
    { 1280, 720 },
    { 1600, 900 },
    { 1920, 1080 },
};
const u32 SCREEN_SIZE_COUNT = sizeof(SCREEN_SIZES) / sizeof(SCREEN_SIZES[0]);

const u16 MENU_LOCALE_KEYS[] = { 9, 10, 13, 11, 2 };
