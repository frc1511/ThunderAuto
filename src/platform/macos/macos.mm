#import <platform/macos/macos.h>
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

PlatformMacOS::PlatformMacOS() { }

PlatformMacOS::~PlatformMacOS() { }

std::string PlatformMacOS::open_file_dialog(FileType type, const char* ext_str) {
  NSOpenPanel* open_dialog = [NSOpenPanel openPanel];
  
  if (type == FileType::FILE) {
    [open_dialog setCanChooseFiles:YES];
    [open_dialog setCanChooseDirectories:NO];
    
    if (ext_str) {
      NSString* ext = [[NSString alloc] initWithBytes:ext_str length:strlen(ext_str) encoding:NSASCIIStringEncoding];
      NSArray* types = [NSArray arrayWithObjects:ext,nil];
      [open_dialog setAllowedFileTypes:types];
    }
  }
  else {
      [open_dialog setCanChooseFiles:NO];
      [open_dialog setCanChooseDirectories:YES];
  }
  
  // Only one file.
  [open_dialog setAllowsMultipleSelection:NO];
  // Show the dialog box.
  if ([open_dialog runModal] == NSModalResponseOK) {
    NSArray* urls = [open_dialog URLs];
   
    for(NSUInteger i = 0; i < [urls count]; i++) {
      NSString* path = [[urls objectAtIndex:i] path];
      
      return [path UTF8String];
    }
  }
  return "";
}

std::string PlatformMacOS::save_file_dialog(const char* ext_str) {
  NSSavePanel* save_dialog = [NSSavePanel savePanel];

  if (ext_str) {
    NSString* ext = [[NSString alloc] initWithBytes:ext_str length:strlen(ext_str) encoding:NSASCIIStringEncoding];
    NSArray* types = [NSArray arrayWithObjects:ext,nil];
    [save_dialog setAllowedFileTypes:types];
  }
  
  // Show the dialog box.
  if ([save_dialog runModal] == NSModalResponseOK) {
    NSString* path = [[save_dialog URL] path];
    
    return [path UTF8String];
  }
  return "";
}

PlatformMacOS PlatformMacOS::instance {};
