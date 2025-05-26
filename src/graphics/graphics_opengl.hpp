#include <ThunderAuto/graphics/graphics.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#if THUNDER_AUTO_WINDOWS
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

  bool is_initialized() const override { return m_init; }

  bool poll_events() override;

  void begin_frame() override;
  void end_frame() override;

  ImVec2 window_size() const override;
  void window_set_size(int width, int height) override;

  ImVec2 window_position() const override;
  void window_set_position(int x, int y) override;

  void window_set_title(const char* title) override;
  void window_set_should_close(bool value) override;
  void window_focus() override;
  bool is_window_focused() override;
  bool is_window_maximized() override;

  void* get_platform_handle() override {
#if THUNDER_AUTO_WINDOWS
    return reinterpret_cast<void*>(glfwGetWin32Window(m_window));
#else
    return nullptr;
#endif
  }
};
