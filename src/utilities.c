#include "raylib.h"
#include <string.h>
#include <stdlib.h>

#include "utilities.h"

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
bool MouseInsideWindow(void) {
    Vector2 m = GetMousePosition();
    return (m.x >= 0 && m.y >= 0 && m.x <= GetScreenWidth() && m.y <= GetScreenHeight());
}

bool MouseInsideZone(Zone *zone) {
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

void InitZoneScrollbars(Zone *z) {
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
void UpdateZoneScroll(Zone *zone) {
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

void FreeGrid(GridData *g) {
    for (int i = 0; i < g->cols; i++) {
        free(g->header[i]);
    }
    for (int i = 0; i < g->rows; i++) {
        for (int j = 0; j < g->cols; j++) {
            free(g->data[i][j]);  // free each string
        }
        free(g->data[i]);
    }
    free(g->data);
}

void PrepareFakeGrid(GridData *grid) {
    const int MAX_ROWS = 200;
    const int MAX_COLS = 6;

    grid->rows = MAX_ROWS;
    grid->cols = MAX_COLS;

    grid->header = malloc(grid->cols * sizeof(char *));
    grid->header[0] = strdup("ID");
    grid->header[1] = strdup("Name");
    grid->header[2] = strdup("Age");
    grid->header[3] = strdup("Job");
    grid->header[4] = strdup("Country");
    grid->header[5] = strdup("Very very long column name");

    grid->data = malloc(grid->rows * sizeof(char **));
    for (int i=0; i < grid->rows; i++) {
        grid->data[i] = malloc(grid->cols * sizeof(char *));
        for (int j = 0; j < grid->cols; j++)
            grid->data[i][j] = NULL;
    }
    grid->data[0][0] = strdup("1");
    grid->data[0][1] = strdup("Alice");
    grid->data[0][2] = strdup("29");
    grid->data[0][3] = strdup("Engineer");
    grid->data[0][4] = strdup("USA");
    grid->data[0][5] = strdup("A");

    grid->data[1][0] = strdup("2");
    grid->data[1][1] = strdup("Bob the super duper ulra very very good builder that is too long");
    grid->data[1][2] = strdup("34");
    grid->data[1][3] = strdup("Designer");
    grid->data[1][4] = strdup("UK");
    grid->data[1][5] = strdup("B");

    grid->data[2][0] = strdup("3");
    grid->data[2][1] = strdup("Charlie");
    grid->data[2][2] = strdup("22");
    grid->data[2][3] = strdup("Student");
    grid->data[2][4] = strdup("Canada");
    grid->data[2][5] = strdup("C");
}
