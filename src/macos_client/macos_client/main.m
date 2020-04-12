//
//  main.m
//  macos_client
//
//  Created by Christian Jäger on 30.3.2020.
//  Copyright © 2020 Christian Jäger. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#include <platform/platform.h>
#include <universal/cmdline.h>
#include <universal/game_api.h>
#include <universal/host.h>

static const char* s_projectRoot;

static const char* GetProjectRoot(void)
{
    return s_projectRoot;
}

struct GameServices g_gameServices;
struct GameApi* g_gameApi;

int main(int argc, const char * argv[]) {
    s_projectRoot = GetCommandLineOption(argc, argv, "--projectroot");
    
    if(!Host_LoadGame(s_projectRoot, &g_gameApi))
    {
        printf("Unable to load game.\n");
    }
    
    printf("Game says: %s\n", g_gameApi->msg());
    
    g_gameServices.getProjectRoot = &GetProjectRoot;
    
    @autoreleasepool {
        // Setup code that might create autoreleased objects goes here.
    }
    return NSApplicationMain(argc, argv);
}
