#include <ThunderAuto/app.h>
#include <imgui_internal.h>
#include <IconsFontAwesome5.h>

#include <ThunderAuto/pages/path_editor.h>
#include <ThunderAuto/pages/path_manager.h>
#include <ThunderAuto/pages/properties.h>
#include <ThunderAuto/pages/settings.h>
#include <ThunderAuto/popups/new_project.h>
#include <ThunderAuto/popups/unsaved.h>
#include <ThunderAuto/popups/new_field.h>
#include <ThunderAuto/popups/welcome.h>

App::App() {
}

App::~App() { }

void App::init(GLFWwindow* win) {
    window = win;
}

void App::present() {
  bool item_new = false,
       item_open = false,
       item_save = false,
       item_save_as = false,
       item_close = false,
       item_undo = false,
       item_redo = false,
       item_cut = false,
       item_copy = false,
       item_paste = false,
       item_delete = false;

  static bool show_path_editor = true,
              show_path_manager = true,
              show_properties = true;

  static bool show_settings = false;

#ifdef THUNDER_AUTO_MACOS
# define CTRL_STR "Cmd+"
# define CTRL_SHIFT_STR "Cmd+Shift+"
#else
# define CTRL_STR "Ctrl+"
# define CTRL_SHIFT_STR "Ctrl+Shift+"
#endif
  
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      ImGui::MenuItem(ICON_FA_FILE "  New",     CTRL_STR "N",       &item_new);
      ImGui::MenuItem(ICON_FA_FOLDER_OPEN "  Open",    CTRL_STR "O",       &item_open);
      
      if (!ProjectManager::get()->has_project()) {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
      }
      
      ImGui::MenuItem(ICON_FA_SAVE "  Save",    CTRL_STR "S",       &item_save);
      ImGui::MenuItem(ICON_FA_SAVE "  Save As", CTRL_SHIFT_STR "S", &item_save_as);
      ImGui::MenuItem(ICON_FA_WINDOW_CLOSE "  Close",   CTRL_STR "W",       &item_close);
      
      if (!ProjectManager::get()->has_project()) {
        ImGui::PopItemFlag();
        ImGui::PopStyleVar();
      }
      
      ImGui::EndMenu();
    }
    
    if (ProjectManager::get()->has_project()) {
      if (ImGui::BeginMenu("Edit")) {
        ImGui::MenuItem(ICON_FA_UNDO "  Undo",       CTRL_STR "Z",       &item_undo);
        ImGui::MenuItem(ICON_FA_REDO "  Redo",       CTRL_SHIFT_STR "Z", &item_redo);
        ImGui::Separator();
        ImGui::MenuItem(ICON_FA_CUT "  Cut",        CTRL_STR "X", &item_cut);
        ImGui::MenuItem(ICON_FA_COPY "  Copy",       CTRL_STR "C", &item_copy);
        ImGui::MenuItem(ICON_FA_PASTE "  Paste",      CTRL_STR "V", &item_paste);
        ImGui::MenuItem(ICON_FA_TRASH "  Delete",     "Delete", &item_delete);
        
        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Tools")) {
        ImGui::MenuItem(ICON_FA_LIST "  Paths", nullptr, &show_path_manager);
        ImGui::MenuItem(ICON_FA_BEZIER_CURVE "  Editor", nullptr, &show_path_editor);
        ImGui::MenuItem(ICON_FA_SLIDERS_H "  Properties", nullptr, &show_properties);
        if (ImGui::MenuItem(ICON_FA_COG "  Settings", nullptr, &show_settings)) {
          SettingsPage::get()->reset();
        }
        
        ImGui::EndMenu();
      }
      
    }
    
    ImGui::EndMenuBar();
  }

  if (item_new)        menu_new();
  if (item_open)       menu_open();
  if (item_save)       menu_save();
  if (item_save_as)    menu_save_as();
  if (item_close)      menu_close();
  if (item_undo)       menu_undo();
  if (item_redo)       menu_redo();
  if (item_cut)        menu_cut();
  if (item_copy)       menu_copy();
  if (item_paste)      menu_paste();
  if (item_delete)     menu_delete(true);
  
  if (ProjectManager::get()->has_project()) {
    if (show_path_editor) PathEditorPage::get()->present(&show_path_editor);
    if (show_path_manager) PathManagerPage::get()->present(&show_path_manager);
    if (show_properties) PropertiesPage::get()->present(&show_properties);
    if (show_settings) SettingsPage::get()->present(&show_settings);
  }

  switch (event_state) {
    case EventState::NONE:
      break;
    case EventState::WELCOME:
      welcome();
      break;
    case EventState::NEW_PROJECT:
      new_project();
      break;
    case EventState::NEW_PROJECT_UNSAVED_OPENED:
      unsaved(EventState::NEW_PROJECT);
      break;
    case EventState::OPEN_PROJECT:
      open_project();
      break;
    case EventState::OPEN_PROJECT_UNSAVED_OPENED:
      unsaved(EventState::OPEN_PROJECT);
      break;
    case EventState::CLOSE_PROJECT:
      ProjectManager::get()->close_project();
      event_state = EventState::WELCOME;
      break;
    case EventState::CLOSE_PROJECT_UNSAVED:
      unsaved(EventState::CLOSE_PROJECT);
      break;
    case EventState::CLOSE_EVERYTHING:
      running = false;
      break;
    case EventState::CLOSE_EVERYTHING_UNSAVED:
      unsaved(EventState::CLOSE_EVERYTHING);
      break;
  }
}

void App::welcome() {
  bool showing_popup = true;

  if (WelcomePopup::get()->is_showing_new_project_popup()) {
    new_project();

    if (event_state == EventState::NONE) {
      if (ProjectManager::get()->has_project()) {
        showing_popup = false;
      }
      else {
        event_state = EventState::WELCOME;
      }
      WelcomePopup::get()->set_showing_new_project_popup(false);
    }
  }
  else if (WelcomePopup::get()->is_opening_project()) {
    open_project();
    if (ProjectManager::get()->has_project()) {
      showing_popup = false;
    }
    else {
      event_state = EventState::WELCOME;
    }
    WelcomePopup::get()->set_opening_project(false);
  }
  else {
    ImGui::OpenPopup(WelcomePopup::get()->get_name().c_str());

    WelcomePopup::get()->present(&showing_popup);

    if (!showing_popup) {
      event_state = EventState::NONE;
    }
  }
}

void App::new_project() {
  bool showing_popup = true;

  if (NewProjectPopup::get()->is_showing_new_field_popup()) {
    ImGui::OpenPopup(NewFieldPopup::get()->get_name().c_str());

    NewFieldPopup::get()->present(&showing_popup);

    if (!showing_popup) {
      NewProjectPopup::get()->set_is_showing_new_field_popup(false);
    }
  }
  else {
    ImGui::OpenPopup(NewProjectPopup::get()->get_name().c_str());

    NewProjectPopup::get()->present(&showing_popup);

    if (!showing_popup) {
      std::optional<ProjectSettings> settings = NewProjectPopup::get()->get_project_settings();

      if (settings) {
        ProjectManager::get()->new_project(settings.value());
      }

      event_state = EventState::NONE;
    }
  }
}

void App::open_project() {
  std::string path = Platform::get_current()->open_file_dialog(FileType::FILE, FILE_FILTER);
  if (path.empty()) {
  }
  else {
    ProjectManager::get()->open_project(path);
  }

  event_state = EventState::NONE;
}

void App::unsaved(EventState next) {
  ImGui::OpenPopup(UnsavedPopup::get()->get_name().c_str());

  bool showing_popup = true;
  UnsavedPopup::get()->present(&showing_popup);

  if (!showing_popup) {
    switch (UnsavedPopup::get()->get_result()) {
      case UnsavedPopup::CANCEL:
        event_state = EventState::NONE;
        break;
      case UnsavedPopup::SAVE: {
        ProjectManager::get()->save_project();
        event_state = next;
        break;
      }
      case UnsavedPopup::DONT_SAVE:
        ProjectManager::get()->close_project();
        event_state = next;
        break;
    }
  }
}

void App::menu_new() {
  if (event_state == EventState::WELCOME) {
    WelcomePopup::get()->set_showing_new_project_popup(true);
  }
  else if (event_state == EventState::NONE) {
    std::cout << "new\n";
    if (ProjectManager::get()->is_unsaved()) {
      event_state = EventState::NEW_PROJECT_UNSAVED_OPENED;
    }
    else {
      event_state = EventState::NEW_PROJECT;
    }
  }
}

void App::menu_open() {
  if (event_state == EventState::WELCOME) {
    WelcomePopup::get()->set_opening_project(true);
  }
  else if (event_state == EventState::NONE) {
    std::cout << "open\n";
    if (ProjectManager::get()->is_unsaved()) {
      event_state = EventState::OPEN_PROJECT_UNSAVED_OPENED;
    }
    else {
      event_state = EventState::OPEN_PROJECT;
    }
  }
}

void App::menu_save() {
  ProjectManager::get()->save_project();
}

void App::menu_save_as() {
  std::string path = Platform::get_current()->save_file_dialog(FILE_FILTER);
  if (path.empty()) return;
  ProjectManager::get()->save_project_as(path);
}

void App::menu_close() {
  if (event_state == EventState::NONE) {
    if (ProjectManager::get()->is_unsaved()) {
      event_state = EventState::CLOSE_PROJECT_UNSAVED;
    }
    else {
      event_state = EventState::CLOSE_PROJECT;
    }
  }
}

void App::menu_undo() {
  std::cout << "undo\n";
}

void App::menu_redo() {
  std::cout << "redo\n";
}

void App::menu_cut() {
  std::cout << "cut\n";
}

void App::menu_copy() {
  std::cout << "copy\n";
}

void App::menu_paste() {
  std::cout << "paste\n";
}

void App::menu_delete(bool from_menu) {
  if (from_menu || PathEditorPage::get()->is_focused()) {
    PathEditorPage::get()->delete_point();
  }
}

void App::close() {
  if (ProjectManager::get()->is_unsaved()) {
    event_state = EventState::CLOSE_EVERYTHING_UNSAVED;
  }
  else {
    event_state = EventState::CLOSE_EVERYTHING;
  }
}

void App::handle_keyboard(int key, int scancode, int action, int mods) {
#ifdef THUNDER_AUTO_MACOS
  int ctrl_key = GLFW_MOD_SUPER;
#else
  int ctrl_key = GLFW_MOD_CONTROL;
#endif
  
#define GET_CTRL_KEY(k)       ((key == k) && (mods == ctrl_key) && (action == GLFW_PRESS))
#define GET_CTRL_SHIFT_KEY(k) ((key == k) && (mods == (ctrl_key | GLFW_MOD_SHIFT)) && (action == GLFW_PRESS))
  
  if (GET_CTRL_KEY(GLFW_KEY_N)) {
    menu_new();
  }
  else if (GET_CTRL_KEY(GLFW_KEY_O)) {
    menu_open();
  }
  else if (ProjectManager::get()->has_project()) {
    if (GET_CTRL_KEY(GLFW_KEY_S)) {
      menu_save();
    }
    else if (GET_CTRL_SHIFT_KEY(GLFW_KEY_S)) {
      menu_save_as();
    }
    else if (GET_CTRL_KEY(GLFW_KEY_W)) {
      menu_close();
    }
    else if (GET_CTRL_KEY(GLFW_KEY_Z)) {
      menu_undo();
    }
    else if (GET_CTRL_SHIFT_KEY(GLFW_KEY_Z) || GET_CTRL_KEY(GLFW_KEY_Y)) {
      menu_redo();
    }
    else if (GET_CTRL_KEY(GLFW_KEY_X)) {
      menu_cut();
    }
    else if (GET_CTRL_KEY(GLFW_KEY_C)) {
      menu_copy();
    }
    else if (GET_CTRL_KEY(GLFW_KEY_V)) {
      menu_paste();
    }
    else if ((key == GLFW_KEY_DELETE || key == GLFW_KEY_BACKSPACE) && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
      menu_delete(false);
    }
  }
}

App App::instance {};
