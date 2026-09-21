#ifndef ZELDA3_DEBUG_LOG_H_
#define ZELDA3_DEBUG_LOG_H_

#include "types.h"

// Tile logging is a toggle, animation logging runs while its key is held.
void DebugLog_ToggleTileLogging(void);
void DebugLog_SetAnimationHeld(bool held);

// Called once per frame after the game has stepped.
void DebugLog_Frame(void);

// Writes one line to stdout and to the log file, for callers that format their own text.
void DebugLog_Message(const char *text);

void DebugLog_Close(void);

#endif  // ZELDA3_DEBUG_LOG_H_
