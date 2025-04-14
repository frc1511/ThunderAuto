#include <ThunderAuto/graphics.hpp>

#include <ThunderAuto/app.hpp>

#define WINDOW_WIDTH  1300
#define WINDOW_HEIGHT 800

#define WINDOW_TITLE "ThunderAuto " THUNDER_AUTO_VERSION_STR

#if THUNDER_AUTO_DIRECTX11
#include <uxtheme.h>
#include <vssym32.h>

#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#else // THUNDER_AUTO_OPENGL
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#endif

#if THUNDER_AUTO_DIRECTX11

static UINT g_resize_width = 0, g_resize_height = 0;

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam,
                                   LPARAM lparam);

#else

//
// GL/GLSL versions.
//
#if defined(IMGUI_IMPL_OPENGL_ES2)
#error "OpenGL ES 2 is not supported"
#endif

#if THUNDER_AUTO_MACOS || THUNDER_AUTO_WINDOWS_TEST_OPENGL_MACOS
#define GLSL_VERSION     "#version 150"
#define GL_VERSION_MAJOR 3
#define GL_VERSION_MINOR 2
#else
#define GLSL_VERSION     "#version 130"
#define GL_VERSION_MAJOR 3
#define GL_VERSION_MINOR 0
#endif

#endif

#if THUNDER_AUTO_WINDOWS

HWND Graphics::hwnd() {
#if THUNDER_AUTO_DIRECTX11
  return m_hwnd;
#else // THUNDER_AUTO_OPENGL
  return glfwGetWin32Window(m_window);
#endif
}

#endif

void Graphics::init() {
#if THUNDER_AUTO_DIRECTX11
  ZeroMemory(&m_wc, sizeof(m_wc));
  m_wc.cbSize = sizeof(WNDCLASSEXW);
  m_wc.lpszClassName = L"ThunderAuto";
  m_wc.lpfnWndProc = WindowProc;
  m_wc.style = CS_HREDRAW | CS_VREDRAW;
  m_wc.hInstance = GetModuleHandleW(nullptr);

  RegisterClassExW(&m_wc);

  const DWORD ws =
      WS_THICKFRAME | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_VISIBLE;

  m_hwnd = CreateWindowExW(
      WS_EX_APPWINDOW, m_wc.lpszClassName, L"" WINDOW_TITLE, ws, 100, 100,
      WINDOW_WIDTH, WINDOW_HEIGHT, nullptr, nullptr, m_wc.hInstance, nullptr);

  // Initialize Direct3D
  if (!init_device()) {
    deinit_device();
    UnregisterClassW(m_wc.lpszClassName, m_wc.hInstance);
    exit(1);
  }

  ShowWindow(m_hwnd, SW_SHOWDEFAULT);
  UpdateWindow(m_hwnd);

#else // THUNDER_AUTO_OPENGL
  //
  // GLFW.
  //
  glfwSetErrorCallback([](int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
  });

  if (!glfwInit()) {
    exit(1);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_VERSION_MAJOR);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_VERSION_MINOR);

#if THUNDER_AUTO_MACOS || THUNDER_AUTO_WINDOWS_TEST_OPENGL_MACOS
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on Mac
#endif

  // Initialize window.
  m_window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE,
                              nullptr, nullptr);
  if (!m_window) exit(1);

  glfwSetWindowSizeLimits(m_window, 700, 500, GLFW_DONT_CARE, GLFW_DONT_CARE);

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

#if THUNDER_AUTO_DIRECTX11
  ImGui_ImplWin32_Init(m_hwnd);
  ImGui_ImplDX11_Init(m_device, m_device_context);

#else // THUNDER_AUTO_OPENGL
  ImGui_ImplGlfw_InitForOpenGL(m_window, true);
  ImGui_ImplOpenGL3_Init(GLSL_VERSION);
#endif
}

bool Graphics::poll_events() {
#if THUNDER_AUTO_DIRECTX11
  MSG msg;
  while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    if (msg.message == WM_QUIT) {
      return true;
    }
  }

  if (m_swap_chain_occluded &&
      m_swap_chain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED) {
    ::Sleep(10);
    return false;
  }
  m_swap_chain_occluded = false;

  if (g_resize_width != 0 && g_resize_height != 0) {
    deinit_render_target();
    m_swap_chain->ResizeBuffers(0, g_resize_width, g_resize_height,
                                DXGI_FORMAT_UNKNOWN, 0);
    g_resize_width = g_resize_height = 0;
    init_render_target();
  }
  return false;

#else
  glfwPollEvents();
  return glfwWindowShouldClose(m_window);
#endif
}

void Graphics::begin_frame() {
#if THUNDER_AUTO_DIRECTX11
  ImGui_ImplDX11_NewFrame();
  ImGui_ImplWin32_NewFrame();
#else // THUNDER_AUTO_OPENGL
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
#endif

  ImGui::NewFrame();
}

void Graphics::end_frame() {
  ImGui::Render();

  const ImVec4 clear_color = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];

#if THUNDER_AUTO_DIRECTX11
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

  HRESULT hr = m_swap_chain->Present(1, 0); // Present with vsync
  m_swap_chain_occluded = (hr == DXGI_STATUS_OCCLUDED);

#else // THUNDER_AUTO_OPENGL
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
#if THUNDER_AUTO_DIRECTX11
  ImGui_ImplDX11_Shutdown();
  ImGui_ImplWin32_Shutdown();
#else // THUNDER_AUTO_OPENGL
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
#endif

  ImGui::DestroyContext();

#if THUNDER_AUTO_DIRECTX11
  deinit_device();
  DestroyWindow(m_hwnd);
  UnregisterClassW(m_wc.lpszClassName, m_wc.hInstance);
#else // THUNDER_AUTO_OPENGL
  glfwDestroyWindow(m_window);
  glfwTerminate();
#endif
}

ImVec2 Graphics::window_size() const {
  int width = 0, height = 0;
#if THUNDER_AUTO_DIRECTX11
#else // THUNDER_AUTO_OPENGL
  glfwGetWindowSize(m_window, &width, &height);
#endif
  return ImVec2(width, height);
}

#if THUNDER_AUTO_DIRECTX11
static bool is_window_maximized(HWND hwnd);
#endif

bool Graphics::is_maximized() {
#if THUNDER_AUTO_DIRECTX11
  return is_window_maximized(m_hwnd);
#endif
  return false;
}

bool Graphics::is_focused() {
#if THUNDER_AUTO_DIRECTX11
  return GetFocus() == m_hwnd;
#else // THUNDER_AUTO_OPENGL
  return glfwGetWindowAttrib(m_window, GLFW_FOCUSED) != 0;
#endif
}

ImVec2 Graphics::window_pos() const {
  int x = 0, y = 0;
#if THUNDER_AUTO_DIRECTX11
#else // THUNDER_AUTO_OPENGL
  glfwGetWindowPos(m_window, &x, &y);
#endif
  return ImVec2(x, y);
}

void Graphics::window_set_size(int width, int height) {
#if THUNDER_AUTO_DIRECTX11
  (void)width;
  (void)height;
#else // THUNDER_AUTO_OPENGL
  glfwSetWindowSize(m_window, width, height);
#endif
}

void Graphics::window_set_pos(int x, int y) {
#if THUNDER_AUTO_DIRECTX11
  (void)x;
  (void)y;
#else // THUNDER_AUTO_OPENGL
  glfwSetWindowPos(m_window, x, y);
#endif
}

void Graphics::window_set_title(const char* title) {
#if THUNDER_AUTO_DIRECTX11
  SetWindowTextA(m_hwnd, title);
#else // THUNDER_AUTO_OPENGL
  glfwSetWindowTitle(m_window, title);
#endif
}

void Graphics::window_focus() {
#if THUNDER_AUTO_DIRECTX11
#else // THUNDER_AUTO_OPENGL
  glfwFocusWindow(m_window);
#endif
}

void Graphics::window_set_should_close(bool value) {
#if THUNDER_AUTO_DIRECTX11
  (void)value;
  // TODO
#else // THUNDER_AUTO_OPENGL
  glfwSetWindowShouldClose(m_window, value);
#endif
}

#if THUNDER_AUTO_DIRECTX11 // DirectX 11 helper functions.

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

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd,
                                                             UINT msg,
                                                             WPARAM wparam,
                                                             LPARAM lparam);

static int dpi_scale(int value, UINT dpi) { return MulDiv(value, dpi, 96); }

static void set_menu_item_state(HMENU menu, MENUITEMINFO* menuItemInfo,
                                UINT item, bool enabled) {
  menuItemInfo->fState = enabled ? MF_ENABLED : MF_DISABLED;
  SetMenuItemInfo(menu, item, false, menuItemInfo);
}

static RECT get_titlebar_rect(HWND hwnd) {
  SIZE title_bar_size;
  const int top_and_bottom_borders = 2;
  HTHEME theme = OpenThemeData(hwnd, L"WINDOW");
  UINT dpi = GetDpiForWindow(hwnd);
  GetThemePartSize(theme, NULL, WP_CAPTION, CS_ACTIVE, NULL, TS_TRUE,
                   &title_bar_size);
  CloseThemeData(theme);

  int height = dpi_scale(title_bar_size.cy, dpi) + top_and_bottom_borders;

  RECT rect;
  GetClientRect(hwnd, &rect);
  rect.bottom = rect.top + height;
  return rect;
}

struct TitleBarButtonRects {
  RECT close;
  RECT maximize;
  RECT minimize;
};

enum class TitleBarButton {
  NONE,
  MINIMIZE,
  MAXIMIZE,
  CLOSE,
};

static TitleBarButtonRects
get_title_bar_button_rects(HWND hwnd, const RECT* title_bar_rect) {
  UINT dpi = GetDpiForWindow(hwnd);
  TitleBarButtonRects button_rects;

  int button_width = dpi_scale(47, dpi);
  button_rects.close = *title_bar_rect;

  button_rects.close.left = button_rects.close.right - button_width;
  button_rects.maximize = button_rects.close;
  button_rects.maximize.left -= button_width;
  button_rects.maximize.right -= button_width;
  button_rects.minimize = button_rects.maximize;
  button_rects.minimize.left -= button_width;
  button_rects.minimize.right -= button_width;
  return button_rects;
}

static bool is_window_maximized(HWND hwnd) {
  WINDOWPLACEMENT placement;
  placement.length = sizeof(WINDOWPLACEMENT);
  if (GetWindowPlacement(hwnd, &placement)) {
    return placement.showCmd == SW_MAXIMIZE;
  }
  return false;
}

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam,
                                   LPARAM lparam) {
  if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) return true;

  TitleBarButton hovered_button =
      static_cast<TitleBarButton>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

  switch (msg) {
  case WM_NCCALCSIZE: {
    if (!wparam) return DefWindowProc(hwnd, msg, wparam, lparam);
    UINT dpi = GetDpiForWindow(hwnd);

    int frame_x = GetSystemMetricsForDpi(SM_CXFRAME, dpi);
    int frame_y = GetSystemMetricsForDpi(SM_CYFRAME, dpi);
    int padding = GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi);

    NCCALCSIZE_PARAMS* params = reinterpret_cast<NCCALCSIZE_PARAMS*>(lparam);
    RECT* requested_client_rect = params->rgrc;

    requested_client_rect->right -= frame_x + padding;
    requested_client_rect->left += frame_x + padding;
    requested_client_rect->bottom -= frame_y + padding;

    if (is_window_maximized(hwnd)) {
      requested_client_rect->top += padding;
    }

    return 0;
  }
  case WM_CREATE: {
    SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
                 SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER);
    break;
  }
  case WM_NCHITTEST: {
    LRESULT hit = DefWindowProc(hwnd, msg, wparam, lparam);
    switch (hit) {
    case HTNOWHERE:
    case HTRIGHT:
    case HTLEFT:
    case HTTOPLEFT:
    case HTTOP:
    case HTTOPRIGHT:
    case HTBOTTOMRIGHT:
    case HTBOTTOM:
    case HTBOTTOMLEFT:
      return hit;
    }

    if (hovered_button == TitleBarButton::MAXIMIZE) {
      // Show SnapLayout on Windows 11
      return HTMAXBUTTON;
    }

    UINT dpi = GetDpiForWindow(hwnd);
    int frame_y = GetSystemMetricsForDpi(SM_CYFRAME, dpi);
    int padding = GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi);
    POINT cursor_point;
    cursor_point.x = GET_X_LPARAM(lparam);
    cursor_point.y = GET_Y_LPARAM(lparam);
    ScreenToClient(hwnd, &cursor_point);
    if (cursor_point.y > 0 && cursor_point.y < frame_y + padding) {
      return HTTOP;
    }

    int menu_bar_width = App::get().menu_bar_width(); // dpi?

    RECT title_bar_rect = get_titlebar_rect(hwnd);
    if (cursor_point.y < title_bar_rect.bottom &&
        cursor_point.x > title_bar_rect.left + menu_bar_width) {
      return HTCAPTION;
    }

    return HTCLIENT;
  }
  case WM_GETMINMAXINFO: {
    UINT dpi = GetDpiForWindow(hwnd);
    App* app = App::get_maybe_uninitialized();
    int menu_bar_width = app ? app->menu_bar_width() : 0; // dpi?
    int button_width = dpi_scale(47, dpi);

    MINMAXINFO* minmax = reinterpret_cast<MINMAXINFO*>(lparam);
    minmax->ptMinTrackSize.x =
        max(menu_bar_width + 3 * button_width + dpi_scale(30, dpi),
            dpi_scale(500, dpi));
    minmax->ptMinTrackSize.y = dpi_scale(400, dpi);
    return 0;
  }
  case WM_NCMOUSEMOVE: {
    POINT cursor_point;
    GetCursorPos(&cursor_point);
    ScreenToClient(hwnd, &cursor_point);

    RECT title_bar_rect = get_titlebar_rect(hwnd);
    TitleBarButtonRects button_rects =
        get_title_bar_button_rects(hwnd, &title_bar_rect);

    TitleBarButton new_hovered_button = TitleBarButton::NONE;
    if (PtInRect(&button_rects.close, cursor_point)) {
      new_hovered_button = TitleBarButton::CLOSE;
    } else if (PtInRect(&button_rects.minimize, cursor_point)) {
      new_hovered_button = TitleBarButton::MINIMIZE;
    } else if (PtInRect(&button_rects.maximize, cursor_point)) {
      new_hovered_button = TitleBarButton::MAXIMIZE;
    }
    if (new_hovered_button != hovered_button) {
      SetWindowLongPtrW(hwnd, GWLP_USERDATA,
                        static_cast<LONG_PTR>(new_hovered_button));
    }
    break;
  }
  case WM_MOUSEMOVE: {
    if (hovered_button != TitleBarButton::NONE) {
      SetWindowLongPtrW(hwnd, GWLP_USERDATA,
                        static_cast<LONG_PTR>(TitleBarButton::NONE));
    }
    break;
  }
  case WM_NCLBUTTONDOWN: {
    if (hovered_button != TitleBarButton::NONE) {
      return 0;
    }
    break;
  }
  case WM_NCLBUTTONUP: {
    if (hovered_button == TitleBarButton::CLOSE) {
      App::get().close();
      return 0;
    } else if (hovered_button == TitleBarButton::MINIMIZE) {
      ShowWindow(hwnd, SW_MINIMIZE);
      return 0;
    } else if (hovered_button == TitleBarButton::MAXIMIZE) {
      int mode = is_window_maximized(hwnd) ? SW_NORMAL : SW_MAXIMIZE;
      ShowWindow(hwnd, mode);
      return 0;
    }
    break;
  }
  case WM_NCRBUTTONUP: {
    if (wparam == HTCAPTION) {
      BOOL const isMaximized = IsZoomed(hwnd);

      MENUITEMINFO menu_item_info;
      ZeroMemory(&menu_item_info, sizeof(menu_item_info));
      menu_item_info.cbSize = sizeof(menu_item_info);
      menu_item_info.fMask = MIIM_STATE;

      HMENU const sys_menu = GetSystemMenu(hwnd, false);
      set_menu_item_state(sys_menu, &menu_item_info, SC_RESTORE, isMaximized);
      set_menu_item_state(sys_menu, &menu_item_info, SC_MOVE, !isMaximized);
      set_menu_item_state(sys_menu, &menu_item_info, SC_SIZE, !isMaximized);
      set_menu_item_state(sys_menu, &menu_item_info, SC_MINIMIZE, true);
      set_menu_item_state(sys_menu, &menu_item_info, SC_MAXIMIZE, !isMaximized);
      set_menu_item_state(sys_menu, &menu_item_info, SC_CLOSE, true);
      BOOL const result =
          TrackPopupMenu(sys_menu, TPM_RETURNCMD, GET_X_LPARAM(lparam),
                         GET_Y_LPARAM(lparam), 0, hwnd, NULL);
      if (result != 0) {
        PostMessage(hwnd, WM_SYSCOMMAND, result, 0);
      }
    }
    break;
  }
  case WM_SETCURSOR: {
    SetCursor(LoadCursor(NULL, IDC_ARROW));
    break;
  }
  case WM_SIZE:
    if (wparam == SIZE_MINIMIZED) return 0;
    g_resize_width = static_cast<UINT>(LOWORD(lparam));
    g_resize_height = static_cast<UINT>(HIWORD(lparam));
    return 0;
  case WM_SYSCOMMAND:
    if ((wparam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
      return 0;
    break;
  case WM_CLOSE:
    App::get().close();
    return 0;
  case WM_DESTROY: {
    PostQuitMessage(0);
    return 0;
  }
  }

  return DefWindowProcW(hwnd, msg, wparam, lparam);
}

#endif
