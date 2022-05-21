#include <app.h>
#include <imgui.h>

#include <iostream>

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
       item_redo = false;
  
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
      ImGui::MenuItem("Undo", CTRL_STR "Z",       &item_undo);
      ImGui::MenuItem("Redo", CTRL_SHIFT_STR "Z", &item_redo);
      
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
}

void App::menu_new() {
  std::cout << "new\n";
}

void App::menu_open() {
  std::cout << "open\n";
}

void App::menu_save() {
  std::cout << "save\n";
}

void App::menu_save_as() {
  std::cout << "save as\n";
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

void App::close() {
  std::cout << "closing everything\n";
  close_priority = ClosePriority::CLOSE_EVERYTHING;
}

void App::handle_keyboard(int key, int scancode, int action, int mods) {
#ifdef THUNDER_PATH_MACOS
  int ctrl_key = GLFW_MOD_SUPER;
#else
  int ctrl_key = GLFW_MOD_CONTROL;
#endif
  
  if (key == GLFW_KEY_N && mods & ctrl_key && action == GLFW_PRESS) {
    menu_new();
  }
  else if (key == GLFW_KEY_O && mods & ctrl_key && action == GLFW_PRESS) {
    menu_open();
  }
  else if (key == GLFW_KEY_S && mods & ctrl_key && mods & GLFW_MOD_SHIFT && action == GLFW_PRESS) {
    menu_save_as();
  }
  else if (key == GLFW_KEY_S && mods & ctrl_key && action == GLFW_PRESS) {
    menu_save();
  }
  else if (key == GLFW_KEY_W && mods & ctrl_key && action == GLFW_PRESS) {
    menu_close();
  }
  else if (((key == GLFW_KEY_Z && mods & GLFW_MOD_SHIFT) || key == GLFW_KEY_Y) && mods & ctrl_key  && action == GLFW_PRESS) {
    menu_redo();
  }
  else if (key == GLFW_KEY_Z && mods & ctrl_key && action == GLFW_PRESS) {
    menu_undo();
  }
}

App App::instance {};
