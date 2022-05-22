#include <popups/new_project.h>
  
NewProjectPopup::NewProjectPopup() { }

NewProjectPopup::~NewProjectPopup() { }

void NewProjectPopup::present(bool* running) {
  if (!ImGui::BeginPopupModal(name.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    return;
  }
    /*
    static constexpr std::array<const char*, 3> years = {
      "FRC 2022 - Homer 2 (Turbulance)",
      "Off-Season - Homer 1",
      "General"
    };
    const char* current_year = years[version];
    if (ImGui::BeginCombo("##year", current_year)) {
      for (size_t i = 0; i < years.size(); ++i) {
        if (ImGui::Selectable(years[i], i == version)) {
          version = static_cast<Version>(i);
        }
      }
      ImGui::EndCombo();
    }
    ImGui::SameLine();
    if (ImGui::Button("Okay")) {
      show_year_selector = false;
      ImGui::CloseCurrentPopup();
    }
    */
  /* ImGui::Button("Yes"); */
  /* ImGui::Button("No"); */
  ImGui::EndPopup();
}

NewProjectPopup NewProjectPopup::instance {};

