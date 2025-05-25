#pragma once

#include <ThunderAuto/thunder_auto.hpp>
#include <ThunderAuto/singleton.hpp>
#include <ThunderAuto/app.hpp>

class Graphics : public SingletonBase {
 public:
  virtual void init(App& app) = 0;
  virtual void deinit() = 0;

  virtual bool is_initialized() const { return false; }
  operator bool() const { return is_initialized(); }

  /**
   * @brief Process events and return whether the window should close.
   *
   * @return true if the window should close, false otherwise.
   */
  virtual bool poll_events() = 0;

  virtual void begin_frame() = 0;
  virtual void end_frame() = 0;

  virtual ImVec2 window_size() const { return ImVec2(0, 0); }
  virtual void window_set_size(int width, int height) {
    UNUSED(width);
    UNUSED(height);
  }

  virtual ImVec2 window_pos() const { return ImVec2(0, 0); }
  virtual void window_set_pos(int x, int y) {
    UNUSED(x);
    UNUSED(y);
  }

  virtual void window_set_title(const char* title) { UNUSED(title); }
  virtual void window_set_should_close(bool value) { UNUSED(value); }

  virtual void window_focus() {}
  virtual bool is_window_focused() { return false; }

  virtual void window_set_maximized(bool value) { UNUSED(value); }
  virtual bool is_window_maximized() { return false; }

  virtual void* get_platform_handle() { return nullptr; }
};

class PlatformGraphics {
 public:
  static Graphics& get();
};
