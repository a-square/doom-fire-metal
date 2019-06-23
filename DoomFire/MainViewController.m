//
//  MainViewController.m
//  DoomFire
//
//  Created by Алексей Аверченко on 23/06/2019.
//  Copyright © 2019 a-square. All rights reserved.
//

#import "MainViewController.h"
#import "Renderer.h"

@implementation MainViewController {
    Renderer* _renderer;
}

- (void)loadView {
    NSLog(@"Creating the MTKView");
    self.view = [Renderer createMetalView];
}

- (void)viewDidLoad {
    [super viewDidLoad];

    MTKView* view = (MTKView*)self.view;
    _renderer = [[Renderer alloc] initWithMetalView:(MTKView*)view];
    view.delegate = _renderer;
}

@end
