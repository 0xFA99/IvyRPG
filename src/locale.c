#include "ivy/locale.h"
#include "ivy/types.h"

#include <stdio.h>
#include <stdlib.h>


Locale *LocaleLoad(const char *path)
{
    FILE *f = fopen(path, "rb");
    fseek(f, 0, SEEK_END);
    const long file_size = ftell(f);
    rewind(f);

    unsigned char *raw = malloc((size_t)file_size);
    fread(raw, 1, (size_t)file_size, f);
    fclose(f);

    const u32 count = (unsigned int)raw[0]       |
                      (unsigned int)raw[1] << 8  |
                      (unsigned int)raw[2] << 16 |
                      (unsigned int)raw[3] << 24;

    char *data_block = (char *)(raw + 4 + count * 4);
    Locale *loc      = malloc(sizeof(Locale));
    loc->_buffer     = (char *)raw;

    for (unsigned int i = 0; i < count; i++)
    {
        const u32 offset = (unsigned int)raw[4 + i*4]           |
                           (unsigned int)raw[4 + i*4 + 1] << 8  |
                           (unsigned int)raw[4 + i*4 + 2] << 16 |
                           (unsigned int)raw[4 + i*4 + 3] << 24;

        loc->strings[i] = data_block + offset;
    }

    return loc;
}

void LocaleUnload(Locale *locale)
{
    free(locale->_buffer);
    free(locale);
}

const char *Tr(const Locale *locale, LocaleKey key)
{
    return locale->strings[key];
}