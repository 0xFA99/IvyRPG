#ifndef IVY_UTILS_H
#define IVY_UTILS_H

#include "ivy/types.h"
#include "ivy/virtual.h"
#include "raylib/raylib.h"

#include <stdio.h>


void    ReadExact(FILE *file, void *dest, size_t n);
u8     *ReadString(FILE *file);

Texture2D LoadTextureFromBin(const char *path);
Texture2D LoadTextureFromImageBin(const char *path);
Font LoadFontBin(const char *path, int fontSize);
Vector2 GetScreenPos(const VirtualResolution *vr, Vector2 vp);

#endif