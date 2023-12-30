#include <GLFW/glfw3.h>
#include <ThunderAuto/platform/platform_manager.h>

#if THUNDER_AUTO_MACOS
#include <ThunderAuto/platform/platform_macos.h>
using Platform = PlatformMacOS;
#elif THUNDER_AUTO_WINDOWS
#include <ThunderAuto/platform/platform_windows.h>
using Platform = PlatformWindows;
#elif THUNDER_AUTO_LINUX
#include <ThunderAuto/platform/platform_linux.h>
using Platform = PlatformLinux;
#endif

PlatformManager::PlatformManager(GLFWwindow* window)
  : m_window(window),
    m_impl(std::make_unique<Platform>(window)) {}

std::string
PlatformManager::open_file_dialog(FileType type,
                                  const FileExtensionList& extensions) {
  std::string result = m_impl->open_file_dialog(type, extensions);

  // Bring focus back to the window.
  glfwFocusWindow(m_window);

  return result;
}

std::string
PlatformManager::save_file_dialog(const FileExtensionList& extensions) {
  std::string result = m_impl->save_file_dialog(extensions);

  // Bring focus back to the window.
  glfwFocusWindow(m_window);

  return result;
}
