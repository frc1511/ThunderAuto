#include <platform/linux/linux.h>

PlatformLinux::PlatformLinux() { }

PlatformLinux::~PlatformLinux() { }

std::string PlatformLinux::open_file_dialog(FileType type, const char* extension) {
  // TODO: Implement.
  return "";
}

std::string PlatformLinux::save_file_dialog(const char* extension) {
  // TODO: Implement.
  return "";
}

PlatformLinux PlatformLinux::instance {};

