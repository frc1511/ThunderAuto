#include <popups/unsaved.h>
  
UnsavedPopup::UnsavedPopup() { }

UnsavedPopup::~UnsavedPopup() { }

void UnsavedPopup::present(bool* running) {
  if (!ImGui::BeginPopupModal(name.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    return;
  }
  if (ImGui::Button("Yes")) {
    result = SAVE;
    goto close;
  }
  if (ImGui::Button("No")) {
    result = DONT_SAVE;
    goto close;
  }
  
  ImGui::Separator();
  
  if (ImGui::Button("Cancel")) {
    result = CANCEL;
    goto close;
  }
  
  ImGui::EndPopup();
  return;

close:
  ImGui::CloseCurrentPopup();
  *running = false;
  ImGui::EndPopup();
}

UnsavedPopup UnsavedPopup::instance {};
