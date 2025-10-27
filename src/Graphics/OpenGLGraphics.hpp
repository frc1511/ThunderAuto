#pragma once

#include <ThunderAuto/Graphics/Graphics.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#if THUNDERAUTO_WINDOWS
#include <Windows.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

class GraphicsOpenGL final : public Graphics, public Singleton<GraphicsOpenGL> {
  bool m_init = false;

  GLFWwindow* m_window = nullptr;
  App* m_app = nullptr;

 public:
  void init(App& app) override;
  void deinit() override;

  bool isInitialized() const override { return m_init; }

  bool pollEvents() override;

  void beginFrame() override;
  void endFrame() override;

  Vec2 getMainWindowSize() const override;
  void setMainWindowSize(int width, int height) override;

  Vec2 getMainWindowPosition() const override;
  void setMainWindowPosition(int x, int y) override;

  void setMainWindowTitle(const char* title) override;
  void setMainWindowShouldClose(bool value) override;
  void focusMainWindow() override;
  bool isMainWindowFocused() override;
  bool isWindowFocused(void* platformHandle) override;
  bool isMainWindowMaximized() override;

  void* getPlatformHandle() override {
#if THUNDERAUTO_WINDOWS
    return reinterpret_cast<void*>(glfwGetWin32Window(m_window));
#else
    return nullptr;
#endif
  }
};
