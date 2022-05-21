#pragma once

#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>

class App {
public:
  static App* get() {
    return &instance;
  }
  
  App(App const&) = delete;
  App& operator=(App const&) = delete;
  
  void present();
  
  void menu_new();
  void menu_open();
  void menu_save();
  void menu_save_as();
  void menu_close();
  void menu_undo();
  void menu_redo();
  
  void close();
  
  void handle_keyboard(int key, int scancode, int action, int mods);
  
  constexpr bool is_running() const { return running; }
  
private:
  App() = default;
  ~App() = default;
  
  enum class ClosePriority {
    DONT_CLOSE = 0,
    CLOSE_PROJECT,
    CLOSE_EVERYTHING
  };
  
  ClosePriority close_priority = ClosePriority::DONT_CLOSE;
  
  bool running = true;
  
  static App instance;
};
