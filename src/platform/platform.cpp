#include <platform/platform.h>

#if THUNDER_AUTO_MACOS
# include <platform/macos/macos.h>
#elif THUNDER_AUTO_WINDOWS
# include <platform/windows/windows.h>
#elif THUNDER_AUTO_LINUX
# include <platform/linux/linux.h>
#endif

Platform* Platform::get_current() {
#ifdef THUNDER_AUTO_MACOS
  return PlatformMacOS::get();
#elif THUNDER_AUTO_WINDOWS
  return PlatformWindows::get();
#elif THUNDER_AUTO_LINUX
  return PlatformLinux::get();
#endif
  std::cerr << "Invalid platform" << std::endl;
  return nullptr;
}