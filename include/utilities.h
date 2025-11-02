#include "raylib.h"

#ifndef UTILITIES_QQ_H
#define UTILITIES_QQ_H

// Catppuccin Mocha selected colors
#define BACKGROUND  CLITERAL(Color){ 30, 30, 46, 255 }
#define TEXT        CLITERAL(Color){ 205, 214, 244, 255 }
#define OVERLAY_0   CLITERAL(Color){ 108, 112, 134, 255 }
#define SURFACE_1   CLITERAL(Color){ 69, 71, 90, 255 }
#define SURFACE_0   CLITERAL(Color){ 49, 50, 68, 255 }
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

bool MouseInsideWindow(void);
bool MouseInsideZone(Zone *zone);

void ClampZoneScroll(Zone *zone);
void InitZoneScrollbars(Zone *z);
void DrawScrollbars(Zone *zone);
void UpdateZoneScroll(Zone *zone);

void HandleZoneSplit(Splitter *splitter, int screenWidth, int screenHeight);
void DrawZone(Zone *zone, Color bgColor, Color contentColor);

#endif
