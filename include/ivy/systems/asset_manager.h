#ifndef IVY_SYSTEMS_ASSET_MANAGER_H
#define IVY_SYSTEMS_ASSET_MANAGER_H

#include "ivy/core/types.h"
#include "ivy/utils/forward.h"

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #define NOGDI
    #define NOUSER

    #include <windows.h>

#else
    #include <sys/stat.h>
    #include <unistd.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    u32 id;         // 4
    u32 offset;     // 4
    u32 size;       // 4
} IvyAssetEntry;    // 12
IVY_ASSERT_STATIC(sizeof(IvyAssetEntry) == 12, "[IvyAssetEntry] must be 12 bytes!");

struct IvyAssetManager {
    IvyAssetEntry   *table;         // 8
    void            *mapped_data;   // 8
    usize            data_size;     // 8
#ifdef _WIN32
    HANDLE           h_file;        // 8
    HANDLE           h_map;         // 8
#else
    i32              fd;            // 4
#endif
    u32              table_mask;    // 4
};
#ifdef _WIN32
    IVY_ASSERT_STATIC(sizeof(IvyAssetManager) == 48, "[IvyAssetEntry] must be 48 bytes!");
#else
    IVY_ASSERT_STATIC(sizeof(IvyAssetManager) == 32, "[IvyAssetEntry] must be 32 bytes!");
#endif

IvyAssetManager* Ivy_AssetManager_Init(IvyArenaLinear *restrict arena, const char *restrict header_path, const char *restrict data_path);
void             Ivy_AssetManager_Destroy(IvyAssetManager *mgr);

IVY_INLINE const void* Ivy_Asset_Get(IvyAssetManager *restrict mgr, const u32 id, usize *restrict out_size)
{
    IVY_ASSERT(mgr != NULL, "[Asset] Manager is NULL!");

    if (IVY_UNLIKELY(!mgr || !mgr->table || !mgr->mapped_data)) {
        if (out_size) *out_size = 0;
        return NULL;
    }

    // direct-mapped lookup.
    const u32 index = id & mgr->table_mask;
    const IvyAssetEntry *entry = &mgr->table[index];

    // assumes perfect hashing at pack time.
    if (IVY_LIKELY(entry->id == id)) {
        if (out_size) *out_size = entry->size;
        return (const u8*)mgr->mapped_data + entry->offset;
    }

    if (out_size) *out_size = 0;
    return NULL;
}

#define Ivy_Asset_GetTyped(mgr, id, type, out_size) ((const type*)Ivy_Asset_Get((mgr), (id), (out_size)))

#ifdef __cplusplus
}
#endif

#endif
