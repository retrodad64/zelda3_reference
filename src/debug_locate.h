#ifndef ZELDA3_DEBUG_LOCATE_H_
#define ZELDA3_DEBUG_LOCATE_H_

#include "types.h"

enum {
    kLocateCategory_Any,
    kLocateCategory_Enemy,
    kLocateCategory_Npc,
    kLocateCategory_Boss,
};

// Reads the overworld sprite tables, so it reports sprites without loading any screen.
void DebugLocate_ListScreen(uint8 screen);
void DebugLocate_ListAll(int category);

// Every liftable map16 cell in the loaded overworld area, with its world coordinate.
void DebugLocate_ListLiftables(void);

// Stands Link beside something he can pick up, facing it. Pass a map16 value to ask for
// one kind, or -1 for the first one he can actually lift.
bool DebugLocate_WarpToLiftable(int wanted);

// Prints what a room is built from, which is the list of drawing routines it needs.
void DebugLocate_DumpRoomObjects(int room);

// The assembled room as map16 cells, for checking a port of the drawing against.
void DebugLocate_DumpRoomMap(int room);

// Reports the sprite slots the game has actually spawned right now.
void DebugLocate_ListLive(void);

// Finds the first sprite of this type and puts Link on a safe tile beside it.
bool DebugLocate_WarpToType(uint8 type, int category);

const char *DebugLocate_CategoryName(int category);
int DebugLocate_CategoryOfType(uint8 type);

// Walks every dungeon room and reports the hazardous tiles its objects draw.
void DebugLocate_DumpRoomHazards(void);

#endif  // ZELDA3_DEBUG_LOCATE_H_
