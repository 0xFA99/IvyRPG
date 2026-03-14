#include "ivy/utils.h"

#include <assert.h>
#include <stdlib.h>
#include <math.h>

void ReadExact(FILE *file, void *dest, const size_t n)
{
    const size_t bytes = fread(dest, 1, n, file);
    assert(bytes == n && "[ERROR] Failed to read file!");
}

u8 *ReadString(FILE *file)
{
    u32 len = 0;
    ReadExact(file, &len, sizeof(u32));

    u8 *str = malloc(len + 1);
    assert(str && "[ERROR] Failed to allocate string buffer!");

    ReadExact(file, str, len);
    str[len] = '\0';
    return str;
}

Texture2D LoadTextureFromImageBin(const char *path)
{
    FILE *f = fopen(path, "rb");
    assert(f && "[ERROR] Failed to open binary file!");

    int header[4];  // width, height, mipmaps, format
    fread(header, sizeof(int), 4, f);

    int dataSize = header[0] * header[1] * 4;  // RGBA = 4 bytes/pixel
    void *data = malloc(dataSize);
    fread(data, 1, dataSize, f);
    fclose(f);

    Image image = {
        .data = data,
        .width = header[0],
        .height = header[1],
        .mipmaps = header[2],
        .format = header[3]
    };
    Texture2D tex = LoadTextureFromImage(image);
    free(data);
    return tex;
}

Font LoadFontBin(const char *path, const int fontSize)
{
    FILE *f = fopen(path, "rb");
    if (!f) return (Font){0};
    assert(f && "[ERROR] Failed to open binary file!");

    fseek(f, 0, SEEK_END);
    const long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    unsigned char *data = malloc(size);
    assert(f && "[ERROR] Failed to allocate memory for fontData!");
    fread(data, 1, size, f);
    fclose(f);

    const Font font = LoadFontFromMemory(".ttf", data, size, fontSize, NULL, 95);

    free(data);
    return font;
}

Vector2 GetScreenPos(const VirtualResolution *vr, const Vector2 vp)
{
    const float scale = vr->scale;

    return (Vector2) {
        .x = floorf(vp.x * scale + vr->destination.x),
        .y = floorf(vp.y * scale + vr->destination.y)
    };
}