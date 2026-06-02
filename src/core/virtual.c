#include "ivy/arena/linear.h"
#include "ivy/core/types.h"
#include "ivy/core/virtual.h"
#include "ivy/utils/forward.h"

#include <math.h>

#define INV_VIRTUAL_WIDTH   (1.0f / VIRTUAL_WIDTH)
#define INV_VIRTUAL_HEIGHT  (1.0f / VIRTUAL_HEIGHT)

IvyVirtualScreen *Ivy_VirtualScreen_Init(IvyArenaLinear *arena, const Vector2 size)
{
    IVY_ASSERT(arena, "[VirtualScreen] Arena not found!");

    IvyVirtualScreen *vs = Ivy_Arena_LinearAllocZero(arena, sizeof(IvyVirtualScreen));
    IVY_ENSURE(vs != NULL);

    vs->target = LoadRenderTexture(VIRTUAL_WIDTH, VIRTUAL_HEIGHT);

    // negative height flips Y because OpenGL texture origin is bottom-left.
    vs->source = (Rectangle){ 0, 0, VIRTUAL_WIDTH, -VIRTUAL_HEIGHT };

    SetTextureFilter(vs->target.texture, TEXTURE_FILTER_BILINEAR);

    Ivy_VirtualScreen_Update(vs, size);

    return vs;
}

void Ivy_VirtualScreen_Update(IvyVirtualScreen *vs, const Vector2 size)
{
    if (IVY_UNLIKELY(size.x <= 0 || size.y <= 0)) return;

    const float scaleX = size.x * INV_VIRTUAL_WIDTH;
    const float scaleY = size.y * INV_VIRTUAL_HEIGHT;

    // pick the smaller scale to fit fully.
    vs->scale = (scaleX < scaleY) ? scaleX : scaleY;

    const float scaledW = VIRTUAL_WIDTH  * vs->scale;
    const float scaledH = VIRTUAL_HEIGHT * vs->scale;

    vs->destination = (Rectangle){
        .x      = floorf((size.x - scaledW) * 0.5f),
        .y      = floorf((size.y - scaledH) * 0.5f),
        .width  = floorf(scaledW),
        .height = floorf(scaledH)
    };
}

void Ivy_VirtualScreen_Draw(const IvyVirtualScreen *vs)
{
    IVY_ASSERT(vs != NULL, "[VirtualScreen] Instant is NULL!");
    DrawTexturePro(vs->target.texture, vs->source, vs->destination, (Vector2){0}, 0.0f, WHITE);
}

void Ivy_VirtualScreen_Unload(const IvyVirtualScreen *vs)
{
    IVY_ASSERT(vs != NULL, "[VirtualScreen] Instant is NULL!");

    if (IVY_LIKELY(vs != NULL)) {
        UnloadRenderTexture(vs->target);
    }
}
