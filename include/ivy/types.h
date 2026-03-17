#ifndef IVY_TYPES_H
#define IVY_TYPES_H

#define MAX_PATH_LEN 256

#define CHARACTER_FRAME_SIZE 64
#define DEFAULT_TILE_SIZE 32
#define DEFAULT_TILE_HALF 16

// CURSORS
#define PRIMARY_CURSOR_PATH     "assets/cursors/cursorW.bin"
#define SECONDARY_CURSOR_PATH   "assets/cursors/cursorY.bin"

// FONTS
#define PRIMARY_FONT_PATH       "assets/fonts/DenkOne-Regular_ttf.bin"
#define SECONDARY_FONT_PATH     "assets/fonts/NotoSansCJKtc-Black_otf.bin"
#define LOAD_FONT_SIZE          64

typedef enum {
    IVY_FONT_PRIMARY = 0,
    IVY_FONT_SECONDARY
} IvyFontType;

typedef enum {
    IVY_CURSOR_PRIMARY = 0,
    IVY_CURSOR_SECONDARY
} IvyCursorType;

typedef enum {
    DIRECTION_FRONT,
    DIRECTION_RIGHT,
    DIRECTION_LEFT,
    DIRECTION_BACK
} Direction;

typedef unsigned int    u32;
typedef unsigned short  u16;
typedef unsigned char   u8;

#endif