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

  Vec2 windowSize() const override;
  void windowSetSize(int width, int height) override;

  Vec2 windowPosition() const override;
  void windowSetPosition(int x, int y) override;

  void windowSetTitle(const char* title) override;
  void windowSetShouldClose(bool value) override;
  void windowFocus() override;
  bool isWindowFocused() override;
  bool isWindowMaximized() override;

  void* getPlatformHandle() override {
#if THUNDERAUTO_WINDOWS
    return reinterpret_cast<void*>(glfwGetWin32Window(m_window));
#else
    return nullptr;
#endif
  }
};
