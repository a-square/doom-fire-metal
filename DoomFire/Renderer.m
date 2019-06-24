//
//  Renderer.m
//  DoomFire
//
//  Created by Алексей Аверченко on 23/06/2019.
//  Copyright © 2019 a-square. All rights reserved.
//

#import "Renderer.h"

#include "ShaderTypes.h"
#include "FireBuffer.h"

static const Vertex QUAD_VERTICES[] = {
    { {-1, -1}, {0, 0} },
    { {-1, 1}, {0, 1} },
    { {1, -1}, {1, 0} },

    { {-1, 1}, {0, 1} },
    { {1, 1}, {1, 1} },
    { {1, -1}, {1, 0} },
};

@implementation Renderer {
    id<MTLDevice> _device;
    id<MTLRenderPipelineState> _pipelineState;
    id<MTLCommandQueue> _commandQueue;
    id<MTLBuffer> _vertexBuffer;

    MTLViewport _viewport;

    simd_uint2 _viewportSize;

    id<MTLTexture> _texture;
    int32_t* _textureBytes;
}

+ (nonnull MTKView *)createMetalView {
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    NSAssert(device, @"Failed to create a metal device");

    MTKView* view = [[MTKView alloc] initWithFrame:[[UIScreen mainScreen] bounds] device:device];
    view.layer.magnificationFilter = kCAFilterNearest;
    view.preferredFramesPerSecond = 30;

    NSAssert(view, @"Metal view failed to initialize");
    return view;
}

- (nonnull instancetype)initWithMetalView:(nonnull MTKView*)view {
    self = [super init];
    if (self) {
        _device = view.device;
        NSAssert(_device, @"MTKView's device not set");

        NSLog(@"Creating the Renderer");

        CGSize size = [UIScreen mainScreen].nativeBounds.size;
        _viewportSize.x = size.width / PIXEL_SIZE;
        _viewportSize.y = size.height / PIXEL_SIZE;

        _textureBytes = createFireBuffer(_viewportSize.x, _viewportSize.y);
        NSAssert(_textureBytes, @"Failed to allocate fire bytes");

        MTLTextureDescriptor* texDtor = [[MTLTextureDescriptor alloc] init];
        texDtor.pixelFormat = MTLPixelFormatR32Sint;
        texDtor.width = _viewportSize.x;
        texDtor.height = _viewportSize.y;
        _texture = [_device newTextureWithDescriptor:texDtor];
        NSAssert(_texture, @"Failed to create the texture");

        id<MTLLibrary> defaultLibrary = [_device newDefaultLibrary];
        id<MTLFunction> vertexFunction = [defaultLibrary newFunctionWithName:@"vertexShader"];
        NSAssert(vertexFunction, @"Could not load the vertex shader");
        id<MTLFunction> fragmentFunction = [defaultLibrary newFunctionWithName:@"fragmentShader"];
        NSAssert(fragmentFunction, @"Could not load the fragment shader");

        MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
        pipelineStateDescriptor.label = @"Fire pipeline";
        pipelineStateDescriptor.vertexFunction = vertexFunction;
        pipelineStateDescriptor.fragmentFunction = fragmentFunction;
        pipelineStateDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;

        NSError* error = nil;
        _pipelineState = [_device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:&error];
        NSAssert(_pipelineState, @"Failed to created pipeline state: %@", error);

        _commandQueue = [_device newCommandQueue];
        NSAssert(_commandQueue, @"Command queue failed to initialize");

        _viewport = (MTLViewport){0, 0, _viewportSize.x * PIXEL_SIZE, _viewportSize.y * PIXEL_SIZE, -1, 1};

        _vertexBuffer = [_device newBufferWithBytes:QUAD_VERTICES length:sizeof(QUAD_VERTICES) options:MTLResourceStorageModeShared];
    }
    return self;
}

- (void)drawInMTKView:(nonnull MTKView*)view {
    id<MTLCommandBuffer> cmdBuffer = [_commandQueue commandBuffer];
    cmdBuffer.label = @"Fire command";

    MTLRenderPassDescriptor* desc = view.currentRenderPassDescriptor;
    if (desc) {
        updateFireBuffer(_textureBytes, _viewportSize.x, _viewportSize.y);

        MTLRegion region = MTLRegionMake2D(0, 0, _viewportSize.x, _viewportSize.y);
        [_texture replaceRegion:region mipmapLevel:0 withBytes:_textureBytes bytesPerRow:sizeof(_textureBytes[0]) * _viewportSize.x];

        id<MTLRenderCommandEncoder> encoder = [cmdBuffer renderCommandEncoderWithDescriptor:desc];
        [encoder setViewport:_viewport];
        [encoder setRenderPipelineState:_pipelineState];
        [encoder setVertexBuffer:_vertexBuffer offset:0 atIndex:InputIndexVertices];
        [encoder setFragmentTexture:_texture atIndex:TextureIndexColor];
        [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:sizeof(QUAD_VERTICES) / sizeof(QUAD_VERTICES[0])];
        [encoder endEncoding];

        [cmdBuffer presentDrawable:view.currentDrawable];
    }

    [cmdBuffer commit];
}

- (void)mtkView:(nonnull MTKView*)view drawableSizeWillChange:(CGSize)size {
    // TODO(a-square)
    NSLog(@"drawableSizeWillChange");
}

- (void)dealloc {
    destroyFireBuffer(_textureBytes);
}

@end
