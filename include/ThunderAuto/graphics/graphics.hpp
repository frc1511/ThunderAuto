#pragma once

#include <ThunderAuto/thunder_auto.hpp>
#include <ThunderAuto/singleton.hpp>
#include <ThunderAuto/app.hpp>

enum UISize {
  UISIZE_TITLEBAR_BUTTON_WIDTH = 0,
  UISIZE_TITLEBAR_BUTTON_ICON_SIZE,
  UISIZE_TITLEBAR_BUTTON_MAXIMIZE_ICON_BOX_ROUNDING,
  UISIZE_TITLEBAR_BUTTON_MAXIMIZE_ICON_BOX_OFFSET,
  UISIZE_TITLEBAR_DRAG_AREA_WIDTH,
  UISIZE_TITLEBAR_FRAME_PADDING,
  UISIZE_TITLEBAR_ITEM_SPACING,
  UISIZE_WINDOW_MIN_WIDTH,
  UISIZE_WINDOW_MIN_HEIGHT,
};

#define DEFAULT_WINDOW_WIDTH 1300
#define DEFAULT_WINDOW_HEIGHT 800

#define DEFAULT_WINDOW_TITLE "ThunderAuto " THUNDER_AUTO_VERSION_STR

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

  virtual float dpi_scale() const { return 1.0f; }

  virtual ImVec2 window_size() const { return ImVec2(0, 0); }
  virtual void window_set_size(int width, int height) {
    UNUSED(width);
    UNUSED(height);
  }

  virtual ImVec2 window_position() const { return ImVec2(0, 0); }
  virtual void window_set_position(int x, int y) {
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

 protected:
  void apply_ui_config();
  void apply_ui_style(bool dark_mode = true);
  void update_ui_scale(float scale);

 private:
  void load_fonts(float scale);
};

class PlatformGraphics {
 public:
  static Graphics& get();
};
