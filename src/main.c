#include "raylib.h"

#include "utilities.h"
#include "assets.h"
#include "display_screen.h"

int main(void)
{
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "QQ");

    Assets assets = {0};
    LoadAssets(&assets);

    Splitter splitter;
    splitter.ratio = 0.5f;
    splitter.height = 4.0f;
    splitter.dragging = false;

    Zone topZone = {0};
    Zone bottomZone = {0};

    // Example content sizes (larger than screen)
    topZone.contentSize = (Vector2){1600, 1200};
    bottomZone.contentSize = (Vector2){2000, 2000};

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        screenWidth = GetScreenWidth();
        if (screenWidth < 100) screenWidth = 100;
        screenHeight = GetScreenHeight();
        if (screenHeight < 100) screenHeight = 100;

        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        HandleZoneSplit(&splitter, screenWidth, screenHeight);

        // Update zones’ bounds and contentSize
        topZone.bounds = (Rectangle){0, 0, screenWidth, splitter.y - splitter.height/2};
        bottomZone.bounds = (Rectangle){0, splitter.y + splitter.height/2, screenWidth, screenHeight - (splitter.y + splitter.height/2)};

        // @TODO: Fix to get content into consideration
        if (topZone.contentSize.x < screenWidth)
            topZone.contentSize = (Vector2){screenWidth, 1200};
        if (bottomZone.contentSize.x < screenWidth)
            bottomZone.contentSize = (Vector2){screenWidth, 2000};

        // Update scroll independently
        UpdateZoneScroll(&topZone);
        UpdateZoneScroll(&bottomZone);

        // --- Drawing ---
        BeginDrawing();
        ClearBackground(BACKGROUND);

        // DrawTextEx(fnt, "Font test", (Vector2){50, 50}, 32, 2.0f, TEXT);

        DrawZone(&topZone, BACKGROUND, TEXT);
        DrawDisplayZone(&bottomZone, &assets);

        DrawRectangleRec(splitter.rect, splitter.dragging ? CRUST : SURFACE_1);

        EndDrawing();
    }

    UnloadAssets(&assets);
    CloseWindow();
    return 0;
}

