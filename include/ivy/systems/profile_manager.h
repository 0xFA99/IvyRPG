#ifndef IVY_SYSTEM_PROFILE_MANAGER_H
#define IVY_SYSTEM_PROFILE_MANAGER_H

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

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IVY_MAX_SAVE_SLOTS 3

typedef struct {
    u32 localeID;
    int keybind[IVY_KEY_MAX];
} IvyProfile;

typedef struct {
    u32 progressLevel;
    u32 playerHP;
    u32 playerMP;
    u16 positionX;
    u16 positionY;
} IvySaveSlot;

typedef struct {
    IvyProfile  profile;
    IvySaveSlot slots[IVY_MAX_SAVE_SLOTS];
} IvySaveSystem;

struct IvySaveManager {
    IvySaveSystem   *save;          // 8
    void            *mapped_data;   // 8
    usize            data_size;     // 8
#ifdef _WIN32
    HANDLE           h_file;        // 8
    HANDLE           h_map;         // 8
#else
    i32              fd;            // 4
    u8               padding[4];       // 4
#endif
};
#ifdef _WIN32
    IVY_ASSERT_STATIC(sizeof(IvySaveManager) == 40, "[IvySaveManager] must be 40 bytes!");
#else
    IVY_ASSERT_STATIC(sizeof(IvySaveManager) == 32, "[IvySaveManager] must be 32 bytes!");
#endif

IvySaveManager *Ivy_SaveManager_Init(IvyArenaLinear *restrict arena, const char *restrict path);
void            Ivy_SaveManager_Destroy(IvySaveManager *mgr);

IVY_INLINE bool Ivy_SaveManager_Flush(const IvySaveManager *saveManager)
{
    IVY_ENSURE(saveManager != NULL);

    if (IVY_UNLIKELY(!saveManager->mapped_data)) return false;

    IVY_CHECK(FlushViewOfFile(saveManager->mapped_data, 0) && FlushFileBuffers(saveManager->h_file),
              "[SaveManager] Failed to flush");
    return true;
}

#ifdef __cplusplus
}
#endif

#endif