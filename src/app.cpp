#include <app.h>
#include <imgui.h>
#include <imgui_internal.h>

#include <iostream>

#ifdef THUNDER_AUTO_MACOS
# include <platform/macos/macos.h>
#elif THUNDER_AUTO_WINDOWS
# include <platform/windows/windows.h>
#elif THUNDER_AUTO_LINUX
# include <platform/linux/linux.h>
#endif

#include <pages/path_editor.h>
#include <pages/path_selector.h>
#include <pages/properties.h>
#include <popups/new_project.h>
#include <popups/unsaved.h>

App::App() {
#ifdef THUNDER_AUTO_MACOS
  platform = PlatformMacOS::get();
#elif THUNDER_AUTO_WINDOWS
  platform = PlatformWindows::get();
#elif THUNDER_AUTO_LINUX
  platform = PlatformLinux::get();
#endif
}

App::~App() { }

void App::present() {
  static bool demo_window = true;
  if (demo_window)
    ImGui::ShowDemoWindow(&demo_window);
  
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
       item_select_all = false;
  
  static bool show_path_editor = true,
              show_path_selector = true,
              show_properties = true;
  
  if (close_priority == ClosePriority::CLOSE_PROJECT && !show_path_editor) {
    close_priority = ClosePriority::DONT_CLOSE;
  }
  
  if (close_priority != ClosePriority::DONT_CLOSE && !show_unsaved_popup) {
    if (close_priority == ClosePriority::CLOSE_EVERYTHING) {
      running = false;
    }
    else {
      project_settings = {};
    }
  }
  
#ifdef THUNDER_PATH_MACOS
# define CTRL_STR "Cmd+"
# define CTRL_SHIFT_STR "Cmd+Shift+"
#else
# define CTRL_STR "Ctrl+"
# define CTRL_SHIFT_STR "Ctrl+Shift+"
#endif
  
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      ImGui::MenuItem("New",     CTRL_STR "N",       &item_new);
      ImGui::MenuItem("Open",    CTRL_STR "O",       &item_open);
      
      if (!project_settings) {
        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
      }
      
      ImGui::MenuItem("Save",    CTRL_STR "S",       &item_save);
      ImGui::MenuItem("Save As", CTRL_SHIFT_STR "S", &item_save_as);
      ImGui::MenuItem("Close",   CTRL_STR "W",       &item_close);
      
      if (!project_settings) {
        ImGui::PopItemFlag();
        ImGui::PopStyleVar();
      }
      
      ImGui::EndMenu();
    }
    
    if (project_settings) {
      if (ImGui::BeginMenu("Edit")) {
        ImGui::MenuItem("Undo",       CTRL_STR "Z",       &item_undo);
        ImGui::MenuItem("Redo",       CTRL_SHIFT_STR "Z", &item_redo);
        ImGui::Separator();
        ImGui::MenuItem("Cut",        CTRL_STR "X", &item_cut);
        ImGui::MenuItem("Copy",       CTRL_STR "C", &item_copy);
        ImGui::MenuItem("Paste",      CTRL_STR "V", &item_paste);
        ImGui::MenuItem("Select All", CTRL_STR "A", &item_select_all);
        
        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Tools")) {
        ImGui::MenuItem("Editor", nullptr, &show_path_editor);
        ImGui::MenuItem("Paths", nullptr, &show_path_selector);
        ImGui::MenuItem("Properties", nullptr, &show_properties);
        
        ImGui::EndMenu();
      }
      
    }
    
    ImGui::EndMenuBar();
  }
  
  if (!project_settings) {
    show_path_editor = false;
    show_path_selector = false;
    show_properties = false;
  }
  
  if (item_new) {
    menu_new();
  }
  if (item_open) {
    menu_open();
  }
  if (item_save) {
    menu_save();
  }
  if (item_save_as) {
    menu_save_as();
  }
  if (item_close) {
    menu_close();
  }
  if (item_undo) {
    menu_undo();
  }
  if (item_redo) {
    menu_redo();
  }
  if (item_cut) {
    menu_cut();
  }
  if (item_copy) {
    menu_copy();
  }
  if (item_paste) {
    menu_paste();
  }
  if (item_select_all) {
    menu_select_all();
  }
  
  if (show_path_editor) {
    PathEditorPage::get()->present(&show_path_editor);
    PathEditorPage::get()->set_unsaved(true);
  }
  if (show_path_selector) {
    PathSelectorPage::get()->present(&show_path_selector);
  }
  if (show_properties) {
    PropertiesPage::get()->present(&show_properties);
  }
  
  bool was_showing_unsaved_popup = show_unsaved_popup,
       was_shoing_new_project_popup = show_new_project_popup;
  
  if (show_unsaved_popup) {
    ImGui::OpenPopup(UnsavedPopup::get()->get_name().c_str());
  }
  else if (show_new_project_popup) {
    ImGui::OpenPopup(NewProjectPopup::get()->get_name().c_str());
  }
  
  UnsavedPopup::get()->present(&show_unsaved_popup);
  NewProjectPopup::get()->present(&show_new_project_popup);
  
  if (show_unsaved_popup != was_showing_unsaved_popup) {
    switch (UnsavedPopup::get()->get_result()) {
      case UnsavedPopup::CANCEL:
        close_priority = ClosePriority::DONT_CLOSE;
        show_new_project_popup = false;
        break;
      case UnsavedPopup::SAVE:
        // TODO Save document.
        project_settings = {};
        break;
      case UnsavedPopup::DONT_SAVE:
        project_settings = {};
        break;
    }
  }
  
  if (show_new_project_popup != was_shoing_new_project_popup) {
    project_settings = NewProjectPopup::get()->get_project_settings();
    if (project_settings) {
      // TODO Create the new document.
      
      show_path_editor = true;
      show_path_selector = true;
      show_properties = true;
    }
  }
}

void App::menu_new() {
  std::cout << "new\n";
  close_priority = ClosePriority::CLOSE_PROJECT;
  show_new_project_popup = true;
  if (project_settings && PathEditorPage::get()->is_unsaved()) {
    show_unsaved_popup = true;
  }
}

void App::menu_open() {
  std::cout << "open\n";
  std::cout << platform->open_file_dialog() << '\n';
}

void App::menu_save() {
  std::cout << "save\n";
}

void App::menu_save_as() {
  std::cout << "save as\n";
  std::cout << platform->save_file_dialog() << '\n';
}

void App::menu_close() {
  std::cout << "closing project\n";
  close_priority = ClosePriority::CLOSE_PROJECT;
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

void App::menu_select_all() {
  std::cout << "select all\n";
}

void App::close() {
  static bool closing = false;
  if (!closing) {
    std::cout << "closing everything\n";
    close_priority = ClosePriority::CLOSE_EVERYTHING;
    closing = true;
    if (project_settings && PathEditorPage::get()->is_unsaved()) {
      show_unsaved_popup = true;
    }
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
  else if (project_settings) {
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
    else if (GET_CTRL_KEY(GLFW_KEY_A)) {
      menu_select_all();
    }
  }
}

App App::instance {};
