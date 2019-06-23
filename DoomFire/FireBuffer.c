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

#define SLACK 16

static inline uint32_t xorshift32(uint32_t x) {
    /* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return x;
}

uint32_t* createFireBuffer(uint32_t width, uint32_t height) {
    uint32_t allocSize = sizeof(uint32_t) * (width * height + 2 * SLACK);
    uint32_t* buffer = malloc(allocSize);
    memset(buffer, 0, allocSize);
    for (int32_t i = 0; i < SLACK + width; ++i) {
        buffer[i] = MAX_LIT;
    }
    return buffer + SLACK;
}

void destroyFireBuffer(uint32_t* restrict buffer) {
    free(buffer - SLACK);
}

void updateFireBuffer(uint32_t* restrict buffer, uint32_t width, uint32_t height) {
    uint32_t rnd = (rand() << 16) ^ (rand() & 0xffffu);
    for (uint32_t y = 0; y < height - 1; ++y) {
        bool rowLit = false;
        for (uint32_t x = 0; x < width; ++x) {
            uint32_t src = y * width + x;
            uint32_t pixel = buffer[src];

            // NOTE(a-square): most pixels aren't going to be lit
            // so an extra branch is probably worth having an early stopping
            // criterion
            if (pixel != 0) {
                uint32_t dst = src + width + 1 - (rnd & 3);
                buffer[dst] = pixel - (rnd & 1);
                rnd = xorshift32(rnd);
                rowLit = true;
            }
        }
        if (!rowLit) {
            break;
        }
    }
}
