#include <pages/path_editor.h>
  
PathEditorPage::PathEditorPage() { }

PathEditorPage::~PathEditorPage() { }

void PathEditorPage::present(bool* running) {
  ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin("Path Editor", running, ImGuiWindowFlags_NoCollapse)) {
    ImGui::End();
    return;
  }
  
  focused = ImGui::IsWindowFocused();
  ImGui::Text("%d\n", focused);
  
  ImGui::End();
}

PathEditorPage PathEditorPage::instance {};
