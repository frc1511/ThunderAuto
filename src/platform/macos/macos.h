#pragma once

#include <platform/platform.h>

class PlatformMacOS: public Platform {
public:
  static PlatformMacOS* get() {
    return &instance;
  }
  
  PlatformMacOS(PlatformMacOS const&) = delete;
  PlatformMacOS& operator=(PlatformMacOS const&) = delete;
  
  std::string open_file_dialog(FileType type, const char* extension = nullptr) override;
  std::string save_file_dialog(const char* extension = nullptr) override;
  
private:
  PlatformMacOS();
  ~PlatformMacOS();
  
  static PlatformMacOS instance;
};
