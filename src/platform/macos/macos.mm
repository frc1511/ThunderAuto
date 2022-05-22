#import <platform/macos/macos.h>
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

PlatformMacOS::PlatformMacOS() { }

PlatformMacOS::~PlatformMacOS() { }

std::string PlatformMacOS::open_file_dialog() {
  NSOpenPanel* open_dialog = [NSOpenPanel openPanel];
  
  // Can be a file.
  [open_dialog setCanChooseFiles:YES];
  // Can't be a directory.
  [open_dialog setCanChooseDirectories:NO];
  // Only one file.
  [open_dialog setAllowsMultipleSelection:NO];
  // Only with extension '.thunderpath'.
  NSArray* types = [NSArray arrayWithObjects:@"thunderauto",nil];
  [open_dialog setAllowedFileTypes:types];
  
  // Show the dialog box.
  if ([open_dialog runModal] == NSModalResponseOK) {
    NSArray* urls = [open_dialog URLs];
   
    for(int i = 0; i < [urls count]; i++) {
      NSString* path = [[urls objectAtIndex:i] path];
      
      return [path UTF8String];
    }
  }
  return "";
}

std::string PlatformMacOS::save_file_dialog() {
  NSSavePanel* save_dialog = [NSSavePanel savePanel];
  
  // Save with extension '.thunderpath'.
  NSArray* types = [NSArray arrayWithObjects:@"thunderauto",nil];
  [save_dialog setAllowedFileTypes:types];
  
  // Show the dialog box.
  if ([save_dialog runModal] == NSModalResponseOK) {
    NSString* path = [[save_dialog URL] path];
    
    return [path UTF8String];
  }
  return "";
}

PlatformMacOS PlatformMacOS::instance {};
