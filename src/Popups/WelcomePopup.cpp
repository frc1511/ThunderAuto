#include <ThunderAuto/Popups/WelcomePopup.hpp>
#include <ThunderAuto/Platform/Platform.hpp>
#include <ThunderAuto/ImGuiScopedField.hpp>
#include <ThunderAuto/Logger.hpp>
#include <IconsFontAwesome5.h>
#include <imgui.h>
#include <imgui_raii.h>

void WelcomePopup::present(bool* running) {
  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), false, ImVec2(0.5f, 0.5f));

  float recentProjectsColumnWidth = GET_UISIZE(WELCOME_POPUP_RECENT_PROJECT_COLUMN_WIDTH);
  float windowWidth = GET_UISIZE(WELCOME_POPUP_WIDTH);

  bool shouldShowRecentProjects = !m_recentProjects.empty();

  if (!shouldShowRecentProjects) {
    // Make window smaller since and aren't showing recent projects.
    windowWidth -= recentProjectsColumnWidth;
    windowWidth -= GET_UISIZE(WELCOME_POPUP_WINDOW_PADDING);
    const ImGuiStyle& style = ImGui::GetStyle();
    windowWidth -= style.ItemSpacing.x * 2.f;
  }

  ImGui::SetNextWindowSizeConstraints(ImVec2(windowWidth, 0), ImVec2(windowWidth, INFINITY));
  ImGui::SetNextWindowSize(ImVec2(windowWidth, GET_UISIZE(WELCOME_POPUP_HEIGHT)));

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(GET_UISIZE(WELCOME_POPUP_WINDOW_PADDING),
                                                          GET_UISIZE(WELCOME_POPUP_WINDOW_PADDING)));

  auto scopedWin = ImGui::Scoped::Window(
      name(), running,
      ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDocking);

  ImGui::PopStyleVar();

  m_result = Result::NONE;
  m_recentProjectIt = std::nullopt;

  if (!scopedWin)
    return;

  if (shouldShowRecentProjects) {
    {
      auto scopedChildWindow =
          ImGui::Scoped::ChildWindow("WindowLeft", ImVec2(recentProjectsColumnWidth, 0.f));

      {
        auto scopedFont = ImGui::Scoped::Font(FontLibrary::get().boldFont, 0.f);
        ImGui::Text("Recent Projects");
        ImGui::Spacing();
      }

      {
        auto scopedChildWindow2 =
            ImGui::Scoped::ChildWindow("RecentProjects", ImVec2(0.f, 0.f), ImGuiChildFlags_Borders);

        size_t i = 0;
        for (auto projectIt = m_recentProjects.begin(); (projectIt != m_recentProjects.end());
             ++projectIt, ++i) {
          const std::filesystem::path& project = *projectIt;

          auto scopedIndexID = ImGui::Scoped::ID(i);

          ImGui::SetNextItemAllowOverlap();
          if (ImGui::Selectable(project.filename().c_str())) {
            ThunderAutoLogger::Info("WelcomePopup: Selected recent project: {}", project.c_str());
            m_result = Result::RECENT_PROJECT;
            m_recentProjectIt = projectIt;
          }

          if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_NoSharedDelay |
                                   ImGuiHoveredFlags_Stationary)) {
            ImGui::SetTooltip("%s", project.c_str());
          }
        }
      }
    }

    ImGui::SameLine();
    ImGui::Dummy(ImVec2(GET_UISIZE(WELCOME_POPUP_WINDOW_PADDING), 0.f));
    ImGui::SameLine();
  }

  {
    auto scopedChildWindow = ImGui::Scoped::ChildWindow("WindowRight");

    {
      auto scopedFont =
          ImGui::Scoped::Font(FontLibrary::get().bigFont, FontLibrary::get().bigFont->LegacySize);
      ImGui::Text("Welcome to ThunderAuto");
    }

    ImGui::Text("Version " THUNDERAUTO_VERSION_STR);

    ImGui::NewLine();

    // New Project.
    {
      auto scopedField = ImGui::ScopedField::Builder("New Project")
                             .customLeftColumn([]() {
                               {
                                 auto scopedFont = ImGui::Scoped::Font(FontLibrary::get().boldFont, 0.f);
                                 ImGui::Text(ICON_FA_FILE "  New Project");
                               }

                               {
                                 const ImGuiStyle& style = ImGui::GetStyle();
                                 auto scopedFont = ImGui::Scoped::Font(NULL, style.FontSizeBase * 0.75f);
                                 ImGui::Text("Create a new project");
                               }
                             })
                             .build();

      if (ImGui::Button("New Project")) {
        m_result = Result::NEW_PROJECT;
      }
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // Open Project.
    {
      auto scopedField = ImGui::ScopedField::Builder("Open Project")
                             .customLeftColumn([]() {
                               {
                                 auto scopedFont = ImGui::Scoped::Font(FontLibrary::get().boldFont, 0.f);
                                 ImGui::Text(ICON_FA_FOLDER_OPEN "  Open Project");
                               }
                               {
                                 const ImGuiStyle& style = ImGui::GetStyle();
                                 auto scopedFont = ImGui::Scoped::Font(NULL, style.FontSizeBase * 0.75f);
                                 ImGui::Text("Open an existing project");
                               }
                             })
                             .build();

      if (ImGui::Button("Open Project")) {
        m_result = Result::OPEN_PROJECT;
      }
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // Documentation
    {
      auto scopedField = ImGui::ScopedField::Builder("Documentation")
                             .customLeftColumn([]() {
                               {
                                 auto scopedFont = ImGui::Scoped::Font(FontLibrary::get().boldFont, 0.f);
                                 ImGui::Text(ICON_FA_BOOK "  Documentation");
                               }
                               {
                                 const ImGuiStyle& style = ImGui::GetStyle();
                                 auto scopedFont = ImGui::Scoped::Font(NULL, style.FontSizeBase * 0.75f);
                                 ImGui::Text("Open the documentation");
                               }
                             })
                             .build();

      if (ImGui::Button("Documentation")) {
        getPlatform().openURL("https://frc1511.github.io/thunderauto");
      }
    }
  }

  if (m_result != Result::NONE) {
    *running = false;
  }
}
