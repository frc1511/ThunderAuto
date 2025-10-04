#include <ThunderAuto/Platform/Platform.hpp>
#include <imgui.h>

const std::pair<const char*, const char*> kThunderAutoFileFilter{"ThunderAuto Files (*.thunderauto)",
                                                                 "thunderauto"};

const char* const kThunderAutoFileExtension = ".thunderauto";

void PlatformImpl::openURL(const std::string& url) {
  const ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
  if (platformIO.Platform_OpenInShellFn) {
    platformIO.Platform_OpenInShellFn(ImGui::GetCurrentContext(), url.c_str());
  }
}
