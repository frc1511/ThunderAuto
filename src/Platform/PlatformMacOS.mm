#include "PlatformMacOS.hpp"

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

NSArray* getFileTypes(const FileExtensionList& extensions) {
  NSMutableArray* types = [[NSMutableArray alloc] init];
  for (const auto& ext : extensions) {
    NSString* extStr = [[NSString alloc] initWithBytes:ext.second
                                          length:std::strlen(ext.second)
                                          encoding:NSASCIIStringEncoding];

    // [type_strs addObject:ext_str];

    UTType* t = [UTType typeWithFilenameExtension:extStr];
    [types addObject:t];
  }

  return types;
}

std::filesystem::path PlatformMacOS::openFileDialog(FileType type, const FileExtensionList& extensions) noexcept {
  @autoreleasepool {

  NSOpenPanel* openDialog = [NSOpenPanel openPanel];
  
  if (type == FileType::FILE) {
    [openDialog setCanChooseFiles:YES];
    [openDialog setCanChooseDirectories:NO];
    
    if (extensions.size()) {
      NSArray* types = getFileTypes(extensions);
      [openDialog setAllowedContentTypes:types];
    }
  }
  else {
    [openDialog setCanChooseFiles:NO];
    [openDialog setCanChooseDirectories:YES];
  }
  
  // Only one file.
  [openDialog setAllowsMultipleSelection:NO];
  // Show the dialog box.
  NSModalResponse response = [openDialog runModal];

  if (response == NSModalResponseOK) {
    NSArray* urls = [openDialog URLs];
   
    for(NSUInteger i = 0; i < [urls count]; i++) {
      NSString* path = [[urls objectAtIndex:i] path];
      
      return [path UTF8String];
    }
  }
  else if (response == NSModalResponseCancel) {
    // User canceled the dialog.
    return "";
  }

  // An error occurred.
  return "";

  }
}

std::filesystem::path PlatformMacOS::saveFileDialog(const FileExtensionList& extensions) noexcept {
  NSSavePanel* save_dialog = [NSSavePanel savePanel];

  if (extensions.size()) {
    NSArray* types = getFileTypes(extensions);
    [save_dialog setAllowedContentTypes:types];
  }
  
  // Show the dialog box.
  if ([save_dialog runModal] == NSModalResponseOK) {
    NSString* path = [[save_dialog URL] path];
    
    return [path UTF8String];
  }
  return "";
}

