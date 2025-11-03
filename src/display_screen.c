#include "raylib.h"
#include <stdio.h>

#include "utilities.h"
#include "assets.h"
#include "display_screen.h"

int countDigits(int n) {
    int count = 0;

    if (n == 0) return 1;
    if (n < 0) n = -n;

    while (n > 0) {
        n /= 10;
        count++;
    }

    return count;
}

void DrawDisplayZone(Zone *zone, Assets *assets)
{
    BeginScissorMode(zone->bounds.x, zone->bounds.y, zone->bounds.width, zone->bounds.height);
    ClearBackground(BACKGROUND);
    Vector2 mouse = GetMousePosition();

    const int MAX_ROWS = 20;
    const int MAX_COLS = 5;
    const char *header[5] = {"ID", "Name", "Age", "Job", "Country"};
    const char *gridData[20][5] = {
        {"1", "Alice", "29", "Engineer", "USA"},
        {"2", "Bob", "34", "Designer", "UK"},
        {"3", "Charlie", "22", "Student", "Canada"}
    };  

    const int cellWidth = 120;
    const int cellHeight = 30;
    const int textPadding = 8;
    int contentWidth = 0;
    int contentHeight = 0;

    // Draw row counter
    int counterColumnWidth = 0;
    int counterColumnCharactersCount = countDigits(MAX_ROWS);
    for (int row = 0; row < MAX_ROWS; row++) {
        int cellX = zone->bounds.x - zone->scroll.x;
        int cellY = zone->bounds.y + (row + 1) * cellHeight - zone->scroll.y;

        Color bg = MANTLE;
        if (MouseInsideZone(zone) && (mouse.y > cellY && mouse.y < cellY + cellHeight)) {
            bg = CRUST;
        }
        DrawRectangle(cellX, cellY, cellWidth, cellHeight, bg); // Draw background
        DrawRectangleLinesEx((Rectangle){cellX, cellY, cellWidth + 1, cellHeight + 1}, 2, bg); // Draw cell border
        char rowText[8];
        snprintf(rowText, sizeof(rowText), "%d", row + 1);
        counterColumnWidth = MeasureText(rowText, 20) + (textPadding * 2);
        int counterColumnLeftPadding = textPadding + (counterColumnCharactersCount - countDigits(row + 1)) * assets->mainFontCharacterWidth;
        DrawTextEx(assets->mainFont, rowText, (Vector2){cellX + counterColumnLeftPadding, cellY + textPadding}, assets->mainFontSize, assets->mainFontSpacing, SURFACE_0); // Draw cell text
    }
    contentWidth += counterColumnWidth;

    // Draw header
    for (int col = 0; col < MAX_COLS; col++) {
        int cellX = zone->bounds.x + col * cellWidth - zone->scroll.x + counterColumnWidth;
        int cellY = zone->bounds.y - zone->scroll.y;

        Color bg = MANTLE;
        if (MouseInsideZone(zone) && (mouse.x > cellX && mouse.x < cellX + cellWidth)) {
            bg = CRUST;
        }
        DrawRectangle(cellX, cellY, cellWidth, cellHeight, bg); // Draw background
        DrawRectangleLinesEx((Rectangle){cellX, cellY, cellWidth + 1, cellHeight + 1}, 2, bg); // Draw cell border
        DrawTextEx(assets->mainFont, header[col], (Vector2){cellX + textPadding, cellY + textPadding}, assets->mainFontSize, assets->mainFontSpacing, TEXT); // Draw cell text
        contentWidth += cellWidth;
    }

    // Draw content cells
    for (int row = 0; row < MAX_ROWS; row++) {
        for (int col = 0; col < MAX_COLS; col++) {
            int cellX = zone->bounds.x + col * cellWidth - zone->scroll.x + counterColumnWidth;
            int cellY = zone->bounds.y + (row + 1) * cellHeight - zone->scroll.y;

            Color bg = (row % 2 == 0) ? BACKGROUND : SURFACE_0;
            if (MouseInsideZone(zone)) {
                if (mouse.x > cellX && mouse.x < cellX + cellWidth) {
                    if (mouse.y > cellY && mouse.y < cellY + cellHeight) {
                        bg = OVERLAY_0;
                    } else {
                        bg = SURFACE_1;
                    }
                } else if (mouse.y > cellY && mouse.y < cellY + cellHeight) {
                    bg = SURFACE_1;
                }
            }

            DrawRectangle(cellX, cellY, cellWidth, cellHeight, bg);

            // Draw cell border
            DrawRectangleLinesEx((Rectangle){cellX, cellY, cellWidth + 1, cellHeight + 1}, 2, MANTLE);

            // Draw cell text
            DrawTextEx(assets->mainFont, gridData[row][col], (Vector2){cellX + textPadding, cellY + textPadding}, assets->mainFontSize, assets->mainFontSpacing, TEXT); // Draw cell text
        }
         
        contentHeight += cellHeight;
    }
    contentHeight += zone->hScrollbar.track.height;

    zone->contentSize.y = contentHeight;
    if (zone->bounds.height > zone->contentSize.y) {
        zone->contentSize.y = zone->bounds.height;
    }
    zone->contentSize.x = contentWidth;
    if (zone->bounds.width > zone->contentSize.x) {
        zone->contentSize.x = zone->bounds.width;
    }

    EndScissorMode();

    DrawScrollbars(zone);
}
