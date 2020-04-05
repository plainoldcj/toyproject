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

int main(int argc, const char * argv[]) {
    const char* option = GetCommandLineOption(argc, argv, "--projectroot");
    
    NSString* msg = [NSString stringWithFormat:@"val is %s", option];
    
    NSAlert* alert = [[NSAlert alloc] init];
    [alert setMessageText:msg];
    [alert runModal];
    
    @autoreleasepool {
        // Setup code that might create autoreleased objects goes here.
    }
    return NSApplicationMain(argc, argv);
}
