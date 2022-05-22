#include <app.h>
#include <imgui.h>

#include <iostream>

#ifdef THUNDER_AUTO_MACOS
# include <platform/macos/macos.h>
#elif THUNDER_AUTO_WINDOWS
# include <platform/windows/windows.h>
#elif THUNDER_AUTO_LINUX
# include <platform/linux/linux.h>
#endif

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
  if (close_priority == ClosePriority::CLOSE_EVERYTHING) {
    running = false;
  }
  
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
  
  static bool show_path_editor = false,
              show_properties = false;
  
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
      ImGui::MenuItem("Save",    CTRL_STR "S",       &item_save);
      ImGui::MenuItem("Save As", CTRL_SHIFT_STR "S", &item_save_as);
      ImGui::MenuItem("Close",   CTRL_STR "W",       &item_close);
      
      ImGui::EndMenu();
    }
    
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
      ImGui::MenuItem("Path Editor", nullptr, &show_path_editor);
      ImGui::MenuItem("Properties", nullptr, &show_properties);
      
      ImGui::EndMenu();
    }
    
    ImGui::EndMenuBar();
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
}

void App::menu_new() {
  std::cout << "new\n";
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
  std::cout << "closing everything\n";
  close_priority = ClosePriority::CLOSE_EVERYTHING;
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
  else if (GET_CTRL_KEY(GLFW_KEY_S)) {
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

App App::instance {};
