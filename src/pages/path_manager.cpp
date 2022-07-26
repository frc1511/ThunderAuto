#include <pages/path_manager.h>

PathManagerPage::PathManagerPage() { }

PathManagerPage::~PathManagerPage() { }

void PathManagerPage::present(bool* running) {
  ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin("Path Selector", running, ImGuiWindowFlags_NoCollapse)) {
    ImGui::End();
    return;
  }
  
  focused = ImGui::IsWindowFocused();
  ImGui::Text("%d\n", focused);
  
  ImGui::End();
}

PathManagerPage PathManagerPage::instance {};
