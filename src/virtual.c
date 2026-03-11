#include "ivy/virtual.h"

#include <math.h>

VirtualResolution InitVirtualScreen(const u32 sw, const u32 sh)
{
    VirtualResolution vr = {0};
    vr.target = LoadRenderTexture(VIRTUAL_WIDTH, VIRTUAL_HEIGHT);
    vr.source = (Rectangle){ 0, 0, VIRTUAL_WIDTH, -VIRTUAL_HEIGHT };

    SetTextureFilter(vr.target.texture, TEXTURE_FILTER_POINT);

    UpdateVirtualResolution(&vr, sw, sh);

    return vr;
}

void UpdateVirtualResolution(VirtualResolution *vr, const u32 sw, const u32 sh)
{
    const float fSW = (float)sw;
    const float fSH = (float)sh;

    const float scaleX = fSW / VIRTUAL_WIDTH;
    const float scaleY = fSH / VIRTUAL_HEIGHT;

    vr->scale = (scaleX < scaleY) ? scaleX : scaleY;

    const float scaledW = VIRTUAL_WIDTH  * vr->scale;
    const float scaledH = VIRTUAL_HEIGHT * vr->scale;

    vr->destination = (Rectangle){
        .x      = floorf((fSW - scaledW) * 0.5f),
        .y      = floorf((fSH - scaledH) * 0.5f),
        .width  = floorf(scaledW),
        .height = floorf(scaledH)
    };
}

void DrawVirtualResolution(const VirtualResolution *vr)
{
    DrawTexturePro(vr->target.texture, vr->source, vr->destination, (Vector2){0}, 0.0f, WHITE);
}