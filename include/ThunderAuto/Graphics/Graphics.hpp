#pragma once

#include <ThunderAuto/Singleton.hpp>
#include <ThunderAuto/App.hpp>
#include <ThunderLibCore/Macros.hpp>
#include <ThunderLibCore/Types.hpp>

#define DEFAULT_WINDOW_WIDTH 1300
#define DEFAULT_WINDOW_HEIGHT 800

#define DEFAULT_WINDOW_TITLE "ThunderAuto " THUNDERAUTO_VERSION_STR

class Graphics : public SingletonBase {
 public:
  virtual void init(App& app) = 0;
  virtual void deinit() = 0;

  virtual bool isInitialized() const { return false; }
  operator bool() const { return isInitialized(); }

  /**
   * @brief Process events and return whether the window should close.
   *
   * @return true if the window should close, false otherwise.
   */
  virtual bool pollEvents() = 0;

  virtual void beginFrame() = 0;
  virtual void endFrame() = 0;

  virtual double getDPIScale() const { return 1.0; }

  virtual Vec2 getMainWindowSize() const { return Vec2(0, 0); }
  virtual void setMainWindowSize(int width, int height) {
    UNUSED(width);
    UNUSED(height);
  }

  virtual Vec2 getMainWindowPosition() const { return Vec2(0, 0); }
  virtual void setMainWindowPosition(int x, int y) {
    UNUSED(x);
    UNUSED(y);
  }
  virtual void moveMainWindowToCenter() {}

  virtual void setMainWindowTitle(const char* title) { UNUSED(title); }
  virtual void setMainWindowShouldClose(bool value) { UNUSED(value); }

  virtual void focusMainWindow() {}
  virtual bool isMainWindowFocused() { return false; }
  virtual bool isWindowFocused(void* platformHandle) {
    UNUSED(platformHandle);
    return false;
  }

  virtual void setMainWindowMaximized(bool value) { UNUSED(value); }
  virtual bool isMainWindowMaximized() { return false; }

  virtual void* getPlatformHandle() { return nullptr; }

 protected:
  void applyUIConfig();
  void applyUIStyle(bool darkMode = true);
  void updateUIScale(double scale);

 private:
  void loadFonts(double scale);
};

Graphics& getPlatformGraphics();
