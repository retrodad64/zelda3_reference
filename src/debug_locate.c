#include "debug_locate.h"

#include <stdio.h>

#include "debug_goto.h"
#include "dungeon.h"
#include "overworld.h"
#include "sprite.h"
#include "tile_detect.h"
#include "variables.h"
#include "zelda_rtl.h"

enum {
    kOverworldScreens = 0x80,
    kSpriteSlots = 16,

    // Entries from 0xF4 up are overlords, which have no sprite of their own.
    kFirstOverlordType = 0xf4,

    // No boss flag exists in the sprite data, so this is a health threshold. 255 means the sprite
    // cannot be hurt at all, which plenty of ordinary props use, so it is not a boss on its own.
    kBossHealthMin = 32,

    // Where Link is tried, in pixels out from the sprite.
    kPlaceFirstRing = 24,
    kPlaceLastRing = 56,
    kPlaceRingStep = 8,
};

// The eight directions a candidate is tried in, nearest ring first.
static const int8 kPlaceDirX[8] = { 0, 0, -1, 1, -1, 1, -1, 1 };
static const int8 kPlaceDirY[8] = { 1, -1, 0, 0, 1, 1, -1, -1 };

const char *DebugLocate_CategoryName(int category) {
    switch (category) {
    case kLocateCategory_Enemy: return "enemy";
    case kLocateCategory_Npc: return "npc";
    case kLocateCategory_Boss: return "boss";
    default: return "any";
    }
}

int DebugLocate_CategoryOfType(uint8 type) {
    uint8 health = kSpriteInit_Health[type];

    if (type >= countof(kSpriteInit_BumpDamage)) {
        return kLocateCategory_Any;
    }

    if (kSpriteInit_BumpDamage[type] == 0) {
        return kLocateCategory_Npc;
    }

    if (health >= kBossHealthMin && health != 255) {
        return kLocateCategory_Boss;
    }

    return kLocateCategory_Enemy;
}

// Undoes what Overworld_LoadSprites and Sprite_Overworld_ProximityMotivatedLoad do between them,
// turning a three byte table entry back into a world pixel position.
static void DecodeEntry(uint8 screen, const uint8 *entry, uint16 *out_x, uint16 *out_y) {
    uint8 packed_low = (uint8)((entry[1] & 0xf) | (entry[0] << 4));
    uint8 packed_high = (uint8)((entry[1] >> 4) + ((entry[0] >> 4) << 2));
    uint16 x_base = (uint16)((screen & 7) << 9);
    uint16 y_base = (uint16)(((((screen & 0x3f) >> 2) & 0xe)) << 8);

    *out_x = (uint16)(x_base + ((packed_high & 3) << 8) + ((packed_low & 0xf) << 4));
    *out_y = (uint16)(y_base + ((packed_high >> 2) << 8) + (packed_low & 0xf0));
}

// Calls back for every sprite the data places on this screen. Returns the number seen.
static int ForEachSpriteOnScreen(uint8 screen, uint8 want_type, bool stop_on_match,
                                 uint16 *out_x, uint16 *out_y, bool *out_found, bool report) {
    const uint8 *entry = GetOverworldSpritePtr(screen);
    int count = 0;

    for (; entry[0] != 0xff; entry += 3) {
        uint8 type = entry[2];
        uint16 x, y;

        if (type >= kFirstOverlordType) {
            continue;
        }

        DecodeEntry(screen, entry, &x, &y);
        count++;

        if (report) {
            int category = DebugLocate_CategoryOfType(type);

            printf("  screen %02X type %02X at %04X,%04X  %-5s hp=%-3d bump=%d\n", screen, type, x, y,
                   DebugLocate_CategoryName(category), kSpriteInit_Health[type],
                   kSpriteInit_BumpDamage[type]);
        }

        if (stop_on_match && type == want_type && !*out_found) {
            *out_x = x;
            *out_y = y;
            *out_found = true;

            if (stop_on_match) {
                return count;
            }
        }
    }

    return count;
}

void DebugLocate_ListScreen(uint8 screen) {
    bool found = false;
    uint16 x = 0, y = 0;
    int count = ForEachSpriteOnScreen(screen, 0, false, &x, &y, &found, true);

    printf("locate: screen %02X has %d sprite(s)\n", screen, count);
}

void DebugLocate_ListAll(int category) {
    int total = 0;

    printf("locate: scanning all %d overworld screens for %s\n", kOverworldScreens,
           DebugLocate_CategoryName(category));

    for (int screen = 0; screen < kOverworldScreens; screen++) {
        const uint8 *entry = GetOverworldSpritePtr((uint8)screen);

        for (; entry[0] != 0xff; entry += 3) {
            uint8 type = entry[2];
            uint16 x, y;

            if (type >= kFirstOverlordType) {
                continue;
            }

            if (category != kLocateCategory_Any && DebugLocate_CategoryOfType(type) != category) {
                continue;
            }

            DecodeEntry((uint8)screen, entry, &x, &y);
            printf("  screen %02X type %02X at %04X,%04X  %-5s hp=%-3d bump=%d\n", screen, type, x, y,
                   DebugLocate_CategoryName(DebugLocate_CategoryOfType(type)),
                   kSpriteInit_Health[type], kSpriteInit_BumpDamage[type]);
            total++;
        }
    }

    printf("locate: %d sprite(s) matched\n", total);
}

void DebugLocate_ListLive(void) {
    int count = 0;

    printf("locate: live sprite slots on screen %02X\n", (uint8)overworld_screen_index);

    for (int slot = 0; slot < kSpriteSlots; slot++) {
        uint8 type;

        if (sprite_state[slot] == 0) {
            continue;
        }

        type = sprite_type[slot];
        printf("  slot %02X type %02X at %04X,%04X state=%02X ai=%02X hp=%-3d %s\n", slot, type,
               (uint16)(sprite_x_lo[slot] | (sprite_x_hi[slot] << 8)),
               (uint16)(sprite_y_lo[slot] | (sprite_y_hi[slot] << 8)),
               sprite_state[slot], sprite_ai_state[slot], sprite_health[slot],
               DebugLocate_CategoryName(DebugLocate_CategoryOfType(type)));
        count++;
    }

    printf("locate: %d live sprite(s)\n", count);

    DebugLocate_ListScreen((uint8)overworld_screen_index);
}

// Nothing happens when Link stands here. The list is the TileBehavior_NothingOW case in
// tile_detect.c, so water, spikes, pits, slopes, ice and walls are all excluded.
static bool IsSafeStandingTile(uint8 attr) {
    switch (attr) {
    case 0x00:
    case 0x05: case 0x06: case 0x07:
    case 0x14: case 0x15: case 0x16: case 0x17:
    case 0x21: case 0x23: case 0x24: case 0x25:
    case 0x38: case 0x39: case 0x3a: case 0x3b: case 0x3c:
    case 0x04:                                    // thick grass outdoors, wall indoors
    case 0x6c: case 0x6d: case 0x6e: case 0x6f:   // normal ground outdoors, wall indoors
    case 0x40:                                    // thick grass, Link walks through it
    case 0x41: case 0x45: case 0x47: case 0x49:
    case 0x48: case 0x4a:                         // diggable ground, ordinary ground underfoot
    case 0x5e: case 0x5f:
    case 0x61: case 0x62: case 0x64: case 0x65: case 0x66:
    case 0xa6: case 0xa7:
    case 0xbe: case 0xbf:
        return true;
    default:
        return attr >= 0xd0 && attr <= 0xef;
    }
}

// Link stands on a 16 wide box with his feet at the bottom, so all four corners have to be clear.
static bool IsSafeForLink(uint16 x, uint16 y) {
    static const uint8 kCornerX[4] = { 3, 12, 3, 12 };
    static const uint8 kCornerY[4] = { 10, 10, 16, 16 };

    for (int i = 0; i < 4; i++) {
        uint16 sx = (uint16)(x + kCornerX[i]);
        uint16 sy = (uint16)(y + kCornerY[i]);

        if (!IsSafeStandingTile(Overworld_GetTileAttributeAtLocation(sx >> 3, sy))) {
            return false;
        }
    }

    return true;
}

static bool FindSafeSpotNear(uint16 sx, uint16 sy, uint16 *out_x, uint16 *out_y) {
    for (int ring = kPlaceFirstRing; ring <= kPlaceLastRing; ring += kPlaceRingStep) {
        for (int dir = 0; dir < 8; dir++) {
            uint16 x = (uint16)(sx + kPlaceDirX[dir] * ring);
            uint16 y = (uint16)(sy + kPlaceDirY[dir] * ring);

            if (IsSafeForLink(x, y)) {
                *out_x = x;
                *out_y = y;
                return true;
            }
        }
    }

    return false;
}

bool DebugLocate_WarpToType(uint8 type, int category) {
    int actual = DebugLocate_CategoryOfType(type);
    uint16 sprite_x = 0, sprite_y = 0, safe_x = 0, safe_y = 0;
    bool found = false;
    int screen;

    if (category != kLocateCategory_Any && actual != category) {
        printf("warp: type %02X looks like a %s, not a %s, going anyway\n", type,
               DebugLocate_CategoryName(actual), DebugLocate_CategoryName(category));
    }

    for (screen = 0; screen < kOverworldScreens && !found; screen++) {
        ForEachSpriteOnScreen((uint8)screen, type, true, &sprite_x, &sprite_y, &found, false);
    }

    if (!found) {
        printf("warp: no sprite of type %02X on any overworld screen\n", type);
        return false;
    }

    screen--;

    // The first jump is what loads the screen's map, which the tile checks below need.
    DebugGoto_JumpToPoint(sprite_x, sprite_y, (screen & 0x40) != 0);

    if (!FindSafeSpotNear(sprite_x, sprite_y, &safe_x, &safe_y)) {
        printf("warp: type %02X is on screen %02X at %04X,%04X but nothing safe is near it\n", type,
               screen, sprite_x, sprite_y);
        return false;
    }

    DebugGoto_JumpToPoint(safe_x, safe_y, (screen & 0x40) != 0);
    printf("warp: type %02X (%s) on screen %02X at %04X,%04X, Link at %04X,%04X\n", type,
           DebugLocate_CategoryName(actual), screen, sprite_x, sprite_y, safe_x, safe_y);
    return true;
}

// Counts the tiles of each hazard kind a room's objects draw. The room has to be loaded and its
// attribute table built first, because attributes come from the drawn tilemap and not from the
// room data directly.
void DebugLocate_DumpRoomHazards(void) {
    enum { kRooms = 320, kAttrBytes = 0x2000 };
    uint16 saved_room = dungeon_room_index;
    uint8 saved_indoors = player_is_indoors;

    player_is_indoors = 1;

    for (int room = 0; room < kRooms; room++) {
        int spike = 0, pit = 0, water = 0, ice = 0, conveyor = 0;

        dungeon_room_index = (uint16)room;
        dungeon_room_index2 = (uint16)room;

        Dungeon_LoadRoom();
        Dungeon_LoadCustomTileAttr();
        Dungeon_LoadAttributeTable();

        for (int i = 0; i < kAttrBytes; i++) {
            uint8 attr = dung_bg2_attr_table[i];

            if (attr == 0x0d || attr == 0x44) {
                spike++;
            } else if (attr == 0x20 || (attr >= 0xb0 && attr <= 0xbd)) {
                pit++;
            } else if (attr == 0x08 || attr == 0x09 || attr == 0x0a) {
                water++;
            } else if (attr == 0x0e || attr == 0x0f) {
                ice++;
            } else if (attr >= 0x68 && attr <= 0x6b) {
                conveyor++;
            }
        }

        printf("ROOMHAZ %03X spike=%d pit=%d water=%d ice=%d conveyor=%d\n", room, spike, pit,
               water, ice, conveyor);
    }

    dungeon_room_index = saved_room;
    dungeon_room_index2 = saved_room;
    player_is_indoors = saved_indoors;

    // dung_bg2 and overworld_tileattr are the same memory, so the rooms just drawn have
    // overwritten the overworld map. Rebuild the screen so the session stays usable.
    if (!player_is_indoors) {
        DebugGoto_JumpTo((uint8)overworld_screen_index);
    }
}

// The map16 values Overworld_HandleLiftableTiles acts on, with the gloves each one wants. The
// gloves come from kGetBestActionToPerformOnTile_a in player.c, indexed by the liftable class,
// and a cell whose class needs more gloves than Link has gets grabbed instead of lifted.
static const struct {
    uint16 cell;
    const char *what;
    bool needs_gloves;  ///< Measured: without them the same A press grabs instead of lifting.
} kLiftableCells[] = {
    {0x036, "bush", false},
    {0x72a, "bush, dark world", false},
    {0x20f, "stone, pale", true},
    {0x239, "stone, dark", true},
    {0x101, "stone, heavy", true},
    {0x36d, "rock pile", true},
    {0x36e, "rock pile", true},
    {0x374, "rock pile", true},
    {0x375, "rock pile", true},
    {0x23b, "rock pile", true},
    {0x23c, "rock pile", true},
    {0x23d, "rock pile", true},
    {0x23e, "rock pile", true},
};

static bool DebugLocate_LiftableNeedsGloves(uint16 cell) {
    for (size_t i = 0; i < countof(kLiftableCells); i++) {
        if (kLiftableCells[i].cell == cell) {
            return kLiftableCells[i].needs_gloves;
        }
    }

    return false;
}

static const char *DebugLocate_LiftableName(uint16 cell) {
    for (size_t i = 0; i < countof(kLiftableCells); i++) {
        if (kLiftableCells[i].cell == cell) {
            return kLiftableCells[i].what;
        }
    }

    return NULL;
}

void DebugLocate_ListLiftables(void) {
    int total = 0;

    if (player_is_indoors) {
        printf("liftables: ignored, only works on the overworld\n");
        return;
    }

    // overworld_tileattr holds the area around Link, addressed the way Overworld_GetLinkMap16Coords
    // addresses it. Walking the index space rather than world pixels keeps the wrap out of it, so
    // every cell is listed once.
    printf("liftables: scanning the loaded area around screen %02X\n",
           (uint8)overworld_screen_index);

    for (uint16 iy = 0; iy <= overworld_offset_mask_y; iy += 16) {
        for (uint16 ix = 0; ix <= overworld_offset_mask_x; ix += 2) {
            uint16 pos = ((iy & overworld_offset_mask_y) << 3) | (ix & overworld_offset_mask_x);
            uint16 cell = overworld_tileattr[pos >> 1];
            const char *what = DebugLocate_LiftableName(cell);

            if (what) {
                printf("  liftable %04X at %04X,%04X  %u,%u  %s\n", cell,
                       (uint16)((overworld_offset_base_x + ix) << 3),
                       (uint16)(overworld_offset_base_y + iy),
                       (unsigned)((overworld_offset_base_x + ix) << 3),
                       (unsigned)(overworld_offset_base_y + iy), what);
                total++;
            }
        }
    }

    printf("liftables: %d found\n", total);
}

// Where the game looks when Link acts on something, per facing. Reading the probe backwards
// gives the spot he has to stand on for a given cell to be the one under it.
enum { kFacingUp = 0, kFacingDown = 2, kFacingLeft = 4, kFacingRight = 6 };

static const uint8 kApproachFacings[4] = { kFacingLeft, kFacingRight, kFacingUp, kFacingDown };

static const char *DebugLocate_FacingName(uint8 facing) {
    switch (facing) {
    case kFacingUp:    return "up";
    case kFacingDown:  return "down";
    case kFacingLeft:  return "left";
    default:           return "right";
    }
}

// Standing beside a bush means standing partly over it, because the cell Link acts on when he
// faces up is the one his own head is in. So the target cell does not count against him here.
static bool IsSafeBeside(uint16 x, uint16 y, uint16 cell_x, uint16 cell_y) {
    static const uint8 kCornerX[4] = { 3, 12, 3, 12 };
    static const uint8 kCornerY[4] = { 10, 10, 16, 16 };

    for (int i = 0; i < 4; i++) {
        uint16 sx = (uint16)(x + kCornerX[i]);
        uint16 sy = (uint16)(y + kCornerY[i]);

        if ((sx & ~0xf) == cell_x && (sy & ~0xf) == cell_y) {
            continue;
        }

        if (!IsSafeStandingTile(Overworld_GetTileAttributeAtLocation(sx >> 3, sy))) {
            return false;
        }
    }

    return true;
}

bool DebugLocate_WarpToLiftable(int wanted) {
    bool dark = savegame_is_darkworld != 0;
    int skipped_heavy = 0;
    int crowded = 0;

    if (player_is_indoors) {
        printf("liftwarp: ignored, only works on the overworld\n");
        return false;
    }

    for (uint16 iy = 0; iy <= overworld_offset_mask_y; iy += 16) {
        for (uint16 ix = 0; ix <= overworld_offset_mask_x; ix += 2) {
            uint16 pos = ((iy & overworld_offset_mask_y) << 3) | (ix & overworld_offset_mask_x);
            uint16 cell = overworld_tileattr[pos >> 1];
            uint16 cell_x, cell_y;
            const char *what = DebugLocate_LiftableName(cell);

            if (!what || (wanted >= 0 && cell != (uint16)wanted)) {
                continue;
            }

            // Without the gloves the same A press grabs instead of lifting, so a heavy one is no
            // use for watching a pickup unless it was asked for by name.
            if (wanted < 0 && DebugLocate_LiftableNeedsGloves(cell)) {
                skipped_heavy++;
                continue;
            }

            cell_x = (uint16)((overworld_offset_base_x + ix) << 3);
            cell_y = (uint16)(overworld_offset_base_y + iy);

            for (int i = 0; i < 4; i++) {
                uint8 facing = kApproachFacings[i];
                int slot = facing >> 1;

                // The probe rounds down to a cell, so anywhere in the window works. The middle of
                // it keeps Link clear of the next cell along.
                uint16 x = (uint16)(cell_x - kGetBestActionToPerformOnTile_x[slot] + 8);
                uint16 y = (uint16)(cell_y - kGetBestActionToPerformOnTile_y[slot] + 8);

                if (!IsSafeBeside(x, y, cell_x, cell_y)) {
                    continue;
                }

                DebugGoto_JumpToPoint(x, y, dark);
                link_direction_facing = facing;

                printf("liftwarp: %04X (%s) at %04X,%04X, Link at %04X,%04X facing %s%s\n", cell,
                       what, cell_x, cell_y, x, y, DebugLocate_FacingName(facing),
                       DebugLocate_LiftableNeedsGloves(cell) ? ", needs gloves" : "");
                return true;
            }

            crowded++;
        }
    }

    printf("liftwarp: nothing reachable on the map around screen %02X"
           " (%d hemmed in, %d needing gloves)\n",
           (uint8)overworld_screen_index, crowded, skipped_heavy);
    return false;
}

// Walks a room's object stream without drawing any of it, which is the list of things a room
// is actually made of. Three layers of objects, then the doors.
void DebugLocate_DumpRoomObjects(int room) {
    const uint8 *data = GetDungeonRoomLayout(room);
    int offs = 0;
    int total = 0;

    if (data == NULL) {
        printf("room %d: no layout\n", room);
        return;
    }

    printf("room %d: floor bytes %02X, layout %02X\n", room, data[0], data[1]);

    // The floor byte and the layout byte come first, the way RoomDraw_DrawFloors reads them.
    offs = 2;

    for (int layer = 0; layer < 3; layer++) {
        printf("  layer %d\n", layer);

        for (;;) {
            uint16 d = data[offs] | (data[offs + 1] << 8);

            if (d == 0xffff || d == 0xfff0) {
                offs += 2;
                break;
            }

            uint8 idx = data[offs + 2];
            offs += 3;

            if ((d & 0xfc) != 0xfc) {
                int x = (uint8)d >> 2;
                int y = d >> 10;
                int w = d & 3;
                int h = (d >> 8) & 3;

                if (idx < 0xf8) {
                    printf("    subtype1 obj %02X at %2d,%2d size %d,%d\n", idx, x, y, w, h);
                } else {
                    int i3 = (idx & 7) << 4 | ((d >> 8) & 3) << 2 | (d & 3);
                    printf("    subtype3 obj %02X at %2d,%2d\n", i3, x, y);
                }
            } else {
                int x = (d & 3) << 4 | (d >> 12 & 0xf);
                int y = ((d >> 8) & 0xf) << 2 | (idx >> 6);

                printf("    subtype2 obj %02X at %2d,%2d\n", idx & 0x3f, x, y);
            }

            total++;
        }
    }

    for (;;) {
        uint16 d = data[offs] | (data[offs + 1] << 8);

        if (d == 0xffff) {
            break;
        }

        offs += 2;
        printf("    door type %02X at slot %d, %s\n", d >> 8, d >> 4 & 0xf,
               (d & 3) == 0 ? "north" : (d & 3) == 1 ? "south" : (d & 3) == 2 ? "west" : "east");
    }

    printf("room %d: %d objects\n", room, total);
}

// The assembled room, as map16 cells. This is what a converter reading the same room data has
// to reproduce, so it is the thing to check a port against.
void DebugLocate_DumpRoomMap(int room) {
    uint8 saved_indoors = player_is_indoors;
    uint16 saved_room = dungeon_room_index;
    char name[64];
    FILE *f;

    player_is_indoors = 1;
    dungeon_room_index = (uint16)room;
    dungeon_room_index2 = (uint16)room;

    Dungeon_LoadRoom();
    Dungeon_LoadCustomTileAttr();
    Dungeon_LoadAttributeTable();

    snprintf(name, sizeof(name), "zelda3_room_%03d.txt", room);
    f = fopen(name, "w");

    if (f == NULL) {
        printf("room %d: could not write %s\n", room, name);
    } else {
        for (int layer = 0; layer < 2; layer++) {
            const uint16 *src = layer ? dung_bg1 : dung_bg2;

            fprintf(f, "%s\n", layer ? "bg1" : "bg2");

            for (int y = 0; y < 64; y++) {
                for (int x = 0; x < 64; x++) {
                    fprintf(f, "%04X%c", src[y * 64 + x], x == 63 ? '\n' : ' ');
                }
            }
        }

        fclose(f);
        printf("wrote %s\n", name);
    }

    player_is_indoors = saved_indoors;
    dungeon_room_index = saved_room;
    dungeon_room_index2 = saved_room;
}
