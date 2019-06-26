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

enum {
    SLACK = 128,
};

static inline u32 rand32() {
    return (rand() << 16) ^ (rand() & 0xffffu);
}

static inline u64 rand64() {
    return ((u64)rand32() << 32) ^ (u64)rand32();
}

static inline u32 xorshift32(u32 x) {
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return x;
}

static inline u64 xorshift64(u64 x) {
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    return x;
}


i32* createFireBuffer(u32 width, u32 height) {
    u32 allocSize = sizeof(u32) * (width * height + 2 * SLACK);
    i32* buffer = malloc(allocSize);
    memset(buffer, 0, allocSize);
    for (i32 i = 0; i < SLACK + width; ++i) {
        buffer[i] = MAX_LIT;
    }
    return buffer + SLACK;
}

void destroyFireBuffer(i32* buffer) {
    free(buffer - SLACK);
}

void updateFireBufferUnrolled(i32* restrict buffer, u32 width, u32 height) {
    u64 rnd1 = rand64();
    u32 rnd2 = rand32();

    // NOTE(a-square): this is NOT how the original Doom fire was spread,
    // because we're iterating in the hot-to-cold order whereas Doom fire
    // iterated in the cold-to-hot order
    //
    // My wife thinks this is more fire-like
    for (u32 y = 0; y < height - 1; ++y) {
        for (u32 x = 0; x < width; x += 32) {
            rnd1 = xorshift64(rnd1);
            rnd2 = xorshift32(rnd2);

            u64 rem1 = rnd1;
            u32 rem2 = rnd2;

            u32 base = y * width + x;
            for (u32 i = 0; i < 32; i += 4) {
                u32 src = base + i;

                i32 p0 = buffer[src];
                i32 p1 = buffer[src + 1];
                i32 p2 = buffer[src + 2];
                i32 p3 = buffer[src + 3];

                u32 dstBase = src + width + 2;

                u32 rem1_32 = (u32)rem1;
                u32 dst0 = dstBase - (rem1_32 & 3);
                u32 dst1 = dstBase + 1 - ((rem1_32 >> 2) & 3);
                u32 dst2 = dstBase + 2 - ((rem1_32 >> 4) & 3);
                u32 dst3 = dstBase + 3 - ((rem1_32 >> 6) & 3);

                buffer[dst0] = p0 - (rem2 & 1);
                buffer[dst1] = p1 - ((rem2 >> 1) & 1);
                buffer[dst2] = p2 - ((rem2 >> 2) & 1);
                buffer[dst3] = p3 - ((rem2 >> 3) & 1);

                rem1 >>= 8;
                rem2 >>= 4;
            }
        }
    }
}
