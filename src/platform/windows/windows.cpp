#include <platform/windows/windows.h>

PlatformWindows::PlatformWindows() { }

PlatformWindows::~PlatformWindows() { }

std::string PlatformWindows::open_file_dialog() {
  // TODO: Implement.
  return "";
}

PlatformWindows PlatformWindows::instance {};
