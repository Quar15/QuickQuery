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

void calculateColumnsWidth(GridData *grid, int *columnsWidth, int textPadding, Assets *assets) {
    const int maximumColWidth = 600;
    const int minimumColWidth = 50;
    for (int col = 0; col < grid->cols; col++) {
        int headerWidth = MeasureTextEx(assets->mainFont, grid->header[col], assets->mainFontSize, assets->mainFontSpacing).x + (textPadding * 3);
        int colWidth = headerWidth > minimumColWidth ? headerWidth : minimumColWidth;
        for (int row = 0; row < grid->rows; row++) {
            int textWidth = MeasureTextEx(assets->mainFont, grid->data[row][col], assets->mainFontSize, assets->mainFontSpacing).x + (textPadding * 5);
            if (textWidth > colWidth) {
                if (textWidth > maximumColWidth) {
                    colWidth = maximumColWidth;
                } else {
                    colWidth = textWidth;
                }
            }
        }
        columnsWidth[col] = colWidth;
    }
}

void DrawDisplayZone(Zone *zone, Assets *assets)
{
    ClearBackground(BACKGROUND);
    Vector2 mouse = GetMousePosition();

    const int cellHeight = 30;
    const int textPadding = 8;

    GridData grid;
    PrepareFakeGrid(&grid);

    int columnsWidth[grid.cols];
    calculateColumnsWidth(&grid, columnsWidth, textPadding, assets);

    int contentWidth = 0;
    int contentHeight = 0;

    int counterColumnWidth = 0;
    int counterColumnCharactersCount = countDigits(grid.rows);
    char rowText[8];
    snprintf(rowText, sizeof(rowText), "%d", grid.rows);
    counterColumnWidth = MeasureTextEx(assets->mainFont, rowText, assets->mainFontSize, assets->mainFontSpacing).x + (textPadding * 2);
    for (int col = 0; col < grid.cols; col++) {
        contentWidth += columnsWidth[col];
    }
    contentWidth += counterColumnWidth;

    BeginScissorMode(zone->bounds.x, zone->bounds.y, zone->bounds.width, zone->bounds.height);
    // Draw content cells
    for (int row = 0; row < grid.rows; row++) {
        for (int col = 0; col < grid.cols; col++) {
            int cellX = zone->bounds.x - zone->scroll.x + counterColumnWidth;
            for (int c = 0; c < col; c++) {
                cellX += columnsWidth[c];
            }
            int cellY = zone->bounds.y + (row + 1) * cellHeight - zone->scroll.y;

            Color bg = (row % 2 == 0) ? BACKGROUND : SURFACE_0;
            if (MouseInsideZone(zone)) {
                if (mouse.x > cellX && mouse.x < cellX + columnsWidth[col]) {
                    if (mouse.y > cellY && mouse.y < cellY + cellHeight) {
                        bg = OVERLAY_0;
                    } else {
                        bg = SURFACE_1;
                    }
                } else if (mouse.y > cellY && mouse.y < cellY + cellHeight) {
                    bg = SURFACE_1;
                }
            }

            DrawRectangle(cellX, cellY, columnsWidth[col], cellHeight, bg);

            // Draw cell border
            DrawRectangleLinesEx((Rectangle){cellX, cellY, columnsWidth[col] + 1, cellHeight + 1}, 2, MANTLE);

            // Draw cell text
            DrawTextEx(assets->mainFont, grid.data[row][col], (Vector2){cellX + textPadding, cellY + textPadding}, assets->mainFontSize, assets->mainFontSpacing, TEXT); // Draw cell text
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

    // Draw header
    for (int col = 0; col < grid.cols; col++) {
        int cellX = zone->bounds.x - zone->scroll.x + counterColumnWidth;
        for (int c = 0; c < col; c++) {
            cellX += columnsWidth[c];
        }
        int cellY = zone->bounds.y;

        Color bg = MANTLE;
        if (MouseInsideZone(zone) && (mouse.x > cellX && mouse.x < cellX + columnsWidth[col])) {
            bg = CRUST;
        }
        DrawRectangle(cellX, cellY, columnsWidth[col], cellHeight, bg); // Draw background
        DrawRectangleLinesEx((Rectangle){cellX, cellY, columnsWidth[col] + 1, cellHeight + 1}, 2, bg); // Draw cell border
        DrawTextEx(assets->mainFont, grid.header[col], (Vector2){cellX + textPadding, cellY + textPadding}, assets->mainFontSize, assets->mainFontSpacing, TEXT); // Draw cell text
    }

    // Draw row counter
    for (int row = 0; row < grid.rows; row++) {
        int cellX = zone->bounds.x;
        int cellY = zone->bounds.y + (row + 1) * cellHeight - zone->scroll.y;

        Color bg = MANTLE;
        if (MouseInsideZone(zone) && (mouse.y > cellY && mouse.y < cellY + cellHeight)) {
            bg = CRUST;
        }
        DrawRectangle(cellX, cellY, counterColumnWidth, cellHeight, bg); // Draw background
        DrawRectangleLinesEx((Rectangle){cellX, cellY, counterColumnWidth + 1, cellHeight + 1}, 2, bg); // Draw cell border
        int counterColumnLeftPadding = textPadding + (counterColumnCharactersCount - countDigits(row + 1)) * assets->mainFontCharacterWidth;
        snprintf(rowText, sizeof(rowText), "%d", row + 1);
        DrawTextEx(assets->mainFont, rowText, (Vector2){cellX + counterColumnLeftPadding, cellY + textPadding}, assets->mainFontSize, assets->mainFontSpacing, SURFACE_0); // Draw cell text
    }

    // Draw left upper corner
    DrawRectangle(zone->bounds.x, zone->bounds.y, counterColumnWidth, cellHeight, MANTLE);

    DrawScrollbars(zone);

    FreeGrid(&grid);
}
