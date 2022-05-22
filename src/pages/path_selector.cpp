#include <pages/path_selector.h>
  
PathSelectorPage::PathSelectorPage() { }

PathSelectorPage::~PathSelectorPage() { }

void PathSelectorPage::present(bool* running) {
  ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin("Path Selector", running, ImGuiWindowFlags_NoCollapse)) {
    ImGui::End();
    return;
  }
  
  focused = ImGui::IsWindowFocused();
  ImGui::Text("%d\n", focused);
  
  ImGui::End();
}

PathSelectorPage PathSelectorPage::instance {};
