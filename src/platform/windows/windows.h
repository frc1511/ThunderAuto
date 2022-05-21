#pragma once

#include <platform/platform.h>

class PlatformWindows: public Platform {
public:
  static PlatformWindows* get() {
    return &instance;
  }
  
  PlatformWindows(PlatformWindows const&) = delete;
  PlatformWindows& operator=(PlatformWindows const&) = delete;

  std::string open_file_dialog();

private:
  PlatformWindows();
  ~PlatformWindows();
  
  static PlatformWindows instance;
};
