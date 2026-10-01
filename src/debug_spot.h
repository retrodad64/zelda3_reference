#ifndef ZELDA3_DEBUG_SPOT_H_
#define ZELDA3_DEBUG_SPOT_H_

// Spots from the pug hero demo, loaded into the real game.
//
// F2 in the demo remembers where Link is standing, in a small text file. After the demo's own
// lines it writes the same place and his whole kit and story in the cartridge's terms: the item
// bytes, each room's save_dung_info word, the overworld's event bytes, and Link's position the
// way link_x_coord and link_y_coord keep it. This reads that file and puts Link there, as he
// was. See spots.md.

#include "types.h"

// Reads a spot. False, and a line on stderr saying why, when it cannot be used.
bool DebugSpot_Load(const char *path);

// True from a load until Link has been put down.
bool DebugSpot_Pending(void);

// Call once a frame, after the frame has run. Takes Link there a step at a time: out of the
// room he starts in, the kit and story set on the overworld, then to the spot, through its
// entrance and down into its room when the spot is indoors.
void DebugSpot_Frame(void);

#endif  // ZELDA3_DEBUG_SPOT_H_
