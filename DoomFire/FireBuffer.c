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
    i32* buffer = NULL;
    posix_memalign((void**)&buffer, 64, allocSize);
    memset(buffer, 0, allocSize);
    for (i32 i = 0; i < SLACK + width; ++i) {
        buffer[i] = MAX_LIT;
    }
    return buffer + SLACK;
}

void destroyFireBuffer(i32* buffer) {
    free(buffer - SLACK);
}

void updateFireBufferNonNegPixels32(i32* restrict buffer, u32 width, u32 height) {
    u32 rnd = rand32();

    // NOTE(a-square): this is NOT how the original Doom fire was spread,
    // because we're iterating in the hot-to-cold order whereas Doom fire
    // iterated in the cold-to-hot order
    //
    // My wife thinks this is more fire-like
    for (u32 y = 0; y < height - 1; ++y) {
        for (u32 x = 0; x < width; ++x) {
            u32 src = y * width + x;
            u32 dst = src + width + 2 - (rnd & 3);
            i32 pixel = buffer[src];
            if (pixel > 0) {
                buffer[dst] = pixel - ((rnd >> 2) & 1);
                rnd = xorshift32(rnd);
            }
        }
    }
}

void updateFireBufferNoRem(i32* restrict buffer, u32 width, u32 height) {
    u32 rnd = rand32();

    // NOTE(a-square): this is NOT how the original Doom fire was spread,
    // because we're iterating in the hot-to-cold order whereas Doom fire
    // iterated in the cold-to-hot order
    //
    // My wife thinks this is more fire-like
    for (u32 y = 0; y < height - 1; ++y) {
        for (u32 x = 0; x < width; ++x) {
            u32 src = y * width + x;
            u32 dst = src + width + 2 - (rnd & 3);
            buffer[dst] = buffer[src] - ((rnd >> 2) & 1);
            rnd = xorshift32(rnd);
        }
    }
}

void updateFireBufferNoRemUnrolled(i32* restrict buffer, u32 width, u32 height) {
    u32 rnd = rand32();

    // NOTE(a-square): this is NOT how the original Doom fire was spread,
    // because we're iterating in the hot-to-cold order whereas Doom fire
    // iterated in the cold-to-hot order
    //
    // My wife thinks this is more fire-like
    for (u32 y = 0; y < height - 1; ++y) {
        for (u32 x = 0; x < width; x += 10) {
            u32 base = y * width + x;
            u32 dstBase = base + width + 2;

            i32 p0 = buffer[base];
            i32 p1 = buffer[base + 1];
            i32 p2 = buffer[base + 2];
            i32 p3 = buffer[base + 3];
            i32 p4 = buffer[base + 4];
            i32 p5 = buffer[base + 5];
            i32 p6 = buffer[base + 6];
            i32 p7 = buffer[base + 7];
            i32 p8 = buffer[base + 8];
            i32 p9 = buffer[base + 9];

            buffer[dstBase - (rnd & 3)] = p0 - ((rnd >> 2) & 1);
            buffer[dstBase + 1 - ((rnd >> 3) & 3)] = p1 - ((rnd >> 5) & 1);
            buffer[dstBase + 2 - ((rnd >> 6) & 3)] = p2 - ((rnd >> 8) & 1);
            buffer[dstBase + 3 - ((rnd >> 9) & 3)] = p3 - ((rnd >> 11) & 1);
            buffer[dstBase + 4 - ((rnd >> 12) & 3)] = p4 - ((rnd >> 14) & 1);
            buffer[dstBase + 5 - ((rnd >> 15) & 3)] = p5 - ((rnd >> 17) & 1);
            buffer[dstBase + 6 - ((rnd >> 18) & 3)] = p6 - ((rnd >> 20) & 1);
            buffer[dstBase + 7 - ((rnd >> 21) & 3)] = p7 - ((rnd >> 23) & 1);
            buffer[dstBase + 8 - ((rnd >> 24) & 3)] = p8 - ((rnd >> 26) & 1);
            buffer[dstBase + 9 - ((rnd >> 27) & 3)] = p9 - ((rnd >> 29) & 1);
            rnd = xorshift32(rnd);
        }
    }
}

void updateFireBufferNoRemUnrolled2(i32* restrict buffer, u32 width, u32 height) {
    buffer = __builtin_assume_aligned(buffer, 64);
    u64 rnd = rand64();

    // NOTE(a-square): this is NOT how the original Doom fire was spread,
    // because we're iterating in the hot-to-cold order whereas Doom fire
    // iterated in the cold-to-hot order
    //
    // My wife thinks this is more fire-like
    u64 src = 0;
    u64 dst = width + 2;
    for (u32 i = 0; i < ((height - 1) * width + 15) / 16; ++i) {
        // prefetch some cache lines ahead of time,
        // the write position is further one line ahead because
        // write positions are perturbed
        __builtin_prefetch(&buffer[src + 4 * 16], 0, 3);
        __builtin_prefetch(&buffer[dst + 5 * 16], 1, 3);

        // read the entire cache line at once
        // NOTE(a-square): the assembly code shows there're enough registers for this
        i32 p0 = buffer[src++];
        i32 p1 = buffer[src++];
        i32 p2 = buffer[src++];
        i32 p3 = buffer[src++];
        i32 p4 = buffer[src++];
        i32 p5 = buffer[src++];
        i32 p6 = buffer[src++];
        i32 p7 = buffer[src++];
        i32 p8 = buffer[src++];
        i32 p9 = buffer[src++];
        i32 pa = buffer[src++];
        i32 pb = buffer[src++];
        i32 pc = buffer[src++];
        i32 pd = buffer[src++];
        i32 pe = buffer[src++];
        i32 pf = buffer[src++];

        buffer[dst++ - (rnd & 3)] = p0 - ((rnd >> 2) & 1);
        buffer[dst++ - ((rnd >> 3) & 3)] = p1 - ((rnd >> 5) & 1);
        buffer[dst++ - ((rnd >> 6) & 3)] = p2 - ((rnd >> 8) & 1);
        buffer[dst++ - ((rnd >> 9) & 3)] = p3 - ((rnd >> 11) & 1);
        buffer[dst++ - ((rnd >> 12) & 3)] = p4 - ((rnd >> 14) & 1);
        buffer[dst++ - ((rnd >> 15) & 3)] = p5 - ((rnd >> 17) & 1);
        buffer[dst++ - ((rnd >> 18) & 3)] = p6 - ((rnd >> 20) & 1);
        buffer[dst++ - ((rnd >> 21) & 3)] = p7 - ((rnd >> 23) & 1);
        buffer[dst++ - ((rnd >> 24) & 3)] = p8 - ((rnd >> 26) & 1);
        buffer[dst++ - ((rnd >> 27) & 3)] = p9 - ((rnd >> 29) & 1);
        buffer[dst++ - ((rnd >> 30) & 3)] = pa - ((rnd >> 32) & 1);
        buffer[dst++ - ((rnd >> 33) & 3)] = pb - ((rnd >> 35) & 1);
        buffer[dst++ - ((rnd >> 36) & 3)] = pc - ((rnd >> 38) & 1);
        buffer[dst++ - ((rnd >> 39) & 3)] = pd - ((rnd >> 41) & 1);
        buffer[dst++ - ((rnd >> 42) & 3)] = pe - ((rnd >> 44) & 1);
        buffer[dst++ - ((rnd >> 45) & 3)] = pf - ((rnd >> 47) & 1);
        rnd = xorshift64(rnd);
    }
}

void updateFireBufferNoRemUnrolled3(i32* restrict buffer, u32 width, u32 height) {
    buffer = __builtin_assume_aligned(buffer, 64);
    u64 rnd = rand64();

    // NOTE(a-square): this is NOT how the original Doom fire was spread,
    // because we're iterating in the hot-to-cold order whereas Doom fire
    // iterated in the cold-to-hot order
    //
    // My wife thinks this is more fire-like
    u64 src = 0;
    u64 dst = width + 2;
    for (u32 i = 0; i < ((height - 1) * width + 15) / 16; ++i) {
        // prefetch some cache lines ahead of time,
        // the write position is further one line ahead because
        // write positions are perturbed
        __builtin_prefetch(&buffer[src + 4 * 16], 0, 3);
        __builtin_prefetch(&buffer[dst + 5 * 16], 1, 3);

        i32 p0 = buffer[src++];
        i32 p1 = buffer[src++];
        i32 p2 = buffer[src++];
        i32 p3 = buffer[src++];
        i32 p4 = buffer[src++];
        i32 p5 = buffer[src++];
        i32 p6 = buffer[src++];
        i32 p7 = buffer[src++];

        buffer[dst++ - (rnd & 3)] = p0 - ((rnd >> 2) & 1);
        buffer[dst++ - ((rnd >> 3) & 3)] = p1 - ((rnd >> 5) & 1);
        buffer[dst++ - ((rnd >> 6) & 3)] = p2 - ((rnd >> 8) & 1);
        buffer[dst++ - ((rnd >> 9) & 3)] = p3 - ((rnd >> 11) & 1);
        buffer[dst++ - ((rnd >> 12) & 3)] = p4 - ((rnd >> 14) & 1);
        buffer[dst++ - ((rnd >> 15) & 3)] = p5 - ((rnd >> 17) & 1);
        buffer[dst++ - ((rnd >> 18) & 3)] = p6 - ((rnd >> 20) & 1);
        buffer[dst++ - ((rnd >> 21) & 3)] = p7 - ((rnd >> 23) & 1);

        i32 p8 = buffer[src++];
        i32 p9 = buffer[src++];
        i32 pa = buffer[src++];
        i32 pb = buffer[src++];
        i32 pc = buffer[src++];
        i32 pd = buffer[src++];
        i32 pe = buffer[src++];
        i32 pf = buffer[src++];

        buffer[dst++ - ((rnd >> 24) & 3)] = p8 - ((rnd >> 26) & 1);
        buffer[dst++ - ((rnd >> 27) & 3)] = p9 - ((rnd >> 29) & 1);
        buffer[dst++ - ((rnd >> 30) & 3)] = pa - ((rnd >> 32) & 1);
        buffer[dst++ - ((rnd >> 33) & 3)] = pb - ((rnd >> 35) & 1);
        buffer[dst++ - ((rnd >> 36) & 3)] = pc - ((rnd >> 38) & 1);
        buffer[dst++ - ((rnd >> 39) & 3)] = pd - ((rnd >> 41) & 1);
        buffer[dst++ - ((rnd >> 42) & 3)] = pe - ((rnd >> 44) & 1);
        buffer[dst++ - ((rnd >> 45) & 3)] = pf - ((rnd >> 47) & 1);
        rnd = xorshift64(rnd);
    }
}

void updateFireBufferNoRemUnrolled4(i32* restrict buffer, u32 width, u32 height) {
    buffer = __builtin_assume_aligned(buffer, 64);
    u64 rnd = rand64();

    // NOTE(a-square): this is NOT how the original Doom fire was spread,
    // because we're iterating in the hot-to-cold order whereas Doom fire
    // iterated in the cold-to-hot order
    //
    // My wife thinks this is more fire-like
    u64 src = 0;
    u64 dst = width + 2;
    for (u32 i = 0; i < ((height - 1) * width + 15) / 16; ++i) {
        // prefetch some cache lines ahead of time,
        // the write position is further one line ahead because
        // write positions are perturbed
        __builtin_prefetch(&buffer[src + 4 * 16], 0, 3);
        __builtin_prefetch(&buffer[dst + 5 * 16], 1, 3);

        i32 p0 = buffer[src++];
        i32 p1 = buffer[src++];
        i32 p2 = buffer[src++];
        i32 p3 = buffer[src++];

        buffer[dst++ - (rnd & 3)] = p0 - ((rnd >> 2) & 1);
        buffer[dst++ - ((rnd >> 3) & 3)] = p1 - ((rnd >> 5) & 1);
        buffer[dst++ - ((rnd >> 6) & 3)] = p2 - ((rnd >> 8) & 1);
        buffer[dst++ - ((rnd >> 9) & 3)] = p3 - ((rnd >> 11) & 1);

        i32 p4 = buffer[src++];
        i32 p5 = buffer[src++];
        i32 p6 = buffer[src++];
        i32 p7 = buffer[src++];

        buffer[dst++ - ((rnd >> 12) & 3)] = p4 - ((rnd >> 14) & 1);
        buffer[dst++ - ((rnd >> 15) & 3)] = p5 - ((rnd >> 17) & 1);
        buffer[dst++ - ((rnd >> 18) & 3)] = p6 - ((rnd >> 20) & 1);
        buffer[dst++ - ((rnd >> 21) & 3)] = p7 - ((rnd >> 23) & 1);

        i32 p8 = buffer[src++];
        i32 p9 = buffer[src++];
        i32 pa = buffer[src++];
        i32 pb = buffer[src++];

        buffer[dst++ - ((rnd >> 24) & 3)] = p8 - ((rnd >> 26) & 1);
        buffer[dst++ - ((rnd >> 27) & 3)] = p9 - ((rnd >> 29) & 1);
        buffer[dst++ - ((rnd >> 30) & 3)] = pa - ((rnd >> 32) & 1);
        buffer[dst++ - ((rnd >> 33) & 3)] = pb - ((rnd >> 35) & 1);

        i32 pc = buffer[src++];
        i32 pd = buffer[src++];
        i32 pe = buffer[src++];
        i32 pf = buffer[src++];

        buffer[dst++ - ((rnd >> 36) & 3)] = pc - ((rnd >> 38) & 1);
        buffer[dst++ - ((rnd >> 39) & 3)] = pd - ((rnd >> 41) & 1);
        buffer[dst++ - ((rnd >> 42) & 3)] = pe - ((rnd >> 44) & 1);
        buffer[dst++ - ((rnd >> 45) & 3)] = pf - ((rnd >> 47) & 1);
        
        rnd = xorshift64(rnd);
    }
}

void updateFireBufferSimple(i32* restrict buffer, u32 width, u32 height) {
    u64 rnd1 = rand64();
    u32 rnd2 = rand32();
    u64 rem1 = rnd1;
    u32 rem2 = rnd2;

    // NOTE(a-square): this is NOT how the original Doom fire was spread,
    // because we're iterating in the hot-to-cold order whereas Doom fire
    // iterated in the cold-to-hot order
    //
    // My wife thinks this is more fire-like
    for (u32 y = 0; y < height - 1; ++y) {
        for (u32 x = 0; x < width; ++x) {
            u32 src = y * width + x;
            u32 dst = src + width + 2 - (u32)(rem1 & 3);
            buffer[dst] = buffer[src] - (rem2 & 1);

            rem1 >>= 2;
            rem2 >>= 1;
            if (x % 32 == 0) {
                rnd1 = xorshift64(rnd1);
                rnd2 = xorshift32(rnd2);
                rem1 = rnd1;
                rem2 = rnd2;
            }
        }
    }
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
