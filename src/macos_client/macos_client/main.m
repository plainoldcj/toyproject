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

int main(int argc, const char * argv[]) {
    s_projectRoot = GetCommandLineOption(argc, argv, "--projectroot");
    
    struct GameApi* gameApi = NULL;
    if(!Host_LoadGame(s_projectRoot, &gameApi))
    {
        printf("Unable to load game.\n");
    }
    
    printf("Game says: %s\n", gameApi->msg());
    
    struct GameServices gameServices;
    gameServices.getProjectRoot = &GetProjectRoot;
    
    // TODO(cj): Where does shutdown code go?
    if(!gameApi->init(&gameServices))
    {
        printf("Unable to initialize the game.\n");
        return 1;
    }
    
    @autoreleasepool {
        // Setup code that might create autoreleased objects goes here.
    }
    return NSApplicationMain(argc, argv);
}
