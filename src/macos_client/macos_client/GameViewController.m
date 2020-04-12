//
//  GameViewController.m
//  macos_client
//
//  Created by Christian Jäger on 30.3.2020.
//  Copyright © 2020 Christian Jäger. All rights reserved.
//

#import "GameViewController.h"
#import "Renderer.h"

#include <universal/game_api.h>
#include <universal/graphics.h>

extern struct GameServices g_gameServices;
extern struct GameApi* g_gameApi;

static struct Graphics s_graphics;

static struct Graphics* GameServices_GetGraphics(void)
{
    return &s_graphics;
}

@implementation GameViewController
{
    MTKView *_view;

    Renderer *_renderer;
}

- (void)viewDidLoad
{
    [super viewDidLoad];

    _view = (MTKView *)self.view;

    _view.device = MTLCreateSystemDefaultDevice();

    if(!_view.device)
    {
        NSLog(@"Metal is not supported on this device");
        self.view = [[NSView alloc] initWithFrame:self.view.frame];
        return;
    }

    _renderer = [[Renderer alloc] initWithMetalKitView:_view];

    [_renderer mtkView:_view drawableSizeWillChange:_view.bounds.size];
    
    [_renderer getGraphics:&s_graphics];

    _view.delegate = _renderer;
    
    g_gameServices.getGraphics = &GameServices_GetGraphics;
    
    // TODO(cj): Where does shutdown code go?
    if(!g_gameApi->init(&g_gameServices))
    {
        printf("Unable to initialize the game.\n");
    }
}

@end
