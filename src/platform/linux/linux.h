#pragma once

#include <platform/platform.h>

class PlatformLinux: public Platform {
public:
  static PlatformLinux* get() {
    return &instance;
  }
  
  PlatformLinux(PlatformLinux const&) = delete;
  PlatformLinux& operator=(PlatformLinux const&) = delete;
  
  std::string open_file_dialog();
  std::string save_file_dialog();
  
private:
  PlatformLinux();
  ~PlatformLinux();
  
  static PlatformLinux instance;
};
