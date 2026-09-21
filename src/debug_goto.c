#include "debug_goto.h"

#include <stdio.h>
#include <SDL.h>

#include "assets.h"
#include "dungeon.h"
#include "load_gfx.h"
#include "overworld.h"
#include "sprite.h"
#include "variables.h"
#include "zelda_rtl.h"

enum {
    kMaxScreenDigits = 2,
    kGlyphWidth = 8,
    kGlyphHeight = 10,
    kOverlayPadding = 4,
    kOverlayOriginX = 8,
    kOverlayOriginY = 8,

    // Each overworld screen is 512x512 pixels on an 8x8 grid.
    kScreenPixelSize = 512,
    kScreenGridMask = 7,

    // Link sits this far from the top left of the view when the camera is centred.
    kViewOffsetX = 0x78,
    kViewOffsetY = 0x5e,
};

static bool g_prompt_open;
static uint8 g_digits[kMaxScreenDigits];
static int g_digit_count;
static uint32 g_blink_counter;

// Rows are bottom of byte to left of glyph, matching the digit font in main.c.
static const uint8 kHexFont[16][kGlyphHeight] = {
    { 0x1c, 0x36, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x36, 0x1c },  // 0
    { 0x18, 0x1c, 0x1e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7e },  // 1
    { 0x3e, 0x63, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x03, 0x63, 0x7f },  // 2
    { 0x3e, 0x63, 0x60, 0x60, 0x3c, 0x60, 0x60, 0x60, 0x63, 0x3e },  // 3
    { 0x30, 0x38, 0x3c, 0x36, 0x33, 0x7f, 0x30, 0x30, 0x30, 0x78 },  // 4
    { 0x7f, 0x03, 0x03, 0x03, 0x3f, 0x60, 0x60, 0x60, 0x63, 0x3e },  // 5
    { 0x1c, 0x06, 0x03, 0x03, 0x3f, 0x63, 0x63, 0x63, 0x63, 0x3e },  // 6
    { 0x7f, 0x63, 0x60, 0x60, 0x30, 0x18, 0x0c, 0x0c, 0x0c, 0x0c },  // 7
    { 0x3e, 0x63, 0x63, 0x63, 0x3e, 0x63, 0x63, 0x63, 0x63, 0x3e },  // 8
    { 0x3e, 0x63, 0x63, 0x63, 0x7e, 0x60, 0x60, 0x60, 0x30, 0x1e },  // 9
    { 0x1c, 0x36, 0x63, 0x63, 0x63, 0x7f, 0x63, 0x63, 0x63, 0x63 },  // A
    { 0x3f, 0x63, 0x63, 0x63, 0x3f, 0x63, 0x63, 0x63, 0x63, 0x3f },  // B
    { 0x1c, 0x36, 0x63, 0x03, 0x03, 0x03, 0x03, 0x63, 0x36, 0x1c },  // C
    { 0x1f, 0x33, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x33, 0x1f },  // D
    { 0x7f, 0x03, 0x03, 0x03, 0x3f, 0x03, 0x03, 0x03, 0x03, 0x7f },  // E
    { 0x7f, 0x03, 0x03, 0x03, 0x3f, 0x03, 0x03, 0x03, 0x03, 0x03 },  // F
};

static const uint8 kCursorGlyph[kGlyphHeight] = { 0, 0, 0, 0, 0, 0, 0, 0, 0x7f, 0x7f };

static void DebugGoto_FillRect(uint8 *pixel_buffer, int pitch, int x, int y, int w, int h, uint32 color) {
    for (int row = 0; row < h; row++) {
        uint32 *dst = (uint32 *)(pixel_buffer + (size_t)(y + row) * pitch) + x;

        for (int col = 0; col < w; col++) {
            dst[col] = color;
        }
    }
}

static void DebugGoto_DrawGlyph(uint8 *pixel_buffer, int pitch, int x, int y, const uint8 *glyph, uint32 color,
                                int scale) {
    for (int row = 0; row < kGlyphHeight; row++) {
        int bits = glyph[row];

        for (int col = 0; bits != 0; col++, bits >>= 1) {
            if ((bits & 1) == 0) {
                continue;
            }

            DebugGoto_FillRect(pixel_buffer, pitch, x + col * scale, y + row * scale, scale, scale, color);
        }
    }
}

// Screens 3, 5, 7 and their dark world counterparts use the other animated tile set.
#define kDarkWorldAnimatedTiles(sc) \
    (((sc) == 0x03 || (sc) == 0x05 || (sc) == 0x07 || (sc) == 0x43 || (sc) == 0x45 || (sc) == 0x47) ? 0x58 : 0x5a)

// Only the overworld is reachable this way, so a jump from inside a dungeon is refused.
// A big overworld area is four screens drawn from one of them, and only that one holds the
// map. Landing on any of the other three makes the game load data it never shows there, which
// looks like the map being wrong when it is the jump that is wrong.
// Where his feet sit inside his sprite: half his width across, his whole height down.
enum { kLinkFeetOffsetX = 8, kLinkFeetOffsetY = 16 };

// The overworld's big areas are two screens square, and the game always names one by its top
// left corner: the other three quadrants are never loaded in their own right. A jump lands on
// whichever screen the coordinate falls in, so a coordinate inside a big area has to be turned
// back into the head before anything is loaded, or the map comes up as a quadrant the game
// never draws.
//
// The big screens sit in blocks of two by two, so the head is found by stepping back to the
// start of the run of big screens across and up. A row can hold two blocks side by side, which
// is why the column steps back in twos from the start of its run rather than to the start.
static uint8 DebugGoto_AreaHead(uint8 screen) {
    uint8 world = screen & 0x40;
    int   col = screen & kScreenGridMask;
    int   row = (screen >> 3) & kScreenGridMask;
    int   run;

    if (kOverworldMapIsSmall[screen & 0x3f]) {
        return screen;
    }

    if (row > 0 && !kOverworldMapIsSmall[(row - 1) * 8 + col]) {
        row--;
    }

    run = col;

    while (run > 0 && !kOverworldMapIsSmall[row * 8 + run - 1]) {
        run--;
    }

    col = run + ((col - run) & ~1);

    return (uint8)(world | (row << 3) | col);
}

static void DebugGoto_JumpToPos(uint8 screen, uint16 target_x, uint16 target_y) {
    uint16 scroll_x;
    uint16 scroll_y;
    uint8 music;
    uint8 saved_submodule;
    uint8 saved_inidisp;

    if (player_is_indoors) {
        printf("goto: ignored, only works on the overworld\n");
        return;
    }

    // Bit 6 of the screen index is the world. The flag has to move with it, because the palette
    // set, the bunny check and the music all read the flag and not the index.
    savegame_is_darkworld = screen & 0x40;

    scroll_x = target_x - kViewOffsetX;
    scroll_y = target_y - kViewOffsetY;

    link_x_coord = target_x;
    link_y_coord = target_y;
    BG1HOFS_copy2 = BG2HOFS_copy2 = BG1HOFS_copy = BG2HOFS_copy = scroll_x;
    BG1VOFS_copy2 = BG2VOFS_copy2 = BG1VOFS_copy = BG2VOFS_copy = scroll_y;
    overworld_area_index = overworld_screen_index = screen;

    camera_x_coord_scroll_low = target_x + 8;
    camera_x_coord_scroll_hi = camera_x_coord_scroll_low - 2;
    camera_y_coord_scroll_low = target_y + 8;
    camera_y_coord_scroll_hi = camera_y_coord_scroll_low - 2;

    overworld_unk1 = 0;
    overworld_unk3 = 0;
    overworld_unk1_neg = 0;
    overworld_unk3_neg = 0;
    ow_entrance_value = 0;
    big_rock_starting_address = 0;

    AdjustLinkBunnyStatus();

    // Picks the tile theme and sprite sheet indexes for the new screen, and the offset base and
    // mask the window below is measured against.
    Overworld_LoadNewScreenProperties();
    Overworld_SetSongList();

    // Top left of the visible window as a byte offset into the map16 buffer. Same shape as the
    // lookup in Overworld_GetTileAttributeAtLocation, keyed on the scroll position instead of on
    // Link. A wrong value here draws the screen shifted against the map its collision comes from.
    map16_load_src_off = (uint16)((((scroll_y - overworld_offset_base_y) & overworld_offset_mask_y) * 8) |
                                  (((scroll_x >> 3) - overworld_offset_base_x) & overworld_offset_mask_x));
    map16_load_var2 = (map16_load_src_off - 0x400 & 0xf80) >> 7;
    map16_load_dst_off = (map16_load_src_off - 0x10 & 0x3e) >> 1;

    // Loads the sheets those indexes name into VRAM. Without this the previous world's tiles stay
    // resident and the new screen is drawn with them. 0x58 is the set those six screens share.
    DecompressAnimatedOverworldTiles(kDarkWorldAnimatedTiles(screen));
    InitializeTilesets();

    OverworldLoadScreensPaletteSet();
    Overworld_LoadPalettes(kOverworldBgPalettes[screen], overworld_sprite_palettes[screen]);
    Palette_SetOwBgColor();
    Overworld_LoadPalettesInner();
    Overworld_SetFixedColAndScroll();

    // Per screen track, the same lookup the mirror warp uses when it lands.
    music = overworld_music[screen];
    music_control = music & 0xf;
    sound_effect_ambient = music >> 4;

    if (screen >= 0x40 && !link_item_moon_pearl) {
        music_control = 4;
    }

    // Redraws the screen and asks the NMI handler to copy the new tilemap into VRAM. Drawing it by
    // hand builds the tilemap in work RAM but never uploads it, which leaves the previous area on
    // screen until the next screen transition uploads one. It also steps the submodule and blanks
    // the screen on its way out, both of which the following submodule would normally undo.
    saved_submodule = submodule_index;
    saved_inidisp = INIDISP_copy;
    Overworld_LoadAndBuildScreen();
    submodule_index = saved_submodule;
    INIDISP_copy = saved_inidisp;

    Sprite_ResetAll();
    Sprite_ReloadAll_Overworld();
    is_standing_in_doorway = 0;
    Dungeon_ResetTorchBackgroundAndPlayerInner();

    printf("goto: screen %02X at %04X,%04X\n", screen, target_x, target_y);
}

void DebugGoto_JumpToSprite(uint8 screen, uint16 sprite_x, uint16 sprite_y) {
    DebugGoto_JumpToPos(screen, sprite_x, sprite_y);
}

void DebugGoto_JumpTo(uint8 screen) {
    uint16 column = screen & kScreenGridMask;
    uint16 row = (screen >> 3) & kScreenGridMask;

    DebugGoto_JumpToPos(DebugGoto_AreaHead(screen), column * kScreenPixelSize + kScreenPixelSize / 2,
                        row * kScreenPixelSize + kScreenPixelSize / 2);
}

void DebugGoto_JumpToPoint(uint16 x, uint16 y, bool dark) {
    // The coordinate names where his feet go, which is the position the tile log prints as
    // feet= and the one another program comparing itself against this one will be working in.
    // link_x_coord and link_y_coord hold the top left of his sprite, so both come back off.
    uint16 sprite_x = (x >= kLinkFeetOffsetX) ? (uint16)(x - kLinkFeetOffsetX) : 0;
    uint16 sprite_y = (y >= kLinkFeetOffsetY) ? (uint16)(y - kLinkFeetOffsetY) : 0;

    // A world is 8 screens square and a coordinate is already in world pixels, so the screen
    // falls out of the coordinate. The world itself doesn't, which is why it's a separate flag.
    uint8 screen = (uint8)((((y / kScreenPixelSize) & kScreenGridMask) << 3) |
                           ((x / kScreenPixelSize) & kScreenGridMask));

    if (dark) {
        screen |= 0x40;
    }

    DebugGoto_JumpToPos(DebugGoto_AreaHead(screen), sprite_x, sprite_y);
}

void DebugGoto_OpenPrompt(void) {
    g_prompt_open = true;
    g_digit_count = 0;
    g_blink_counter = 0;
}

bool DebugGoto_IsPromptOpen(void) {
    return g_prompt_open;
}

bool DebugGoto_HandleKey(int sdl_keycode) {
    int value = -1;

    if (!g_prompt_open) {
        return false;
    }

    if (sdl_keycode == SDLK_ESCAPE) {
        g_prompt_open = false;
        return true;
    }

    if (sdl_keycode == SDLK_BACKSPACE) {
        if (g_digit_count > 0) {
            g_digit_count--;
        }

        return true;
    }

    if (sdl_keycode == SDLK_RETURN || sdl_keycode == SDLK_KP_ENTER) {
        if (g_digit_count > 0) {
            uint8 screen = 0;

            for (int i = 0; i < g_digit_count; i++) {
                screen = (screen << 4) | g_digits[i];
            }

            DebugGoto_JumpTo(screen);
        }

        g_prompt_open = false;
        return true;
    }

    if (sdl_keycode >= SDLK_0 && sdl_keycode <= SDLK_9) {
        value = sdl_keycode - SDLK_0;
    } else if (sdl_keycode >= SDLK_a && sdl_keycode <= SDLK_f) {
        value = sdl_keycode - SDLK_a + 10;
    }

    if (value >= 0 && g_digit_count < kMaxScreenDigits) {
        g_digits[g_digit_count++] = (uint8)value;
    }

    // Everything else is swallowed so the game does not act on it while the prompt is up.
    return true;
}

void DebugGoto_DrawOverlay(uint8 *pixel_buffer, int pitch, int render_scale) {
    int scale;
    int box_x;
    int box_y;
    int box_w;
    int box_h;
    int pen_x;
    int pen_y;

    if (!g_prompt_open || pixel_buffer == NULL) {
        return;
    }

    scale = render_scale;
    box_x = kOverlayOriginX * scale;
    box_y = kOverlayOriginY * scale;
    box_w = (kOverlayPadding * 2 + kGlyphWidth * (kMaxScreenDigits + 1)) * scale;
    box_h = (kOverlayPadding * 2 + kGlyphHeight) * scale;

    DebugGoto_FillRect(pixel_buffer, pitch, box_x, box_y, box_w, box_h, 0x000000);
    DebugGoto_FillRect(pixel_buffer, pitch, box_x, box_y, box_w, scale, 0xffffff);
    DebugGoto_FillRect(pixel_buffer, pitch, box_x, box_y + box_h - scale, box_w, scale, 0xffffff);
    DebugGoto_FillRect(pixel_buffer, pitch, box_x, box_y, scale, box_h, 0xffffff);
    DebugGoto_FillRect(pixel_buffer, pitch, box_x + box_w - scale, box_y, scale, box_h, 0xffffff);

    pen_x = box_x + kOverlayPadding * scale;
    pen_y = box_y + kOverlayPadding * scale;

    for (int i = 0; i < g_digit_count; i++) {
        DebugGoto_DrawGlyph(pixel_buffer, pitch, pen_x, pen_y, kHexFont[g_digits[i]], 0xffffff, scale);
        pen_x += kGlyphWidth * scale;
    }

    g_blink_counter++;

    if ((g_blink_counter & 0x10) == 0) {
        DebugGoto_DrawGlyph(pixel_buffer, pitch, pen_x, pen_y, kCursorGlyph, 0xffffff, scale);
    }
}
