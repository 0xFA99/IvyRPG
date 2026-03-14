#ifndef IVY_COLLISION_H
#define IVY_COLLISION_H

#include "ivy/tilemap/tilemap.h"

typedef struct {
    Rectangle  *rect;
    u32         rectCount;
} Collision;

Collision  *InitCollisionAllLayers(const Tilemap *tilemap);
void        DestroyCollision(Collision *collision);

#endif