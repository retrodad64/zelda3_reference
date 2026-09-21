# Debug image dumps

Two of the debug keys write PNG files next to `zelda3.ini`. Shift+T writes
`zelda3_tiles_NNNN.png`, a picture of the graphics currently resident in VRAM.
Shift+A writes `zelda3_anim_NNNN.png`, one cell per frame you held the key.

The counter starts at 0001 and climbs for the life of the run, so a second dump
never overwrites the first. Both patterns are in `.gitignore`.

## Tile dump

Shift+T toggles the text tile log. The PNG is written on the press that turns
logging on, so it's a snapshot of that moment. Toggle off and on again for a
second one.

The image is 512 by 512, split into four panels of 256 by 256.

| | Left | Right |
| --- | --- | --- |
| Top | BG character tiles | Sprite character tiles |
| Bottom | BG1 tilemap | BG2 tilemap |

Here is the whole layout, in order:

<sub><code>src/debug_image.c</code></sub>
```c
    // Top left is the BG character data, top right the sprite character data. Neither carries a
    // palette of its own, so both use their first one and only the layout is meaningful.
    DrawTileSheet(&canvas, 0, 0, ppu->bgLayer[0].tileAdr, kBgSheetRows, 0);
    DrawTileSheet(&canvas, kMapPixels, 0, ppu->objTileAdr1, kObjSheetRows, 0x80);

    // Bottom row is BG1 and BG2 assembled from those tiles, in their real colours.
    DrawTilemap(&canvas, 0, kMapPixels, 0);
    DrawTilemap(&canvas, kMapPixels, kMapPixels, 1);
```

### The two tile sheets

Both sheets are a plain grid, 32 tiles across, each tile 8 by 8 pixels. The top
left sheet is 32 rows, so 1024 tiles, read from the address BG layer 0 is using.
The top right sheet is 16 rows, so 512 tiles, read from the first sprite
character address. The right panel is half the height of the left one, which is
why there's a band of empty background under it.

Character data carries no palette. A tile is four bits per pixel and the palette
comes from whatever references it, so the sheets pick one and stick to it. The
BG sheet uses palette 0 and the sprite sheet uses the first sprite palette at
CGRAM index 0x80. Colour in these two panels is arbitrary. Read them for layout
and content, not for colour.

That's still enough to answer the question they exist for. If the wrong sheet is
loaded, or a sheet is stale after a screen change, it's obvious at a glance
because the shapes are wrong.

### The two tilemaps

The bottom panels are the real thing. Each is a 32 by 32 grid of tilemap
entries, drawn with the tile number, palette and flip bits each entry names:

<sub><code>src/debug_image.c</code></sub>
```c
            uint16 entry = ppu->vram[(bg->tilemapAdr + row * kMapTiles + col) & 0x7fff];

            DrawTile(canvas, ox + col * kTileSize, oy + row * kTileSize, bg->tileAdr, entry & 0x3ff,
                     (entry & 0x1c00) >> 6, (entry & 0x4000) != 0, (entry & 0x8000) != 0);
```

Bits 0 to 9 are the tile number, bits 10 to 12 the palette, bit 14 horizontal
flip and bit 15 vertical flip. Colours here are correct, so the bottom right
panel usually looks like the screen you're standing on.

On the overworld the map sits on BG2, so the bottom right panel is the one that
looks like the ground you're standing on. The bottom left panel is mostly flat
there. Which layer carries what changes elsewhere in the game, so check both.

A tilemap panel is 32 tiles square, which is 256 pixels. The visible screen is
wider than that once the extra left and right area is on, so a panel is not a
screenshot. It's the tilemap as stored.

## Animation sheet

Shift+A logs while held. Each frame it captures one 64 by 64 cell, and the sheet
is written when you let go. Cells run left to right, 16 per row, so 45 frames
gives 3 rows with the last row part empty. The empty area is a lighter grey than
the cell background, which makes the end of the sequence easy to find.

A cell holds every sprite overlapping a window centred on Link:

<sub><code>src/debug_image.c</code></sub>
```c
    int origin_x = (int)link_x_coord - (int)BG2HOFS_copy2 - kCellSize / 2 + kTileSize;
    int origin_y = (int)link_y_coord - (int)BG2VOFS_copy2 - kCellSize / 2 + kTileSize;
```

Link's world position minus the BG scroll gives his screen position, and the
window is placed so he lands in the middle. Every OAM slot is walked, not just
Link's, so anything standing next to him appears too. That's deliberate, because
a sprite drawn on the wrong frame is usually what you're looking for.

Colour in the cells is correct. Sprites use their own palettes from CGRAM index
0x80 upward, and colour 0 is transparent, so the cell background shows through.

The capture stops at 256 frames, a little over four seconds. Holding longer
keeps logging text but adds no more cells.

## Reading a cell

A cell is 64 pixels, Link is 16, so there's room for about two tiles of
neighbours on each side. Sprites are drawn 8 by 8 at a time, and a sprite whose
corner falls outside the window is clipped rather than dropped, so a partial
sprite at a cell edge is expected.

If a cell is empty apart from the background, Link had no sprite that frame.
That happens during a screen transition and while the game is in a menu.

## Where the files land

The game changes directory to wherever it finds `zelda3.ini`, so the PNGs land
in the repo root next to `zelda3_debug.log`, whatever directory you launched
from. The same note applies as for the log file.

Each write prints a line to stdout naming the file, and the animation line also
gives the frame count.
