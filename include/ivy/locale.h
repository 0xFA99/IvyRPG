#ifndef IVY_LOCALE_H
#define IVY_LOCALE_H

#define IVY_TR(loc, key) Tr(loc, key)


typedef enum {
    /* Title scene */
    LOC_MENU_NEW_GAME = 0,
    LOC_MENU_CONTINUE,
    LOC_MENU_OPTIONS,
    LOC_MENU_EXIT,

    /* Options scene */
    LOC_OPT_TITLE,
    LOC_OPT_SCREEN_SIZE,
    LOC_OPT_FULLSCREEN,
    LOC_OPT_LANGUAGE,
    LOC_OPT_BACK,
    LOC_OPT_ON,
    LOC_OPT_OFF,
    LOC_OPT_LANG_NAME,

    LOC_KEY_COUNT
} LocaleKey;

typedef struct {
    char  *_buffer;
    char  *strings[LOC_KEY_COUNT];
} Locale;

Locale     *LocaleLoad(const char *path);
void        LocaleUnload(Locale *locale);

const char *Tr(const Locale *locale, LocaleKey key);

#endif