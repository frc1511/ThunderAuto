#include <pages/properties.h>
  
PropertiesPage::PropertiesPage() { }

PropertiesPage::~PropertiesPage() { }

void PropertiesPage::present(bool* running) {
  ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin("Properties", running, ImGuiWindowFlags_NoCollapse)) {
    ImGui::End();
    return;
  }
  
  focused = ImGui::IsWindowFocused();
  ImGui::Text("%d\n", focused);
  
  ImGui::End();
}

PropertiesPage PropertiesPage::instance {};
