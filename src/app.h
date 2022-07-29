#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <platform/platform.h>
#include <project.h>
#include <thunder_auto.h>

class App {
public:
  static App* get() {
    return &instance;
  }
  
  App(App const&) = delete;
  App& operator=(App const&) = delete;

  void init(GLFWwindow* window);

  inline GLFWwindow* get_window() { return window; }
  
  void present();
  
  void menu_new();
  void menu_open();
  void menu_save();
  void menu_save_as();
  void menu_close();
  void menu_undo();
  void menu_redo();
  void menu_cut();
  void menu_copy();
  void menu_paste();
  void menu_select_all();
  void menu_delete(bool from_menu);
  
  void close();
  
  void handle_keyboard(int key, int scancode, int action, int mods);
  
  constexpr bool is_running() const { return running; }
  
private:
  App();
  ~App();

  void new_project();
  void open_project();

  enum class EventState {
    NONE = 0,
    NEW_PROJECT,
    NEW_PROJECT_UNSAVED_OPENED,
    OPEN_PROJECT,
    OPEN_PROJECT_UNSAVED_OPENED,
    CLOSE_PROJECT,
    CLOSE_PROJECT_UNSAVED,
    CLOSE_EVERYTHING,
    CLOSE_EVERYTHING_UNSAVED,
  };

  void unsaved(EventState next);

  EventState event_state = EventState::NONE;
  
  bool running = true;
  
  GLFWwindow* window = nullptr;
  
  static App instance;
};
