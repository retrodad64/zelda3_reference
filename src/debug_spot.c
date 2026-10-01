#include "debug_spot.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assets.h"
#include "debug_goto.h"
#include "debug_scene.h"
#include "dungeon.h"
#include "hud.h"
#include "overworld.h"
#include "variables.h"
#include "zelda_rtl.h"

enum {
    kMaxItems = 64,
    kMaxRoomWords = 0x128,
    kOwEventAreas = 0xC0,
    kDungeonKeySlots = 16,
    kLineLength = 1024,

    // The item bytes, 0xF340 to 0xF35F: the Y items, the sword, shield and mail, the bottles.
    kItemFirst = 0xF340,
    kItemBytes = 0x20,

    // Modules: the overworld, a dungeon or house, and the falling entrance that --entrance uses.
    kModuleDungeon = 7,
    kModuleOverworld = 9,
    kModuleFallingEntrance = 0x11,

    // Module 7's submodule 7 is the fall through a pit into the room below (Module07_07).
    kSubmoduleFall = 7,

    // A room is 512 pixels a side. The room's column and row in the dungeon grid put it there,
    // 16 rooms to a row (dung_loade_bgoffs_h_copy and _v_copy).
    kRoomPx = 0x200,

    // How far over the spot he starts his fall, so he drops onto it from just above.
    kFallFrom = 0x30,

    // Where the camera sits from his sprite's corner, and how far the room's lower bounds
    // reach past the top of a quarter (the status bar), as an entrance's own numbers have them.
    kCameraX = 0x78,
    kCameraY = 0x70,
    kBoundsDown = 0x10,

    // A step that never gets its module gives up after this many frames.
    kStepFrames = 1200,

    // The reference's own feet are 16 below the sprite's corner (debug_goto.c).
    kFeetX = 8,
    kFeetY = 16,

    kHeartEighths = 8,
};

// Rooms 0x106 and 0x107 start every file with their first four doors open (the 0xF000 the file
// select writes at 0x20C and 0x20E), so a spot's words are put on top of that.
static const uint16 kNewFileRooms[2] = {0x106, 0x107};
static const uint16 kNewFileDoors = 0xF000;

typedef enum SpotStep {
    kStepNone,
    kStepLeaveRoom,   // Waiting for the game, then out of whatever room it starts in.
    kStepOnOverworld, // Waiting for the overworld, where the kit and story go down.
    kStepInEntrance,  // Waiting in the entrance's room for the drop into the spot's.
    kStepLanded,      // Waiting for the drop to finish.
} SpotStep;

typedef struct SpotItem {
    char key[32];
    int  level;
} SpotItem;

typedef struct SpotRoomWord {
    int    room;
    uint16 word;
} SpotRoomWord;

typedef struct Spot {
    SpotStep step;
    int      waited;

    bool indoors;
    bool dark;
    int  room;
    int  entrance;
    int  floor;
    int  link_x;  // The top left of his sprite: world pixels outdoors, room pixels in a room.
    int  link_y;
    int  face;

    SpotItem items[kMaxItems];
    int      item_count;

    int hearts;
    int health;
    int magic;
    int magic_rate;
    int rupees;
    int bombs;
    int arrows;
    int heart_pieces;
    int dungeon_keys[kDungeonKeySlots];

    uint16 big_keys;
    uint16 maps;
    uint16 compasses;

    int story;
    int progress_flags;
    int progress3;
    int map_icons;
    int start_point;
    int follower;

    SpotRoomWord room_words[kMaxRoomWords];
    int          room_word_count;
    uint8        ow_events[kOwEventAreas];
} Spot;

static Spot g_spot;

static bool DebugSpot_ReadLine(char *line, int number, bool *versioned) {
    char *word = strtok(line, " \t\r\n");
    char *rest = strtok(NULL, "\r\n");

    if (word == NULL || word[0] == '#') {
        return true;
    }

    // The demo's own lines come first, and are for the demo.
    if (strcmp(word, "cart") == 0) {
        if (rest == NULL || atoi(rest) != 1) {
            fprintf(stderr, "spot: line %d: format %s, only 1 is known\n", number, rest ? rest : "(none)");
            return false;
        }

        *versioned = true;
        return true;
    }

    if (strcmp(word, "place") != 0 && strncmp(word, "cart_", 5) != 0) {
        return true;
    }

    if (rest == NULL) {
        fprintf(stderr, "spot: line %d: %s has no value\n", number, word);
        return false;
    }

    if (strcmp(word, "place") == 0) {
        g_spot.indoors = strncmp(rest, "room", 4) == 0;
    } else if (strcmp(word, "cart_realm") == 0) {
        g_spot.dark = strncmp(rest, "dark", 4) == 0;
    } else if (strcmp(word, "cart_room") == 0) {
        g_spot.room = (int)strtol(rest, NULL, 16);
    } else if (strcmp(word, "cart_entrance") == 0) {
        g_spot.entrance = (int)strtol(rest, NULL, 16);
    } else if (strcmp(word, "cart_floor") == 0) {
        g_spot.floor = atoi(rest);
    } else if (strcmp(word, "cart_link") == 0) {
        if (sscanf(rest, "%d %d", &g_spot.link_x, &g_spot.link_y) != 2) {
            fprintf(stderr, "spot: line %d: cart_link wants x and y\n", number);
            return false;
        }
    } else if (strcmp(word, "cart_face") == 0) {
        g_spot.face = atoi(rest) & 6;
    } else if (strcmp(word, "cart_item") == 0) {
        SpotItem *it = &g_spot.items[g_spot.item_count];

        if (g_spot.item_count == kMaxItems || sscanf(rest, "%31s %d", it->key, &it->level) != 2) {
            fprintf(stderr, "spot: line %d: cart_item wants a name and a level, and at most %d of them\n", number,
                    kMaxItems);
            return false;
        }

        g_spot.item_count++;
    } else if (strcmp(word, "cart_hearts") == 0) {
        g_spot.hearts = atoi(rest);
    } else if (strcmp(word, "cart_health") == 0) {
        g_spot.health = atoi(rest);
    } else if (strcmp(word, "cart_magic") == 0) {
        g_spot.magic = atoi(rest);
    } else if (strcmp(word, "cart_magic_rate") == 0) {
        g_spot.magic_rate = atoi(rest);
    } else if (strcmp(word, "cart_rupees") == 0) {
        g_spot.rupees = atoi(rest);
    } else if (strcmp(word, "cart_bombs") == 0) {
        g_spot.bombs = atoi(rest);
    } else if (strcmp(word, "cart_arrows") == 0) {
        g_spot.arrows = atoi(rest);
    } else if (strcmp(word, "cart_heart_pieces") == 0) {
        g_spot.heart_pieces = atoi(rest);
    } else if (strcmp(word, "cart_dungeon_keys") == 0) {
        char *at = rest;

        for (int i = 0; i < kDungeonKeySlots; i++) {
            char *end = NULL;
            long  n = strtol(at, &end, 10);

            if (end == at) {
                break;
            }

            g_spot.dungeon_keys[i] = (int)n;
            at = end;
        }
    } else if (strcmp(word, "cart_big_keys") == 0) {
        g_spot.big_keys = (uint16)strtol(rest, NULL, 16);
    } else if (strcmp(word, "cart_maps") == 0) {
        g_spot.maps = (uint16)strtol(rest, NULL, 16);
    } else if (strcmp(word, "cart_compasses") == 0) {
        g_spot.compasses = (uint16)strtol(rest, NULL, 16);
    } else if (strcmp(word, "cart_story") == 0) {
        g_spot.story = atoi(rest);
    } else if (strcmp(word, "cart_progress_flags") == 0) {
        g_spot.progress_flags = (int)strtol(rest, NULL, 16);
    } else if (strcmp(word, "cart_progress3") == 0) {
        g_spot.progress3 = (int)strtol(rest, NULL, 16);
    } else if (strcmp(word, "cart_map_icons") == 0) {
        g_spot.map_icons = atoi(rest);
    } else if (strcmp(word, "cart_start_point") == 0) {
        g_spot.start_point = atoi(rest);
    } else if (strcmp(word, "cart_follower") == 0) {
        g_spot.follower = atoi(rest);
    } else if (strcmp(word, "cart_room_word") == 0) {
        unsigned room = 0;
        unsigned bits = 0;

        if (g_spot.room_word_count == kMaxRoomWords || sscanf(rest, "%x %x", &room, &bits) != 2 ||
            room >= kMaxRoomWords) {
            fprintf(stderr, "spot: line %d: cart_room_word wants a room under 128 hex and a word\n", number);
            return false;
        }

        g_spot.room_words[g_spot.room_word_count].room = (int)room;
        g_spot.room_words[g_spot.room_word_count].word = (uint16)bits;
        g_spot.room_word_count++;
    } else if (strcmp(word, "cart_ow_event") == 0) {
        unsigned area = 0;
        unsigned bits = 0;

        if (sscanf(rest, "%x %x", &area, &bits) != 2 || area >= kOwEventAreas) {
            fprintf(stderr, "spot: line %d: cart_ow_event wants an area under C0 hex and its bits\n", number);
            return false;
        }

        g_spot.ow_events[area] = (uint8)bits;
    } else {
        // A later demo may say more; what this does not know it leaves alone.
        fprintf(stderr, "spot: line %d: %s is not known, left out\n", number, word);
    }

    return true;
}

bool DebugSpot_Load(const char *path) {
    FILE *f = fopen(path, "r");
    char  line[kLineLength];
    int   number = 0;
    bool  versioned = false;
    bool  placed = false;

    if (f == NULL) {
        fprintf(stderr, "spot: cannot open '%s'\n", path);
        return false;
    }

    memset(&g_spot, 0, sizeof(g_spot));
    g_spot.room = -1;
    g_spot.entrance = -1;
    g_spot.face = 2;
    g_spot.hearts = 3;
    g_spot.health = -1;

    while (fgets(line, sizeof(line), f) != NULL) {
        number++;

        if (strncmp(line, "place", 5) == 0) {
            placed = true;
        }

        if (!DebugSpot_ReadLine(line, number, &versioned)) {
            fclose(f);
            return false;
        }
    }

    fclose(f);

    if (!versioned || !placed) {
        fprintf(stderr, "spot: '%s' has no cartridge lines; F2 in a newer pug hero demo writes them\n", path);
        return false;
    }

    if (g_spot.indoors && g_spot.room < 0) {
        fprintf(stderr, "spot: '%s' is in a room but does not say which\n", path);
        return false;
    }

    g_spot.step = kStepLeaveRoom;
    return true;
}

bool DebugSpot_Pending(void) {
    return g_spot.step != kStepNone;
}

// The way in to use for a room: the one he came in by, or else the first that leads into it.
static int DebugSpot_Entrance(void) {
    int count = (int)(kEntranceData_rooms_SIZE / sizeof(uint16));

    if (g_spot.entrance >= 0) {
        return g_spot.entrance;
    }

    for (int i = 0; i < count; i++) {
        if (kEntranceData_rooms[i] == g_spot.room) {
            return i;
        }
    }

    return -1;
}

static int DebugSpot_OwEventCount(void) {
    int n = 0;

    for (int i = 0; i < kOwEventAreas; i++) {
        n += g_spot.ow_events[i] != 0;
    }

    return n;
}

// Everything the save keeps, from the spot. The rooms and the overworld are cleared first, so
// what the file does not name is as a new file has it.
static void DebugSpot_ApplyState(void) {
    memset(&g_ram[kItemFirst], 0, kItemBytes);
    link_which_pendants = 0;
    link_has_crystals = 0;

    for (int i = 0; i < g_spot.item_count; i++) {
        if (!DebugScene_SetItem(g_spot.items[i].key, g_spot.items[i].level) &&
            strcmp(g_spot.items[i].key, "bombs") != 0) {
            fprintf(stderr, "spot: no item called %s, left out\n", g_spot.items[i].key);
        }
    }

    // The bottle that is out is the first one he has.
    link_item_bottle_index = 0;

    for (int i = 0; i < 4; i++) {
        if (link_bottle_info[i] != 0) {
            link_item_bottle_index = (uint8)(i + 1);
            break;
        }
    }

    link_item_bombs = (uint8)g_spot.bombs;
    link_num_arrows = (uint8)g_spot.arrows;
    link_rupees_goal = link_rupees_actual = (uint16)g_spot.rupees;
    link_magic_power = (uint8)g_spot.magic;
    link_magic_consumption = (uint8)g_spot.magic_rate;
    link_heart_pieces = (uint8)g_spot.heart_pieces;
    link_health_capacity = (uint8)(g_spot.hearts * kHeartEighths);
    link_health_current = (uint8)((g_spot.health < 0 || g_spot.health > link_health_capacity) ? link_health_capacity
                                                                                              : g_spot.health);

    for (int i = 0; i < kDungeonKeySlots; i++) {
        link_keys_earned_per_dungeon[i] = (uint8)g_spot.dungeon_keys[i];
    }

    // Outdoors no dungeon's keys count; a dungeon's own come back as he goes in.
    link_num_keys = 0xff;
    link_bigkey = g_spot.big_keys;
    link_dungeon_map = g_spot.maps;
    link_compass = g_spot.compasses;

    sram_progress_indicator = (uint8)g_spot.story;
    sram_progress_flags = (uint8)g_spot.progress_flags;
    sram_progress_indicator_3 = (uint8)g_spot.progress3;
    savegame_map_icons_indicator = (uint8)g_spot.map_icons;
    which_starting_point = (uint8)g_spot.start_point;

    // The light world's sprite sheets change with the story, and the game reloads them itself
    // whenever it changes the story (PrepareDungeonExitFromBossFight, after Agahnim).
    Sprite_LoadGraphicsProperties_light_world_only();
    follower_indicator = (uint8)g_spot.follower;

    memset(save_dung_info, 0, kMaxRoomWords * sizeof(uint16));

    for (int i = 0; i < 2; i++) {
        save_dung_info[kNewFileRooms[i]] = kNewFileDoors;
    }

    for (int i = 0; i < g_spot.room_word_count; i++) {
        save_dung_info[g_spot.room_words[i].room] |= g_spot.room_words[i].word;
    }

    memcpy(save_ow_event_info, g_spot.ow_events, kOwEventAreas);

    Hud_Rebuild();
    DebugScene_ReloadKitGraphicsSoon();

    printf("spot: kit set, sword %d, shield %d, mail %d, %d hearts, %d rooms and %d areas remembered\n",
           link_sword_type, link_shield_type, link_armor, g_spot.hearts, g_spot.room_word_count,
           DebugSpot_OwEventCount());
}

// Module07_07 from where he stands: the room he is in is the one he falls from, and the spot's
// is the one he lands in. Dungeon_AdjustAfterSpiralStairs moves him from the one room's grid
// square to the other's, so his position is given in the room he is leaving.
static void DebugSpot_DropIntoRoom(void) {
    uint16 from = dungeon_room_index;
    uint16 base_x = (uint16)((from & 0xf) * kRoomPx);
    uint16 base_y = (uint16)(((from & 0xff0) >> 4) * kRoomPx);

    dungeon_room_index_prev = from;
    dungeon_room_index = (uint16)g_spot.room;

    link_x_coord = (uint16)(base_x + g_spot.link_x);
    tiledetect_which_y_pos[0] = (uint16)(base_y + g_spot.link_y);
    link_y_coord = (uint16)(tiledetect_which_y_pos[0] - kFallFrom);

    // The quarter of the room he lands in, and the camera and its bounds for it, the way a
    // room's own way in leaves them (Dungeon_LoadEntrance): a fall keeps whatever the room he
    // falls from had, and he falls from somewhere else in it.
    {
        int    qx = g_spot.link_x >= kRoomPx / 2;
        int    qy = g_spot.link_y >= kRoomPx / 2;
        uint16 cam_x = (uint16)(link_x_coord - kCameraX);
        uint16 cam_y = (uint16)(tiledetect_which_y_pos[0] - kCameraY);

        link_quadrant_x = (uint8)qx;
        link_quadrant_y = (uint8)(qy * 2);

        room_bounds_x.a0 = room_bounds_x.a1 = (uint16)(base_x + qx * (kRoomPx / 2));
        room_bounds_x.b0 = base_x;
        room_bounds_x.b1 = (uint16)(base_x + kRoomPx / 2);
        room_bounds_y.a0 = (uint16)(base_y + qy * (kRoomPx / 2));
        room_bounds_y.a1 = (uint16)(room_bounds_y.a0 + kBoundsDown);
        room_bounds_y.b0 = base_y;
        room_bounds_y.b1 = (uint16)(base_y + kRoomPx / 2 + kBoundsDown);

        if ((int16)(cam_x - base_x) < 0) cam_x = base_x;
        if (cam_x > room_bounds_x.b1) cam_x = room_bounds_x.b1;
        if ((int16)(cam_y - base_y) < 0) cam_y = base_y;
        if (cam_y > room_bounds_y.b1) cam_y = room_bounds_y.b1;

        BG1HOFS_copy2 = BG2HOFS_copy2 = BG1HOFS_copy = BG2HOFS_copy = cam_x;
        BG1VOFS_copy2 = BG2VOFS_copy2 = BG1VOFS_copy = BG2VOFS_copy = cam_y;
    }
    link_is_on_lower_level = link_is_on_lower_level_mirror = (uint8)g_spot.floor;
    link_direction_facing = (uint8)g_spot.face;

    // What falling down a pit sets as he goes (LinkState_HandlingPits).
    link_this_controls_sprite_oam = 6;
    player_near_pit_state = 3;
    link_visibility_status = 12;
    link_speed_modifier = 16;
    link_state_bits = 0;
    link_picking_throw_state = 0;
    link_grabbing_wall = 0;

    submodule_index = kSubmoduleFall;
    subsubmodule_index = 0;
}

static bool DebugSpot_Waited(const char *what) {
    if (++g_spot.waited < kStepFrames) {
        return false;
    }

    fprintf(stderr, "spot: never %s, giving up\n", what);
    g_spot.step = kStepNone;
    return true;
}

void DebugSpot_Frame(void) {
    bool in_play = submodule_index == 0 && (main_module_index == kModuleOverworld || main_module_index == kModuleDungeon);

    switch (g_spot.step) {
    case kStepNone:
        return;

    case kStepLeaveRoom:
        if (!in_play) {
            DebugSpot_Waited("got into the game");
            return;
        }

        // Out by the room's own way out, which is where the game goes from any room.
        if (player_is_indoors) {
            Dung_HandleExitToOverworld();
        }

        g_spot.step = kStepOnOverworld;
        g_spot.waited = 0;
        return;

    case kStepOnOverworld:
        if (!in_play || main_module_index != kModuleOverworld || player_is_indoors) {
            DebugSpot_Waited("reached the overworld");
            return;
        }

        DebugSpot_ApplyState();
        g_spot.waited = 0;

        if (!g_spot.indoors) {
            DebugGoto_JumpToPoint((uint16)(g_spot.link_x + kFeetX), (uint16)(g_spot.link_y + kFeetY), g_spot.dark);
            link_direction_facing = (uint8)g_spot.face;
            g_spot.step = kStepNone;
            printf("spot: outdoors at %d,%d, %s world\n", g_spot.link_x, g_spot.link_y, g_spot.dark ? "dark" : "light");
            return;
        }

        {
            int entrance = DebugSpot_Entrance();

            if (entrance < 0) {
                fprintf(stderr, "spot: no entrance leads to room %03X, so he stays outside\n", g_spot.room);
                g_spot.step = kStepNone;
                return;
            }

            // The world outside the dungeon, which the palettes and the bunny go by.
            savegame_is_darkworld = g_spot.dark ? 0x40 : 0;
            which_entrance = (uint8)entrance;
            main_module_index = kModuleFallingEntrance;
            submodule_index = 0;
            subsubmodule_index = 0;
            g_spot.step = kStepInEntrance;
            printf("spot: in by entrance %02X for room %03X\n", entrance, g_spot.room);
        }
        return;

    case kStepInEntrance:
        if (!in_play || main_module_index != kModuleDungeon) {
            DebugSpot_Waited("got through the entrance");
            return;
        }

        DebugSpot_DropIntoRoom();
        g_spot.step = kStepLanded;
        g_spot.waited = 0;
        return;

    case kStepLanded:
        if (!in_play) {
            DebugSpot_Waited("landed in the room");
            return;
        }

        link_direction_facing = (uint8)g_spot.face;
        Hud_RebuildIndoor();
        DebugScene_ReloadKitGraphicsSoon();
        g_spot.step = kStepNone;
        printf("spot: room %03X at %d,%d on floor %d\n", g_spot.room, g_spot.link_x, g_spot.link_y, g_spot.floor);
        return;
    }
}
