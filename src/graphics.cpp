#include <ThunderAuto/graphics.h>

#define WINDOW_WIDTH  1300
#define WINDOW_HEIGHT 800

#define WINDOW_TITLE "ThunderAuto"

#include <imgui_impl_glfw.h>

#if TH_DIRECTX11
#include <imgui_impl_dx11.h>
#else // TH_OPENGL
#include <imgui_impl_opengl3.h>
#endif

#if TH_OPENGL

//
// GL/GLSL versions.
//
#if defined(IMGUI_IMPL_OPENGL_ES2)
#error "OpenGL ES 2 is not supported"
#endif

#if TH_MACOS || TH_WINDOWS_TEST_OPENGL_MACOS
#define GLSL_VERSION     "#version 150"
#define GL_VERSION_MAJOR 3
#define GL_VERSION_MINOR 2
#else
#define GLSL_VERSION     "#version 130"
#define GL_VERSION_MAJOR 3
#define GL_VERSION_MINOR 0
#endif

#endif

#if TH_WINDOWS

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

HWND Graphics::hwnd() {
  return glfwGetWin32Window(m_window);
}

#endif

void Graphics::init() {
  //
  // GLFW.
  //
  glfwSetErrorCallback([](int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
  });

  if (!glfwInit()) {
    exit(1);
  }

#if TH_DIRECTX11
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

#else // TH_OPENGL
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_VERSION_MAJOR);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_VERSION_MINOR);

#if TH_MACOS || TH_WINDOWS_TEST_OPENGL_MACOS
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on Mac
#endif
#endif

  // Initialize window.
  m_window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE,
                              nullptr, nullptr);
  if (!m_window) exit(1);

#if TH_DIRECTX11
  // Initialize Direct3D
  if (!init_device()) {
    deinit_device();
    exit(1);
  }
#else // TH_OPENGL
  glfwMakeContextCurrent(m_window);

  // VSync.
  glfwSwapInterval(true);

  // Load OpenGL functions.
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
#endif

  //
  // Imgui.
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

#if TH_DIRECTX11
  ImGui_ImplGlfw_InitForOther(m_window, true);
  ImGui_ImplDX11_Init(m_device, m_device_context);

  glfwSetWindowSizeCallback(m_window, [](GLFWwindow*, int width, int height) {
    if (width > 0 && height > 0) {
      Graphics::get().deinit_render_target();
      Graphics::get().m_swap_chain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
      Graphics::get().init_render_target();
    }
  });

#else // TH_OPENGL
  ImGui_ImplGlfw_InitForOpenGL(m_window, true);
  ImGui_ImplOpenGL3_Init(GLSL_VERSION);
#endif
}

bool Graphics::poll_events() {
  glfwPollEvents();
  return glfwWindowShouldClose(m_window);
}

void Graphics::begin_frame() {
#if TH_DIRECTX11
  ImGui_ImplDX11_NewFrame();
#else // TH_OPENGL
  ImGui_ImplOpenGL3_NewFrame();
#endif
  ImGui_ImplGlfw_NewFrame();

  ImGui::NewFrame();
}

void Graphics::end_frame() {
  ImGui::Render();

  const ImVec4 clear_color = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];

#if TH_DIRECTX11
  const float clear_color_with_alpha[4] = {
      clear_color.x * clear_color.w, clear_color.y * clear_color.w,
      clear_color.z * clear_color.w, clear_color.w};
  m_device_context->OMSetRenderTargets(1, &m_main_render_target_view, nullptr);
  m_device_context->ClearRenderTargetView(m_main_render_target_view,
                                          clear_color_with_alpha);
  ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

  // Update and Render additional Platform Windows
  if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
  }

  m_swap_chain->Present(1, 0); // Present with vsync

#else // TH_OPENGL
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
#endif
}

void Graphics::deinit() {
#if TH_DIRECTX11
  ImGui_ImplDX11_Shutdown();
#else // TH_OPENGL
  ImGui_ImplOpenGL3_Shutdown();
#endif
  ImGui_ImplGlfw_Shutdown();

  ImGui::DestroyContext();

#if TH_DIRECTX11
  deinit_device();
#else // TH_OPENGL
#endif
  glfwDestroyWindow(m_window);
  glfwTerminate();
}

ImVec2 Graphics::window_size() const {
  int width, height;
  glfwGetWindowSize(m_window, &width, &height);
  return ImVec2(width, height);
}

ImVec2 Graphics::window_pos() const {
  int x, y;
  glfwGetWindowPos(m_window, &x, &y);
  return ImVec2(x, y);
}

void Graphics::window_set_size(int width, int height) {
  glfwSetWindowSize(m_window, width, height);
}

void Graphics::window_set_pos(int x, int y) {
  glfwSetWindowPos(m_window, x, y);
}

void Graphics::window_set_title(const char* title) {
  glfwSetWindowTitle(m_window, title);
}

void Graphics::window_focus() {
  glfwFocusWindow(m_window);
}

void Graphics::window_set_should_close(bool value) {
  glfwSetWindowShouldClose(m_window, value);
}

#if TH_DIRECTX11 // DirectX 11 helper functions.

bool Graphics::init_device() {
  // Setup swap chain
  DXGI_SWAP_CHAIN_DESC sd;
  ZeroMemory(&sd, sizeof(sd));
  sd.BufferCount = 2;
  sd.BufferDesc.Width = 0;
  sd.BufferDesc.Height = 0;
  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferDesc.RefreshRate.Numerator = 60;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.OutputWindow = hwnd();
  sd.SampleDesc.Count = 1;
  sd.SampleDesc.Quality = 0;
  sd.Windowed = TRUE;
  sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

  UINT createDeviceFlags = 0;
  // createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
  D3D_FEATURE_LEVEL featureLevel;
  const D3D_FEATURE_LEVEL featureLevelArray[2] = {
      D3D_FEATURE_LEVEL_11_0,
      D3D_FEATURE_LEVEL_10_0,
  };
  HRESULT res = D3D11CreateDeviceAndSwapChain(
      nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags,
      featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &m_swap_chain, &m_device,
      &featureLevel, &m_device_context);
  if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software
                                     // driver if hardware is not available.
    res = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags,
        featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &m_swap_chain, &m_device,
        &featureLevel, &m_device_context);
  if (res != S_OK) return false;

  init_render_target();
  return true;
}

void Graphics::deinit_device() {
  deinit_render_target();
  if (m_swap_chain) {
    m_swap_chain->Release();
    m_swap_chain = nullptr;
  }
  if (m_device_context) {
    m_device_context->Release();
    m_device_context = nullptr;
  }
  if (m_device) {
    m_device->Release();
    m_device = nullptr;
  }
}

void Graphics::init_render_target() {
  ID3D11Texture2D* pBackBuffer;
  m_swap_chain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
  m_device->CreateRenderTargetView(pBackBuffer, nullptr,
                                   &m_main_render_target_view);
  pBackBuffer->Release();
}

void Graphics::deinit_render_target() {
  if (m_main_render_target_view) {
    m_main_render_target_view->Release();
    m_main_render_target_view = nullptr;
  }
}

#endif

