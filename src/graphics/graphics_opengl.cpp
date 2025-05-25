#include "graphics_opengl.hpp"

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#define WINDOW_WIDTH 1300
#define WINDOW_HEIGHT 800

#define WINDOW_TITLE "ThunderAuto " THUNDER_AUTO_VERSION_STR

//
// GL/GLSL versions.
//
#if defined(IMGUI_IMPL_OPENGL_ES2)
#error "OpenGL ES 2 is not supported"
#endif

#if THUNDER_AUTO_MACOS || THUNDER_AUTO_WINDOWS_TEST_OPENGL_MACOS
#define GLSL_VERSION "#version 150"
#define GL_VERSION_MAJOR 3
#define GL_VERSION_MINOR 2
#else
#define GLSL_VERSION "#version 130"
#define GL_VERSION_MAJOR 3
#define GL_VERSION_MINOR 0
#endif

void GraphicsOpenGL::init(App& app) {
  if (m_init)
    return;

  m_app = &app;

  //
  // GLFW
  //
  glfwSetErrorCallback([](int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
  });

  if (!glfwInit())
    exit(1);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_VERSION_MAJOR);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_VERSION_MINOR);

#if THUNDER_AUTO_MACOS || THUNDER_AUTO_WINDOWS_TEST_OPENGL_MACOS
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);  // Required on Mac
#endif

  // Initialize window.
  m_window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE,
                              nullptr, nullptr);
  if (!m_window)
    exit(1);

  glfwSetWindowSizeLimits(m_window, 700, 500, GLFW_DONT_CARE, GLFW_DONT_CARE);

  glfwMakeContextCurrent(m_window);

  // VSync.
  glfwSwapInterval(true);

  // Load OpenGL functions.
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

  //
  // Imgui
  //
  IMGUI_CHECKVERSION();

  ImGui::CreateContext();

  {
    ImGuiIO* io = &ImGui::GetIO();
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io->ConfigWindowsMoveFromTitleBarOnly = true;
  }

  ImGui_ImplGlfw_InitForOpenGL(m_window, true);
  ImGui_ImplOpenGL3_Init(GLSL_VERSION);

  m_init = true;
}

void GraphicsOpenGL::deinit() {
  if (!m_init)
    return;

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();

  ImGui::DestroyContext();

  glfwDestroyWindow(m_window);
  glfwTerminate();

  m_window = nullptr;
  m_app = nullptr;
}

bool GraphicsOpenGL::poll_events() {
  if (!m_init)
    return false;

  glfwPollEvents();
  return glfwWindowShouldClose(m_window);
}

void GraphicsOpenGL::begin_frame() {
  if (!m_init)
    return;

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();

  ImGui::NewFrame();
}

void GraphicsOpenGL::end_frame() {
  if (!m_init)
    return;

  ImGui::Render();

  const ImVec4 clear_color = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];

  int buf_width, buf_height;
  glfwGetFramebufferSize(m_window, &buf_width, &buf_height);

  glViewport(0, 0, buf_width, buf_height);
  glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
               clear_color.z * clear_color.w, clear_color.w);
  glClear(GL_COLOR_BUFFER_BIT);

  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    GLFWwindow* backup_current_context = glfwGetCurrentContext();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    glfwMakeContextCurrent(backup_current_context);
  }

  glfwSwapBuffers(m_window);
}

ImVec2 GraphicsOpenGL::window_size() const {
  if (!m_init)
    return ImVec2(0, 0);

  int width = 0, height = 0;
  glfwGetWindowSize(m_window, &width, &height);
  return ImVec2(width, height);
}

void GraphicsOpenGL::window_set_size(int width, int height) {
  if (!m_init)
    return;

  glfwSetWindowSize(m_window, width, height);
}

ImVec2 GraphicsOpenGL::window_pos() const {
  if (!m_init)
    return ImVec2(0, 0);

  int x = 0, y = 0;
  glfwGetWindowPos(m_window, &x, &y);
  return ImVec2(x, y);
}

void GraphicsOpenGL::window_set_pos(int x, int y) {
  if (!m_init)
    return;

  glfwSetWindowPos(m_window, x, y);
}

void GraphicsOpenGL::window_set_title(const char* title) {
  if (!m_init)
    return;

  glfwSetWindowTitle(m_window, title);
}

void GraphicsOpenGL::window_set_should_close(bool value) {
  if (!m_init)
    return;

  glfwSetWindowShouldClose(m_window, value);
}

void GraphicsOpenGL::window_focus() {
  if (!m_init)
    return;

  glfwFocusWindow(m_window);
}

bool GraphicsOpenGL::is_window_focused() {
  if (!m_init)
    return true;

  return glfwGetWindowAttrib(m_window, GLFW_FOCUSED) != 0;
}

bool GraphicsOpenGL::is_window_maximized() {
  if (!m_init)
    return false;

  return glfwGetWindowAttrib(m_window, GLFW_MAXIMIZED) != 0;
}
