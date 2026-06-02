#include "ivy/arena/linear.h"
#include "ivy/core/types.h"
#include "ivy/systems/asset_manager.h"
#include "ivy/systems/locale_manager.h"
#include "ivy/utils/file_ids.h"
#include "ivy/utils/forward.h"
#include "ivy/utils/io.h"

#ifdef IVY_DEBUG
#include <stdio.h>
#endif

#define LOCALE_HEADER_SIZE 4

static const u32 IVY_LOCALES[IVY_LOCALE_LANG_MAX] = {
    [IVY_LOCALE_LANG_EN] = ASSET_LOCALES_EN_BIN,
    [IVY_LOCALE_LANG_ID] = ASSET_LOCALES_ID_BIN
};

static void ParseBinary(IvyLocale *restrict locale, const void *data, const usize dataSize, const u32 id)
{
    const u8 *buf = (const u8 *)data;

    IVY_CHECK(dataSize >= LOCALE_HEADER_SIZE + sizeof(u32), "Locale binary too small: %zu bytes", dataSize);

    // parse header
    const u32 count = *(const u32 *)buf;
    IVY_CHECK(count == IVY_LOCALE_MAX, "Entry mismatch: got %u, expected %u", count, (u32)IVY_LOCALE_MAX);

    const usize minSize = LOCALE_HEADER_SIZE + (count * sizeof(u32));
    IVY_CHECK(dataSize >= minSize, "Locale truncated: %zu < %zu", dataSize, minSize);

    locale->buffer      = buf;
    locale->num_entries = count;
    locale->offsets     = (const u32 *)(buf + LOCALE_HEADER_SIZE);
    locale->hashID      = id;
    locale->index       = (id >> 4) & 1;

    for (u32 i = 0; i < count; i++) {
        const u32 off = locale->offsets[i];
        IVY_CHECK(off >= minSize && off < (u32)dataSize, "Invalid offset[%u]: %u (file size: %zu)", i, off, dataSize);
    }
}

IvyLocale *Ivy_Locale_Load(IvyAssetManager *restrict mgr, const u32 id, IvyArenaLinear *restrict arena)
{
    IVY_ASSERT(mgr && arena, "Manager or Arena is NULL");

    usize dataSize;
    const void *data = Ivy_Asset_Get(mgr, id, &dataSize);
    IVY_CHECK(data, "Locale asset 0x%08X not found", id);

    IvyLocale *locale = Ivy_Arena_LinearAllocZero(arena, sizeof(IvyLocale));
    IVY_ENSURE(locale);

    ParseBinary(locale, data, dataSize, id);

#ifdef IVY_DEBUG
    printf("[Locale] Loaded 0x%08X: %u keys, %zu bytes\n", id, locale->num_entries, dataSize);
#endif
    return locale;
}

void Ivy_Locale_Update(IvyAssetManager *restrict assetManager, IvyLocale *restrict locale, const u32 id)
{
    IVY_ASSERT(assetManager && locale, "Manager or Locale is NULL");

    usize dataSize;
    const void *data = Ivy_Asset_Get(assetManager, id, &dataSize);
    IVY_CHECK(data, "Locale asset 0x%08X not found", id);

    memset(locale, 0, sizeof(IvyLocale));

    ParseBinary(locale, data, dataSize, id);
}

void Ivy_Locale_Next(IvyLocale *locale)
{
    const u32 nextIndex = (locale->index + 1) % IVY_LOCALE_LANG_MAX;

    locale->index = nextIndex;
    locale->hashID = IVY_LOCALES[nextIndex];
}

void Ivy_Locale_DebugPrint(const IvyLocale *locale)
{
#ifdef IVY_DEBUG
    if (!locale) { printf("[Locale] NULL\n"); return; }

    static const char *keys[] = {
        #define X(name) #name,
        LOCALE_LIST
        #undef X
    };

    printf("Locale %p (%u keys)\n", (void*)locale, locale->num_entries);

    for (u32 i = 0; i < IVY_LOCALE_MAX; i++)
    {
        const char *text = IVY_TR(locale, (IvyLocaleKey)i);
        const u32 length = IVY_TR_LEN(locale, (IvyLocaleKey)i);

        printf(" [%2u] %-35s | offset=%-4u | \"%.*s\"\n", i, keys[i], locale->offsets[i], length, text);
    }
    printf("\n");
#endif
}
