#ifndef IVY_UTILS_IO_H
#define IVY_UTILS_IO_H

#include "ivy/arena/linear.h"
#include "ivy/core/types.h"
#include "ivy/systems/asset_manager.h"
#include "ivy/systems/profile_manager.h"
#include "ivy/utils/forward.h"

#include <stdio.h>

#ifdef _WIN32
    #include <io.h>
    #define F_OK 0
    #define ACCESS _access
#else
    #include <unistd.h>
    #define ACCESS access
#endif

#ifdef __cplusplus
extern "C" {
#endif

IVY_INLINE void Ivy_IO_ReadExact(FILE *restrict file, void *restrict dest, const usize n)
{
    const usize bytes = fread(dest, 1, n, file);
    IVY_ASSERT(bytes == n, "[IO] Failed to read expected bytes (%zu != %zu).", bytes, n);
}

IVY_INLINE u8 *Ivy_IO_ReadString(FILE *restrict file, IvyArenaLinear *restrict arena)
{
    u32 len = 0;
    Ivy_IO_ReadExact(file, &len, sizeof(u32));

    const u64 alloc_size = (u64)len + 1;

    u8 *str = (u8 *)Ivy_Arena_LinearAlloc(arena, alloc_size);
    IVY_CHECK(str != NULL, "[IO] String allocation failed for %u bytes.", len);

    Ivy_IO_ReadExact(file, str, len);
    str[len] = '\0';
    return str;
}

IVY_INLINE u8 *Ivy_IO_LoadFile(const char *filename, usize *restrict out_size, IvyArenaLinear *restrict arena)
{
    IVY_ASSERT(filename != NULL, "[IO] Filename is NULL");

    FILE *file = fopen(filename, "rb");
    if (IVY_UNLIKELY(!file)) return NULL;

    fseek(file, 0, SEEK_END);
    const long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    IVY_CHECK(fsize >= 0, "[IO] ftell failed for file %s", filename);
    const usize size = (usize)fsize;

    u8 *buffer = (u8 *)Ivy_Arena_LinearAlloc(arena, size);
    IVY_CHECK(buffer != NULL, "[IO] Failed to allocate %zu bytes for file %s", size, filename);

    Ivy_IO_ReadExact(file, buffer, size);

    if (IVY_LIKELY(out_size)) *out_size = size;

    fclose(file);
    return buffer;
}

IVY_INLINE bool Ivy_IO_FileExists(const char *filename)
{
    return ACCESS(filename, F_OK) == 0;
}

#ifdef __cplusplus
}
#endif

#endif