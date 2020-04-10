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

int main(int argc, const char * argv[]) {
    const char* projectRoot = GetCommandLineOption(argc, argv, "--projectroot");
    
    struct GameApi* gameApi = NULL;
    if(!Host_LoadGame(projectRoot, &gameApi))
    {
        printf("Unable to load game.\n");
    }
    
    printf("Game says: %s\n", gameApi->msg());
    
    @autoreleasepool {
        // Setup code that might create autoreleased objects goes here.
    }
    return NSApplicationMain(argc, argv);
}
