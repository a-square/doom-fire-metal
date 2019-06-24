//
//  FireBuffer.c
//  DoomFire
//
//  Created by Алексей Аверченко on 23/06/2019.
//  Copyright © 2019 a-square. All rights reserved.
//

#include "FireBuffer.h"
#include "ShaderTypes.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define SLACK 32

static inline uint32_t xorshift32(uint32_t x) {
    /* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return x;
}

int32_t* createFireBuffer(uint32_t width, uint32_t height) {
    uint32_t allocSize = sizeof(uint32_t) * (width * height + 2 * SLACK);
    int32_t* buffer = malloc(allocSize);
    memset(buffer, 0, allocSize);
    for (int32_t i = 0; i < SLACK + width; ++i) {
        buffer[i] = MAX_LIT;
    }
    return buffer + SLACK;
}

void destroyFireBuffer(int32_t* restrict buffer) {
    free(buffer - SLACK);
}

void updateFireBuffer(int32_t* restrict buffer, uint32_t width, uint32_t height) {
    uint32_t rnd = (rand() << 16) ^ (rand() & 0xffffu);
    uint32_t effectiveHeight = height - (height >> 2);
    for (uint32_t y = 0; y < effectiveHeight; ++y) {
        for (uint32_t x = 0; x < width; x += 16) {
            rnd = xorshift32(rnd);
            uint32_t rndRem = rnd;
            uint32_t base = y * width + x;
            for (uint32_t i = 0; i < 16; i += 4) {
                uint32_t src = base + i;

                int32_t p0 = buffer[src];
                int32_t p1 = buffer[src + 1];
                int32_t p2 = buffer[src + 2];
                int32_t p3 = buffer[src + 3];

                uint32_t dstBase = src + width + 2;

                uint32_t rem0 = rndRem;
                uint32_t rem1 = rndRem >> 2;
                uint32_t rem2 = rndRem >> 4;
                uint32_t rem3 = rndRem >> 6;

                uint32_t dst0 = dstBase - (rem0 & 3);
                uint32_t dst1 = dstBase + 1 - (rem1 & 3);
                uint32_t dst2 = dstBase + 2 - (rem2 & 3);
                uint32_t dst3 = dstBase + 3 - (rem3 & 3);

                buffer[dst0] = p0 - (rem0 & 1);
                buffer[dst1] = p1 - (rem1 & 1);
                buffer[dst2] = p2 - (rem2 & 1);
                buffer[dst3] = p3 - (rem3 & 1);

                rndRem >>= 8;
            }
        }
    }
}
