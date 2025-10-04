#include <ThunderAuto/Platform/PlatformManager.hpp>

#include <ThunderAuto/Graphics/Graphics.hpp>

#if THUNDERAUTO_MACOS
#include <ThunderAuto/Platform/MacOS.hpp>
using Platform = PlatformMacOS;
#elif THUNDERAUTO_WINDOWS
#include <ThunderAuto/Platform/Windows.hpp>
using Platform = PlatformWindows;
#elif THUNDERAUTO_LINUX
#include <ThunderAuto/Platform/Linux.hpp>
using Platform = PlatformLinux;
#endif

PlatformManager::PlatformManager() : m_impl(std::make_unique<Platform>()) {}

void PlatformManager::openURL(const std::string& url) {
  m_impl->openURL(url);
}

std::string PlatformManager::openFileDialog(FileType type, const FileExtensionList& extensions) {
  std::string result = m_impl->openFileDialog(type, extensions);

  // Bring focus back to the window.
  PlatformGraphics::get().windowFocus();

  return result;
}

std::string PlatformManager::saveFileDialog(const FileExtensionList& extensions) {
  std::string result = m_impl->saveFileDialog(extensions);

  // Bring focus back to the window.
  PlatformGraphics::get().windowFocus();

  return result;
}
