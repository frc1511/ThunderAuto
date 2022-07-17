#include <platform/windows/windows.h>

PlatformWindows::PlatformWindows() { }

PlatformWindows::~PlatformWindows() { }

std::string PlatformWindows::open_file_dialog(FileType type, const char* extension) {
  // TODO: Implement.
  return "";
}

std::string PlatformWindows::save_file_dialog(const char* extension) {
  // TODO: Implement.
  return "";
}

PlatformWindows PlatformWindows::instance {};
