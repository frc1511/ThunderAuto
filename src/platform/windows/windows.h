#pragma once

#include <platform/platform.h>

class PlatformWindows: public Platform {
public:
  static PlatformWindows* get() {
    return &instance;
  }
  
  PlatformWindows(PlatformWindows const&) = delete;
  PlatformWindows& operator=(PlatformWindows const&) = delete;
  
  std::string open_file_dialog(FileType type, const char* extension);
  std::string save_file_dialog(const char* extension);
  
private:
  PlatformWindows();
  ~PlatformWindows();
  
  static PlatformWindows instance;
};
