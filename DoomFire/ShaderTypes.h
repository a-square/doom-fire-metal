//
//  ShaderTypes.h
//  DoomFire
//
//  Created by Алексей Аверченко on 23/06/2019.
//  Copyright © 2019 a-square. All rights reserved.
//

#pragma once

#include <simd/simd.h>

enum {
    MAX_LIT = 128,
    PIXEL_SIZE = 4,
};

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
