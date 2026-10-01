#ifndef ZELDA3_DEBUG_GOTO_H_
#define ZELDA3_DEBUG_GOTO_H_

#include "types.h"

// Same jump the prompt performs, for callers that already know the screen.
void DebugGoto_JumpTo(uint8 screen);

// Lands on an exact world pixel instead of the middle of a screen.
void DebugGoto_JumpToPoint(uint16 x, uint16 y, bool dark);

// Loads a screen and puts the top left of Link's sprite at a world pixel, which is the
// position the game itself keeps. The screen is used as given, so name a big area by its head.
void DebugGoto_JumpToSprite(uint8 screen, uint16 sprite_x, uint16 sprite_y);

// Once a frame, after the game's own. Finishes a jump made on an earlier frame.
void DebugGoto_Frame(void);

void DebugGoto_OpenPrompt(void);
bool DebugGoto_IsPromptOpen(void);

// Returns true when the key belonged to the prompt and must not reach the game.
bool DebugGoto_HandleKey(int sdl_keycode);

void DebugGoto_DrawOverlay(uint8 *pixel_buffer, int pitch, int render_scale);

#endif  // ZELDA3_DEBUG_GOTO_H_
