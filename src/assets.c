#include <stdlib.h>

#include "assets.h"
#include "resource_dir.h"

void LoadAssets(Assets *assets) {
    SearchAndSetResourceDir("resources");
    // @WARN: Font load needs to be done after InitWindow
    assets->mainFontSize = 21;
    assets->mainFontSpacing = 1.0f;
    assets->mainFont = LoadFontEx("FiraCodeNerdFontMono-Regular.ttf", assets->mainFontSize, NULL, 0);
    if (!IsFontValid(assets->mainFont)) {
        TraceLog(LOG_ERROR, "Font failed to load!");
    }
    assets->mainFontCharacterWidth = MeasureTextEx(assets->mainFont, "X", assets->mainFontSize, assets->mainFontSpacing).x;
}

void UnloadAssets(Assets *assets) {
    UnloadFont(assets->mainFont);
}
