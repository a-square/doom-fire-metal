//
//  ShaderTypes.h
//  DoomFire
//
//  Created by Алексей Аверченко on 23/06/2019.
//  Copyright © 2019 a-square. All rights reserved.
//

#pragma once

#include <simd/simd.h>

typedef uint32_t u32;
typedef int32_t i32;
typedef uint64_t u64;
typedef int64_t i64;

#define MAX_LIT 128
#define PIXEL_SIZE 4

typedef enum {
    InputIndexVertices = 0,
    //InputIndexViewportSize = 1,
} InputIndex;

typedef enum {
    TextureIndexColor = 0,
} TextureIndex;

typedef struct {
    vector_float2 position;
    vector_float2 textureCoordinate;
} Vertex;
