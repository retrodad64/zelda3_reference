#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL.h>
#ifdef _WIN32
#include "platform/win32/volume_control.h"
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include "snes/ppu.h"

#include "types.h"
#include "variables.h"

#include "zelda_rtl.h"
#include "zelda_cpu_infra.h"

#include "config.h"
#include "assets.h"
#include "messaging.h"
#include "select_file.h"
#include "load_gfx.h"
#include "util.h"
#include "audio.h"
#include "debug_log.h"
#include "png_writer.h"
#include "debug_goto.h"
#include "debug_spot.h"
#include "debug_scene.h"
#include "player.h"
#include "debug_locate.h"

static bool g_run_without_emu = 0;

// Forwards
static bool LoadRom(const char *filename);
static void LoadLinkGraphics();
static void RenderNumber(uint8 *dst, size_t pitch, int n, bool big);
static void HandleInput(int keyCode, int modCode, bool pressed);
static void HandleCommand(uint32 j, bool pressed);
static int RemapSdlButton(int button);
static void HandleGamepadInput(int button, bool pressed);
static void HandleGamepadAxisInput(int gamepad_id, int axis, int value);
static void OpenOneGamepad(int i);
static void HandleVolumeAdjustment(int volume_adjustment);
static void LoadAssets();
static void SwitchDirectory();

enum {
  kDefaultFullscreen = 0,
  kMaxWindowScale = 10,
  kDefaultFreq = 44100,
  kDefaultChannels = 2,
  kDefaultSamples = 2048,
};

static const char kWindowTitle[] = "The Legend of Zelda: A Link to the Past";
static uint32 g_win_flags = SDL_WINDOW_RESIZABLE;
static SDL_Window *g_window;

static uint8 g_paused, g_turbo, g_replay_turbo = true, g_cursor = true;
static uint8 g_current_window_scale;
static uint8 g_gamepad_buttons;
static int g_input1_state;
static bool g_display_perf;
static int g_curr_fps;
static int g_ppu_render_flags = 0;
static int g_snes_width, g_snes_height;
static int g_sdl_audio_mixer_volume = SDL_MIX_MAXVOLUME;
static struct RendererFuncs g_renderer_funcs;
static uint32 g_gamepad_modifiers;
static uint16 g_gamepad_last_cmd[kGamepadBtn_Count];

void NORETURN Die(const char *error) {
#if defined(NDEBUG) && defined(_WIN32)
  SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, kWindowTitle, error, NULL);
#endif
  fprintf(stderr, "Error: %s\n", error);
  exit(1);
}

void ChangeWindowScale(int scale_step) {
  if ((SDL_GetWindowFlags(g_window) & (SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_MINIMIZED | SDL_WINDOW_MAXIMIZED)) != 0)
    return;
  int screen = SDL_GetWindowDisplayIndex(g_window);
  if (screen < 0) screen = 0;
  int max_scale = kMaxWindowScale;
  SDL_Rect bounds;
  int bt = -1, bl, bb, br;
  // note this takes into effect Windows display scaling, i.e., resolution is divided by scale factor
  if (SDL_GetDisplayUsableBounds(screen, &bounds) == 0) {
    // this call may take a while before it is reported by Windows (or not at all in my testing)
    if (SDL_GetWindowBordersSize(g_window, &bt, &bl, &bb, &br) != 0) {
      // guess based on Windows 10/11 defaults
      bl = br = bb = 1;
      bt = 31;
    }
    // Allow a scale level slightly above the max that fits on screen
    int mw = (bounds.w - bl - br + g_snes_width / 4) / g_snes_width;
    int mh = (bounds.h - bt - bb + g_snes_height / 4) / g_snes_height;
    max_scale = IntMin(mw, mh);
  }
  int new_scale = IntMax(IntMin(g_current_window_scale + scale_step, max_scale), 1);
  g_current_window_scale = new_scale;
  int w = new_scale * g_snes_width;
  int h = new_scale * g_snes_height;

  //SDL_RenderSetLogicalSize(g_renderer, w, h);
  SDL_SetWindowSize(g_window, w, h);
  if (bt >= 0) {
    // Center the window on top of the mouse
    int mx, my;
    SDL_GetGlobalMouseState(&mx, &my);
    int wx = IntMax(IntMin(mx - w / 2, bounds.x + bounds.w - bl - br - w), bounds.x + bl);
    int wy = IntMax(IntMin(my - h / 2, bounds.y + bounds.h - bt - bb - h), bounds.y + bt);
    SDL_SetWindowPosition(g_window, wx, wy);
  } else {
    SDL_SetWindowPosition(g_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
  }
}

#define RESIZE_BORDER 20
static SDL_HitTestResult HitTestCallback(SDL_Window *win, const SDL_Point *pt, void *data) {
  uint32 flags = SDL_GetWindowFlags(win);
  if ((flags & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0 || (flags & SDL_WINDOW_FULLSCREEN) != 0)
    return SDL_HITTEST_NORMAL;

  if ((SDL_GetModState() & KMOD_CTRL) != 0)
    return SDL_HITTEST_DRAGGABLE;

  int w, h;
  SDL_GetWindowSize(win, &w, &h);

  if (pt->y < RESIZE_BORDER) {
    return (pt->x < RESIZE_BORDER) ? SDL_HITTEST_RESIZE_TOPLEFT :
           (pt->x >= w - RESIZE_BORDER) ? SDL_HITTEST_RESIZE_TOPRIGHT : SDL_HITTEST_RESIZE_TOP;
  } else if (pt->y >= h - RESIZE_BORDER) {
    return (pt->x < RESIZE_BORDER) ? SDL_HITTEST_RESIZE_BOTTOMLEFT :
           (pt->x >= w - RESIZE_BORDER) ? SDL_HITTEST_RESIZE_BOTTOMRIGHT : SDL_HITTEST_RESIZE_BOTTOM;
  } else {
    if (pt->x < RESIZE_BORDER) {
      return SDL_HITTEST_RESIZE_LEFT;
    } else if (pt->x >= w - RESIZE_BORDER) {
      return SDL_HITTEST_RESIZE_RIGHT;
    }
  }
  return SDL_HITTEST_NORMAL;
}

// Shift+S, or "screenshot" in a key script. Written from the frame the renderer just drew,
// so it is exactly what is on screen, message boxes and all.
static bool g_want_screenshot;
static int g_screenshot_counter;

static void SaveScreenshot(const uint8 *pixels, int pitch, int width, int height) {
  uint8 *rgb = malloc((size_t)width * height * 3);
  char name[64];

  if (rgb == NULL)
    return;

  // The texture is ARGB8888, so each pixel is four bytes with the blue one first in memory.
  for (int y = 0; y < height; y++) {
    const uint8 *src = pixels + (size_t)y * pitch;
    uint8 *dst = rgb + (size_t)y * width * 3;

    for (int x = 0; x < width; x++, src += 4, dst += 3) {
      dst[0] = src[2];
      dst[1] = src[1];
      dst[2] = src[0];
    }
  }

  snprintf(name, sizeof(name), "zelda3_frame_%04d.png", ++g_screenshot_counter);

  if (WritePng(name, width, height, rgb))
    printf("wrote %s (%dx%d)\n", name, width, height);
  else
    fprintf(stderr, "could not write %s\n", name);

  free(rgb);
}

static void DrawPpuFrameWithPerf() {
  int render_scale = PpuGetCurrentRenderScale(g_zenv.ppu, g_ppu_render_flags);
  uint8 *pixel_buffer = 0;
  int pitch = 0;

  g_renderer_funcs.BeginDraw(g_snes_width * render_scale,
                             g_snes_height * render_scale,
                             &pixel_buffer, &pitch);
  if (g_display_perf || g_config.display_perf_title) {
    static float history[64], average;
    static int history_pos;
    uint64 before = SDL_GetPerformanceCounter();
    ZeldaDrawPpuFrame(pixel_buffer, pitch, g_ppu_render_flags);
    uint64 after = SDL_GetPerformanceCounter();
    float v = (double)SDL_GetPerformanceFrequency() / (after - before);
    average += v - history[history_pos];
    history[history_pos] = v;
    history_pos = (history_pos + 1) & 63;
    g_curr_fps = average * (1.0f / 64);
  } else {
    ZeldaDrawPpuFrame(pixel_buffer, pitch, g_ppu_render_flags);
  }
  if (g_display_perf)
    RenderNumber(pixel_buffer + pitch * render_scale, pitch, g_curr_fps, render_scale == 4);
  DebugGoto_DrawOverlay(pixel_buffer, pitch, render_scale);

  if (g_want_screenshot) {
    g_want_screenshot = false;
    SaveScreenshot(pixel_buffer, pitch, g_snes_width * render_scale,
                   g_snes_height * render_scale);
  }

  g_renderer_funcs.EndDraw();
}

static SDL_mutex *g_audio_mutex;
static uint8 *g_audiobuffer, *g_audiobuffer_cur, *g_audiobuffer_end;
static int g_frames_per_block;
static uint8 g_audio_channels;

static void SDLCALL AudioCallback(void *userdata, Uint8 *stream, int len) {
  if (SDL_LockMutex(g_audio_mutex)) Die("Mutex lock failed!");
  while (len != 0) {
    if (g_audiobuffer_end - g_audiobuffer_cur == 0) {
      ZeldaRenderAudio((int16*)g_audiobuffer, g_frames_per_block, g_audio_channels);
      g_audiobuffer_cur = g_audiobuffer;
      g_audiobuffer_end = g_audiobuffer + g_frames_per_block * g_audio_channels * sizeof(int16);
    }
    int n = IntMin(len, g_audiobuffer_end - g_audiobuffer_cur);
    if (g_sdl_audio_mixer_volume == SDL_MIX_MAXVOLUME) {
      memcpy(stream, g_audiobuffer_cur, n);
    } else {
      SDL_memset(stream, 0, n);
      SDL_MixAudioFormat(stream, g_audiobuffer_cur, AUDIO_S16, n, g_sdl_audio_mixer_volume);
    }
    g_audiobuffer_cur += n;
    stream += n;
    len -= n;
  }

  ZeldaDiscardUnusedAudioFrames();
  SDL_UnlockMutex(g_audio_mutex);
}

// State for sdl renderer
static SDL_Renderer *g_renderer;
static SDL_Texture *g_texture;
static SDL_Rect g_sdl_renderer_rect;

static bool SdlRenderer_Init(SDL_Window *window) {

  if (g_config.shader)
    fprintf(stderr, "Warning: Shaders are supported only with the OpenGL backend\n");

  SDL_Renderer *renderer = SDL_CreateRenderer(g_window, -1,
                                              g_config.output_method == kOutputMethod_SDLSoftware ? SDL_RENDERER_SOFTWARE :
                                              SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (renderer == NULL) {
    printf("Failed to create renderer: %s\n", SDL_GetError());
    return false;
  }
  SDL_RendererInfo renderer_info;
  SDL_GetRendererInfo(renderer, &renderer_info);
  if (kDebugFlag) {
    printf("Supported texture formats:");
    for (int i = 0; i < renderer_info.num_texture_formats; i++)
      printf(" %s", SDL_GetPixelFormatName(renderer_info.texture_formats[i]));
    printf("\n");
  }
  g_renderer = renderer;
  if (!g_config.ignore_aspect_ratio)
    SDL_RenderSetLogicalSize(renderer, g_snes_width, g_snes_height);
  if (g_config.linear_filtering)
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");

  int tex_mult = (g_ppu_render_flags & kPpuRenderFlags_4x4Mode7) ? 4 : 1;
  g_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                                g_snes_width * tex_mult, g_snes_height * tex_mult);
  if (g_texture == NULL) {
    printf("Failed to create texture: %s\n", SDL_GetError());
    return false;
  }
  return true;
}

static void SdlRenderer_Destroy() {
  SDL_DestroyTexture(g_texture);
  SDL_DestroyRenderer(g_renderer);
}

static void SdlRenderer_BeginDraw(int width, int height, uint8 **pixels, int *pitch) {
  g_sdl_renderer_rect.w = width;
  g_sdl_renderer_rect.h = height;
  if (SDL_LockTexture(g_texture, &g_sdl_renderer_rect, (void **)pixels, pitch) != 0) {
    printf("Failed to lock texture: %s\n", SDL_GetError());
    return;
  }
}

static void SdlRenderer_EndDraw() {

//  uint64 before = SDL_GetPerformanceCounter();
  SDL_UnlockTexture(g_texture);
//  uint64 after = SDL_GetPerformanceCounter();
//  float v = (double)(after - before) / SDL_GetPerformanceFrequency();
//  printf("%f ms\n", v * 1000);
  SDL_RenderClear(g_renderer);
  SDL_RenderCopy(g_renderer, g_texture, &g_sdl_renderer_rect, NULL);
  SDL_RenderPresent(g_renderer); // vsyncs to 60 FPS?
}

static const struct RendererFuncs kSdlRendererFuncs  = {
  &SdlRenderer_Init,
  &SdlRenderer_Destroy,
  &SdlRenderer_BeginDraw,
  &SdlRenderer_EndDraw,
};

void OpenGLRenderer_Create(struct RendererFuncs *funcs, bool use_opengl_es);

#undef main
// --keys: drive the game from a script of key presses, so a run can be repeated exactly and
// without a person at the keyboard. Events go through HandleCommand, the same path a real key
// takes, which is what makes a held command like LogAnimation work.
static void HandleCommand(uint32 j, bool pressed);

typedef struct KeyScriptEvent {
  uint32 frame;
  uint16 cmd;
  uint8  pressed;
} KeyScriptEvent;

static KeyScriptEvent *g_key_script;
static int g_key_script_len;
static int g_key_script_cap;
static int g_key_script_pos;
static uint32 g_key_script_frame;

// --quit-after: stop after this many frames, so a scripted run ends on its own.
static uint32 g_quit_after;

// The names a script may use. The joypad comes first, in the order the ini lists it, and the
// rest are the command names from that same file.
typedef struct KeyScriptName {
  const char *name;
  uint16      cmd;
} KeyScriptName;

static const KeyScriptName kKeyScriptNames[] = {
  {"up",     kKeys_Controls + 0},
  {"down",   kKeys_Controls + 1},
  {"left",   kKeys_Controls + 2},
  {"right",  kKeys_Controls + 3},
  {"select", kKeys_Controls + 4},
  {"start",  kKeys_Controls + 5},
  {"a",      kKeys_Controls + 6},
  {"b",      kKeys_Controls + 7},
  {"x",      kKeys_Controls + 8},
  {"y",      kKeys_Controls + 9},
  {"l",      kKeys_Controls + 10},
  {"r",      kKeys_Controls + 11},
  {"logtiles",      kKeys_LogTiles},
  {"loganimation",  kKeys_LogAnimation},
  {"gotoscreen",    kKeys_GotoScreen},
  {"cheatlife",     kKeys_CheatLife},
  {"cheatequipment", kKeys_CheatEquipment},
  {"walkthroughwalls", kKeys_CheatWalkThroughWalls},
  {"invincible",    kKeys_CheatInvincible},
  {"nomusic",       kKeys_NoMusic},
  {"listsprites",   kKeys_ListSprites},
  {"screenshot",    kKeys_Screenshot},
  {"copylocation",  kKeys_CopyLocation},
  {"pause",         kKeys_Pause},
  {"turbo",         kKeys_Turbo},
};

static bool KeyScript_Lookup(const char *name, uint16 *out) {
  for (size_t i = 0; i < countof(kKeyScriptNames); i++) {
    if (strcasecmp(kKeyScriptNames[i].name, name) == 0) {
      *out = kKeyScriptNames[i].cmd;
      return true;
    }
  }

  // Savestate slots, written the way the ini writes them: Load1 through Load10.
  int slot = 0;

  if (sscanf(name, "load%d", &slot) == 1 && slot >= 1 && slot <= 10) {
    *out = (uint16)(kKeys_Load + slot - 1);
    return true;
  }

  if (sscanf(name, "save%d", &slot) == 1 && slot >= 1 && slot <= 10) {
    *out = (uint16)(kKeys_Save + slot - 1);
    return true;
  }

  return false;
}

static void KeyScript_Add(uint32 frame, uint16 cmd, bool pressed) {
  if (g_key_script_len == g_key_script_cap) {
    g_key_script_cap = g_key_script_cap ? g_key_script_cap * 2 : 64;
    g_key_script = realloc(g_key_script, g_key_script_cap * sizeof(*g_key_script));
  }

  g_key_script[g_key_script_len].frame = frame;
  g_key_script[g_key_script_len].cmd = cmd;
  g_key_script[g_key_script_len].pressed = pressed;
  g_key_script_len++;
}

// Read a script. One statement per line, acted on in order:
//
//   wait <frames>          nothing happens for this long
//   tap <name>             press and release, two frames
//   hold <name> <frames>   press, wait, release
//
// Blank lines and anything after a # are ignored. A bad line stops the program, because a
// script that half ran would be worse than one that did not run at all.
static void KeyScript_Load(const char *path) {
  FILE *f = fopen(path, "r");

  if (f == NULL) {
    fprintf(stderr, "Unable to open key script '%s'\n", path);
    exit(1);
  }

  char line[256];
  uint32 at = 0;
  int lineno = 0;

  while (fgets(line, sizeof(line), f)) {
    lineno++;

    char *hash = strchr(line, '#');

    if (hash)
      *hash = '\0';

    char verb[64], name[64];
    int frames = 0;

    if (sscanf(line, "%63s %63s %d", verb, name, &frames) >= 1) {
      uint16 cmd = 0;

      if (strcasecmp(verb, "wait") == 0 && sscanf(line, "%63s %d", verb, &frames) == 2 &&
          frames > 0) {
        at += (uint32)frames;
        continue;
      }

      if (strcasecmp(verb, "tap") == 0 && KeyScript_Lookup(name, &cmd)) {
        KeyScript_Add(at, cmd, true);
        KeyScript_Add(at + 2, cmd, false);
        at += 2;
        continue;
      }

      if (strcasecmp(verb, "hold") == 0 && frames > 0 && KeyScript_Lookup(name, &cmd)) {
        KeyScript_Add(at, cmd, true);
        KeyScript_Add(at + (uint32)frames, cmd, false);
        at += (uint32)frames;
        continue;
      }

      // press and release take no time, so two of them can bracket other actions. That is the
      // only way to capture an animation that a button press starts, because the logging key
      // has to already be down when the button goes in.
      if (strcasecmp(verb, "press") == 0 && KeyScript_Lookup(name, &cmd)) {
        KeyScript_Add(at, cmd, true);
        continue;
      }

      if (strcasecmp(verb, "release") == 0 && KeyScript_Lookup(name, &cmd)) {
        KeyScript_Add(at, cmd, false);
        continue;
      }

      fprintf(stderr, "%s:%d: cannot read '%s'\n", path, lineno, verb);
      fclose(f);
      exit(1);
    }
  }

  fclose(f);
  printf("*** Key script '%s': %d events over %u frames\n", path, g_key_script_len, at);
}

// Fire everything the script has due this frame, then move its clock on.
// True while something asked for on the command line still hasn't happened. The script clock is
// held at zero until this clears, so a script's frame numbers mean the same thing every run no
// matter how long a save load or an intro skip took.
static bool KeyScript_SetupPending(void);

// Frames to let the game settle after the last of that lands. A jump ends with the screen blanked
// and the submodule stepped, and the following frame is what undoes both.
enum { kKeyScriptSettleFrames = 4 };

static int g_key_script_settle;

static void KeyScript_RunFrame(void) {
  if (KeyScript_SetupPending()) {
    g_key_script_settle = kKeyScriptSettleFrames;
    return;
  }

  if (g_key_script_settle > 0) {
    g_key_script_settle--;
    return;
  }

  if (g_key_script_frame == 0)
    printf("*** Key script starts now\n");

  while (g_key_script_pos < g_key_script_len &&
         g_key_script[g_key_script_pos].frame <= g_key_script_frame) {
    HandleCommand(g_key_script[g_key_script_pos].cmd,
                  g_key_script[g_key_script_pos].pressed != 0);
    g_key_script_pos++;
  }

  g_key_script_frame++;
}

// --skip-intro: start a fresh file straight away instead of sitting through the title, the
// attract loop and the file select. Set from the command line, acted on once the first frame
// has run, because the game only initialises itself on that frame.
static bool g_skip_intro;

// --load-save <file>: load a savestate once the first frame has run. Same thing the F1 to F10
// keys do, but for any path. Relative paths resolve from the directory holding zelda3.ini.
static const char *g_load_save_path;

// --screen-id <hex>: jump to an overworld screen once the game gets there. It waits rather than
// firing on a fixed frame, because a save load and an intro skip take different amounts of time.
// --enemy-warp, --npc-warp, --boss-warp. Held until the overworld is up, like the screen jump.
static int g_warp_type = -1;
static bool g_list_sprites;
static bool g_list_liftables;

// --lift-warp [hex]: stand Link beside something he can pick up. It runs after any screen
// jump, so the jump chooses the area and this chooses the spot inside it.
static int g_lift_warp = -2;

// --entrance <hex>: go through a door without having to find it. The game is dropped into
// the falling entrance module, which loads the room the same way walking in does.
static int g_entrance = -1;

// --dump-room-objects <n>: print what one room is built from, then carry on.
static int g_dump_room_objects = -1;
static bool g_dump_rooms;
// --no-music: start with the music off, so an unattended run is quiet.
static bool g_start_muted;

// --invincible: turn the damage cheat on before the first frame, for a run that is not about
// combat and should not end because something wandered into Link.
static bool g_start_invincible;

// --give-flippers: put the flippers in Link's kit once the game is running, so a save made
// for testing deep water does not have to be played up to the Zora first.
static bool g_give_flippers;
static int g_warp_category;

// Options that all decide where Link ends up. Naming two of them is a mistake worth stopping for,
// so the first one claims the slot and the second one fails. Add to this as new options arrive.
static const char *g_position_option;

static void ClaimPositionOption(const char *name) {
  if (g_position_option != NULL) {
    fprintf(stderr, "%s cannot be used with %s, they both decide where Link ends up\n",
            name, g_position_option);
    exit(1);
  }
  g_position_option = name;
}

static int g_screen_id = -1;
static int g_screen_id_waited;
static int g_goto_x = -1;
static int g_goto_y = -1;
static bool g_goto_dark;

static bool KeyScript_SetupPending(void) {
  return g_load_save_path != NULL || g_skip_intro || g_screen_id >= 0 || g_goto_x >= 0 ||
         g_warp_type >= 0 || g_list_liftables || g_lift_warp != -2 || g_entrance >= 0 ||
         DebugScene_Pending() || DebugSpot_Pending();
}

// Start file one on a brand new save, the way picking an empty slot and naming it does.
//
// The steps are lifted from NameFile_EraseSave and the tail of NameFile_DoTheNaming in
// select_file.c, which is the game's own new-file path, and then CopySaveToWRAM, which is what
// choosing a file on the select screen calls. That lands in module 5, the file loader, which
// puts Link in his house with his uncle already gone.
static void SkipIntroToNewGame(void) {
  static const uint8 kSramInit_Normal[60] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0,    0,    0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0,    0,    0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0x18, 0x18, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0xf8, 0, 0,
  };

  const int slot = 0;
  uint8 *sram = &g_zenv.sram[slot * 0x500];

  memset(sram, 0, 0x500);

  // Six blanks for the name, which is what the naming screen starts with.
  uint16 *name = (uint16 *)(sram + kSrmOffs_Name);
  for (int i = 0; i < 6; i++)
    name[i] = 0xa9;

  WORD(sram[0x3e5]) = 0x55aa;
  WORD(sram[0x20c]) = 0xf000;
  WORD(sram[0x20e]) = 0xf000;
  WORD(sram[kSrmOffs_DiedCounter]) = 0xffff;
  memcpy(sram + 0x340, kSramInit_Normal, 60);
  Intro_FixCksum(sram);

  srm_var1 = slot * 2 + 2;
  WORD(g_ram[0]) = slot * 0x500;
  CopySaveToWRAM();

  // Skip the opening as well. Module_PreDungeon only tucks Link into bed, which is what starts
  // the long telepathy from Zelda, while this flag is clear, so setting it lands him standing
  // in the house with nothing to read through. It is set in RAM rather than in the save block
  // because CopySaveToWRAM has just copied the one to the other.
  sram_progress_flags |= 0x10;
}


// Everything the argument parsing below accepts. Keep this in step with it: a flag that is
// parsed but not listed here is a flag nobody finds.
static void PrintUsage(void) {
  static const char *kUsage[] = {
    "zelda3 [options] [rom]",
    "",
    "A bare path is a ROM for the reference emulator. The game itself runs from",
    "zelda3_assets.dat and does not need one.",
    "",
    "Setup",
    "  --config <file>        Read settings from this file. Must come first, and",
    "                         stops the search for zelda3.ini.",
    "  --skip-intro           Start a new file in Link's house.",
    "  --load-save <file>     Load a savestate, the files F1 to F10 use.",
    "",
    "Going somewhere. Only one of these at a time.",
    "  --screen-id <hex>      Overworld screen, 00 to 7F.",
    "  --goto <x,y>           An exact world position.",
    "  --entrance <hex>       Entrance number, 00 to FF.",
    "  --enemy-warp <hex>     Beside the first sprite of that type.",
    "  --npc-warp <hex>       The same, for a harmless sprite.",
    "  --boss-warp <hex>      The same, for a high health sprite.",
    "  --lift-warp [<hex>]    Beside a liftable cell. Without a value, any.",
    "  --scene <file>         Rebuild a scene saved by the pug hero entity sandbox.",
    "  --spot <file>          Go where F2 in the pug hero demo left him, with his kit.",
    "",
    "Listing and dumping",
    "  --list-sprites         Every sprite the overworld data places.",
    "  --list-liftables       Every liftable cell on the current screen.",
    "  --dump-rooms           Hazard tile counts for all 320 dungeon rooms.",
    "  --dump-room-objects <hex>  The objects one room draws.",
    "",
    "Running unattended",
    "  --keys <file>          Play from a key script.",
    "  --quit-after <n>       Exit cleanly after this many frames.",
    "  --no-music             Start with the music off.",
    "  --invincible           Start with the invincible cheat on.",
    "  --give-flippers        Put the flippers in Link's kit once the save has loaded.",
    "",
    "  --help                 This list.",
    "",
    "Keys are not set here. They live in the [KeyMap] section of zelda3.ini.",
    "command_line.md has the detail.",
  };

  for (size_t i = 0; i < countof(kUsage); i++) {
    printf("%s\n", kUsage[i]);
  }
}

int main(int argc, char** argv) {
  argc--, argv++;

  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      PrintUsage();
      return 0;
    }
  }

  const char *config_file = NULL;
  if (argc >= 2 && strcmp(argv[0], "--config") == 0) {
    config_file = argv[1];
    argc -= 2, argv += 2;
  } else {
    SwitchDirectory();
  }

  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "--keys") == 0 && i + 1 < argc) {
      KeyScript_Load(argv[i + 1]);
      memmove(argv + i, argv + i + 2, (argc - i - 2) * sizeof(*argv));
      argc -= 2;
      break;
    }
  }

  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "--quit-after") == 0 && i + 1 < argc) {
      g_quit_after = (uint32)strtoul(argv[i + 1], NULL, 10);
      memmove(argv + i, argv + i + 2, (argc - i - 2) * sizeof(*argv));
      argc -= 2;
      break;
    }
  }

  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "--skip-intro") == 0) {
      g_skip_intro = true;
      memmove(argv + i, argv + i + 1, (argc - i - 1) * sizeof(*argv));
      argc--;
      break;
    }
  }

  for (int i = 0; i < argc; i++) {
    int taken = 0;
    if (strncmp(argv[i], "--load-save=", 12) == 0) {
      g_load_save_path = argv[i] + 12;
      taken = 1;
    } else if (strcmp(argv[i], "--load-save") == 0) {
      if (i + 1 >= argc)
        Die("--load-save needs a file name");
      g_load_save_path = argv[i + 1];
      taken = 2;
    }
    if (taken) {
      // Fail now rather than after the window is up, so a typo is obvious.
      FILE *f = fopen(g_load_save_path, "rb");
      if (!f) {
        fprintf(stderr, "Unable to open save file '%s'\n", g_load_save_path);
        exit(1);
      }
      fclose(f);
      memmove(argv + i, argv + i + taken, (argc - i - taken) * sizeof(*argv));
      argc -= taken;
      break;
    }
  }

  for (int i = 0; i < argc; i++) {
    const char *value = NULL;
    int taken = 0;
    if (strncmp(argv[i], "--screen-id=", 12) == 0) {
      value = argv[i] + 12;
      taken = 1;
    } else if (strcmp(argv[i], "--screen-id") == 0) {
      if (i + 1 >= argc)
        Die("--screen-id needs a screen number");
      value = argv[i + 1];
      taken = 2;
    }
    if (taken) {
      char *end = NULL;
      long id = strtol(value, &end, 16);
      if (end == value || *end != 0 || id < 0 || id > 0x7f) {
        fprintf(stderr, "--screen-id wants an overworld screen in hex, 00 to 7F, got '%s'\n", value);
        exit(1);
      }
      ClaimPositionOption("--screen-id");
      g_screen_id = (int)id;
      memmove(argv + i, argv + i + taken, (argc - i - taken) * sizeof(*argv));
      argc -= taken;
      break;
    }
  }

  // --enemy-warp, --npc-warp and --boss-warp all take a sprite type in hex. They do the same
  // search, and the category only decides the warning when the id is not the kind you named.
  static const struct { const char *flag; int category; } kWarpFlags[] = {
    {"--enemy-warp", kLocateCategory_Enemy},
    {"--npc-warp",   kLocateCategory_Npc},
    {"--boss-warp",  kLocateCategory_Boss},
  };

  for (size_t w = 0; w < countof(kWarpFlags); w++) {
    size_t flaglen = strlen(kWarpFlags[w].flag);

    for (int i = 0; i < argc; i++) {
      const char *value = NULL;
      int taken = 0;
      if (strncmp(argv[i], kWarpFlags[w].flag, flaglen) == 0 && argv[i][flaglen] == '=') {
        value = argv[i] + flaglen + 1;
        taken = 1;
      } else if (strcmp(argv[i], kWarpFlags[w].flag) == 0) {
        if (i + 1 >= argc)
          Die("warp flag needs a sprite type");
        value = argv[i + 1];
        taken = 2;
      }
      if (taken) {
        char *end = NULL;
        long id = strtol(value, &end, 16);
        if (end == value || *end != 0 || id < 0 || id > 0xf2) {
          fprintf(stderr, "%s wants a sprite type in hex, 00 to F2, got '%s'\n",
                  kWarpFlags[w].flag, value);
          exit(1);
        }
        ClaimPositionOption(kWarpFlags[w].flag);
        g_warp_type = (int)id;
        g_warp_category = kWarpFlags[w].category;
        memmove(argv + i, argv + i + taken, (argc - i - taken) * sizeof(*argv));
        argc -= taken;
        break;
      }
    }
  }

  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "--no-music") == 0) {
      g_start_muted = true;
      memmove(argv + i, argv + i + 1, (argc - i - 1) * sizeof(*argv));
      argc--;
      break;
    }
  }

  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "--invincible") == 0) {
      g_start_invincible = true;
      memmove(argv + i, argv + i + 1, (argc - i - 1) * sizeof(*argv));
      argc--;
      break;
    }
  }

  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "--give-flippers") == 0) {
      g_give_flippers = true;
      memmove(argv + i, argv + i + 1, (argc - i - 1) * sizeof(*argv));
      argc--;
      break;
    }
  }

  // --scene: a place built in the pug demo's sandbox. It is read now, so a file that will not
  // do stops the run before the window opens rather than after the save has loaded.
  // --spot: the spot F2 remembers in the pug demo, with his kit and story. Read now for the same
  // reason. The spot carries the whole save, so without a savestate it starts from a new file.
  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "--spot") == 0) {
      if (i + 1 >= argc)
        Die("--spot needs a file");
      ClaimPositionOption("--spot");
      if (!DebugSpot_Load(argv[i + 1]))
        exit(1);
      memmove(argv + i, argv + i + 2, (argc - i - 2) * sizeof(*argv));
      argc -= 2;
      break;
    }
  }

  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "--scene") == 0) {
      if (i + 1 >= argc)
        Die("--scene needs a file");
      ClaimPositionOption("--scene");
      if (!DebugScene_Load(argv[i + 1]))
        exit(1);
      memmove(argv + i, argv + i + 2, (argc - i - 2) * sizeof(*argv));
      argc -= 2;
      break;
    }
  }

  for (int i = 0; i < argc; i++) {
    const char *value = NULL;
    int taken = 0;
    if (strncmp(argv[i], "--lift-warp=", 12) == 0) {
      value = argv[i] + 12;
      taken = 1;
    } else if (strcmp(argv[i], "--lift-warp") == 0) {
      value = (i + 1 < argc && argv[i + 1][0] != '-') ? argv[i + 1] : NULL;
      taken = value ? 2 : 1;
    }
    if (taken) {
      g_lift_warp = -1;
      if (value) {
        char *end = NULL;
        long id = strtol(value, &end, 16);
        if (end == value || *end != 0 || id < 0 || id > 0xfff) {
          fprintf(stderr, "--lift-warp wants a map16 value in hex, got '%s'\n", value);
          exit(1);
        }
        g_lift_warp = (int)id;
      }
      memmove(argv + i, argv + i + taken, (argc - i - taken) * sizeof(*argv));
      argc -= taken;
      break;
    }
  }

  for (int i = 0; i < argc; i++) {
    const char *value = NULL;
    int taken = 0;
    if (strncmp(argv[i], "--dump-room-objects=", 20) == 0) {
      value = argv[i] + 20;
      taken = 1;
    } else if (strcmp(argv[i], "--dump-room-objects") == 0) {
      if (i + 1 >= argc)
        Die("--dump-room-objects needs a room number");
      value = argv[i + 1];
      taken = 2;
    }
    if (taken) {
      g_dump_room_objects = (int)strtol(value, NULL, 10);
      memmove(argv + i, argv + i + taken, (argc - i - taken) * sizeof(*argv));
      argc -= taken;
      break;
    }
  }

  for (int i = 0; i < argc; i++) {
    const char *value = NULL;
    int taken = 0;
    if (strncmp(argv[i], "--entrance=", 11) == 0) {
      value = argv[i] + 11;
      taken = 1;
    } else if (strcmp(argv[i], "--entrance") == 0) {
      if (i + 1 >= argc)
        Die("--entrance needs a number");
      value = argv[i + 1];
      taken = 2;
    }
    if (taken) {
      char *end = NULL;
      long id = strtol(value, &end, 16);
      if (end == value || *end != 0 || id < 0 || id > 0xff) {
        fprintf(stderr, "--entrance wants an entrance number in hex, got '%s'\n", value);
        exit(1);
      }
      g_entrance = (int)id;
      memmove(argv + i, argv + i + taken, (argc - i - taken) * sizeof(*argv));
      argc -= taken;
      break;
    }
  }

  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "--list-liftables") == 0) {
      g_list_liftables = true;
      memmove(argv + i, argv + i + 1, (argc - i - 1) * sizeof(*argv));
      argc--;
      break;
    }
  }

  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "--dump-rooms") == 0) {
      g_dump_rooms = true;
      memmove(argv + i, argv + i + 1, (argc - i - 1) * sizeof(*argv));
      argc--;
      break;
    }
  }

  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "--list-sprites") == 0) {
      g_list_sprites = true;
      memmove(argv + i, argv + i + 1, (argc - i - 1) * sizeof(*argv));
      argc--;
      break;
    }
  }
  for (int i = 0; i < argc; i++) {
    const char *value = NULL;
    int taken = 0;
    if (strncmp(argv[i], "--goto=", 7) == 0) {
      value = argv[i] + 7;
      taken = 1;
    } else if (strcmp(argv[i], "--goto") == 0) {
      if (i + 1 >= argc)
        Die("--goto needs a coordinate");
      value = argv[i + 1];
      taken = 2;
    }
    if (taken) {
      // "x,y" is the light world and "d:x,y" the dark one. The screen falls out of the
      // coordinate but the world does not, so it has to be said.
      const char *cursor = value;
      char *end = NULL;
      long x, y;
      bool dark = false;
      if (cursor[0] == 'd' && cursor[1] == ':') {
        dark = true;
        cursor += 2;
      }
      x = strtol(cursor, &end, 10);
      if (end == cursor || *end != ',')
        goto goto_bad;
      cursor = end + 1;
      y = strtol(cursor, &end, 10);
      if (end == cursor || *end != 0 || x < 0 || x > 4095 || y < 0 || y > 4095) {
goto_bad:
        fprintf(stderr, "--goto wants [d:]x,y in world pixels, 0 to 4095, got '%s'\n", value);
        exit(1);
      }
      ClaimPositionOption("--goto");
      g_goto_x = (int)x;
      g_goto_y = (int)y;
      g_goto_dark = dark;
      memmove(argv + i, argv + i + taken, (argc - i - taken) * sizeof(*argv));
      argc -= taken;
      break;
    }
  }
  ParseConfigFile(config_file);
  LoadAssets();
  LoadLinkGraphics();

  ZeldaInitialize();


  g_zenv.ppu->extraLeftRight = UintMin(g_config.extended_aspect_ratio, kPpuExtraLeftRight);
  g_snes_width = (g_config.extended_aspect_ratio * 2 + 256);
  g_snes_height = (g_config.extend_y ? 240 : 224);


  // Delay actually setting those features in ram until any snapshots finish playing.
  g_wanted_zelda_features = g_config.features0;

  g_ppu_render_flags = g_config.new_renderer * kPpuRenderFlags_NewRenderer |
                       g_config.enhanced_mode7 * kPpuRenderFlags_4x4Mode7 |
                       g_config.extend_y * kPpuRenderFlags_Height240 |
                       g_config.no_sprite_limits * kPpuRenderFlags_NoSpriteLimits;
  ZeldaEnableMsu(g_config.enable_msu);
  ZeldaSetLanguage(g_config.language);

  if (g_config.fullscreen == 1)
    g_win_flags ^= SDL_WINDOW_FULLSCREEN_DESKTOP;
  else if (g_config.fullscreen == 2)
    g_win_flags ^= SDL_WINDOW_FULLSCREEN;

  // Window scale (1=100%, 2=200%, 3=300%, etc.)
  g_current_window_scale = (g_config.window_scale == 0) ? 2 : IntMin(g_config.window_scale, kMaxWindowScale);

  // audio_freq: Use common sampling rates (see user config file. values higher than 48000 are not supported.)
  if (g_config.audio_freq < 11025 || g_config.audio_freq > 48000)
    g_config.audio_freq = kDefaultFreq;

  // Currently, the SPC/DSP implementation only supports up to stereo.
  if (g_config.audio_channels < 1 || g_config.audio_channels > 2)
    g_config.audio_channels = kDefaultChannels;

  // audio_samples: power of 2
  if (g_config.audio_samples <= 0 || ((g_config.audio_samples & (g_config.audio_samples - 1)) != 0))
    g_config.audio_samples = kDefaultSamples;

  // set up SDL
  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) != 0) {
    printf("Failed to init SDL: %s\n", SDL_GetError());
    return 1;
  }

  bool custom_size  = g_config.window_width != 0 && g_config.window_height != 0;
  int window_width  = custom_size ? g_config.window_width  : g_current_window_scale * g_snes_width;
  int window_height = custom_size ? g_config.window_height : g_current_window_scale * g_snes_height;

  if (g_config.output_method == kOutputMethod_OpenGL ||
      g_config.output_method == kOutputMethod_OpenGL_ES) {
    g_win_flags |= SDL_WINDOW_OPENGL;
    OpenGLRenderer_Create(&g_renderer_funcs, (g_config.output_method == kOutputMethod_OpenGL_ES));
  } else {
    g_renderer_funcs = kSdlRendererFuncs;
  }

  SDL_Window* window = SDL_CreateWindow(kWindowTitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_width, window_height, g_win_flags);
  if(window == NULL) {
    printf("Failed to create window: %s\n", SDL_GetError());
    return 1;
  }
  g_window = window;
  SDL_SetWindowHitTest(window, HitTestCallback, NULL);

  if (!g_renderer_funcs.Initialize(window))
    return 1;

  SDL_AudioDeviceID device = 0;
  SDL_AudioSpec want = { 0 }, have;
  g_audio_mutex = SDL_CreateMutex();
  if (!g_audio_mutex) Die("No mutex");

  if (g_config.enable_audio) {
    want.freq = g_config.audio_freq;
    want.format = AUDIO_S16;
    want.channels = g_config.audio_channels;
    want.samples = g_config.audio_samples;
    want.callback = &AudioCallback;
    device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (device == 0) {
      printf("Failed to open audio device: %s\n", SDL_GetError());
      return 1;
    }
    g_audio_channels = have.channels;
    g_frames_per_block = (534 * have.freq) / 32000;
    g_audiobuffer = malloc(g_frames_per_block * have.channels * sizeof(int16));
  }

  if (argc >= 1 && !g_run_without_emu)
    LoadRom(argv[0]);

  // A spot carries the whole save, so with no savestate to start from it starts from a new file.
  if (DebugSpot_Pending() && g_load_save_path == NULL)
    g_skip_intro = true;

#if defined(_WIN32)
  _mkdir("saves");
#else
  mkdir("saves", 0755);
#endif

  ZeldaReadSram();

  if (g_start_muted)
    ZeldaToggleMusic();

  if (g_start_invincible) {
    g_cheat_invincible = true;
    printf("invincible on\n");
  }

  for (int i = 0; i < SDL_NumJoysticks(); i++)
    OpenOneGamepad(i);

  bool running = true;
  SDL_Event event;
  uint32 lastTick = SDL_GetTicks();
  uint32 curTick = 0;
  uint32 frameCtr = 0;
  bool audiopaused = true;

  if (g_config.autosave)
    HandleCommand(kKeys_Load + 0, true);

  while(running) {
    while(SDL_PollEvent(&event)) {
      switch(event.type) {
      case SDL_CONTROLLERDEVICEADDED:
        OpenOneGamepad(event.cdevice.which);
        break;
      case SDL_CONTROLLERAXISMOTION:
        HandleGamepadAxisInput(event.caxis.which, event.caxis.axis, event.caxis.value);
        break;
      case SDL_CONTROLLERBUTTONDOWN:
      case SDL_CONTROLLERBUTTONUP: {
        int b = RemapSdlButton(event.cbutton.button);
        if (b >= 0)
          HandleGamepadInput(b, event.type == SDL_CONTROLLERBUTTONDOWN);
        break;
      }
      case SDL_MOUSEWHEEL:
        if (SDL_GetModState() & KMOD_CTRL && event.wheel.y != 0)
          ChangeWindowScale(event.wheel.y > 0 ? 1 : -1);
        break;
      case SDL_MOUSEBUTTONDOWN:
        if (event.button.button == SDL_BUTTON_LEFT && event.button.state == SDL_PRESSED && event.button.clicks == 2) {
          if ((g_win_flags & SDL_WINDOW_FULLSCREEN_DESKTOP) == 0 && (g_win_flags & SDL_WINDOW_FULLSCREEN) == 0 && SDL_GetModState() & KMOD_SHIFT) {
            g_win_flags ^= SDL_WINDOW_BORDERLESS;
            SDL_SetWindowBordered(g_window, (g_win_flags & SDL_WINDOW_BORDERLESS) == 0);
          }
        }
        break;
      case SDL_KEYDOWN:
        HandleInput(event.key.keysym.sym, event.key.keysym.mod, true);
        break;
      case SDL_KEYUP:
        HandleInput(event.key.keysym.sym, event.key.keysym.mod, false);
        break;
      case SDL_QUIT:
        running = false;
        break;
      }
    }

    if (g_paused != audiopaused) {
      audiopaused = g_paused;
      if (device)
        SDL_PauseAudioDevice(device, audiopaused);
    }

    if (g_paused) {
      SDL_Delay(16);
      continue;
    }

    if (g_key_script_len)
      KeyScript_RunFrame();

    // Clear gamepad inputs when joypad directional inputs to avoid wonkiness
    int inputs = g_input1_state;
    if (g_input1_state & 0xf0)
      g_gamepad_buttons = 0;
    inputs |= g_gamepad_buttons;


    SDL_LockMutex(g_audio_mutex);
    bool is_replay = ZeldaRunFrame(inputs);

    // Before anything below can jump, so a jump's overlay always waits a frame.
    DebugGoto_Frame();

    // One frame in, the game has initialised itself, which is the first point where dropping
    // it straight into a game takes.
    if (g_load_save_path) {
      // A savestate carries the whole machine, so it replaces anything --skip-intro would set up.
      const char *path = g_load_save_path;
      g_load_save_path = NULL;
      g_skip_intro = false;
      if (!SaveLoadFile(kSaveLoad_Load, path))
        fprintf(stderr, "Unable to open save file '%s'\n", path);
    }

    if (g_skip_intro) {
      g_skip_intro = false;
      SkipIntroToNewGame();
    }

    // After the save has loaded, so it is not overwritten by one that has no flippers.
    if (g_give_flippers) {
      g_give_flippers = false;
      PatchCommand('f');
      printf("flippers given\n");
    }

    // Module 9 is the overworld. Anything else means the game is still loading, or is somewhere
    // the jump cannot reach, so give it ten seconds and then say so.
    if (g_screen_id >= 0 || g_goto_x >= 0 || g_warp_type >= 0 || g_list_sprites || g_list_liftables || g_lift_warp != -2 ||
        g_entrance >= 0 || g_dump_room_objects >= 0 || g_dump_rooms || DebugScene_Pending()) {
      if (main_module_index == 9 && !player_is_indoors) {
        if (DebugScene_Pending())
          DebugScene_Apply();
        if (g_goto_x >= 0)
          DebugGoto_JumpToPoint((uint16)g_goto_x, (uint16)g_goto_y, g_goto_dark);
        else if (g_screen_id >= 0)
          DebugGoto_JumpTo((uint8)g_screen_id);
        // The warp runs last, so it wins if a screen was named as well.
        if (g_list_sprites) {
          g_list_sprites = false;
          DebugLocate_ListAll(g_warp_type >= 0 ? g_warp_category : kLocateCategory_Any);
        }
        if (g_list_liftables) {
          g_list_liftables = false;
          DebugLocate_ListLiftables();
        }
        if (g_entrance >= 0) {
          which_entrance = (uint8)g_entrance;
          g_entrance = -1;
          // The falling entrance sequence loads the room and puts the player in it, which is
          // all a look inside needs.
          main_module_index = 0x11;
          submodule_index = 0;
          subsubmodule_index = 0;
        }
        if (g_lift_warp != -2) {
          int wanted = g_lift_warp;
          g_lift_warp = -2;
          DebugLocate_WarpToLiftable(wanted);
        }
        if (g_dump_room_objects >= 0) {
          int room = g_dump_room_objects;
          g_dump_room_objects = -1;
          DebugLocate_DumpRoomObjects(room);
          DebugLocate_DumpRoomMap(room);
        }
        if (g_dump_rooms) {
          g_dump_rooms = false;
          DebugLocate_DumpRoomHazards();
        }
        if (g_warp_type >= 0)
          DebugLocate_WarpToType((uint8)g_warp_type, g_warp_category);
        g_screen_id = -1;
        g_goto_x = -1;
        g_warp_type = -1;
      } else if (++g_screen_id_waited >= 600) {
        fprintf(stderr, "warp or jump: never reached the overworld, ignoring\n");
        DebugScene_Forget();
        g_screen_id = -1;
        g_goto_x = -1;
        g_warp_type = -1;
      }
    }

    DebugSpot_Frame();
    DebugScene_Frame();

    SDL_UnlockMutex(g_audio_mutex);

    DebugLog_Frame();


    frameCtr++;

    if (g_quit_after && frameCtr >= g_quit_after)
      running = false;

    if ((g_turbo ^ (is_replay & g_replay_turbo)) && (frameCtr & (g_turbo ? 0xf : 0x7f)) != 0) {
      continue;
    }

    DrawPpuFrameWithPerf();

    if (g_config.display_perf_title) {
      char title[60];
      snprintf(title, sizeof(title), "%s | FPS: %d", kWindowTitle, g_curr_fps);
      SDL_SetWindowTitle(g_window, title);
    }

    // if vsync isn't working, delay manually
    curTick = SDL_GetTicks();

    if (!g_config.disable_frame_delay) {
      static const uint8 delays[3] = { 17, 17, 16 }; // 60 fps
      lastTick += delays[frameCtr % 3];

      if (lastTick > curTick) {
        uint32 delta = lastTick - curTick;
        if (delta > 500) {
          lastTick = curTick - 500;
          delta = 500;
        }
//        printf("Sleeping %d\n", delta);
        SDL_Delay(delta);
      } else if (curTick - lastTick > 500) {
        lastTick = curTick;
      }
    }
  }
  if (g_config.autosave)
    HandleCommand(kKeys_Save + 0, true);

  // clean sdl
  if (g_config.enable_audio) {
    SDL_PauseAudioDevice(device, 1);
    SDL_CloseAudioDevice(device);
  }

  SDL_DestroyMutex(g_audio_mutex);
  free(g_audiobuffer);

  g_renderer_funcs.Destroy();

  SDL_DestroyWindow(window);
  DebugLog_Close();
  SDL_Quit();
  //SaveConfigFile();
  return 0;
}

static void RenderDigit(uint8 *dst, size_t pitch, int digit, uint32 color, bool big) {
  static const uint8 kFont[] = {
    0x1c, 0x36, 0x63, 0x63, 0x63, 0x63, 0x63, 0x63, 0x36, 0x1c,
    0x18, 0x1c, 0x1e, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7e,
    0x3e, 0x63, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x03, 0x63, 0x7f,
    0x3e, 0x63, 0x60, 0x60, 0x3c, 0x60, 0x60, 0x60, 0x63, 0x3e,
    0x30, 0x38, 0x3c, 0x36, 0x33, 0x7f, 0x30, 0x30, 0x30, 0x78,
    0x7f, 0x03, 0x03, 0x03, 0x3f, 0x60, 0x60, 0x60, 0x63, 0x3e,
    0x1c, 0x06, 0x03, 0x03, 0x3f, 0x63, 0x63, 0x63, 0x63, 0x3e,
    0x7f, 0x63, 0x60, 0x60, 0x30, 0x18, 0x0c, 0x0c, 0x0c, 0x0c,
    0x3e, 0x63, 0x63, 0x63, 0x3e, 0x63, 0x63, 0x63, 0x63, 0x3e,
    0x3e, 0x63, 0x63, 0x63, 0x7e, 0x60, 0x60, 0x60, 0x30, 0x1e,
  };
  const uint8 *p = kFont + digit * 10;
  if (!big) {
    for (int y = 0; y < 10; y++, dst += pitch) {
      int v = *p++;
      for (int x = 0; v; x++, v >>= 1) {
        if (v & 1)
          ((uint32 *)dst)[x] = color;
      }
    }
  } else {
    for (int y = 0; y < 10; y++, dst += pitch * 2) {
      int v = *p++;
      for (int x = 0; v; x++, v >>= 1) {
        if (v & 1) {
          ((uint32 *)dst)[x * 2 + 1] = ((uint32 *)dst)[x * 2] = color;
          ((uint32 *)(dst+pitch))[x * 2 + 1] = ((uint32 *)(dst + pitch))[x * 2] = color;
        }
      }
    }
  }
}

static void RenderNumber(uint8 *dst, size_t pitch, int n, bool big) {
  char buf[32], *s;
  int i;
  sprintf(buf, "%d", n);
  for (s = buf, i = 2 * 4; *s; s++, i += 8 * 4)
    RenderDigit(dst + ((pitch + i + 4) << big), pitch, *s - '0', 0x404040, big);
  for (s = buf, i = 2 * 4; *s; s++, i += 8 * 4)
    RenderDigit(dst + (i << big), pitch, *s - '0', 0xffffff, big);
}

static void HandleCommand_Locked(uint32 j, bool pressed);

static void HandleCommand(uint32 j, bool pressed) {
  if (j <= kKeys_Controls_Last) {
    static const uint8 kKbdRemap[] = { 0, 4, 5, 6, 7, 2, 3, 8, 0, 9, 1, 10, 11 };
    if (pressed)
      g_input1_state |= 1 << kKbdRemap[j];
    else
      g_input1_state &= ~(1 << kKbdRemap[j]);
    return;
  }

  if (j == kKeys_Turbo) {
    g_turbo = pressed;
    return;
  }

  if (j == kKeys_LogAnimation) {
    DebugLog_SetAnimationHeld(pressed);
    return;
  }

  // Everything that might access audio state
  // (like SaveLoad and Reset) must have the lock.
  SDL_LockMutex(g_audio_mutex);
  HandleCommand_Locked(j, pressed);
  SDL_UnlockMutex(g_audio_mutex);
}

void ZeldaApuLock() {
  SDL_LockMutex(g_audio_mutex);
}

void ZeldaApuUnlock() {
  SDL_UnlockMutex(g_audio_mutex);
}


// Shift+L. The overworld screen and the dark world flag come from the same byte, and the
// coordinates are Link's world position, which is what the goto and warp options take.
static void CopyLocationToClipboard(void) {
  char text[128];

  if (player_is_indoors) {
    snprintf(text, sizeof(text), "indoors, room %03X, coordinate (%d,%d)",
             dungeon_room_index, link_x_coord, link_y_coord);
  } else {
    uint8 screen = (uint8)overworld_screen_index;
    snprintf(text, sizeof(text), "%s world, screen %02X, coordinate (%d,%d)",
             (screen & 0x40) ? "dark" : "light", screen, link_x_coord, link_y_coord);
  }

  if (SDL_SetClipboardText(text) != 0) {
    fprintf(stderr, "Unable to set the clipboard: %s\n", SDL_GetError());
  }

  DebugLog_Message(text);
}

static void HandleCommand_Locked(uint32 j, bool pressed) {
  if (!pressed)
    return;
  if (j <= kKeys_Load_Last) {
    SaveLoadSlot(kSaveLoad_Load, j - kKeys_Load);
  } else if (j <= kKeys_Save_Last) {
    SaveLoadSlot(kSaveLoad_Save, j - kKeys_Save);
  } else if (j <= kKeys_Replay_Last) {
    SaveLoadSlot(kSaveLoad_Replay, j - kKeys_Replay);
  } else if (j <= kKeys_LoadRef_Last) {
    SaveLoadSlot(kSaveLoad_Load, 256 + j - kKeys_LoadRef);
  } else if (j <= kKeys_ReplayRef_Last) {
    SaveLoadSlot(kSaveLoad_Replay, 256 + j - kKeys_ReplayRef);
  } else {
    switch (j) {
    case kKeys_CheatLife: PatchCommand('w'); break;
    case kKeys_CheatEquipment: PatchCommand('W'); break;
    case kKeys_CheatKeys: PatchCommand('o'); break;
    case kKeys_CheatWalkThroughWalls: PatchCommand('E'); break;
    case kKeys_ClearKeyLog: PatchCommand('k'); break;
    case kKeys_StopReplay: PatchCommand('l'); break;
    case kKeys_Fullscreen:
      g_win_flags ^= SDL_WINDOW_FULLSCREEN_DESKTOP;
      SDL_SetWindowFullscreen(g_window, g_win_flags & SDL_WINDOW_FULLSCREEN_DESKTOP);
      g_cursor = !g_cursor;
      SDL_ShowCursor(g_cursor);
      break;
    case kKeys_Reset:
      ZeldaReset(true);
      break;
    case kKeys_Pause: g_paused = !g_paused; break;
    case kKeys_PauseDimmed:
      g_paused = !g_paused;
      // SDL_RenderPresent may not be called more than once per frame.
      // Seems to work on Windows still. Temporary measure until it's fixed.
#ifdef _WIN32
      if (g_paused) {
        SDL_SetRenderDrawBlendMode(g_renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 159);
        SDL_RenderFillRect(g_renderer, NULL);
        SDL_RenderPresent(g_renderer);
      }
#endif
      break;
    case kKeys_ReplayTurbo: g_replay_turbo = !g_replay_turbo; break;
    case kKeys_WindowBigger: ChangeWindowScale(1); break;
    case kKeys_WindowSmaller: ChangeWindowScale(-1); break;
    case kKeys_DisplayPerf: g_display_perf ^= 1; break;
    case kKeys_ToggleRenderer: g_ppu_render_flags ^= kPpuRenderFlags_NewRenderer; break;
    case kKeys_VolumeUp:
    case kKeys_VolumeDown: HandleVolumeAdjustment(j == kKeys_VolumeUp ? 1 : -1); break;
    case kKeys_LogTiles: DebugLog_ToggleTileLogging(); break;
    case kKeys_GotoScreen: DebugGoto_OpenPrompt(); break;
    case kKeys_ListSprites: DebugLocate_ListLive(); break;
    case kKeys_NoMusic:
      ZeldaToggleMusic();
      printf("music %s\n", g_cheat_no_music ? "off" : "on");
      break;
    case kKeys_CopyLocation: CopyLocationToClipboard(); break;
    case kKeys_Screenshot:
      g_want_screenshot = true;
      break;
    case kKeys_CheatInvincible:
      g_cheat_invincible = !g_cheat_invincible;
      printf("invincible %s\n", g_cheat_invincible ? "on" : "off");
      break;
    default: assert(0);
    }
  }
}

// Keycode of the key currently holding animation logging open.
static int g_anim_log_keycode;

static void HandleInput(int keyCode, int keyMod, bool pressed) {
  // The prompt eats key presses so the game does not also act on them. Releases still pass through,
  // otherwise a control held before the prompt opened would stay stuck down.
  if (pressed && DebugGoto_HandleKey(keyCode))
    return;

  // Match the release by keycode, so letting go of shift before the letter still stops logging.
  if (!pressed && g_anim_log_keycode != 0 && keyCode == g_anim_log_keycode) {
    g_anim_log_keycode = 0;
    DebugLog_SetAnimationHeld(false);
  }

  int j = FindCmdForSdlKey(keyCode, keyMod);
  if (j == kKeys_LogAnimation && pressed)
    g_anim_log_keycode = keyCode;
  if (j != 0)
    HandleCommand(j, pressed);
}

static void OpenOneGamepad(int i) {
  if (SDL_IsGameController(i)) {
    SDL_GameController *controller = SDL_GameControllerOpen(i);
    if (!controller)
      fprintf(stderr, "Could not open gamepad %d: %s\n", i, SDL_GetError());
  }
}

static int RemapSdlButton(int button) {
  switch (button) {
  case SDL_CONTROLLER_BUTTON_A: return kGamepadBtn_A;
  case SDL_CONTROLLER_BUTTON_B: return kGamepadBtn_B;
  case SDL_CONTROLLER_BUTTON_X: return kGamepadBtn_X;
  case SDL_CONTROLLER_BUTTON_Y: return kGamepadBtn_Y;
  case SDL_CONTROLLER_BUTTON_BACK: return kGamepadBtn_Back;
  case SDL_CONTROLLER_BUTTON_GUIDE: return kGamepadBtn_Guide;
  case SDL_CONTROLLER_BUTTON_START: return kGamepadBtn_Start;
  case SDL_CONTROLLER_BUTTON_LEFTSTICK: return kGamepadBtn_L3;
  case SDL_CONTROLLER_BUTTON_RIGHTSTICK: return kGamepadBtn_R3;
  case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: return kGamepadBtn_L1;
  case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return kGamepadBtn_R1;
  case SDL_CONTROLLER_BUTTON_DPAD_UP: return kGamepadBtn_DpadUp;
  case SDL_CONTROLLER_BUTTON_DPAD_DOWN: return kGamepadBtn_DpadDown;
  case SDL_CONTROLLER_BUTTON_DPAD_LEFT: return kGamepadBtn_DpadLeft;
  case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: return kGamepadBtn_DpadRight;
  default: return -1;
  }
}

static void HandleGamepadInput(int button, bool pressed) {
  if (!!(g_gamepad_modifiers & (1 << button)) == pressed)
    return;
  g_gamepad_modifiers ^= 1 << button;
  if (pressed)
    g_gamepad_last_cmd[button] = FindCmdForGamepadButton(button, g_gamepad_modifiers);
  if (g_gamepad_last_cmd[button] != 0)
    HandleCommand(g_gamepad_last_cmd[button], pressed);
}

static void HandleVolumeAdjustment(int volume_adjustment) {
#if SYSTEM_VOLUME_MIXER_AVAILABLE
  int current_volume = GetApplicationVolume();
  int new_volume = IntMin(IntMax(0, current_volume + volume_adjustment * 5), 100);
  SetApplicationVolume(new_volume);
  printf("[System Volume]=%i\n", new_volume);
#else
  g_sdl_audio_mixer_volume = IntMin(IntMax(0, g_sdl_audio_mixer_volume + volume_adjustment * (SDL_MIX_MAXVOLUME >> 4)), SDL_MIX_MAXVOLUME);
  printf("[SDL mixer volume]=%i\n", g_sdl_audio_mixer_volume);
#endif
}

// Approximates atan2(y, x) normalized to the [0,4) range
// with a maximum error of 0.1620 degrees
// normalized_atan(x) ~ (b x + x^2) / (1 + 2 b x + x^2)
static float ApproximateAtan2(float y, float x) {
  uint32 sign_mask = 0x80000000;
  float b = 0.596227f;
  // Extract the sign bits
  uint32 ux_s = sign_mask & *(uint32 *)&x;
  uint32 uy_s = sign_mask & *(uint32 *)&y;
  // Determine the quadrant offset
  float q = (float)((~ux_s & uy_s) >> 29 | ux_s >> 30);
  // Calculate the arctangent in the first quadrant
  float bxy_a = b * x * y;
  if (bxy_a < 0.0f) bxy_a = -bxy_a;  // avoid fabs
  float num = bxy_a + y * y;
  float atan_1q = num / (x * x + bxy_a + num + 0.000001f);
  // Translate it to the proper quadrant
  uint32_t uatan_2q = (ux_s ^ uy_s) | *(uint32 *)&atan_1q;
  return q + *(float *)&uatan_2q;
}

static void HandleGamepadAxisInput(int gamepad_id, int axis, int value) {
  static int last_gamepad_id, last_x, last_y;
  if (axis == SDL_CONTROLLER_AXIS_LEFTX || axis == SDL_CONTROLLER_AXIS_LEFTY) {
    // ignore other gamepads unless they have a big input
    if (last_gamepad_id != gamepad_id) {
      if (value > -16000 && value < 16000)
        return;
      last_gamepad_id = gamepad_id;
      last_x = last_y = 0;
    }
    *(axis == SDL_CONTROLLER_AXIS_LEFTX ? &last_x : &last_y) = value;
    int buttons = 0;
    if (last_x * last_x + last_y * last_y >= 10000 * 10000) {
      // in the non deadzone part, divide the circle into eight 45 degree
      // segments rotated by 22.5 degrees that control which direction to move.
      // todo: do this without floats?
      static const uint8 kSegmentToButtons[8] = {
        1 << 4,           // 0 = up
        1 << 4 | 1 << 7,  // 1 = up, right
        1 << 7,           // 2 = right
        1 << 7 | 1 << 5,  // 3 = right, down
        1 << 5,           // 4 = down
        1 << 5 | 1 << 6,  // 5 = down, left
        1 << 6,           // 6 = left
        1 << 6 | 1 << 4,  // 7 = left, up
      };
      uint8 angle = (uint8)(int)(ApproximateAtan2(last_y, last_x) * 64.0f + 0.5f);
      buttons = kSegmentToButtons[(uint8)(angle + 16 + 64) >> 5];
    }
    g_gamepad_buttons = buttons;
  } else if ((axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT || axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)) {
    if (value < 12000 || value >= 16000)  // hysteresis
      HandleGamepadInput(axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT ? kGamepadBtn_L2 : kGamepadBtn_R2, value >= 12000);
  }
}

static bool LoadRom(const char *filename) {
  size_t length = 0;
  uint8 *file = ReadWholeFile(filename, &length);
  if(!file) Die("Failed to read file");
  bool result = EmuInitialize(file, length);
  free(file);
  return result;
}

static bool ParseLinkGraphics(uint8 *file, size_t length) {
  if (length < 27 || memcmp(file, "ZSPR", 4) != 0)
    return false;
  uint32 pixel_offs = DWORD(file[9]);
  uint32 pixel_length = WORD(file[13]);
  uint32 palette_offs = DWORD(file[15]);
  uint32 palette_length = WORD(file[19]);
  if ((uint64)pixel_offs + pixel_length > length ||
      (uint64)palette_offs + palette_length > length ||
      pixel_length != 0x7000)
    return false;
  if (kPalette_ArmorAndGloves_SIZE != 150 || kLinkGraphics_SIZE != 0x7000)
    Die("ParseLinkGraphics: Invalid asset sizes");
  memcpy(kLinkGraphics, file + pixel_offs, 0x7000);
  if (palette_length >= 120)
    memcpy(kPalette_ArmorAndGloves, file + palette_offs, 120);
  if (palette_length >= 124)
    memcpy(kGlovesColor, file + palette_offs + 120, 4);
  return true;
}

static void LoadLinkGraphics() {
  if (g_config.link_graphics) {
    fprintf(stderr, "Loading Link Graphics: %s\n", g_config.link_graphics);
    size_t length = 0;
    uint8 *file = ReadWholeFile(g_config.link_graphics, &length);
    if (file == NULL || !ParseLinkGraphics(file, length))
      Die("Unable to load file");
    free(file);
  }
}


const uint8 *g_asset_ptrs[kNumberOfAssets];
uint32 g_asset_sizes[kNumberOfAssets];

static void LoadAssets() {
  size_t length = 0;
  uint8 *data = ReadWholeFile("zelda3_assets.dat", &length);
  if (!data) {
    size_t bps_length, bps_src_length;
    uint8 *bps, *bps_src;
    bps = ReadWholeFile("zelda3_assets.bps", &bps_length);
    if (!bps)
      Die("Failed to read zelda3_assets.dat. Please see the README for information about how you get this file.");
    bps_src = ReadWholeFile("zelda3.sfc", &bps_src_length);
    if (!bps_src)
      Die("Missing file: zelda3.sfc");
    data = ApplyBps(bps_src, bps_src_length, bps, bps_length, &length);
    if (!data)
      Die("Unable to apply zelda3_assets.bps. Please make sure you got the right version of 'zelda3.sfc'");
  }

  static const char kAssetsSig[] = { kAssets_Sig };

  if (length < 16 + 32 + 32 + 8 + kNumberOfAssets * 4 ||
      memcmp(data, kAssetsSig, 48) != 0 ||
      *(uint32*)(data + 80) != kNumberOfAssets)
    Die("Invalid assets file");

  uint32 offset = 88 + kNumberOfAssets * 4 + *(uint32 *)(data + 84);

  for (size_t i = 0; i < kNumberOfAssets; i++) {
    uint32 size = *(uint32 *)(data + 88 + i * 4);
    offset = (offset + 3) & ~3;
    if ((uint64)offset + size > length)
      Die("Assets file corruption");
    g_asset_sizes[i] = size;
    g_asset_ptrs[i] = data + offset;
    offset += size;
  }

  if (g_config.features0 & kFeatures0_DimFlashes) { // patch dungeon floor palettes
    kPalette_DungBgMain[0x484] = 0x70;
    kPalette_DungBgMain[0x485] = 0x95;
    kPalette_DungBgMain[0x486] = 0x57;
  }
}

// Go some steps up and find zelda3.ini
static void SwitchDirectory() {
  char buf[4096];
  if (!getcwd(buf, sizeof(buf) - 32))
    return;
  size_t pos = strlen(buf);

  for (int step = 0; pos != 0 && step < 3; step++) {
    memcpy(buf + pos, "/zelda3.ini", 12);
    FILE *f = fopen(buf, "rb");
    if (f) {
      fclose(f);
      buf[pos] = 0;
      if (step != 0) {
        printf("Found zelda3.ini in %s\n", buf);
        int err = chdir(buf);
        (void)err;
      }
      return;
    }
    pos--;
    while (pos != 0 && buf[pos] != '/' && buf[pos] != '\\')
      pos--;
  }
}

MemBlk FindInAssetArray(int asset, int idx) {
  return FindIndexInMemblk((MemBlk) { g_asset_ptrs[asset], g_asset_sizes[asset] }, idx);
}
