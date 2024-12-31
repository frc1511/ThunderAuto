#include <ThunderAuto/platform/platform_macos.h>
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

std::string PlatformMacOS::open_file_dialog(FileType type, const FileExtensionList& extensions) {
  NSOpenPanel* open_dialog = [NSOpenPanel openPanel];
  
  if (type == FileType::FILE) {
    [open_dialog setCanChooseFiles:YES];
    [open_dialog setCanChooseDirectories:NO];
    

    if (extensions.size()) {
      NSMutableArray* types = [NSMutableArray array];
      for (const auto& ext : extensions) {
        NSString* ext_str = [[NSString alloc] initWithBytes:ext.second
                                              length:std::strlen(ext.second)
                                              encoding:NSASCIIStringEncoding];
        [types addObject:ext_str];
      }
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

std::string PlatformMacOS::save_file_dialog(const FileExtensionList& extensions) {
  NSSavePanel* save_dialog = [NSSavePanel savePanel];

  if (extensions.size()) {
    NSMutableArray* types = [NSMutableArray array];
    for (const auto& ext : extensions) {
      NSString* ext_str = [[NSString alloc] initWithBytes:ext.second
                                            length:std::strlen(ext.second)
                                            encoding:NSASCIIStringEncoding];
      [types addObject:ext_str];
    }
    [save_dialog setAllowedFileTypes:types];
  }
  
  // Show the dialog box.
  if ([save_dialog runModal] == NSModalResponseOK) {
    NSString* path = [[save_dialog URL] path];
    
    return [path UTF8String];
  }
  return "";
}
