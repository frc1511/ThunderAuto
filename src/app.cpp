#include <ThunderAuto/app.h>

#include <IconsFontAwesome5.h>
#include <ThunderAuto/file_types.h>
#include <ThunderAuto/imgui_util.h>

#include <ThunderAuto/graphics.h>

void App::setup_dockspace(ImGuiID dockspace_id) {
  ImGuiViewport* viewport = ImGui::GetMainViewport();

  bool dockspace_created = ImGui::DockBuilderGetNode(dockspace_id) != nullptr;

  ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

  if (!dockspace_created) {
    puts("Creating dockspace layout");

    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

    auto dockspace_id_right_up = ImGui::DockBuilderSplitNode(
        dockspace_id, ImGuiDir_Right, 0.25f, nullptr, &dockspace_id);

    auto dockspace_id_right_down =
        ImGui::DockBuilderSplitNode(dockspace_id_right_up, ImGuiDir_Down, 0.22f,
                                    nullptr, &dockspace_id_right_up);

    auto dockspace_id_left = ImGui::DockBuilderSplitNode(
        dockspace_id, ImGuiDir_Left, 0.2f, nullptr, &dockspace_id);

    ImGui::DockBuilderDockWindow(m_path_editor_page.name(), dockspace_id);
    ImGui::DockBuilderDockWindow(m_properties_page.name(),
                                 dockspace_id_right_up);
    ImGui::DockBuilderDockWindow(m_actions_page.name(),
                                 dockspace_id_right_down);
    ImGui::DockBuilderDockWindow(m_path_manager_page.name(), dockspace_id_left);
  }
}

void App::focus_was_changed(bool focused) {
  bool auto_export = m_document_manager.settings().auto_export;

  if (focused || !auto_export) return;

  m_export_success =
      m_document_edit_manager.current_state().export_all_paths_to_csv(
          m_document_manager.settings());

  m_export_popup = true;
  m_exported_index = -1;
}

void App::present() {
  present_menu_bar();

  present_export_popup();

  switch (m_event_state) {
    using enum EventState;
  case NONE:
    if (m_document_manager.is_open()) break;

    m_event_state = WELCOME;
    [[fallthrough]];
  case WELCOME:
    welcome();
    break;
  case NEW_PROJECT:
    new_project();
    break;
  case NEW_FIELD:
    new_field();
    break;
  case OPEN_PROJECT:
    open_project();
    break;
  case CLOSE_PROJECT:
    m_document_manager.close();
    m_event_state = WELCOME;
    break;
  case CLOSE_EVERYTHING:
    m_running = false;
    break;
  case UNSAVED:
    unsaved();
    break;
  }

  if (m_document_manager.is_open()) {
    std::string title(m_document_manager.name());
    if (m_document_manager.is_unsaved()) {
      if (m_document_manager.settings().auto_save) {
        m_document_manager.save();
      } else {
        title += " - Unsaved";
      }
    }
    Graphics::get().window_set_title(title.c_str());

  } else {
    Graphics::get().window_set_title("ThunderAuto");
  }
}

void App::close() {
  if (try_change_state(EventState::CLOSE_EVERYTHING)) {
    m_document_manager.close();
  }
}

void App::data_clear() { m_recent_projects.clear(); }

bool App::data_should_open(const char* name) {
  return strcmp(name, "RecentProjects") == 0;
}

void App::data_read_line(const char* line) {
  assert(line);
  if (line[0] == '\0') return;

  m_recent_projects.push_back(line);
}

void App::data_apply() {}

void App::data_write(const char* type_name, ImGuiTextBuffer* buf) {
  buf->appendf("[%s][%s]\n", type_name, "RecentProjects");

  size_t i = 0;
  for (const std::string& project : m_recent_projects) {
    buf->appendf("%s\n", project.c_str());
    if (++i >= 5) break;
  }

  buf->append("\n");
}

void App::present_menu_bar() {
  if (ImGui::BeginMenuBar()) {
    present_file_menu();

    if (m_document_manager.is_open()) {
      present_edit_menu();
      present_view_menu();
      present_path_menu();

      if (ImGui::MenuItem("Export All Paths")) {
        m_export_success =
            m_document_edit_manager.current_state().export_all_paths_to_csv(
                m_document_manager.settings());

        m_export_popup = true;
        m_exported_index = -1;
      }

      present_tools_menu();
    }

    ImGui::EndMenuBar();
  }
}

#ifdef TH_MACOS
#define CTRL_STR       "Cmd+"
#define CTRL_SHIFT_STR "Cmd+Shift+"
#else
#define CTRL_STR       "Ctrl+"
#define CTRL_SHIFT_STR "Ctrl+Shift+"
#endif

void App::present_file_menu() {
  bool item_new = false, item_open = false, item_save = false,
       item_save_as = false, item_close = false;

  ProjectSettings& settings = m_document_manager.settings();

  if (ImGui::BeginMenu("File")) {
    ImGui::MenuItem(ICON_FA_FILE "  New", CTRL_STR "N", &item_new);
    ImGui::MenuItem(ICON_FA_FOLDER_OPEN "  Open", CTRL_STR "O", &item_open);

    {
      ImGuiScopedDisabled disabled(!m_document_manager.is_open());

      ImGui::MenuItem(ICON_FA_SAVE "  Save", CTRL_STR "S", &item_save);
      ImGui::MenuItem(ICON_FA_SAVE "  Save As", CTRL_SHIFT_STR "S",
                      &item_save_as);

      ImGui::Separator();

      if (ImGui::MenuItem(ICON_FA_SAVE "  Auto Save", nullptr, &settings.auto_save)) {
        if (settings.auto_save) {
          m_document_manager.save();
        }
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Automatically save project\n"
                          "when changes are made");
      }

      if (ImGui::MenuItem(ICON_FA_FILE_CSV "  Auto Export", nullptr,
                      &settings.auto_export)) {
        if (settings.auto_export) {
          m_document_manager.history()->mark_unsaved();

          m_export_success =
              m_document_edit_manager.current_state().export_all_paths_to_csv(
                  settings);

          m_export_popup = true;
          m_exported_index = -1;
        }
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Automatically export all paths\n"
                          "when focus is lost");
      }

      ImGui::Separator();

      ImGui::MenuItem(ICON_FA_WINDOW_CLOSE "  Close", CTRL_STR "W",
                      &item_close);
    }

    ImGui::EndMenu();
  }

  if (item_new) try_change_state(EventState::NEW_PROJECT);
  if (item_open) try_change_state(EventState::OPEN_PROJECT);
  if (item_save) m_document_manager.save();
  if (item_save_as) save_as();
  if (item_close) try_change_state(EventState::CLOSE_PROJECT);
}

void App::present_edit_menu() {
  if (ImGui::BeginMenu("Edit")) {
    if (ImGui::MenuItem(ICON_FA_UNDO "  Undo", CTRL_STR "Z")) {
      undo();
    }
    if (ImGui::MenuItem(ICON_FA_REDO "  Redo", CTRL_SHIFT_STR "Z")) {
      redo();
    }

    ImGui::Separator();

    if (ImGui::MenuItem(ICON_FA_TRASH "  Delete", "Delete")) {
    }

    ImGui::EndMenu();
  }
}

void App::present_view_menu() {
  if (ImGui::BeginMenu("View")) {
    if (ImGui::MenuItem(ICON_FA_REDO "  Reset View", CTRL_STR "0")) {
      m_path_editor_page.reset_view();
    }
    ImGui::EndMenu();
  }
}

void App::present_path_menu() {
  bool item_export = false, item_reverse = false, item_duplicate = false,
       item_delete = false;

  ProjectState state = m_document_manager.history()->current_state();

  if (ImGui::BeginMenu("Path")) {
    ImGui::MenuItem(ICON_FA_FILE_CSV "  Export to CSV", nullptr, &item_export);

    ImGui::MenuItem("\xef\x8d\xa3"
                    "  Reverse Direction",
                    nullptr, &item_reverse);
    ImGui::MenuItem(ICON_FA_COPY "  Duplicate", nullptr, &item_duplicate);
    {
      ImGuiScopedDisabled disabled(state.paths().size() <= 1);

      ImGui::MenuItem(ICON_FA_TRASH_ALT "  Delete", nullptr, &item_delete);
    }
    ImGui::EndMenu();
  }

  if (item_export) {
    m_export_success =
        m_document_manager.history()
            ->current_state()
            .export_current_path_to_csv(m_document_manager.settings());

    m_export_popup = true;
    m_exported_index = state.current_path_index();
  }

  if (item_reverse) {
    m_path_manager_page.reverse_path(state, state.current_path_index());
  }

  if (item_duplicate) {
    m_path_manager_page.duplicate_path(state, state.current_path_index());
  }

  if (item_delete) {
    m_path_manager_page.delete_path(state, state.current_path_index());
  }
}

void App::present_tools_menu() {
  static bool show_path_editor = true, show_path_manager = true,
              show_properties = true, show_actions = true;

  if (ImGui::BeginMenu("Tools")) {
    ImGui::MenuItem(ICON_FA_LIST "  Paths", nullptr, &show_path_manager);
    ImGui::MenuItem(ICON_FA_BEZIER_CURVE "  Editor", nullptr,
                    &show_path_editor);
    ImGui::MenuItem(ICON_FA_SLIDERS_H "  Properties", nullptr,
                    &show_properties);
    ImGui::MenuItem(ICON_FA_COG "  Actions", nullptr, &show_actions);

    ImGui::EndMenu();
  }

  if (show_actions) m_actions_page.present(&show_actions);
  if (show_properties) m_properties_page.present(&show_properties);
  if (show_path_manager) m_path_manager_page.present(&show_path_manager);
  if (show_path_editor) m_path_editor_page.present(&show_path_editor);
}

void App::present_export_popup() {
  if (m_export_popup) {
    ImGui::OpenPopup("Export Done");
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), false,
                            ImVec2(0.5f, 0.5f));
    m_export_popup = false;
  }

  if (ImGui::BeginPopupModal("Export Done", nullptr,
                             ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_NoMove)) {
    ImGui::SetWindowSize(ImVec2(-1.f, -1.f));

    const std::vector<std::pair<std::string, Curve>>& paths =
        m_document_manager.history()->current_state().paths();

    if (!m_export_success)
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));

    ImGui::Text("%s CSV file(s):",
                m_export_success ? "Successfully exported" : "FAILED to export");

    if (!m_export_success) ImGui::PopStyleColor();

    ImGui::Indent(10.0);

    if (m_exported_index < 0 || size_t(m_exported_index) >= paths.size()) {
      for (size_t i = 0; i < paths.size(); ++i) {
        const char* name = paths.at(i).first.c_str();
        ImGui::Text("%s.csv", name);
      }
    } else {
      const char* name = paths.at(m_exported_index).first.c_str();
      ImGui::Text("%s.csv", name);
    }

    ImGui::Unindent(10.0);

    if (ImGui::Button("Ok") || ImGui::IsKeyPressed(ImGuiKey_Escape) ||
        ImGui::IsKeyPressed(ImGuiKey_Enter) ||
        ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }
}

bool App::try_change_state(EventState desired_state) {
  if (m_document_manager.is_unsaved()) {
    m_event_state = EventState::UNSAVED;
    m_next_event_state = desired_state;

    return false;
  }

  if (m_event_state == EventState::CLOSE_EVERYTHING) {
    return false;
  }

  if (m_document_manager.is_open()) m_document_manager.close();

  m_event_state = desired_state;
  return true;
}

void App::welcome() {
  ImGui::OpenPopup(m_welcome_popup.name());

  bool showing_popup = true;

  m_welcome_popup.present(&showing_popup);

  if (showing_popup) return;

  WelcomePopup::Result result = m_welcome_popup.result();

  switch (result) {
    using enum WelcomePopup::Result;
  case NEW_PROJECT:
    m_event_state = EventState::NEW_PROJECT;
    break;
  case OPEN_PROJECT:
    m_event_state = EventState::OPEN_PROJECT;
    break;
  case RECENT_PROJECT: {
    std::string* recent_path = m_welcome_popup.recent_project();
    assert(recent_path);
    open_from_path(*recent_path);
    break;
  }
  default:
    assert(false);
  }
}

void App::new_project() {
  ImGui::OpenPopup(m_new_project_popup.name());

  bool showing_popup = true;

  m_new_project_popup.present(&showing_popup);

  if (showing_popup) return;

  NewProjectPopup::Result result = m_new_project_popup.result();

  m_event_state = EventState::NONE;

  switch (result) {
    using enum NewProjectPopup::Result;
  case NEW_FIELD:
    m_event_state = EventState::NEW_FIELD;
    break;
  case CREATE:
    m_document_manager.new_project(m_new_project_popup.result_project());
    m_path_editor_page.setup_field(m_document_manager.settings());
    m_properties_page.setup(m_document_manager.settings());

    m_recent_projects.push_front(m_document_manager.path().string());
    break;
  case CANCEL:
    break;
  default:
    assert(false);
  }
}

void App::new_field() {
  ImGui::OpenPopup(m_new_field_popup.name());

  bool showing_popup = true;

  m_new_field_popup.present(&showing_popup);

  if (showing_popup) return;

  NewFieldPopup::Result result = m_new_field_popup.result();

  m_event_state = EventState::NEW_PROJECT;

  switch (result) {
    using enum NewFieldPopup::Result;
  case CREATE:
    m_new_project_popup.set_field(m_new_field_popup.field());
    break;
  case CANCEL:
    break;
  default:
    assert(false);
  }
}

void App::open_project() {
  std::string path = m_platform_manager.open_file_dialog(
      FileType::FILE, {THUNDERAUTO_FILE_FILTER});

  open_from_path(path);
}

void App::open_from_path(std::string path) {
  for (auto it = m_recent_projects.begin(); it != m_recent_projects.end();) {
    if (*it == path) {
      it = m_recent_projects.erase(it);
    } else {
      ++it;
    }
  }

  m_event_state = EventState::NONE;

  if (path.empty()) return;
  if (!std::filesystem::exists(path) || !std::filesystem::is_regular_file(path))
    return;

  m_document_manager.open_project(path);
  const ProjectSettings& settings = m_document_manager.settings();
  m_path_editor_page.setup_field(settings);
  m_properties_page.setup(settings);

  m_recent_projects.push_front(path);
}

void App::save_as() {
  std::string path =
      m_platform_manager.save_file_dialog({THUNDERAUTO_FILE_FILTER});
  if (!path.empty()) {
    m_document_manager.save_as(path);
  }
}

void App::unsaved() {
  if (!m_document_manager.is_unsaved()) {
    m_event_state = m_next_event_state;
    m_next_event_state = EventState::NONE;
    return;
  }

  ImGui::OpenPopup(m_unsaved_popup.name());

  bool showing_popup = true;

  m_unsaved_popup.present(&showing_popup);

  if (showing_popup) return;

  UnsavedPopup::Result result = m_unsaved_popup.result();

  switch (result) {
    using enum UnsavedPopup::Result;
  case SAVE:
    m_document_manager.save();
    m_document_manager.close();
    break;
  case DONT_SAVE:
    m_document_manager.close();
    break;
  case CANCEL:
    m_event_state = m_next_event_state = EventState::NONE;
    Graphics::get().window_set_should_close(false);
    break;
  default:
    assert(false);
  }
}

void App::undo() {
  m_document_manager.undo();

  m_document_edit_manager.current_state().current_path().output(
      m_cached_curve, preview_output_curve_settings);
}

void App::redo() {
  m_document_manager.redo();

  m_document_edit_manager.current_state().current_path().output(
      m_cached_curve, preview_output_curve_settings);
}

#if TH_MACOS
#define CTRL_KEY (io.KeySuper)
#else
#define CTRL_KEY (io.KeyCtrl)
#endif

#define CTRL_DOWN       (CTRL_KEY && !io.KeyAlt && !io.KeyShift)
#define CTRL_SHIFT_DOWN (CTRL_KEY && !io.KeyAlt && io.KeyShift)

#define KEY_DOWN(key) (io.KeysData[key].DownDuration == 0.0f)

#define KEY_DOWN_OR_REPEAT(key) ImGui::IsKeyPressed(key, true)

void App::process_input() {
  const ImGuiIO& io = ImGui::GetIO();

  if (CTRL_DOWN && KEY_DOWN(ImGuiKey_N)) { // Ctrl+N
    try_change_state(EventState::NEW_PROJECT);

  } else if (CTRL_DOWN && KEY_DOWN(ImGuiKey_O)) { // Ctrl+O
    try_change_state(EventState::OPEN_PROJECT);

  } else if (m_document_manager.is_open()) {
    if (CTRL_DOWN && KEY_DOWN(ImGuiKey_S)) { // Ctrl+S
      m_document_manager.save();

    } else if (CTRL_SHIFT_DOWN && KEY_DOWN(ImGuiKey_S)) { // Ctrl+Shift+S
      save_as();

    } else if (CTRL_DOWN && KEY_DOWN(ImGuiKey_W)) { // Ctrl+W
      try_change_state(EventState::CLOSE_PROJECT);

    } else if (CTRL_DOWN && KEY_DOWN_OR_REPEAT(ImGuiKey_Z)) { // Ctrl+Z
      undo();

    } else if ((CTRL_DOWN && KEY_DOWN_OR_REPEAT(ImGuiKey_Y)) || // Ctrl+Y
               (CTRL_SHIFT_DOWN &&
                KEY_DOWN_OR_REPEAT(ImGuiKey_Z))) { // Ctrl+Shift+Z
      redo();

    } else if (CTRL_DOWN &&
               (KEY_DOWN(ImGuiKey_E) || KEY_DOWN(ImGuiKey_Apostrophe))) {
      m_export_success =
          m_document_edit_manager.current_state().export_all_paths_to_csv(
              m_document_manager.settings());

      m_export_popup = true;
      m_exported_index = -1;

    } else if (CTRL_DOWN && KEY_DOWN(ImGuiKey_0)) { // Ctrl+0
      m_path_editor_page.reset_view();

    } else if (m_path_editor_page.is_focused()) {
      if (KEY_DOWN(ImGuiKey_Tab)) {
        ProjectState state = m_document_edit_manager.current_state();
        if (io.KeyShift) {
          m_path_editor_page.select_previous_point(state);
        } else {
          m_path_editor_page.select_next_point(state);
        }

      } else if (KEY_DOWN(ImGuiKey_Space)) {
        m_path_editor_page.toggle_playback();
      }
    }
  }
}

