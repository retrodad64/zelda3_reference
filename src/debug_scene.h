#ifndef ZELDA3_DEBUG_SCENE_H_
#define ZELDA3_DEBUG_SCENE_H_

// Scenes from the pug hero demo's entity sandbox, rebuilt in the real game.
//
// The sandbox saves a place it has built beside its map as a .scene file: the ground as map16
// cells, the things standing in it as map16 cells and sprite numbers, and Link's kit. This
// reads one and puts the same place down on an overworld area, so a thing can be watched doing
// what it does here and there side by side. See scenes.md.

#include "types.h"

// Reads a scene. False, and a line on stderr saying why, when it cannot be used.
bool DebugScene_Load(const char *path);

// True once a scene is loaded and waiting to be put down.
bool DebugScene_Pending(void);

// Drops a loaded scene that never got the chance to be put down.
void DebugScene_Forget(void);

// Call once a frame. Finishes the parts of putting a scene down that wait on the screen.
void DebugScene_Frame(void);

// Puts the loaded scene down. Call on the overworld; it loads the area it builds on itself.
void DebugScene_Apply(void);

// Sets one thing in Link's kit by the name the pug demo gives it: an item's byte, a pendant or
// a crystal. False for a name it does not know. Pendants and crystals are only ever added, so
// clear link_which_pendants and link_has_crystals first to set them from nothing.
bool DebugScene_SetItem(const char *key, int level);

// Draws the sword, the shield and the mail again a few frames from now, from their levels,
// once whatever screen is going up has its own graphics in. Needs DebugScene_Frame each frame.
void DebugScene_ReloadKitGraphicsSoon(void);

#endif  // ZELDA3_DEBUG_SCENE_H_
