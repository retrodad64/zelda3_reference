#include "debug_log.h"

#include "debug_image.h"

#include <stdarg.h>
#include <stdio.h>

#include "overworld.h"
#include "player.h"
#include "tile_detect.h"
#include "variables.h"
#include "zelda_rtl.h"

// Appended to, never truncated, so a capture survives across runs.
static const char kDebugLogPath[] = "zelda3_debug.log";

// Sampling grid for the tile dump, in 8 pixel steps around Link's feet.
enum {
    kTileGridRadius = 1,
    kTileGridStep = 8,

    // The map16 grid is in whole cells, and reaches two of them out so a thing Link is walking
    // toward shows up before he's standing on it.
    kCellGridRadius = 2,
    kCellGridStep = 16,

    kSpriteSlotCount = 16,
    kAncillaSlotCount = 10,
};

static FILE *g_log_file;
static bool g_tile_logging_enabled;
static bool g_animation_held;
static uint32 g_log_frame_counter;

static void DebugLog_Printf(const char *format, ...) {
    va_list args;

    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    if (g_log_file == NULL) {
        g_log_file = fopen(kDebugLogPath, "a");
        if (g_log_file == NULL) {
            return;
        }

        setvbuf(g_log_file, NULL, _IOLBF, 0);
    }

    va_start(args, format);
    vfprintf(g_log_file, format, args);
    va_end(args);
}

// GetTileAttribute in sprite.c writes sprite_tiletype, so the lookup is repeated here without that store.
static uint8 DebugLog_ReadTileAttribute(uint8 floor, uint16 x, uint16 y) {
    if (player_is_indoors) {
        int offset = (floor >= 1) ? 0x1000 : 0;

        offset += (x & 0x1f8) >> 3;
        offset += (y & 0x1f8) << 3;
        return dung_bg2_attr_table[offset];
    }

    return Overworld_GetTileAttributeAtLocation(x >> 3, y);
}

// Map16 index the attribute above was resolved through. Outdoors only.
static uint16 DebugLog_ReadMap16Index(uint16 x, uint16 y) {
    uint16 xt = x >> 3;
    uint16 offset;

    offset = ((y - overworld_offset_base_y) & overworld_offset_mask_y) * 8;
    offset |= ((xt - overworld_offset_base_x) & overworld_offset_mask_x);
    return overworld_tileattr[offset >> 1];
}

// The map16 values Overworld_HandleLiftableTiles acts on. Anything else in front of Link is
// scenery he can only walk into.
static bool DebugLog_IsLiftableCell(uint16 cell) {
    switch (cell) {
    case 0x036: case 0x72a: case 0x20f: case 0x239: case 0x101:
    case 0x36d: case 0x36e: case 0x374: case 0x375:
    case 0x23b: case 0x23c: case 0x23d: case 0x23e:
        return true;
    default:
        return false;
    }
}

static const char *DebugLog_PlayerStateName(uint8 state) {
    switch (state) {
    case kPlayerState_Ground: return "Ground";
    case kPlayerState_FallingIntoHole: return "FallingIntoHole";
    case kPlayerState_RecoilWall: return "RecoilWall";
    case kPlayerState_SpinAttacking: return "SpinAttacking";
    case kPlayerState_Swimming: return "Swimming";
    case kPlayerState_TurtleRock: return "TurtleRock";
    case kPlayerState_RecoilOther: return "RecoilOther";
    case kPlayerState_Electrocution: return "Electrocution";
    case kPlayerState_Ether: return "Ether";
    case kPlayerState_Bombos: return "Bombos";
    case kPlayerState_Quake: return "Quake";
    case kPlayerState_FallOfLeftRightLedge: return "FallOfLeftRightLedge";
    case kPlayerState_JumpOffLedgeDiag: return "JumpOffLedgeDiag";
    case kPlayerState_StartDash: return "StartDash";
    case kPlayerState_StopDash: return "StopDash";
    case kPlayerState_Hookshot: return "Hookshot";
    case kPlayerState_Mirror: return "Mirror";
    case kPlayerState_HoldUpItem: return "HoldUpItem";
    case kPlayerState_AsleepInBed: return "AsleepInBed";
    case kPlayerState_PermaBunny: return "PermaBunny";
    case kPlayerState_ReceivingEther: return "ReceivingEther";
    case kPlayerState_ReceivingBombos: return "ReceivingBombos";
    case kPlayerState_OpeningDesertPalace: return "OpeningDesertPalace";
    case kPlayerState_TempBunny: return "TempBunny";
    case kPlayerState_PullForRupees: return "PullForRupees";
    case kPlayerState_SpinAttackMotion: return "SpinAttackMotion";
    default: return "?";
    }
}

static void DebugLog_LogTiles(void) {
    // Feet rather than the sprite origin, so the sampled tile is the one Link collides with.
    uint16 feet_x = link_x_coord + 8;
    uint16 feet_y = link_y_coord + 16;
    uint8 floor = link_is_on_lower_level;

    DebugLog_Printf("[%u] tiles module=%02X/%02X indoors=%u floor=%u", g_log_frame_counter,
                    main_module_index, submodule_index, player_is_indoors, floor);

    if (player_is_indoors) {
        DebugLog_Printf(" room=%03X", dungeon_room_index);
    } else {
        DebugLog_Printf(" screen=%02X", (uint8)overworld_screen_index);
    }

    DebugLog_Printf(" link=%04X,%04X feet=%04X,%04X attr=%02X", link_x_coord, link_y_coord, feet_x, feet_y,
                    DebugLog_ReadTileAttribute(floor, feet_x, feet_y));

    if (!player_is_indoors) {
        Point16U front;
        uint16 cell = overworld_tileattr[Overworld_GetLinkMap16Coords(&front) >> 1];

        DebugLog_Printf(" map16=%04X", DebugLog_ReadMap16Index(feet_x, feet_y));

        // The cell Link would act on if A went in right now, which is the one the lift reads.
        DebugLog_Printf(" front=%04X@%04X,%04X attr=%02X%s", cell, front.x, front.y,
                        DebugLog_ReadTileAttribute(floor, front.x + 4, front.y + 4),
                        DebugLog_IsLiftableCell(cell) ? " LIFTABLE" : "");
    }

    DebugLog_Printf("\n");

    for (int row = -kTileGridRadius; row <= kTileGridRadius; row++) {
        DebugLog_Printf("      ");

        for (int col = -kTileGridRadius; col <= kTileGridRadius; col++) {
            uint16 sample_x = feet_x + col * kTileGridStep;
            uint16 sample_y = feet_y + row * kTileGridStep;

            DebugLog_Printf(" %02X", DebugLog_ReadTileAttribute(floor, sample_x, sample_y));
        }

        DebugLog_Printf("\n");
    }

    if (player_is_indoors) {
        return;
    }

    // Map16 indexes are stable across runs, unlike anything derived from VRAM, so these are the
    // numbers worth writing down. The centre cell is the one under Link's feet.
    for (int row = -kCellGridRadius; row <= kCellGridRadius; row++) {
        DebugLog_Printf("   m16");

        for (int col = -kCellGridRadius; col <= kCellGridRadius; col++) {
            uint16 sample_x = feet_x + col * kCellGridStep;
            uint16 sample_y = feet_y + row * kCellGridStep;

            DebugLog_Printf(" %04X", DebugLog_ReadMap16Index(sample_x, sample_y));
        }

        DebugLog_Printf("\n");
    }
}

static void DebugLog_LogAnimation(void) {
    DebugLog_Printf("[%u] pos=%d,%d z=%d velz=%d aux=%d lower=%d ledgetimer=%d\n", g_log_frame_counter,
                    (int)link_x_coord, (int)link_y_coord, (int16)link_z_coord,
                    (int8)link_actual_vel_z, link_auxiliary_state, link_is_on_lower_level,
                    (int8)link_timer_jump_ledge);
    DebugLog_Printf("[%u] anim oam=%02X/%02X state=%02X(%s) steps=%02X facing=%02X dir=%02X pose_item=%02X pose_opening=%02X speed=%02X incap=%02X joy=%02X%02X newHL=%02X%02X\n"
                    "       abtn=%02X bmask=%02X statebits=%02X grab=%02X inhand=%02X posmode=%02X bframes=%02X action=%02X pick=%02X timer=%02X htimer=%02X gloves=%02X\n",
                    g_log_frame_counter, value_computed_for_player_oam, BYTE(index_of_interacting_tile),
                    link_player_handler_state, DebugLog_PlayerStateName(link_player_handler_state),
                    link_animation_steps, link_direction_facing, link_direction, link_pose_for_item,
                    link_pose_during_opening, link_speed_setting, link_incapacitated_timer,
                    joypad1H_last, joypad1L_last, filtered_joypad_H, filtered_joypad_L,
                    bitfield_for_a_button, button_mask_b_y, link_state_bits, link_grabbing_wall,
                    link_item_in_hand, link_position_mode, button_b_frames, tile_action_index,
                    link_picking_throw_state, some_animation_timer, player_handler_timer,
                    link_item_gloves);

    for (int slot = 0; slot < kSpriteSlotCount; slot++) {
        if (sprite_state[slot] == 0) {
            continue;
        }

        DebugLog_Printf("      sprite %02X type=%02X state=%02X ai=%02X gfx=%02X pos=%04X,%04X z=%02X zvel=%02X\n", slot,
                        sprite_type[slot], sprite_state[slot], sprite_ai_state[slot], sprite_graphics[slot],
                        (uint16)(sprite_x_lo[slot] | (sprite_x_hi[slot] << 8)),
                        (uint16)(sprite_y_lo[slot] | (sprite_y_hi[slot] << 8)),
                        sprite_z[slot], sprite_z_vel[slot]);
    }

    // Link's own effects live here rather than in the sprite slots: sparkles, splashes, the
    // marks that come off him while he heaves at something.
    for (int slot = 0; slot < kAncillaSlotCount; slot++) {
        if (ancilla_type[slot] == 0) {
            continue;
        }

        DebugLog_Printf("      ancilla %02X type=%02X step=%02X timer=%02X pos=%04X,%04X\n", slot,
                        ancilla_type[slot], ancilla_step[slot], ancilla_timer[slot],
                        (uint16)(ancilla_x_lo[slot] | (ancilla_x_hi[slot] << 8)),
                        (uint16)(ancilla_y_lo[slot] | (ancilla_y_hi[slot] << 8)));
    }
}

void DebugLog_ToggleTileLogging(void) {
    g_tile_logging_enabled = !g_tile_logging_enabled;
    DebugLog_Printf("--- tile logging %s ---\n", g_tile_logging_enabled ? "on" : "off");

    // A snapshot of what is resident at the moment logging starts.
    if (g_tile_logging_enabled) {
        DebugImage_WriteTileDump();
    }
}

void DebugLog_SetAnimationHeld(bool held) {
    if (held == g_animation_held) {
        return;
    }

    g_animation_held = held;
    DebugLog_Printf("--- animation logging %s ---\n", held ? "on" : "off");

    if (held) {
        DebugImage_BeginAnimation();
    } else {
        DebugImage_EndAnimation();
    }
}

void DebugLog_Frame(void) {
    g_log_frame_counter++;

    if (g_tile_logging_enabled) {
        DebugLog_LogTiles();
    }

    if (g_animation_held) {
        DebugLog_LogAnimation();
        DebugImage_CaptureAnimationFrame();
    }
}

void DebugLog_Message(const char *text) {
    DebugLog_Printf("%s\n", text);
}

void DebugLog_Close(void) {
    if (g_log_file != NULL) {
        fclose(g_log_file);
        g_log_file = NULL;
    }
}
