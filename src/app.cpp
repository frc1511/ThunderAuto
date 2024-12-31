#include <ThunderAuto/app.h>

#include <IconsFontAwesome5.h>
#include <ThunderAuto/file_types.h>

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

void App::present() {
  present_menu_bar();

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
      title += " - Unsaved";
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

void App::present_menu_bar() {
  if (ImGui::BeginMenuBar()) {
    present_file_menu();

    if (m_document_manager.is_open()) {
      present_edit_menu();
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

  if (ImGui::BeginMenu("File")) {
    ImGui::MenuItem(ICON_FA_FILE "  New", CTRL_STR "N", &item_new);
    ImGui::MenuItem(ICON_FA_FOLDER_OPEN "  Open", CTRL_STR "O", &item_open);

    if (!m_document_manager.is_open()) {
      ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
      ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
    }

    ImGui::MenuItem(ICON_FA_SAVE "  Save", CTRL_STR "S", &item_save);
    ImGui::MenuItem(ICON_FA_SAVE "  Save As", CTRL_SHIFT_STR "S",
                    &item_save_as);
    ImGui::MenuItem(ICON_FA_WINDOW_CLOSE "  Close", CTRL_STR "W", &item_close);

    if (!m_document_manager.is_open()) {
      ImGui::PopItemFlag();
      ImGui::PopStyleVar();
    }

    ImGui::EndMenu();
  }

  if (item_new) try_change_state(EventState::NEW_PROJECT);
  if (item_open) try_change_state(EventState::OPEN_PROJECT);
  if (item_save) m_document_manager.save();
  if (item_save_as) {
  }
  if (item_close) try_change_state(EventState::CLOSE_PROJECT);
}

void App::present_edit_menu() {
  bool item_undo = false, item_redo = false, item_cut = false,
       item_copy = false, item_paste = false, item_delete = false;

  if (ImGui::BeginMenu("Edit")) {
    ImGui::MenuItem(ICON_FA_UNDO "  Undo", CTRL_STR "Z", &item_undo);
    ImGui::MenuItem(ICON_FA_REDO "  Redo", CTRL_SHIFT_STR "Z", &item_redo);
    ImGui::Separator();
    ImGui::MenuItem(ICON_FA_CUT "  Cut", CTRL_STR "X", &item_cut);
    ImGui::MenuItem(ICON_FA_COPY "  Copy", CTRL_STR "C", &item_copy);
    ImGui::MenuItem(ICON_FA_PASTE "  Paste", CTRL_STR "V", &item_paste);
    ImGui::MenuItem(ICON_FA_TRASH "  Delete", "Delete", &item_delete);

    ImGui::EndMenu();
  }

  if (item_undo) undo();
  if (item_redo) redo();
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
  if (!path.empty()) {
    m_document_manager.open_project(path);
    const ProjectSettings& settings = m_document_manager.settings();
    m_path_editor_page.setup_field(settings);
    m_properties_page.setup(settings);
  }

  m_event_state = EventState::NONE;
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

    } else if (CTRL_DOWN && io.KeyShift &&
               KEY_DOWN(ImGuiKey_S)) { // Ctrl+Shift+S

      std::string path =
          m_platform_manager.save_file_dialog({THUNDERAUTO_FILE_FILTER});
      if (!path.empty()) {
        m_document_manager.save_as(path);
      }

    } else if (CTRL_DOWN && KEY_DOWN(ImGuiKey_W)) { // Ctrl+W
      try_change_state(EventState::CLOSE_PROJECT);

    } else if (CTRL_DOWN && KEY_DOWN_OR_REPEAT(ImGuiKey_Z)) { // Ctrl+Z
      undo();

    } else if ((CTRL_DOWN && KEY_DOWN_OR_REPEAT(ImGuiKey_Y)) || // Ctrl+Y
               (CTRL_SHIFT_DOWN &&
                KEY_DOWN_OR_REPEAT(ImGuiKey_Z))) { // Ctrl+Shift+Z
      redo();

    } else if (CTRL_DOWN && KEY_DOWN(ImGuiKey_X)) { // Ctrl+X
      // TODO: Cut

    } else if (CTRL_DOWN && KEY_DOWN(ImGuiKey_C)) { // Ctrl+C
      // TODO: Copy

    } else if (CTRL_DOWN && KEY_DOWN(ImGuiKey_V)) { // Ctrl+V
      // TODO: Paste
    }
  }
}
