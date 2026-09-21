#ifndef ZELDA3_DEBUG_IMAGE_H_
#define ZELDA3_DEBUG_IMAGE_H_

#include "types.h"

// Writes the loaded character tiles and the BG tilemaps built from them.
void DebugImage_WriteTileDump(void);

// One cell per frame while the animation key is held, written out as a sheet on release.
void DebugImage_BeginAnimation(void);
void DebugImage_CaptureAnimationFrame(void);
void DebugImage_EndAnimation(void);

#endif  // ZELDA3_DEBUG_IMAGE_H_
