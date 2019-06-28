//
//  Shaders.metal
//  DoomFire
//
//  Created by Алексей Аверченко on 23/06/2019.
//  Copyright © 2019 a-square. All rights reserved.
//

#include <metal_stdlib>
using namespace metal;

#include "ShaderTypes.h"

typedef struct {
    float4 position [[position]];
    float2 textureCoordinate;
} RasterizerData;

// Vertex Function
vertex RasterizerData
vertexShader(uint vertexID [[ vertex_id ]],
             constant Vertex *vertexArray [[ buffer(InputIndexVertices) ]])
{
    RasterizerData out;

    float2 pixelSpacePosition = vertexArray[vertexID].position.xy;

    out.position = vector_float4(0.0f, 0.0f, 0.0f, 1.0f);
    out.position.xy = pixelSpacePosition;
    out.textureCoordinate = vertexArray[vertexID].textureCoordinate;

    return out;
}

// Fragment function
fragment float4
fragmentShader(RasterizerData in [[stage_in]],
               texture2d<int> colorTexture [[ texture(TextureIndexColor) ]])
{
    // TODO(a-square): read without sampling?
    constexpr sampler textureSampler (mag_filter::nearest,
                                      min_filter::nearest);

    int4 sample = colorTexture.sample(textureSampler, in.textureCoordinate);
    float gammaIntensity = saturate(sample.x / (float)MAX_LIT);
    float halfGammaIntensity = pow(gammaIntensity, 1.5f);
    float intensity = pow(gammaIntensity, 2.2f);
    float r = saturate(1.8f * halfGammaIntensity);
    float g = saturate(1.4f * intensity - 0.3f);
    float b = saturate(2.0f * intensity - 1.0f);
    return vector_float4(r, g, b, 1.0f);
}
