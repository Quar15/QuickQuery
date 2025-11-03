#ifndef ASSETS_H
#define ASSETS_H

#include "raylib.h"

typedef struct {
    Font mainFont;
    float mainFontSpacing;
    int mainFontSize;
    float mainFontCharacterWidth;
} Assets;

void LoadAssets(Assets *assets);
void UnloadAssets(Assets *assets);

#endif
