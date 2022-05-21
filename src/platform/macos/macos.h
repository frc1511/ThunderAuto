#pragma once

#include <platform/platform.h>

class PlatformMacOS: public Platform {
public:
  static PlatformMacOS* get() {
    return &instance;
  }
  
  PlatformMacOS(PlatformMacOS const&) = delete;
  PlatformMacOS& operator=(PlatformMacOS const&) = delete;

  std::string open_file_dialog();

private:
  PlatformMacOS();
  ~PlatformMacOS();
  
  static PlatformMacOS instance;
};
