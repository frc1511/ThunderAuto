#include <ThunderAuto/popups/welcome_popup.hpp>

#include <IconsFontAwesome5.h>
#include <ThunderAuto/imgui_util.hpp>

void WelcomePopup::present(bool* running) {
  // Center window.
  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), false,
                          ImVec2(0.5f, 0.5f));
  if (!ImGui::Begin(m_name, running,
                    ImGuiWindowFlags_AlwaysAutoResize |
                        ImGuiWindowFlags_NoTitleBar |
                        ImGuiWindowFlags_NoDocking)) {
    return;
  }

  m_result = Result::NONE;
  m_recent_project = nullptr;

  ImGui::PushFont(FontLibrary::get().big_font);
  ImGui::Text("Welcome to ThunderAuto");
  ImGui::PopFont();

  ImGui::Text("Version " THUNDER_AUTO_VERSION_STR);

  ImGui::NewLine();

  //
  // New Project.
  //
  {
    ImGuiScopedField field = ImGuiScopedField::Builder("New Project")
                                 .custom_left_column([]() {
                                   ImGui::Text(ICON_FA_FILE "  New Project");
                                   ImGui::SetWindowFontScale(0.75f);
                                   ImGui::Text("Create a new project");
                                   ImGui::SetWindowFontScale(1.0f);
                                 })
                                 .build();

    if (ImGui::Button("New Project")) {
      m_result = Result::NEW_PROJECT;
    }
  }

  ImGui::Spacing();
  ImGui::Spacing();

  //
  // Open Project.
  //
  {
    ImGuiScopedField field =
        ImGuiScopedField::Builder("Open Project")
            .custom_left_column([]() {
              ImGui::Text(ICON_FA_FOLDER_OPEN "  Open Project");
              ImGui::SetWindowFontScale(0.75f);
              ImGui::Text("Open an existing project");
              ImGui::SetWindowFontScale(1.0f);
            })
            .build();

    if (ImGui::Button("Open Project")) {
      m_result = Result::OPEN_PROJECT;
    }
  }

  //
  // Recent Projects.
  //
  if (!m_recent_projects.empty()) {
    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::Text(ICON_FA_CLIPBOARD "  Recent Projects");
    ImGui::PushID("##Recents");
    ImGui::Indent(GET_UISIZE(INDENT_MEDIUM));
    size_t i = 0;
    for (auto& project : m_recent_projects) {
      std::string id =
          std::filesystem::path(project).filename().string() + " - " + project;

      ImGui::PushID(i);
      if (ImGui::Selectable(id.c_str())) {
        m_result = Result::RECENT_PROJECT;
        m_recent_project = &project;
      }
      ImGui::PopID();
      if (++i >= 5)
        break;
    }
    ImGui::Unindent(GET_UISIZE(INDENT_MEDIUM));
    ImGui::PopID();
  }

  ImGui::End();

  if (m_result != Result::NONE) {
    *running = false;
  }
}
