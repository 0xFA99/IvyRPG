#include "ivy/core/types.h"
#include "ivy/arena/linear.h"
#include "ivy/systems/profile_manager.h"
#include "ivy/utils/file_ids.h"

#include <string.h>
#include <stdbool.h>

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
#else
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <unistd.h>
#endif

#define SAVE_MAGIC_0 'I'
#define SAVE_MAGIC_1 'V'
#define SAVE_MAGIC_2 'Y'
#define SAVE_MAGIC_3 'S'
#define SAVE_CURRENT_VERSION 1

typedef struct {
    char magic[4];        // 4
    u32  version;         // 4
    u32  payload_size;    // 4
} IvySaveHeader;          // 12
IVY_ASSERT_STATIC(sizeof(IvySaveHeader) == 12, "IvySaveHeader must be 12 bytes!");

#define SAVE_FILE_SIZE (sizeof(IvySaveHeader) + sizeof(IvySaveSystem))

#ifdef _WIN32

static void Ivy_SaveClose(IvySaveManager *mgr)
{
    if (mgr->mapped_data) {
        UnmapViewOfFile(mgr->mapped_data);
        mgr->mapped_data = NULL;
    }
    if (mgr->h_map && mgr->h_map != INVALID_HANDLE_VALUE) {
        CloseHandle(mgr->h_map);
        mgr->h_map = NULL;
    }
    if (mgr->h_file && mgr->h_file != INVALID_HANDLE_VALUE) {
        CloseHandle(mgr->h_file);
        mgr->h_file = INVALID_HANDLE_VALUE;
    }
}

static bool Ivy_SaveMap(IvySaveManager *mgr, const char *path)
{
    mgr->h_file = CreateFileA(
        path,
        GENERIC_READ | GENERIC_WRITE,
        0, NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (mgr->h_file == INVALID_HANDLE_VALUE) return false;

    LARGE_INTEGER file_size;
    if (!GetFileSizeEx(mgr->h_file, &file_size)) goto fail;

    if ((usize)file_size.QuadPart != SAVE_FILE_SIZE) {
        LARGE_INTEGER target;
        target.QuadPart = (LONGLONG)SAVE_FILE_SIZE;
        if (!SetFilePointerEx(mgr->h_file, target, NULL, FILE_BEGIN)) goto fail;
        if (!SetEndOfFile(mgr->h_file)) goto fail;
    }

    mgr->data_size = SAVE_FILE_SIZE;

    mgr->h_map = CreateFileMappingA(mgr->h_file, NULL, PAGE_READWRITE, 0, 0, NULL);
    if (!mgr->h_map) goto fail;

    mgr->mapped_data = MapViewOfFile(mgr->h_map, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (!mgr->mapped_data) goto fail;

    return true;

fail:
    Ivy_SaveClose(mgr);
    return false;
}

#else

static void Ivy_SaveClose(IvySaveManager *mgr)
{
    if (mgr->mapped_data && mgr->mapped_data != MAP_FAILED) {
        munmap(mgr->mapped_data, mgr->data_size);
        mgr->mapped_data = NULL;
    }
    if (mgr->fd != -1) {
        close(mgr->fd);
        mgr->fd = -1;
    }
}

static bool Ivy_SaveMap(IvySaveManager *mgr, const char *path)
{
    mgr->fd = open(path, O_RDWR | O_CREAT, 0644);
    if (mgr->fd == -1) return false;

    struct stat st;
    if (fstat(mgr->fd, &st) == -1) goto fail;

    if ((usize)st.st_size != SAVE_FILE_SIZE) {
        if (ftruncate(mgr->fd, (off_t)SAVE_FILE_SIZE) == -1) goto fail;
    }

    mgr->data_size   = SAVE_FILE_SIZE;
    mgr->mapped_data = mmap(NULL, mgr->data_size,
                            PROT_READ | PROT_WRITE, MAP_SHARED,
                            mgr->fd, 0);
    if (mgr->mapped_data == MAP_FAILED) goto fail;

    return true;

fail:
    Ivy_SaveClose(mgr);
    return false;
}

bool Ivy_SaveManager_Flush(IvySaveManager *mgr)
{
    IVY_ASSERT(mgr != NULL,          "[SaveManager] Flush: mgr is NULL!");
    IVY_ASSERT(mgr->mapped_data != NULL, "[SaveManager] Flush: not mapped!");

    return msync(mgr->mapped_data, mgr->data_size, MS_SYNC) == 0;
}

#endif

static void Ivy_SaveHeader_Init(void *mapped_data)
{
    IvySaveHeader *hdr   = (IvySaveHeader *)mapped_data;
    hdr->magic[0]        = SAVE_MAGIC_0;
    hdr->magic[1]        = SAVE_MAGIC_1;
    hdr->magic[2]        = SAVE_MAGIC_2;
    hdr->magic[3]        = SAVE_MAGIC_3;
    hdr->version         = SAVE_CURRENT_VERSION;
    hdr->payload_size    = (u32)sizeof(IvySaveSystem);

    IvySaveSystem *sys   = (IvySaveSystem *)((u8 *)mapped_data + sizeof(IvySaveHeader));
    memset(sys, 0, sizeof(IvySaveSystem));
    sys->profile.localeID = ASSET_LOCALES_EN_BIN;
}

IvySaveManager *Ivy_SaveManager_Init(IvyArenaLinear *restrict arena,
                                     const char     *restrict path)
{
    IVY_ASSERT(arena != NULL, "[SaveManager] arena is NULL!");
    IVY_ASSERT(path  != NULL, "[SaveManager] path is NULL!");

    IvySaveManager *mgr = (IvySaveManager *)Ivy_Arena_LinearAllocZero(arena, sizeof(IvySaveManager));
    IVY_ENSURE(mgr != NULL);

#ifndef _WIN32
    mgr->fd = -1;
#else
    mgr->h_file = INVALID_HANDLE_VALUE;
#endif

    Ivy_SaveMap(mgr, path);

    IvySaveHeader *hdr = (IvySaveHeader *)mgr->mapped_data;

    const bool is_blank =
        hdr->magic[0] == 0 && hdr->magic[1] == 0 &&
        hdr->magic[2] == 0 && hdr->magic[3] == 0;

    if (is_blank) {
        Ivy_SaveHeader_Init(mgr->mapped_data);
        Ivy_SaveManager_Flush(mgr);
    }
#ifdef IVY_DEBUG
    else {
        const bool status = hdr->magic[0] == SAVE_MAGIC_0 && hdr->magic[1] == SAVE_MAGIC_1 &&
                            hdr->magic[2] == SAVE_MAGIC_2 && hdr->magic[3] == SAVE_MAGIC_3;

        IVY_CHECK(status, "[SaveManager] Invalid magic in: %s", path);

        IVY_CHECK(hdr->version == SAVE_CURRENT_VERSION, "[SaveManager] Version mismatch: got %u, expected %u",
                  hdr->version, SAVE_CURRENT_VERSION);

        IVY_CHECK(hdr->payload_size == (u32)sizeof(IvySaveSystem), "[SaveManager] Payload size mismatch: got %u, expected %zu",
                  hdr->payload_size, sizeof(IvySaveSystem));
    }
#endif

    mgr->save = (IvySaveSystem *)((u8 *)mgr->mapped_data + sizeof(IvySaveHeader));
    return mgr;
}

void Ivy_SaveManager_Destroy(IvySaveManager *mgr)
{
    IVY_ENSURE(mgr != NULL);
    Ivy_SaveManager_Flush(mgr);
    Ivy_SaveClose(mgr);
    mgr->save = NULL;
}
