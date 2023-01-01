#pragma once

#include <ThunderAuto/platform/platform.h>

class PlatformLinux: public Platform {
public:
  static PlatformLinux* get() {
    return &instance;
  }
  
  PlatformLinux(PlatformLinux const&) = delete;
  PlatformLinux& operator=(PlatformLinux const&) = delete;
  
  std::string open_file_dialog(FileType type, const char* extension);
  std::string save_file_dialog(const char* extension);
  
private:
  PlatformLinux();
  ~PlatformLinux();
  
  static PlatformLinux instance;
};
