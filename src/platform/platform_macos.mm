#include <ThunderAuto/platform/platform_macos.hpp>
#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

NSArray* get_file_types(const FileExtensionList& extensions) {
      NSMutableArray* types = [[NSMutableArray alloc] init];
      for (const auto& ext : extensions) {
        NSString* ext_str = [[NSString alloc] initWithBytes:ext.second
                                              length:std::strlen(ext.second)
                                              encoding:NSASCIIStringEncoding];

        // [type_strs addObject:ext_str];

        UTType* t = [UTType typeWithFilenameExtension:ext_str];
        [types addObject:t];
      }

      return types;
}

std::string PlatformMacOS::open_file_dialog(FileType type, const FileExtensionList& extensions) {
  @autoreleasepool {

  NSOpenPanel* open_dialog = [NSOpenPanel openPanel];
  
  if (type == FileType::FILE) {
    [open_dialog setCanChooseFiles:YES];
    [open_dialog setCanChooseDirectories:NO];
    
    if (extensions.size()) {
      NSArray* types = get_file_types(extensions);
      [open_dialog setAllowedContentTypes:types];
    }
  }
  else {
    [open_dialog setCanChooseFiles:NO];
    [open_dialog setCanChooseDirectories:YES];
  }
  
  // Only one file.
  [open_dialog setAllowsMultipleSelection:NO];
  // Show the dialog box.
  NSModalResponse response = [open_dialog runModal];

  if (response == NSModalResponseOK) {
    NSArray* urls = [open_dialog URLs];
   
    for(NSUInteger i = 0; i < [urls count]; i++) {
      NSString* path = [[urls objectAtIndex:i] path];
      
      return [path UTF8String];
    }
  }
  else if (response == NSModalResponseCancel) {
    // User canceled the dialog.
    return "";
  }
  else {
    // An error occurred.
    return "";
  }

  return "";

  }
}

std::string PlatformMacOS::save_file_dialog(const FileExtensionList& extensions) {
  NSSavePanel* save_dialog = [NSSavePanel savePanel];

  if (extensions.size()) {
    NSArray* types = get_file_types(extensions);
    [save_dialog setAllowedContentTypes:types];
  }
  
  // Show the dialog box.
  if ([save_dialog runModal] == NSModalResponseOK) {
    NSString* path = [[save_dialog URL] path];
    
    return [path UTF8String];
  }
  return "";
}
