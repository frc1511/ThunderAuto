#include "OpenGLGraphics.hpp"

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

//
// GL/GLSL versions.
//
#if defined(IMGUI_IMPL_OPENGL_ES2)
#error "OpenGL ES 2 is not supported"
#endif

#if THUNDERAUTO_MACOS || THUNDERAUTO_WINDOWS_TEST_OPENGL_MACOS
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
  // Imgui
  //
  IMGUI_CHECKVERSION();

  ImGui::CreateContext();

  //
  // GLFW
  //
  glfwSetErrorCallback(
      [](int error, const char* description) { fprintf(stderr, "GLFW Error %d: %s\n", error, description); });

  if (!glfwInit())
    exit(1);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_VERSION_MAJOR);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_VERSION_MINOR);

#if THUNDERAUTO_MACOS || THUNDERAUTO_WINDOWS_TEST_OPENGL_MACOS
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#endif

  // Initialize window.
  m_window =
      glfwCreateWindow(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, DEFAULT_WINDOW_TITLE, nullptr, nullptr);
  if (!m_window)
    exit(1);

  glfwSetWindowSizeLimits(m_window, 700, 500, GLFW_DONT_CARE, GLFW_DONT_CARE);

  glfwMakeContextCurrent(m_window);

  // VSync.
  glfwSwapInterval(true);

  // Load OpenGL functions.
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

  //
  // More ImGui setup.
  //

  applyUIConfig();
  applyUIStyle();

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

bool GraphicsOpenGL::pollEvents() {
  if (!m_init)
    return false;

  glfwPollEvents();
  return glfwWindowShouldClose(m_window);
}

void GraphicsOpenGL::beginFrame() {
  if (!m_init)
    return;

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();

  ImGui::NewFrame();
}

void GraphicsOpenGL::endFrame() {
  if (!m_init)
    return;

  ImGui::Render();

  const ImVec4 clear_color = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];

  int buf_width, buf_height;
  glfwGetFramebufferSize(m_window, &buf_width, &buf_height);

  glViewport(0, 0, buf_width, buf_height);
  glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w,
               clear_color.w);
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

Vec2 GraphicsOpenGL::getMainWindowSize() const {
  if (!m_init)
    return Vec2(0, 0);

  int width = 0, height = 0;
  glfwGetWindowSize(m_window, &width, &height);
  return Vec2(width, height);
}

void GraphicsOpenGL::setMainWindowSize(int width, int height) {
  if (!m_init)
    return;

  glfwSetWindowSize(m_window, width, height);
}

Vec2 GraphicsOpenGL::getMainWindowPosition() const {
  if (!m_init)
    return Vec2(0, 0);

  int x = 0, y = 0;
  glfwGetWindowPos(m_window, &x, &y);
  return Vec2(x, y);
}

void GraphicsOpenGL::setMainWindowPosition(int x, int y) {
  if (!m_init)
    return;

  glfwSetWindowPos(m_window, x, y);
}

void GraphicsOpenGL::setMainWindowTitle(const char* title) {
  if (!m_init)
    return;

  glfwSetWindowTitle(m_window, title);
}

void GraphicsOpenGL::setMainWindowShouldClose(bool value) {
  if (!m_init)
    return;

  glfwSetWindowShouldClose(m_window, value);
}

void GraphicsOpenGL::focusMainWindow() {
  if (!m_init)
    return;

  glfwFocusWindow(m_window);
}

bool GraphicsOpenGL::isMainWindowFocused() {
  if (!m_init)
    return true;

  return glfwGetWindowAttrib(m_window, GLFW_FOCUSED) != 0;
}

bool GraphicsOpenGL::isWindowFocused(void* platformHandle) {
  if (!platformHandle) {
    return false;
  }

  GLFWwindow* window = static_cast<GLFWwindow*>(platformHandle);
  return glfwGetWindowAttrib(window, GLFW_FOCUSED) != 0;
}

bool GraphicsOpenGL::isMainWindowMaximized() {
  if (!m_init)
    return false;

  return glfwGetWindowAttrib(m_window, GLFW_MAXIMIZED) != 0;
}
