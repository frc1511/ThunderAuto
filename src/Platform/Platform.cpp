#include <ThunderAuto/Platform/Platform.hpp>
#include <imgui.h>

#if THUNDERAUTO_MACOS
#include "PlatformMacOS.hpp"
#elif THUNDERAUTO_WINDOWS
#include "PlatformWindows.hpp"
#elif THUNDERAUTO_LINUX
#include "PlatformLinux.hpp"
#endif

const std::pair<const char*, const char*> kThunderAutoFileFilter{"ThunderAuto Files (*.thunderauto)",
                                                                 "thunderauto"};

const char* const kThunderAutoFileExtension = ".thunderauto";

void Platform::openURL(const std::string& url) noexcept {
  const ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
  if (platformIO.Platform_OpenInShellFn) {
    platformIO.Platform_OpenInShellFn(ImGui::GetCurrentContext(), url.c_str());
  }
}

Platform& getPlatform() noexcept {
#if THUNDERAUTO_MACOS
  return PlatformMacOS::get();
#elif THUNDERAUTO_WINDOWS
  return PlatformWindows::get();
#elif THUNDERAUTO_LINUX
  return PlatformLinux::get();
#endif
}
