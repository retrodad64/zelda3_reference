#include "debug_scene.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assets.h"
#include "debug_goto.h"
#include "hud.h"
#include "load_gfx.h"
#include "overworld.h"
#include "sprite.h"
#include "variables.h"
#include "zelda_rtl.h"

enum {
    // The arena is built on a big area, the one place a scene as large as the sandbox's usual
    // one fits. Kakariko in the light world, the village of outcasts in the dark.
    kArenaScreenLight = 0x18,
    kArenaScreenDark = 0x58,

    // A big area is 64 map16 cells a side, 1024 pixels, and its map16 grid is 64 wide.
    kAreaCells = 64,
    kCellPx = 16,
    kScreenPx = 512,

    // A ring of wall one cell thick goes round the scene, so the scene starts one cell in.
    kArenaMargin = 1,
    kArenaMaxCells = kAreaCells - 2 * kArenaMargin,

    kMaxThings = 512,
    kMaxItems = 64,
    kLineLength = 2048,

    // Tile attributes: a plain wall, and the three kinds of plain ground Link walks on with
    // nothing happening, the last two being ground the shovel digs.
    kAttrWall = 0x01,
    kAttrGround = 0x00,
    kAttrDiggable = 0x48,
    kAttrDiggableToo = 0x4a,

    // The sword, the shield and the mail are reloaded this many frames after the scene goes
    // down, once the screen's own sheets have gone up. Reloading them at once decompresses
    // through the same scratch memory the screen's sheets are still waiting in.
    kKitGfxDelay = 4,

    kMap16Count = 0x1000,

    // A sprite waiting to be set up. The game's own setup runs on its next frame.
    kSpriteStateInit = 8,
    kSpriteSlots = 16,

    kHeartEighths = 8,
    kFaceDown = 2,
};

// Which bit of the crystal byte each crystal is, in the order the dungeons give them.
static const uint8 kCrystalBits[7] = {0x02, 0x40, 0x08, 0x20, 0x01, 0x04, 0x10};

typedef struct SceneThing {
    int x;
    int y;
    int value;  // A map16 cell for a tile, a sprite number for a sprite.
} SceneThing;

typedef struct SceneItem {
    char key[32];
    int  level;
} SceneItem;

typedef struct Scene {
    bool loaded;
    bool dark;
    int  w;
    int  h;
    int  link_x;
    int  link_y;
    int  hearts;
    int  health;
    int  magic;
    int  rupees;
    int  bombs;
    int  arrows;
    int  keys;
    int  face;   // The way Link looks, as the game keeps it: 0 up, 2 down, 4 left, 6 right.
    int  kills;  // Creatures put down since the tongue in the wall last paid.
    int  hits;   // Blows taken in that time.

    int16 ground[kArenaMaxCells * kArenaMaxCells];  // -1 for plain ground.

    SceneThing walls[kMaxThings];
    int        wall_count;
    SceneThing tiles[kMaxThings];
    int        tile_count;
    SceneThing sprites[kMaxThings];
    int        sprite_count;
    SceneItem  items[kMaxItems];
    int        item_count;
    int        skipped;
} Scene;

static Scene g_scene;
static int   g_kit_gfx_countdown;

static void DebugScene_AddThing(SceneThing *list, int *count, int x, int y, int value) {
    if (*count >= kMaxThings) {
        return;
    }

    list[*count].x = x;
    list[*count].y = y;
    list[*count].value = value;
    (*count)++;
}

// One "ground <row> <cell> <cell> ..." line, where a cell is - or a map16 in hex.
static void DebugScene_ReadGroundRow(char *rest) {
    char *tok = strtok(rest, " \t\r\n");

    if (tok == NULL) {
        return;
    }

    int row = atoi(tok);
    int col = 0;

    while ((tok = strtok(NULL, " \t\r\n")) != NULL) {
        if (row >= 0 && row < kArenaMaxCells && col < kArenaMaxCells) {
            g_scene.ground[row * kArenaMaxCells + col] =
                (tok[0] == '-') ? -1 : (int16)strtol(tok, NULL, 16);
        }

        col++;
    }
}

static void DebugScene_ReadLine(char *line, int number, int *version) {
    char word[32];
    int  a = 0;
    int  b = 0;
    char hex[16];

    if (line[0] == '#' || sscanf(line, "%31s", word) != 1) {
        return;
    }

    char *rest = line + strlen(word);

    if (strcmp(word, "scene") == 0) {
        sscanf(rest, "%d", version);
    } else if (strcmp(word, "realm") == 0) {
        g_scene.dark = strstr(rest, "dark") != NULL;
    } else if (strcmp(word, "size") == 0) {
        sscanf(rest, "%d %d", &g_scene.w, &g_scene.h);
    } else if (strcmp(word, "link") == 0) {
        sscanf(rest, "%d %d", &g_scene.link_x, &g_scene.link_y);
    } else if (strcmp(word, "hearts") == 0) {
        sscanf(rest, "%d", &g_scene.hearts);
    } else if (strcmp(word, "health") == 0) {
        sscanf(rest, "%d", &g_scene.health);
    } else if (strcmp(word, "magic") == 0) {
        sscanf(rest, "%d", &g_scene.magic);
    } else if (strcmp(word, "rupees") == 0) {
        sscanf(rest, "%d", &g_scene.rupees);
    } else if (strcmp(word, "bombs") == 0) {
        sscanf(rest, "%d", &g_scene.bombs);
    } else if (strcmp(word, "arrows") == 0) {
        sscanf(rest, "%d", &g_scene.arrows);
    } else if (strcmp(word, "keys") == 0) {
        sscanf(rest, "%d", &g_scene.keys);
    } else if (strcmp(word, "kills") == 0) {
        sscanf(rest, "%d", &g_scene.kills);
    } else if (strcmp(word, "hits") == 0) {
        sscanf(rest, "%d", &g_scene.hits);
    } else if (strcmp(word, "item") == 0) {
        if (g_scene.item_count < kMaxItems &&
            sscanf(rest, "%31s %d", g_scene.items[g_scene.item_count].key,
                   &g_scene.items[g_scene.item_count].level) == 2) {
            g_scene.item_count++;
        }
    } else if (strcmp(word, "ground") == 0) {
        DebugScene_ReadGroundRow(rest);
    } else if (strcmp(word, "wall") == 0) {
        if (sscanf(rest, "%d %d", &a, &b) == 2) {
            DebugScene_AddThing(g_scene.walls, &g_scene.wall_count, a, b, 0);
        }
    } else if (strcmp(word, "tile") == 0) {
        if (sscanf(rest, "%d %d %15s", &a, &b, hex) == 3) {
            DebugScene_AddThing(g_scene.tiles, &g_scene.tile_count, a, b, (int)strtol(hex, NULL, 16));
        }
    } else if (strcmp(word, "sprite") == 0) {
        if (sscanf(rest, "%15s %d %d", hex, &a, &b) == 3) {
            DebugScene_AddThing(g_scene.sprites, &g_scene.sprite_count, a, b, (int)strtol(hex, NULL, 16));
        }
    } else if (strcmp(word, "skip") == 0) {
        g_scene.skipped++;
    } else if (strcmp(word, "face") == 0) {
        static const char *const kFaces[4] = {"up", "down", "left", "right"};
        char way[16] = "";

        sscanf(rest, "%15s", way);

        for (int i = 0; i < 4; i++) {
            if (strcmp(way, kFaces[i]) == 0)
                g_scene.face = i * 2;
        }
    } else {
        fprintf(stderr, "scene: line %d: '%s' is not something a scene says, ignored\n", number, word);
    }
}

bool DebugScene_Load(const char *path) {
    FILE *f = fopen(path, "r");

    if (f == NULL) {
        fprintf(stderr, "scene: cannot open '%s'\n", path);
        return false;
    }

    memset(&g_scene, 0, sizeof(g_scene));
    memset(g_scene.ground, 0xff, sizeof(g_scene.ground));
    g_scene.hearts = 3;
    g_scene.health = -1;
    g_scene.face = kFaceDown;
    g_scene.magic = 0x80;

    char line[kLineLength];
    int  version = 0;
    int  number = 0;

    while (fgets(line, sizeof(line), f) != NULL) {
        DebugScene_ReadLine(line, ++number, &version);
    }

    fclose(f);

    if (version != 1) {
        fprintf(stderr, "scene: '%s' is not a version 1 scene\n", path);
        return false;
    }

    if (g_scene.w <= 0 || g_scene.h <= 0) {
        fprintf(stderr, "scene: '%s' gives no size\n", path);
        return false;
    }

    // A scene bigger than the area is cut down to what fits, and says so.
    if (g_scene.w > kArenaMaxCells || g_scene.h > kArenaMaxCells) {
        fprintf(stderr, "scene: %dx%d is larger than the %dx%d an area holds; the rest is cut off\n",
                g_scene.w, g_scene.h, kArenaMaxCells, kArenaMaxCells);
        g_scene.w = (g_scene.w > kArenaMaxCells) ? kArenaMaxCells : g_scene.w;
        g_scene.h = (g_scene.h > kArenaMaxCells) ? kArenaMaxCells : g_scene.h;
    }

    g_scene.loaded = true;
    printf("scene: %s, %dx%d cells, %d sprites, %d tiles, %d walls, %d things it cannot place\n", path,
           g_scene.w, g_scene.h, g_scene.sprite_count, g_scene.tile_count, g_scene.wall_count,
           g_scene.skipped);
    return true;
}

bool DebugScene_Pending(void) {
    return g_scene.loaded;
}

void DebugScene_Forget(void) {
    g_scene.loaded = false;
}

static uint8 DebugScene_Attr(uint16 m16, int quarter) {
    return kMap8DataToTileAttr[GetMap16toMap8Table()[m16 * 4 + quarter] & 0x1ff];
}

static bool DebugScene_AttrIsOpen(uint8 attr) {
    return attr == kAttrGround || attr == kAttrDiggable || attr == kAttrDiggableToo;
}

// True when every quarter of a map16 cell is open ground, or every quarter is plain wall.
static bool DebugScene_CellIs(uint16 m16, bool wall) {
    for (int q = 0; q < 4; q++) {
        uint8 attr = DebugScene_Attr(m16, q);

        if (wall ? attr != kAttrWall : !DebugScene_AttrIsOpen(attr)) {
            return false;
        }
    }

    return true;
}

// The cell the loaded area uses most that is all open ground, or all wall.
static uint16 DebugScene_MostCommon(bool wall, uint16 fallback) {
    static uint16 counts[kMap16Count];
    uint16 best = fallback;
    int    best_count = 0;

    memset(counts, 0, sizeof(counts));

    for (int i = 0; i < kAreaCells * kAreaCells; i++) {
        uint16 m16 = dung_bg2[i];

        if (m16 < kMap16Count && DebugScene_CellIs(m16, wall)) {
            counts[m16]++;
        }
    }

    for (int m16 = 0; m16 < kMap16Count; m16++) {
        if (counts[m16] > best_count) {
            best_count = counts[m16];
            best = (uint16)m16;
        }
    }

    return best;
}

static uint16 *DebugScene_Cell(int scene_x, int scene_y) {
    return &dung_bg2[(scene_y + kArenaMargin) * kAreaCells + scene_x + kArenaMargin];
}

// Lays the scene into the area's map16 grid and has the screen drawn again from it.
static void DebugScene_BuildArena(void) {
    uint16 ground = DebugScene_MostCommon(false, 0x034);
    uint16 wall = DebugScene_MostCommon(true, 0x000);

    for (int i = 0; i < kAreaCells * kAreaCells; i++) {
        dung_bg2[i] = wall;
    }

    for (int y = 0; y < g_scene.h; y++) {
        for (int x = 0; x < g_scene.w; x++) {
            int16 m16 = g_scene.ground[y * kArenaMaxCells + x];

            *DebugScene_Cell(x, y) = (m16 >= 0) ? (uint16)m16 : ground;
        }
    }

    // A wall only goes where the ground is open. Where the scene's own ground is a cliff or a
    // tree, the cell already stops him and keeps the look it has.
    for (int i = 0; i < g_scene.wall_count; i++) {
        const SceneThing *t = &g_scene.walls[i];

        if (t->x < 0 || t->y < 0 || t->x >= g_scene.w || t->y >= g_scene.h) {
            continue;
        }

        uint16 *cell = DebugScene_Cell(t->x, t->y);

        if (DebugScene_CellIs(*cell, false)) {
            *cell = wall;
        }
    }

    for (int i = 0; i < g_scene.tile_count; i++) {
        const SceneThing *t = &g_scene.tiles[i];

        if (t->x >= 0 && t->y >= 0 && t->x < g_scene.w && t->y < g_scene.h) {
            *DebugScene_Cell(t->x, t->y) = (uint16)t->value;
        }
    }

    // The tilemap is drawn again from the grid, and the NMI uploads it on the next frame. The
    // submodule step and the screen blank it does on the way out are put back, the same as the
    // jump does for the screen it loads.
    uint8 saved_submodule = submodule_index;
    uint8 saved_inidisp = INIDISP_copy;

    Overworld_LoadAmbientOverlay(false);
    submodule_index = saved_submodule;
    INIDISP_copy = saved_inidisp;

    printf("scene: ground %03X, wall %03X\n", ground, wall);
}

// Puts one sprite down the way the game puts down one from an area's list, so its own setup
// runs on the next frame. It belongs to no list, so it is never taken back or put down again.
static void DebugScene_Spawn(uint8 type, uint16 x, uint16 y) {
    for (int k = kSpriteSlots - 1; k >= 0; k--) {
        if (sprite_state[k] != 0) {
            continue;
        }

        sprite_type[k] = type;
        sprite_state[k] = kSpriteStateInit;
        sprite_N_word[k] = 0xffff;
        sprite_x_lo[k] = (uint8)x;
        sprite_x_hi[k] = (uint8)(x >> 8);
        sprite_y_lo[k] = (uint8)y;
        sprite_y_hi[k] = (uint8)(y >> 8);
        sprite_floor[k] = 0;
        sprite_subtype[k] = 0;
        sprite_die_action[k] = 0;
        return;
    }

    fprintf(stderr, "scene: no free sprite slot for %02X at %d,%d\n", type, x, y);
}

static uint8 *DebugScene_ItemVar(const char *key) {
    static const struct {
        const char *key;
        uint16      addr;
    } kItems[] = {
        {"bow", 0xF340},         {"boomerang", 0xF341},    {"hookshot", 0xF342},
        {"powder", 0xF344},      {"fire_rod", 0xF345},     {"ice_rod", 0xF346},
        {"bombos", 0xF347},      {"ether", 0xF348},        {"quake", 0xF349},
        {"lamp", 0xF34A},        {"hammer", 0xF34B},       {"flute", 0xF34C},
        {"bug_net", 0xF34D},     {"book", 0xF34E},         {"cane_somaria", 0xF350},
        {"cane_byrna", 0xF351},  {"cape", 0xF352},         {"mirror", 0xF353},
        {"gloves", 0xF354},      {"boots", 0xF355},        {"flippers", 0xF356},
        {"moon_pearl", 0xF357},  {"sword", 0xF359},        {"shield", 0xF35A},
        {"mail", 0xF35B},        {"bottle_1", 0xF35C},     {"bottle_2", 0xF35D},
        {"bottle_3", 0xF35E},    {"bottle_4", 0xF35F},
    };

    for (size_t i = 0; i < sizeof(kItems) / sizeof(kItems[0]); i++) {
        if (strcmp(kItems[i].key, key) == 0) {
            return &g_ram[kItems[i].addr];
        }
    }

    return NULL;
}

bool DebugScene_SetItem(const char *key, int level) {
    uint8 *var = DebugScene_ItemVar(key);
    int    n = 0;

    if (var != NULL) {
        *var = (uint8)level;
    } else if (strcmp(key, "pendant_courage") == 0) {
        link_which_pendants |= level ? 1 : 0;
    } else if (strcmp(key, "pendant_wisdom") == 0) {
        link_which_pendants |= level ? 2 : 0;
    } else if (strcmp(key, "pendant_power") == 0) {
        link_which_pendants |= level ? 4 : 0;
    } else if (sscanf(key, "crystal_%d", &n) == 1 && n >= 1 && n <= 7) {
        link_has_crystals |= level ? kCrystalBits[n - 1] : 0;
    } else {
        return false;
    }

    return true;
}

void DebugScene_ReloadKitGraphicsSoon(void) {
    g_kit_gfx_countdown = kKitGfxDelay;
}

static void DebugScene_ApplyKit(void) {
    link_which_pendants = 0;
    link_has_crystals = 0;

    for (int i = 0; i < g_scene.item_count; i++) {
        DebugScene_SetItem(g_scene.items[i].key, g_scene.items[i].level);
    }

    // The bottle that is out is the first one he has.
    link_item_bottle_index = 0;

    for (int i = 0; i < 4; i++) {
        if (link_bottle_info[i] != 0) {
            link_item_bottle_index = (uint8)(i + 1);
            break;
        }
    }

    // The count of bombs is the kit's bombs; having any is the same thing to the game.
    link_item_bombs = (uint8)g_scene.bombs;
    link_num_arrows = (uint8)g_scene.arrows;
    link_num_keys = (uint8)g_scene.keys;
    link_rupees_goal = link_rupees_actual = (uint16)g_scene.rupees;
    link_magic_power = (uint8)g_scene.magic;

    // What the tongue in the wall pays out turns on these.
    num_sprites_killed = (uint8)g_scene.kills;
    number_of_times_hurt_by_sprites = (uint8)g_scene.hits;
    link_health_capacity = (uint8)(g_scene.hearts * kHeartEighths);
    link_health_current = (uint8)((g_scene.health < 0 || g_scene.health > link_health_capacity)
                                      ? link_health_capacity
                                      : g_scene.health);

    Hud_Rebuild();

    // The sword, the shield and the mail are drawn from sheets and colours picked by level,
    // which go up a few frames from now.
    g_kit_gfx_countdown = kKitGfxDelay;
}

void DebugScene_Frame(void) {
    if (g_kit_gfx_countdown == 0 || --g_kit_gfx_countdown != 0) {
        return;
    }

    DecompressSwordGraphics();
    DecompressShieldGraphics();
    Palette_Load_Sword();
    Palette_Load_Shield();
    Palette_Load_LinkArmorAndGloves();
}

void DebugScene_Apply(void) {
    if (!g_scene.loaded) {
        return;
    }

    g_scene.loaded = false;

    uint8  screen = g_scene.dark ? kArenaScreenDark : kArenaScreenLight;
    uint16 origin_x = (uint16)((screen & 7) * kScreenPx + kArenaMargin * kCellPx);
    uint16 origin_y = (uint16)(((screen & 0x3f) >> 3) * kScreenPx + kArenaMargin * kCellPx);

    DebugGoto_JumpToSprite(screen, (uint16)(origin_x + g_scene.link_x), (uint16)(origin_y + g_scene.link_y));
    link_direction_facing = (uint8)g_scene.face;

    DebugScene_BuildArena();

    // The area's own sprites go, and so does its list, so none of them walks in from off screen.
    Sprite_ResetAll();
    memset(sprite_where_in_overworld, 0, 0x1000);
    memset(overworld_sprite_was_loaded, 0, 0x200);

    for (int i = 0; i < g_scene.sprite_count; i++) {
        const SceneThing *s = &g_scene.sprites[i];

        DebugScene_Spawn((uint8)s->value, (uint16)(origin_x + s->x), (uint16)(origin_y + s->y));
    }

    DebugScene_ApplyKit();

    if (g_scene.sprite_count > kSpriteSlots) {
        fprintf(stderr, "scene: the game holds %d sprites at once, so %d were not put down\n", kSpriteSlots,
                g_scene.sprite_count - kSpriteSlots);
    }

    printf("scene: put down on screen %02X, Link at %d,%d\n", screen, origin_x + g_scene.link_x,
           origin_y + g_scene.link_y);
}
