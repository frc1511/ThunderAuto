#include <platform/linux/linux.h>

PlatformLinux::PlatformLinux() { }

PlatformLinux::~PlatformLinux() { }

std::string PlatformLinux::open_file_dialog() {
  // TODO: Implement.
  return "";
}

std::string PlatformLinux::save_file_dialog() {
  // TODO: Implement.
  return "";
}

PlatformLinux PlatformLinux::instance {};

