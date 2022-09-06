#include <popups/welcome.h>
#include <font_manager.h>
#include <IconsFontAwesome5.h>

#define COL_WIDTH 150

WelcomePopup::WelcomePopup() { }

WelcomePopup::~WelcomePopup() { }

void WelcomePopup::present(bool* running) {
  if (!ImGui::BeginPopupModal(name.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    return;
  }

  ImGui::PushFont(FontManager::get()->big);
  ImGui::Text("Welcome to ThunderAuto");
  ImGui::PopFont();

  ImGui::Text("Version " THUNDER_AUTO_VERSION_STR);

  ImGui::Dummy(ImVec2(0.0f, 15.0f));

  // --- New Project ---

  ImGui::PushID("New Project");
  ImGui::Columns(2, nullptr, false);
  ImGui::SetColumnWidth(0, COL_WIDTH);
  ImGui::Text(ICON_FA_FILE "  New Project");
  ImGui::SetWindowFontScale(0.75f);
  ImGui::Text("Create a new project");
  ImGui::SetWindowFontScale(1.0f);
  ImGui::NextColumn();

  if (ImGui::Button("New Project")) {
    new_project = true;
  }

  ImGui::Columns(1);
  ImGui::PopID();

  ImGui::Dummy(ImVec2(0.0f, 10.0f));

  // --- Open Project ---

  ImGui::PushID("Open Project");
  ImGui::Columns(2, nullptr, false);
  ImGui::SetColumnWidth(0, COL_WIDTH);
  ImGui::Text(ICON_FA_FOLDER_OPEN "  Open Project");
  ImGui::SetWindowFontScale(0.75f);
  ImGui::Text("Open an existing project");
  ImGui::SetWindowFontScale(1.0f);
  ImGui::NextColumn();

  if (ImGui::Button("Open Project")) {
    opening_project = true;
  }

  ImGui::Columns(1);
  ImGui::PopID();

  ImGui::EndPopup();
}

WelcomePopup WelcomePopup::instance;
