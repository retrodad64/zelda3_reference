#include "debug_image.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "png_writer.h"
#include "variables.h"
#include "zelda_rtl.h"
#include "snes/ppu.h"

enum {
    kTileSize = 8,
    kTileWords = 16,

    // Tile sheet panels, in tiles.
    kSheetCols = 32,
    kBgSheetRows = 32,
    kObjSheetRows = 16,

    // A BG tilemap is 32x32 entries.
    kMapTiles = 32,
    kMapPixels = kMapTiles * kTileSize,

    kTileDumpWidth = kMapPixels * 2,
    kTileDumpHeight = kMapPixels * 2,

    // Animation sheet.
    kCellSize = 64,
    kCellCols = 16,
    kMaxAnimFrames = 256,

    kOamSlots = 128,
};

typedef struct Canvas {
    int width;
    int height;
    uint8 *pixels;
} Canvas;

static const uint8 kSpriteSizeTable[8][2] = {
    { 8, 16 }, { 8, 32 }, { 8, 64 }, { 16, 32 },
    { 16, 64 }, { 32, 64 }, { 16, 32 }, { 16, 32 },
};

static uint8 *g_anim_pixels;
static int g_anim_frames;
static int g_tile_dump_counter;
static int g_anim_dump_counter;

static bool Canvas_Init(Canvas *canvas, int width, int height, uint8 fill) {
    canvas->width = width;
    canvas->height = height;
    canvas->pixels = (uint8 *)malloc((size_t)width * height * 3);

    if (canvas->pixels == NULL) {
        return false;
    }

    memset(canvas->pixels, fill, (size_t)width * height * 3);
    return true;
}

static void Canvas_Plot(Canvas *canvas, int x, int y, uint16 snes_color) {
    uint8 *dst;

    if (x < 0 || y < 0 || x >= canvas->width || y >= canvas->height) {
        return;
    }

    dst = canvas->pixels + ((size_t)y * canvas->width + x) * 3;
    dst[0] = (uint8)(((snes_color & 0x1f) << 3) | ((snes_color & 0x1f) >> 2));
    dst[1] = (uint8)((((snes_color >> 5) & 0x1f) << 3) | (((snes_color >> 5) & 0x1f) >> 2));
    dst[2] = (uint8)((((snes_color >> 10) & 0x1f) << 3) | (((snes_color >> 10) & 0x1f) >> 2));
}

// One 8x8 4bpp tile. Colour 0 is transparent and is left alone.
static void DrawTile(Canvas *canvas, int dx, int dy, uint16 tile_addr, int tile, int palette_base,
                     bool flip_x, bool flip_y) {
    const Ppu *ppu = g_zenv.ppu;

    for (int row = 0; row < kTileSize; row++) {
        int src_row = flip_y ? kTileSize - 1 - row : row;
        const uint16 *addr = &ppu->vram[(tile_addr + tile * kTileWords + src_row) & 0x7fff];
        uint32 plane = addr[0] | ((uint32)addr[8] << 16);

        for (int col = 0; col < kTileSize; col++) {
            int shift = flip_x ? col : 7 - col;
            uint32 bits = plane >> shift;
            int pixel = (bits >> 0) & 1 | (bits >> 7) & 2 | (bits >> 14) & 4 | (bits >> 21) & 8;

            if (pixel != 0) {
                Canvas_Plot(canvas, dx + col, dy + row, ppu->cgram[(palette_base + pixel) & 0xff]);
            }
        }
    }
}

// Character data laid out as a grid, so a wrong or stale sheet is obvious at a glance.
static void DrawTileSheet(Canvas *canvas, int ox, int oy, uint16 tile_addr, int rows, int palette_base) {
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < kSheetCols; col++) {
            DrawTile(canvas, ox + col * kTileSize, oy + row * kTileSize, tile_addr,
                     row * kSheetCols + col, palette_base, false, false);
        }
    }
}

// A 32x32 entry tilemap with the palette, flip and tile number each entry names.
static void DrawTilemap(Canvas *canvas, int ox, int oy, int layer) {
    const Ppu *ppu = g_zenv.ppu;
    const BgLayer *bg = &ppu->bgLayer[layer];

    for (int row = 0; row < kMapTiles; row++) {
        for (int col = 0; col < kMapTiles; col++) {
            uint16 entry = ppu->vram[(bg->tilemapAdr + row * kMapTiles + col) & 0x7fff];

            DrawTile(canvas, ox + col * kTileSize, oy + row * kTileSize, bg->tileAdr, entry & 0x3ff,
                     (entry & 0x1c00) >> 6, (entry & 0x4000) != 0, (entry & 0x8000) != 0);
        }
    }
}

void DebugImage_WriteTileDump(void) {
    const Ppu *ppu = g_zenv.ppu;
    Canvas canvas;
    char path[64];

    if (!Canvas_Init(&canvas, kTileDumpWidth, kTileDumpHeight, 0x20)) {
        return;
    }

    // Top left is the BG character data, top right the sprite character data. Neither carries a
    // palette of its own, so both use their first one and only the layout is meaningful.
    DrawTileSheet(&canvas, 0, 0, ppu->bgLayer[0].tileAdr, kBgSheetRows, 0);
    DrawTileSheet(&canvas, kMapPixels, 0, ppu->objTileAdr1, kObjSheetRows, 0x80);

    // Bottom row is BG1 and BG2 assembled from those tiles, in their real colours.
    DrawTilemap(&canvas, 0, kMapPixels, 0);
    DrawTilemap(&canvas, kMapPixels, kMapPixels, 1);

    snprintf(path, sizeof(path), "zelda3_tiles_%04d.png", ++g_tile_dump_counter);

    if (WritePng(path, canvas.width, canvas.height, canvas.pixels)) {
        printf("wrote %s\n", path);
    } else {
        fprintf(stderr, "Unable to write %s\n", path);
    }

    free(canvas.pixels);
}

// Every sprite overlapping a window around Link, drawn into one cell of the sheet.
static void DrawOamIntoCell(Canvas *canvas, int ox, int oy) {
    const Ppu *ppu = g_zenv.ppu;
    int origin_x = (int)link_x_coord - (int)BG2HOFS_copy2 - kCellSize / 2 + kTileSize;
    int origin_y = (int)link_y_coord - (int)BG2VOFS_copy2 - kCellSize / 2 + kTileSize;

    for (int slot = 0; slot < kOamSlots; slot++) {
        int index = slot * 2;
        int y = ppu->oam[index] >> 8;
        int high = ppu->oam[0x100 + (index >> 4)] >> (index & 15);
        int size = kSpriteSizeTable[ppu->objSize][(high >> 1) & 1];
        int x = (ppu->oam[index] & 0xff) + (high & 1) * 256;
        int oam1 = ppu->oam[index + 1];
        uint16 tile_addr = (oam1 & 0x100) ? ppu->objTileAdr2 : ppu->objTileAdr1;
        int palette_base = 0x80 + 16 * ((oam1 & 0xe00) >> 9);
        bool flip_x = (oam1 & 0x4000) != 0;
        bool flip_y = (oam1 & 0x8000) != 0;

        if (y == 0xf0) {
            continue;
        }

        if (x >= 256) {
            x -= 512;
        }

        for (int sy = 0; sy < size; sy += kTileSize) {
            for (int sx = 0; sx < size; sx += kTileSize) {
                int used_row = flip_y ? size - kTileSize - sy : sy;
                int used_col = flip_x ? size - kTileSize - sx : sx;
                int tile = ((((oam1 & 0xff) >> 4) + (used_row >> 3)) << 4) |
                           (((oam1 & 0xf) + (used_col >> 3)) & 0xf);
                int px = ox + x + sx - origin_x;
                int py = oy + y + sy - origin_y;

                if (px <= ox - kTileSize || py <= oy - kTileSize ||
                    px >= ox + kCellSize || py >= oy + kCellSize) {
                    continue;
                }

                DrawTile(canvas, px, py, tile_addr, tile, palette_base, flip_x, flip_y);
            }
        }
    }
}

void DebugImage_BeginAnimation(void) {
    if (g_anim_pixels == NULL) {
        g_anim_pixels = (uint8 *)malloc((size_t)kMaxAnimFrames * kCellSize * kCellSize * 3);
    }

    g_anim_frames = 0;
}

void DebugImage_CaptureAnimationFrame(void) {
    Canvas cell;

    if (g_anim_pixels == NULL || g_anim_frames >= kMaxAnimFrames) {
        return;
    }

    cell.width = kCellSize;
    cell.height = kCellSize;
    cell.pixels = g_anim_pixels + (size_t)g_anim_frames * kCellSize * kCellSize * 3;
    memset(cell.pixels, 0x18, (size_t)kCellSize * kCellSize * 3);

    DrawOamIntoCell(&cell, 0, 0);
    g_anim_frames++;
}

void DebugImage_EndAnimation(void) {
    Canvas sheet;
    char path[64];
    int rows;

    if (g_anim_pixels == NULL || g_anim_frames == 0) {
        return;
    }

    rows = (g_anim_frames + kCellCols - 1) / kCellCols;

    if (!Canvas_Init(&sheet, kCellCols * kCellSize, rows * kCellSize, 0x30)) {
        g_anim_frames = 0;
        return;
    }

    for (int i = 0; i < g_anim_frames; i++) {
        const uint8 *src = g_anim_pixels + (size_t)i * kCellSize * kCellSize * 3;
        int ox = (i % kCellCols) * kCellSize;
        int oy = (i / kCellCols) * kCellSize;

        for (int row = 0; row < kCellSize; row++) {
            memcpy(sheet.pixels + (((size_t)(oy + row) * sheet.width) + ox) * 3,
                   src + (size_t)row * kCellSize * 3, (size_t)kCellSize * 3);
        }
    }

    snprintf(path, sizeof(path), "zelda3_anim_%04d.png", ++g_anim_dump_counter);

    if (WritePng(path, sheet.width, sheet.height, sheet.pixels)) {
        printf("wrote %s, %d frames\n", path, g_anim_frames);
    } else {
        fprintf(stderr, "Unable to write %s\n", path);
    }

    free(sheet.pixels);
    g_anim_frames = 0;
}
