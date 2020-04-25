//
//  GameView.m
//  macos_client
//
//  Created by Christian Jäger on 20.4.2020.
//  Copyright © 2020 Christian Jäger. All rights reserved.
//

#import "GameView.h"

#import <Foundation/Foundation.h>

#include <stdio.h>

#include <universal/app_event.h>
#include <universal/game_api.h>

#define MAX_APP_EVENTS 1024

extern struct GameServices g_gameServices;

// TODO(cj): Make this either double-buffered or a proper ring buffer.
static struct AppEvent s_appEvents[MAX_APP_EVENTS];
static uint16_t s_appEventCount = 0;

static bool PollEvent(struct AppEvent* event)
{
    // TODO(cj): Make this either double-buffered or a proper ring buffer.
    if(s_appEventCount == 0)
    {
        return false;
    }
    
    *event = s_appEvents[0];
    for(int i = 1; i < s_appEventCount; ++i)
    {
        s_appEvents[i - 1] = s_appEvents[i];
    }
    --s_appEventCount;
    
    return true;
}

@implementation GameView

-(instancetype)initWithCoder:(NSCoder *)coder
{
    self = [super initWithCoder:coder];
    if(self)
    {
        g_gameServices.pollEvent = &PollEvent;
    }
    return self;
}

-(BOOL)acceptsFirstResponder;
{
    return TRUE;
}

-(void)mouseUp:(NSEvent *)event;
{
    NSPoint windowPos = [event locationInWindow];
    NSPoint viewPos = [self convertPoint:windowPos fromView:nil];
    
    // NSView origin is in lower-left corner, but game expects origin in upper-left corner.
    const float y = self.bounds.size.height - viewPos.y;
    
    struct AppEvent* appEvent = &s_appEvents[s_appEventCount++];
    appEvent->type = APP_EVENT_MOUSE_BUTTON_UP;
    appEvent->mouse.x = (uint16_t)viewPos.x;
    appEvent->mouse.y = y;
}

@end


