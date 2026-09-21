#ifndef ZELDA3_PNG_WRITER_H_
#define ZELDA3_PNG_WRITER_H_

#include "types.h"

// Writes a 24 bit RGB image. pixels holds width * height * 3 bytes, row major, no padding.
bool WritePng(const char *path, int width, int height, const uint8 *pixels);

#endif  // ZELDA3_PNG_WRITER_H_
