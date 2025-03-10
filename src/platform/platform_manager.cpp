#include <ThunderAuto/platform/platform_manager.h>

#include <ThunderAuto/graphics.h>

#if TH_MACOS
#include <ThunderAuto/platform/platform_macos.h>
using Platform = PlatformMacOS;
#elif TH_WINDOWS
#include <ThunderAuto/platform/platform_windows.h>
using Platform = PlatformWindows;
#elif TH_LINUX
#include <ThunderAuto/platform/platform_linux.h>
using Platform = PlatformLinux;
#endif

PlatformManager::PlatformManager()
:   m_impl(std::make_unique<Platform>()) {}

std::string
PlatformManager::open_file_dialog(FileType type,
                                  const FileExtensionList& extensions) {
  std::string result = m_impl->open_file_dialog(type, extensions);

  // Bring focus back to the window.
  Graphics::get().window_focus();

  return result;
}

std::string
PlatformManager::save_file_dialog(const FileExtensionList& extensions) {
  std::string result = m_impl->save_file_dialog(extensions);

  // Bring focus back to the window.
  Graphics::get().window_focus();

  return result;
}

