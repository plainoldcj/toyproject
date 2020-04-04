//
//  main.m
//  macos_client
//
//  Created by Christian Jäger on 30.3.2020.
//  Copyright © 2020 Christian Jäger. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#include <platform/platform.h>

int main(int argc, const char * argv[]) {
    int v = f();
    
    NSString* msg = [NSString stringWithFormat:@"val is %d", v];
    
    NSAlert* alert = [[NSAlert alloc] init];
    [alert setMessageText:msg];
    [alert runModal];
    
    @autoreleasepool {
        // Setup code that might create autoreleased objects goes here.
    }
    return NSApplicationMain(argc, argv);
}
