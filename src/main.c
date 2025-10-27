#include "raylib.h"

// Catppuccin Mocha selected colors
#define BACKGROUND  CLITERAL(Color){ 30, 30, 46, 255 }
#define TEXT        CLITERAL(Color){ 205, 214, 244, 255 }
#define OVERLAY_0   CLITERAL(Color){ 108, 112, 134, 255 }
#define SURFACE_1   CLITERAL(Color){ 69, 71, 90, 255 }
#define MANTLE      CLITERAL(Color){ 24, 24, 37, 255 }
#define CRUST       CLITERAL(Color){ 17, 17, 27, 255 }

typedef struct Splitter {
    Rectangle rect;
    bool dragging;
    float ratio;
    float height;
    float y;
} Splitter;

typedef struct Scrollbar {
    Rectangle track;        // The bar background area
    Rectangle thumb;        // The draggable thumb
    bool dragging;          // Is user dragging it?
    float grabOffset;       // Mouse offset from thumb start
    float value;            // 0.0 -> 1.0 (scroll position ratio)
} Scrollbar;

typedef struct Zone {
    Rectangle bounds;       // visible area
    Vector2 scroll;         // current scroll offset
    Vector2 contentSize;    // virtual content size
    Scrollbar vScrollbar;
    Scrollbar hScrollbar;
} Zone;

// Clamp scroll so it never exceeds content limits
void ClampZoneScroll(Zone *zone) {
    if (zone->scroll.x < 0) zone->scroll.x = 0;
    if (zone->scroll.y < 0) zone->scroll.y = 0;
    if (zone->scroll.x > zone->contentSize.x - zone->bounds.width)
        zone->scroll.x = zone->contentSize.x - zone->bounds.width;
    if (zone->scroll.y > zone->contentSize.y - zone->bounds.height)
        zone->scroll.y = zone->contentSize.y - zone->bounds.height;
}

// Helper: check whether mouse is inside main window area
// (useful to cancel dragging if cursor leaves window)
static bool MouseInsideWindow(void) {
    Vector2 m = GetMousePosition();
    return (m.x >= 0 && m.y >= 0 && m.x <= GetScreenWidth() && m.y <= GetScreenHeight());
}

static bool MouseInsideZone(Zone *zone) {
    Vector2 m = GetMousePosition();
    return (m.x >= zone->bounds.x && m.y >= zone->bounds.y && m.x <= zone->bounds.x + zone->bounds.width && m.y <= zone->bounds.y + zone->bounds.height);
}

void HandleZoneSplit(Splitter *splitter, int screenWidth, int screenHeight) {
    // Convert ratio -> Y position
    splitter->y = splitter->ratio * screenHeight;

    Vector2 mouse = GetMousePosition();
    // Handle splitter drag
    splitter->rect = (Rectangle){0, splitter->y - splitter->height/2, (float)screenWidth, splitter->height};
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse, splitter->rect))
        splitter->dragging = true;
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        splitter->dragging = false;
    if (splitter->dragging) {
        SetMouseCursor(MOUSE_CURSOR_RESIZE_NS);
        splitter->y = mouse.y;
        if (splitter->y < 10) splitter->y = 10;
        if (splitter->y > screenHeight - 10) splitter->y = screenHeight - 10;
        splitter->ratio = splitter->y / screenHeight;
    }
}

void InitZoneScrollbars(Zone *z)
{
    // Vertical scrollbar
    z->vScrollbar.value = z->bounds.height / z->contentSize.y;
    z->vScrollbar.track = (Rectangle){ z->bounds.x + z->bounds.width - 10, z->bounds.y, 10, z->bounds.height - 10 };
    z->vScrollbar.thumb = (Rectangle){
        z->vScrollbar.track.x,
        z->vScrollbar.track.y + (z->scroll.y / (z->contentSize.y - z->bounds.height)) * (z->vScrollbar.track.height - z->vScrollbar.value * z->vScrollbar.track.height),
        10,
        z->vScrollbar.value * z->vScrollbar.track.height
    };

    // Horizontal scrollbar
    z->hScrollbar.value = z->bounds.width / z->contentSize.x;
    z->hScrollbar.track = (Rectangle){ z->bounds.x, z->bounds.y + z->bounds.height - 10, z->bounds.width - 10, 10 };
    z->hScrollbar.thumb = (Rectangle){
        z->hScrollbar.track.x + (z->scroll.x / (z->contentSize.x - z->bounds.width)) * (z->hScrollbar.track.width - z->hScrollbar.value * z->hScrollbar.track.width),
        z->hScrollbar.track.y,
        z->hScrollbar.value * z->hScrollbar.track.width,
        10
    };
}

// Helper: handle mouse wheel and drag to scroll
void UpdateZoneScroll(Zone *zone)
{
    InitZoneScrollbars(zone);
    Vector2 mouse = GetMousePosition();

    if (!IsWindowFocused() || !MouseInsideZone(zone)) {
        zone->vScrollbar.dragging = false;
        zone->hScrollbar.dragging = false;
    }

    // Only scroll if mouse inside the zone
    if (CheckCollisionPointRec(mouse, zone->bounds))
    {
        if (IsKeyDown(KEY_LEFT_SHIFT)) {
            // Mouse wheel scroll (horizontal)
            zone->scroll.x -= GetMouseWheelMove() * 40;
        } else {
            // Mouse wheel scroll (vertical)
            zone->scroll.y -= GetMouseWheelMove() * 40;
        }

        // Handle dragging vertical scrollbar
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse, zone->vScrollbar.thumb)) {
            zone->vScrollbar.dragging = true;
            zone->vScrollbar.grabOffset = mouse.y - zone->vScrollbar.thumb.y;
        }
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) zone->vScrollbar.dragging = false;

        if (zone->vScrollbar.dragging) {
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
            float newY = mouse.y - zone->vScrollbar.grabOffset;
            float posRatio = (newY - zone->vScrollbar.track.y) / (zone->vScrollbar.track.height - zone->vScrollbar.thumb.height);
            if (posRatio < 0.0f) posRatio = 0.0f;
            if (posRatio > 1.0f) posRatio = 1.0f;
            zone->vScrollbar.value = posRatio;
            zone->scroll.y = posRatio * (zone->contentSize.y - zone->bounds.height);
        }

        // Handle dragging horizontal scrollbar
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse, zone->hScrollbar.thumb)) {
            zone->hScrollbar.dragging = true;
            zone->hScrollbar.grabOffset = mouse.x - zone->hScrollbar.thumb.x;
        }
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) zone->hScrollbar.dragging = false;

        if (zone->hScrollbar.dragging) {
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
            float newX = mouse.x - zone->hScrollbar.grabOffset;
            float posRatio = (newX - zone->hScrollbar.track.x) / ( zone->hScrollbar.track.width -  zone->hScrollbar.thumb.width);
            if (posRatio < 0.0f) posRatio = 0.0f;
            if (posRatio > 1.0f) posRatio = 1.0f;
            zone->hScrollbar.value = posRatio;
            zone->scroll.x = posRatio * (zone->contentSize.x - zone->bounds.width);
        }

        ClampZoneScroll(zone);
    }
}

void DrawScrollbars(Zone *zone) {
    DrawRectangleRec(zone->vScrollbar.track, MANTLE);
    DrawRectangleRec(zone->vScrollbar.thumb, zone->vScrollbar.dragging ? SURFACE_1 : OVERLAY_0);
    DrawRectangleRec(zone->hScrollbar.track, MANTLE);
    DrawRectangleRec(zone->hScrollbar.thumb, zone->hScrollbar.dragging ? SURFACE_1 : OVERLAY_0);
}

void DrawZone(Zone *zone, Color bgColor, Color contentColor)
{
    BeginScissorMode(zone->bounds.x, zone->bounds.y, zone->bounds.width, zone->bounds.height);
    ClearBackground(bgColor);

    DrawText("Scrollable Content", zone->bounds.x + 20 - zone->scroll.x, zone->bounds.y + 20 - zone->scroll.y, 20, contentColor);

    EndScissorMode();

    DrawScrollbars(zone);
}

int main(void)
{
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "QQ");

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

        DrawZone(&topZone, BACKGROUND, TEXT);
        DrawZone(&bottomZone, BACKGROUND, TEXT);

        DrawRectangleRec(splitter.rect, splitter.dragging ? CRUST : SURFACE_1);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

