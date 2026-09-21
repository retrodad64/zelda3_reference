#include "png_writer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Deflate stored blocks carry at most this many bytes each.
enum {
    kStoredBlockMax = 65535,
    kBytesPerPixel = 3,
};

static uint32 Crc32(const uint8 *data, size_t length, uint32 crc) {
    static uint32 table[256];
    static bool table_built;

    if (!table_built) {
        for (uint32 i = 0; i < 256; i++) {
            uint32 c = i;

            for (int k = 0; k < 8; k++) {
                c = (c & 1) ? 0xedb88320u ^ (c >> 1) : c >> 1;
            }

            table[i] = c;
        }

        table_built = true;
    }

    crc = ~crc;

    for (size_t i = 0; i < length; i++) {
        crc = table[(crc ^ data[i]) & 0xff] ^ (crc >> 8);
    }

    return ~crc;
}

static uint32 Adler32(const uint8 *data, size_t length) {
    uint32 a = 1;
    uint32 b = 0;

    for (size_t i = 0; i < length; i++) {
        a = (a + data[i]) % 65521;
        b = (b + a) % 65521;
    }

    return (b << 16) | a;
}

static void PutBigEndian32(uint8 *dst, uint32 value) {
    dst[0] = (uint8)(value >> 24);
    dst[1] = (uint8)(value >> 16);
    dst[2] = (uint8)(value >> 8);
    dst[3] = (uint8)value;
}

static bool WriteChunk(FILE *f, const char *type, const uint8 *data, size_t length) {
    uint8 header[8];
    uint8 crc_bytes[4];
    uint32 crc;

    PutBigEndian32(header, (uint32)length);
    memcpy(header + 4, type, 4);

    crc = Crc32(header + 4, 4, 0);
    crc = Crc32(data, length, crc);
    PutBigEndian32(crc_bytes, crc);

    return fwrite(header, 1, 8, f) == 8 &&
           (length == 0 || fwrite(data, 1, length, f) == length) &&
           fwrite(crc_bytes, 1, 4, f) == 4;
}

// Raw scanlines with a leading filter byte, wrapped in a zlib stream of stored deflate blocks.
// No compression keeps this self contained, and these are debug dumps where size does not matter.
static uint8 *BuildZlibStream(int width, int height, const uint8 *pixels, size_t *out_size) {
    size_t stride = (size_t)width * kBytesPerPixel;
    size_t raw_size = (stride + 1) * (size_t)height;
    size_t block_count = (raw_size + kStoredBlockMax - 1) / kStoredBlockMax;
    size_t stream_size = 2 + raw_size + block_count * 5 + 4;
    uint8 *raw = (uint8 *)malloc(raw_size);
    uint8 *stream = (uint8 *)malloc(stream_size);
    uint8 *dst;
    size_t offset;

    if (raw == NULL || stream == NULL) {
        free(raw);
        free(stream);
        return NULL;
    }

    for (int y = 0; y < height; y++) {
        raw[(stride + 1) * (size_t)y] = 0;
        memcpy(raw + (stride + 1) * (size_t)y + 1, pixels + stride * (size_t)y, stride);
    }

    dst = stream;
    *dst++ = 0x78;
    *dst++ = 0x01;

    for (offset = 0; offset < raw_size; offset += kStoredBlockMax) {
        size_t chunk = raw_size - offset;
        bool final;

        if (chunk > kStoredBlockMax) {
            chunk = kStoredBlockMax;
        }

        final = (offset + chunk >= raw_size);
        *dst++ = final ? 1 : 0;
        *dst++ = (uint8)chunk;
        *dst++ = (uint8)(chunk >> 8);
        *dst++ = (uint8)~(uint8)chunk;
        *dst++ = (uint8)~(uint8)(chunk >> 8);
        memcpy(dst, raw + offset, chunk);
        dst += chunk;
    }

    PutBigEndian32(dst, Adler32(raw, raw_size));
    dst += 4;

    free(raw);
    *out_size = (size_t)(dst - stream);
    return stream;
}

bool WritePng(const char *path, int width, int height, const uint8 *pixels) {
    static const uint8 kSignature[8] = { 0x89, 'P', 'N', 'G', '\r', '\n', 0x1a, '\n' };
    uint8 ihdr[13];
    uint8 *stream;
    size_t stream_size = 0;
    FILE *f;
    bool ok;

    if (width <= 0 || height <= 0) {
        return false;
    }

    stream = BuildZlibStream(width, height, pixels, &stream_size);

    if (stream == NULL) {
        return false;
    }

    f = fopen(path, "wb");

    if (f == NULL) {
        free(stream);
        return false;
    }

    PutBigEndian32(ihdr, (uint32)width);
    PutBigEndian32(ihdr + 4, (uint32)height);
    ihdr[8] = 8;
    ihdr[9] = 2;
    ihdr[10] = 0;
    ihdr[11] = 0;
    ihdr[12] = 0;

    ok = fwrite(kSignature, 1, 8, f) == 8;
    ok = ok && WriteChunk(f, "IHDR", ihdr, sizeof(ihdr));
    ok = ok && WriteChunk(f, "IDAT", stream, stream_size);
    ok = ok && WriteChunk(f, "IEND", NULL, 0);

    fclose(f);
    free(stream);
    return ok;
}
