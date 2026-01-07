#include <ThunderAuto/Popups/RecursiveActionErrorPopup.hpp>
#include <ThunderAuto/FontLibrary.hpp>
#include <imgui.h>
#include <imgui_raii.h>
#include <fmt/ranges.h>
#include <IconsLucide.h>

void RecursiveActionErrorPopup::present(bool* running) {
  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), false, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(ImVec2(GET_UISIZE(RECURSIVE_ACTION_ERROR_POPUP_START_WIDTH),
                                  GET_UISIZE(RECURSIVE_ACTION_ERROR_POPUP_START_HEIGHT)),
                           ImGuiCond_FirstUseEver);

  auto scopedPopup = ImGui::Scoped::PopupModal(name(), nullptr, ImGuiWindowFlags_NoMove);
  if (!scopedPopup || !*running) {
    return;
  }

  {
    auto scopedFont = ImGui::Scoped::Font(FontLibrary::get().boldFont, 0.f);
    ImGui::Text("Operation not allowed");
  }

  ImGui::TextWrapped(
      "Action '%s' cannot be added to the group '%s' because that would create a cycle in which '%s' calls "
      "itself",
      m_actionToAddName.c_str(), m_groupActionName.c_str(), m_actionToAddName.c_str());

  ImGui::Spacing();

  ImVec2 regionAvail = ImGui::GetContentRegionAvail();
  ImVec2 textSize = ImVec2(regionAvail.x, ImGui::GetTextLineHeight() * 2.f);

  ImGui::InputTextMultiline("##Path", const_cast<char*>(m_recursionPath.c_str()), m_recursionPath.size() + 1,
                            textSize, ImGuiInputTextFlags_ReadOnly);

  ImGui::Spacing();

  if (ImGui::Button("Ok") || ImGui::IsKeyPressed(ImGuiKey_Escape) ||
      ImGui::IsKeyPressed(ImGuiKey_Enter)) {
    *running = false;
  }
}

void RecursiveActionErrorPopup::setActionRecursionPath(const std::list<std::string>& recursionPath) noexcept {
  m_recursionPath.clear();

  if (recursionPath.empty()) {
    m_actionToAddName.clear();
    return;
  }

  m_actionToAddName = recursionPath.front();
  m_recursionPath = fmt::format("{}", fmt::join(recursionPath, " " ICON_LC_MOVE_RIGHT " "));
}

void RecursiveActionErrorPopup::setGroupAction(const std::string& actionName) noexcept {
  m_groupActionName = actionName;
}

