#import <platform/macos/macos.h>
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

PlatformMacOS::PlatformMacOS() { }

PlatformMacOS::~PlatformMacOS() { }

std::string PlatformMacOS::open_file_dialog() {
  NSOpenPanel* open_dialog = [NSOpenPanel openPanel];
  
  // File.
  [open_dialog setCanChooseFiles:YES];
  // Not directory.
  [open_dialog setCanChooseDirectories:NO];
  // One file.
  [open_dialog setAllowsMultipleSelection:NO];
  
  // Show the dialog box.
  if ([open_dialog runModal] == NSModalResponseOK) {
    NSArray* urls = [open_dialog URLs];
   
    for(int i = 0; i < [urls count]; i++) {
      NSString* name = [[urls objectAtIndex:i] path];
      
      return [name UTF8String];
    }
  }
  return "";
}

PlatformMacOS PlatformMacOS::instance {};
