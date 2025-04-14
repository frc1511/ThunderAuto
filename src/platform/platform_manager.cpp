#include <ThunderAuto/platform/platform_manager.hpp>

#include <ThunderAuto/graphics.hpp>

#if THUNDER_AUTO_MACOS
#include <ThunderAuto/platform/platform_macos.hpp>
using Platform = PlatformMacOS;
#elif THUNDER_AUTO_WINDOWS
#include <ThunderAuto/platform/platform_windows.hpp>
using Platform = PlatformWindows;
#elif THUNDER_AUTO_LINUX
#include <ThunderAuto/platform/platform_linux.hpp>
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

