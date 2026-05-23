#include "ivy/core/types.h"
#include "ivy/systems/asset_manager.h"
#include "ivy/arena/linear.h"

#include <stdio.h>
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

#ifdef IVY_DEBUG
    const char MAGIC[4] = { 'I', 'V', 'Y', 'H' };
    const u32 CURRENT_VERSION = 1;
#endif

typedef struct {
    char     magic[4];
    u32      version;
    u32      table_size;
} IvyHeader;
IVY_ASSERT_STATIC(sizeof(IvyHeader) == 12, "IvyHeader must be 12 bytes!");

#ifdef _WIN32
static void Ivy_ClosePlatformHandles(IvyAssetManager *mgr)
{
    if (mgr->mapped_data) {
        UnmapViewOfFile(mgr->mapped_data);
        mgr->mapped_data = NULL;
    }
    if (mgr->h_map != NULL && mgr->h_map != INVALID_HANDLE_VALUE) {
        CloseHandle(mgr->h_map);
        mgr->h_map = NULL;
    }
    if (mgr->h_file != NULL && mgr->h_file != INVALID_HANDLE_VALUE) {
        CloseHandle(mgr->h_file);
        mgr->h_file = INVALID_HANDLE_VALUE;
    }
}

static bool Ivy_MapDataFile(IvyAssetManager *mgr, const char *data_path)
{
    // FILE_FLAG_SEQUENTIAL_SCAN: hint to cache manager for bulk read.
    mgr->h_file = CreateFileA(data_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
        NULL
    );

    if (mgr->h_file == INVALID_HANDLE_VALUE) return false;

    LARGE_INTEGER file_size;
    if (!GetFileSizeEx(mgr->h_file, &file_size)) {
        Ivy_ClosePlatformHandles(mgr);
        return false;
    }

    mgr->data_size = (usize)file_size.QuadPart;

    if (mgr->data_size == 0) {
        Ivy_ClosePlatformHandles(mgr);
        return false;
    }

    // PAGE_READONLY + FILE_MAP_READ: read-only mapping.
    mgr->h_map = CreateFileMappingA(mgr->h_file, NULL, PAGE_READONLY, 0, 0, NULL);

    if (!mgr->h_map) {
        Ivy_ClosePlatformHandles(mgr);
        return false;
    }

    mgr->mapped_data = MapViewOfFile(mgr->h_map, FILE_MAP_READ, 0, 0, 0);

    if (!mgr->mapped_data) {
        Ivy_ClosePlatformHandles(mgr);
        return false;
    }

    return true;
}

#else

static void Ivy_ClosePlatformHandles(IvyAssetManager *mgr)
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

static bool Ivy_MapDataFile(IvyAssetManager *mgr, const char *data_path)
{
    mgr->fd = open(data_path, O_RDONLY);
    if (mgr->fd == -1) return false;

    struct stat st;
    if (fstat(mgr->fd, &st) == -1) {
        Ivy_ClosePlatformHandles(mgr);
        return false;
    }

    mgr->data_size = (usize)st.st_size;
    if (mgr->data_size == 0) {
        Ivy_ClosePlatformHandles(mgr);
        return false;
    }

    // MAP_PRIVATE: copy-on-write.
    mgr->mapped_data = mmap(NULL, mgr->data_size, PROT_READ, MAP_PRIVATE, mgr->fd, 0);
    if (mgr->mapped_data == MAP_FAILED) {
        mgr->mapped_data = NULL;
        Ivy_ClosePlatformHandles(mgr);
        return false;
    }

    // hint sequential access pattern. SSOONN!!!
    // madvise(mgr->mapped_data, mgr->data_size, MADV_SEQUENTIAL);

    return true;
}
#endif


IvyAssetManager* Ivy_AssetManager_Init(IvyArenaLinear *restrict arena, const char *restrict header_path, const char *restrict data_path)
{
    IVY_ASSERT(arena != NULL, "Arena is NULL");
    IVY_ASSERT(header_path != NULL, "header_path is NULL");
    IVY_ASSERT(data_path != NULL, "data_path is NULL");

    FILE *f_header = fopen(header_path, "rb");
    IVY_CHECK(f_header != NULL, "[AssetManager] Failed to open header: %s\n", header_path);

    IvyHeader header;
    const usize read_count = fread(&header, sizeof(header), 1, f_header);
    IVY_CHECK(read_count == 1, "[AssetManager] Failed to read header struct!", NULL);

#ifdef IVY_DEBUG
    IVY_CHECK(memcmp(header.magic, MAGIC, 4) == 0, "[AssetManager] Invalid magic number!", NULL);
    IVY_CHECK(header.version == CURRENT_VERSION, "[AssetManger] Version mismatch: %u", header.version);
#endif
    IVY_CHECK(header.table_size > 0 && Ivy_Arena_IsPowerOfTwo(header.table_size),
                "[AssetManager] Invalid table size: %u", header.table_size);

    // snapshot before allocation.
    // const IvyArenaLinearSnapshot snapshot = Ivy_Arena_LinearGetSnapshot(arena);

    IvyAssetManager *mgr = (IvyAssetManager*)Ivy_Arena_LinearAllocZero(arena, sizeof(IvyAssetManager));
    IVY_ENSURE(mgr != NULL);

    mgr->table = (IvyAssetEntry*)Ivy_Arena_LinearAlloc(arena, sizeof(IvyAssetEntry) * header.table_size);
    if (IVY_UNLIKELY(!mgr->table)) {
        IVY_ASSERT(false, "[AssetManager] Arena OOM for table size %u", header.table_size);

        // Unreachable codes
        // fclose(f_header);
        // Ivy_Arena_LinearRestore(arena, snapshot);
        // return NULL;
    }

    mgr->table_mask = header.table_size - 1;

    const usize entries_read = fread(mgr->table, sizeof(IvyAssetEntry), header.table_size, f_header);
    fclose(f_header);

    if (IVY_UNLIKELY(entries_read != header.table_size)) {
        IVY_CHECK(false, "[AssetManager] Table entry mismatch (read %zu/%u)", entries_read, header.table_size);
        // Unreachable codes
        // Ivy_Arena_LinearRestore(arena, snapshot);
        // return NULL;
    }

    const bool map_ok = Ivy_MapDataFile(mgr, data_path);
    if (IVY_UNLIKELY(!map_ok)) {
        IVY_CHECK(false, "[AssetManager] Failed to map data file: %s", data_path);
        // Unreachable codes
        // Ivy_Arena_LinearRestore(arena, snapshot);
        // return NULL;
    }

    return mgr;
}

void Ivy_AssetManager_Destroy(IvyAssetManager *mgr)
{
    IVY_ENSURE(mgr != NULL);
    Ivy_ClosePlatformHandles(mgr);
}
