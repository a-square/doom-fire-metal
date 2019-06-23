//
//  Renderer.h
//  DoomFire
//
//  Created by Алексей Аверченко on 23/06/2019.
//  Copyright © 2019 a-square. All rights reserved.
//

@import MetalKit;

@interface Renderer : NSObject<MTKViewDelegate>

+ (nonnull MTKView*) createMetalView;
- (nonnull instancetype) initWithMetalView:(nonnull MTKView*)view;

@end
