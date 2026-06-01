#include "ivy/graphics/gfx.h"
#include "ivy/systems/asset_manager.h"
#include "ivy/utils/io.h"

#include "raylib/external/glad.h"

#include <math.h>

#define CODEPOINTS      95
#define LINE_SPACING    2

// rltexgpu.h
typedef struct {
    u32 size;
    u32 flags;
    u32 fourcc;
    u32 rgb_bit_count;
    u32 r_bit_mask;
    u32 g_bit_mask;
    u32 b_bit_mask;
    u32 a_bit_mask;
} dds_pixel_format;

typedef struct {
    u32 size;
    u32 flags;
    u32 height;
    u32 width;
    u32 pitch_or_linear_size;
    u32 depth;
    u32 mipmap_count;
    u32 reserved1[11];
    dds_pixel_format ddspf;
    u32 caps;
    u32 caps2;
    u32 caps3;
    u32 caps4;
    u32 reserved2;
} dds_header;

typedef struct {
    u32 codepoint;
    u16 x, y;
    u16 width, height;
    i16 xoffset, yoffset;
    u16 xadvance, padding;
} IvyGlyphInfo;

typedef struct {
    IvyGlyphInfo glyphs[CODEPOINTS];
    u32          atlasWidth;
    u32          atlasHeight;
} IvyFontAtlasHeader;

Texture2D Ivy_Gfx_LoadTextureDDS(IvyAssetManager *mgr, const u32 id)
{
    usize data_size;
    const u8 *data = (const u8 *)Ivy_Asset_Get(mgr, id, &data_size);

    // standard DDS, pixel data always starts at 128 bytes.
    const void *pixelData = data + 128;

    u32 texId;
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    u32 width, height;
    memcpy(&width, data + 4 + offsetof(dds_header, width), sizeof(u32));
    memcpy(&height, data + 4 + offsetof(dds_header, height), sizeof(u32));

    // DXT5, width * height (1 byte per 4x4 block)
    glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, (int)width, (int)height, 0, (int)(width * height), pixelData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindTexture(GL_TEXTURE_2D, 0);

    return (Texture2D) { texId, (int)width, (int)height, 1, PIXELFORMAT_COMPRESSED_DXT5_RGBA };
}

void Ivy_Gfx_UnloadTexture(Texture2D *texture)
{
    glDeleteTextures(1, &texture->id);
    texture->id = 0;
}

Font Ivy_Gfx_LoadFont(IvyArenaLinear *restrict arena, IvyAssetManager *restrict mgr,
                      const u32 metaId, const u32 atlasId, const int fontSize)
{
    IVY_ASSERT(mgr != NULL, "[Gfx] Asset manager is NULL!");

    usize meta_size;
    const Texture2D atlasTex = Ivy_Gfx_LoadTextureDDS(mgr, atlasId);

    GlyphInfo *glyphs = Ivy_Arena_LinearAlloc(arena, sizeof(GlyphInfo) * CODEPOINTS);
    Rectangle *recs   = Ivy_Arena_LinearAlloc(arena, sizeof(Rectangle) * CODEPOINTS);

    IVY_ASSERT(glyphs != NULL, "[Gfx] Failed to allocate glyphs!");
    IVY_ASSERT(recs != NULL, "[Gfx] Failed to allocate rectangles!");

    const Font font = {
        .baseSize     = fontSize,
        .glyphCount   = CODEPOINTS,
        .glyphPadding = 1,
        .texture      = atlasTex,
        .recs         = recs,
        .glyphs       = glyphs
    };

    const u8 *rawData = (const u8 *)Ivy_Asset_Get(mgr, metaId, &meta_size);
    const u8 *glyphsStart = rawData;

    for (i32 i = 0; i < CODEPOINTS; i++)
    {
        const u8 *ptr = glyphsStart + (i * 20);

        i32 codepoint;
        u16 x, y, width, height, xadvance;
        i16 xoffset, yoffset;

        memcpy(&codepoint, ptr + 0,  4);
        memcpy(&x,         ptr + 4,  2);
        memcpy(&y,         ptr + 6,  2);
        memcpy(&width,     ptr + 8,  2);
        memcpy(&height,    ptr + 10, 2);
        memcpy(&xoffset,   ptr + 12, 2);
        memcpy(&yoffset,   ptr + 14, 2);
        memcpy(&xadvance,  ptr + 16, 2);

        font.glyphs[i].value    = codepoint;
        font.glyphs[i].offsetX  = (i32)xoffset;
        font.glyphs[i].offsetY  = (i32)yoffset;
        font.glyphs[i].advanceX = (i32)xadvance;
        font.glyphs[i].image    = (Image){0};

        font.recs[i].x          = (float)x;
        font.recs[i].y          = (float)y;
        font.recs[i].width      = (float)width;
        font.recs[i].height     = (float)height;
    }

    return font;
}

void Ivy_Gfx_UnloadFont(Font *font)
{
    // only free GPU texture, glyphs owned by arena.
    if (font && font->texture.id) {
        glDeleteTextures(1, &font->texture.id);
        font->texture.id = 0;
    }
}

void Ivy_Gfx_DrawLocaleText(const Font font, const char *text, const u32 len, const Vector2 position,
                            const float fontSize, const float spacing, const Color tint)
{
    if (IVY_UNLIKELY(len == 0)) return;

    const float scaleFactor = fontSize / (float)font.baseSize;
    const float lineHeight  = fontSize + LINE_SPACING;

    float offsetX = 0.0f;
    float offsetY = 0.0f;

    const u8 *ptr = (const u8*)text;
    const u8 *end = ptr + len;

    while (IVY_LIKELY(ptr < end))
    {
        int codepointByteCount;
        const int codepoint = GetCodepointNext((const char*)ptr, &codepointByteCount);

        if (codepoint == '\n')
        {
            offsetX = 0.0f;
            offsetY += lineHeight;
        }
        else if (codepoint != ' ' && codepoint != '\t')
        {
            const int glyphIdx = GetGlyphIndex(font, codepoint);
            const GlyphInfo *glyph = &font.glyphs[glyphIdx];
            const Rectangle *rec   = &font.recs[glyphIdx];

            const float posX = position.x + offsetX + (float)glyph->offsetX * scaleFactor;
            const float posY = position.y + offsetY + (float)glyph->offsetY * scaleFactor;

            const Rectangle src = {
                .x      = rec->x,
                .y      = rec->y,
                .width  = rec->width,
                .height = rec->height
            };

            const Rectangle dst = {
                .x      = posX,
                .y      = posY,
                .width  = rec->width * scaleFactor,
                .height = rec->height * scaleFactor
            };

            DrawTexturePro(font.texture, src, dst, (Vector2){0}, 0.0f, tint);

            offsetX += (glyph->advanceX != 0 ? (float)glyph->advanceX * scaleFactor : (float)rec->width * scaleFactor) + spacing;
        }
        else {
            int glyphIdx = GetGlyphIndex(font, codepoint);
            if (glyphIdx < 0 || glyphIdx >= CODEPOINTS) glyphIdx = 0;

            offsetX += ((float)font.glyphs[glyphIdx].advanceX * scaleFactor) + spacing;
        }

        ptr += codepointByteCount;
    }
}

Vector2 Ivy_Gfx_GetScreenPos(const IvyVirtualScreen *vr, const Vector2 vp)
{
    IVY_ASSERT(vr != NULL, "[Gfx](IvyVirtualResolution) Instance is NULL");

    return (Vector2) {
        .x = floorf(vp.x * vr->scale + vr->destination.x),
        .y = floorf(vp.y * vr->scale + vr->destination.y)
    };
}
