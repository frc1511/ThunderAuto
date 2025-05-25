#include "graphics_directx11.hpp"

#include <wrl/client.h>
#include <windowsx.h>

#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#define WINDOW_WIDTH 1300
#define WINDOW_HEIGHT 800

#define WINDOW_TITLE "ThunderAuto " THUNDER_AUTO_VERSION_STR

#define TITLEBAR_BUTTON_WIDTH 47
#define TITLEBAR_BUTTON_ICON_SIZE 10

static UINT g_resize_width = 0, g_resize_height = 0;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

void GraphicsDirectX11::init(App& app) {
  if (m_init)
    return;

  m_app = &app;

  //
  // Window
  //
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

  // Initialize DirectX
  if (!init_directx()) {
    UnregisterClassW(m_wc.lpszClassName, m_wc.hInstance);
    exit(1);
  }

  ShowWindow(m_hwnd, SW_SHOWDEFAULT);
  UpdateWindow(m_hwnd);

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

  ImGui_ImplWin32_Init(m_hwnd);
  ImGui_ImplDX11_Init(m_device, m_device_context);

  m_init = true;
}

void GraphicsDirectX11::deinit() {
  if (!m_init)
    return;

  ImGui_ImplDX11_Shutdown();
  ImGui_ImplWin32_Shutdown();

  ImGui::DestroyContext();

  deinit_directx();
  DestroyWindow(m_hwnd);
  UnregisterClassW(m_wc.lpszClassName, m_wc.hInstance);
}

bool GraphicsDirectX11::poll_events() {
  if (!m_init)
    return false;

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
    Sleep(10);
    return false;
  }
  m_swap_chain_occluded = false;

  if (g_resize_width != 0 && g_resize_height != 0) {
    deinit_render_targets();
    m_swap_chain->ResizeBuffers(0, g_resize_width, g_resize_height,
                                DXGI_FORMAT_UNKNOWN, 0);
    g_resize_width = g_resize_height = 0;
    init_render_targets();
  }
  return false;
}

void GraphicsDirectX11::begin_frame() {
  if (!m_init)
    return;

  ImGui_ImplDX11_NewFrame();
  ImGui_ImplWin32_NewFrame();

  ImGui::NewFrame();
}

void GraphicsDirectX11::end_frame() {
  if (!m_init)
    return;

  ImGui::Render();

  const ImVec4 clear_color = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];

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

  // Render custom minimize/maximize/close buttons in the title bar.
  m_d2d_render_target->BeginDraw();
  draw_titlebar_buttons();
  m_d2d_render_target->EndDraw();

  HRESULT hr = m_swap_chain->Present(1, 0);  // Present with vsync
  m_swap_chain_occluded = (hr == DXGI_STATUS_OCCLUDED);
}

void GraphicsDirectX11::window_set_title(const char* title) {
  if (!m_init)
    return;

  SetWindowTextA(m_hwnd, title);
}

void GraphicsDirectX11::window_set_should_close(bool value) {
  if (!m_init)
    return;

  if (value) {
    PostMessageW(m_hwnd, WM_CLOSE, 0, 0);
  } else {
    // ShowWindow(m_hwnd, SW_SHOW);
  }
}

void GraphicsDirectX11::window_focus() {
  if (!m_init)
    return;

  SetForegroundWindow(m_hwnd);
  SetFocus(m_hwnd);
}

bool GraphicsDirectX11::is_window_focused() {
  if (!m_init)
    return true;

  return GetFocus() == m_hwnd;
}

bool GraphicsDirectX11::is_window_maximized() {
  if (!m_init)
    return false;

  WINDOWPLACEMENT placement;
  placement.length = sizeof(WINDOWPLACEMENT);
  if (GetWindowPlacement(m_hwnd, &placement)) {
    return placement.showCmd == SW_MAXIMIZE;
  }
}

static inline D2D_RECT_F to_d2d_rect(const RECT& rect) {
  return D2D1::RectF((float)rect.left, (float)rect.top, (float)rect.right,
                     (float)rect.bottom);
}

static D2D_RECT_F get_centered_rect_in_rect(const D2D_RECT_F& outer_rect,
                                            float center_width,
                                            float center_height) {
  float outer_width = outer_rect.right - outer_rect.left;
  float outer_height = outer_rect.bottom - outer_rect.top;

  float padding_x = (outer_width - center_width) / 2.f;
  float padding_y = (outer_height - center_height) / 2.f;

  D2D_RECT_F center;
  center.left = outer_rect.left + padding_x;
  center.top = outer_rect.top + padding_y;
  center.right = center.left + center_width;
  center.bottom = center.top + center_height;

  return center;
}

void GraphicsDirectX11::draw_titlebar_buttons() {
  HRESULT hr;
  D2D1_BRUSH_PROPERTIES brush_props;
  brush_props.opacity = 1.f;

  const ImGuiStyle& style = ImGui::GetStyle();

  Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> title_bar_bg_brush;
  D2D1_COLOR_F* title_bar_color =
      (D2D1_COLOR_F*)&style.Colors[ImGuiCol_TitleBg];
  hr = m_d2d_render_target->CreateSolidColorBrush(title_bar_color, &brush_props,
                                                  &title_bar_bg_brush);
  if (FAILED(hr))
    return;

  Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> button_hovered_bg_brush;
  D2D1_COLOR_F* button_hovered_color =
      (D2D1_COLOR_F*)&style.Colors[ImGuiCol_HeaderHovered];
  hr = m_d2d_render_target->CreateSolidColorBrush(
      button_hovered_color, &brush_props, &button_hovered_bg_brush);
  if (FAILED(hr))
    return;

  Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> close_button_hovered_bg_brush;
  D2D1_COLOR_F close_button_hovered_color = D2D1::ColorF(0.8f, 0.f, 0.f, 1.f);
  hr = m_d2d_render_target->CreateSolidColorBrush(
      &close_button_hovered_color, &brush_props,
      &close_button_hovered_bg_brush);
  if (FAILED(hr))
    return;

  Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> button_icon_brush;
  D2D1_COLOR_F button_icon_color = *(D2D1_COLOR_F*)&style.Colors[ImGuiCol_Text];
  if (!is_window_focused())
    button_icon_color.a = .5f;
  hr = m_d2d_render_target->CreateSolidColorBrush(
      &button_icon_color, &brush_props, &button_icon_brush);
  if (FAILED(hr))
    return;

  TitleBarButtonRects button_rects = get_title_bar_button_rects();

  // Draw background color when hovering a button.

  TitleBarButton hovered_button =
      static_cast<TitleBarButton>(GetWindowLongPtrW(m_hwnd, GWLP_USERDATA));

  if (hovered_button != BUTTON_NONE) {
    auto rect = to_d2d_rect(button_rects.rects[hovered_button]);
    auto brush = (hovered_button == BUTTON_CLOSE)
                     ? close_button_hovered_bg_brush.Get()
                     : button_hovered_bg_brush.Get();
    draw_titlebar_button_hover(rect, brush);
  }

  // Draw the icons on the buttons.

  auto minimize_rect = to_d2d_rect(button_rects.minimize);
  auto maximize_rect = to_d2d_rect(button_rects.maximize);
  auto close_rect = to_d2d_rect(button_rects.close);

  draw_titlebar_minimize_icon(minimize_rect, button_icon_brush.Get());
  draw_titlebar_maximize_icon(maximize_rect, title_bar_bg_brush.Get(),
                              button_icon_brush.Get());
  draw_titlebar_close_icon(close_rect, button_icon_brush.Get());
}

void GraphicsDirectX11::draw_titlebar_button_hover(
    const D2D_RECT_F& button_rect,
    ID2D1SolidColorBrush* brush) {
  m_d2d_render_target->FillRectangle(&button_rect, brush);
}

void GraphicsDirectX11::draw_titlebar_minimize_icon(
    const D2D_RECT_F& button_rect,
    ID2D1SolidColorBrush* brush) {
  UINT dpi = GetDpiForWindow(m_hwnd);
  int icon_size = dpi_scale(TITLEBAR_BUTTON_ICON_SIZE, dpi);

  D2D_RECT_F icon_rect = get_centered_rect_in_rect(button_rect, icon_size, 0);

  D2D1_POINT_2F line_begin = D2D1::Point2F(icon_rect.left, icon_rect.top);
  D2D1_POINT_2F line_end = D2D1::Point2F(icon_rect.right, icon_rect.top);

  m_d2d_render_target->DrawLine(line_begin, line_end, brush);
}

void GraphicsDirectX11::draw_titlebar_maximize_icon(
    const D2D_RECT_F& button_rect,
    ID2D1SolidColorBrush* bg_brush,
    ID2D1SolidColorBrush* icon_brush) {
  UINT dpi = GetDpiForWindow(m_hwnd);
  int icon_size = dpi_scale(TITLEBAR_BUTTON_ICON_SIZE, dpi);

  D2D_RECT_F icon_rect =
      get_centered_rect_in_rect(button_rect, icon_size, icon_size);

  int box_corner_radius = dpi_scale(1, dpi);

  if (!is_window_maximized()) {
    D2D1_ROUNDED_RECT icon_rect_rounded =
        D2D1::RoundedRect(icon_rect, box_corner_radius, box_corner_radius);
    m_d2d_render_target->DrawRoundedRectangle(&icon_rect_rounded, icon_brush);
    return;
  }

  int box_offset = dpi_scale(2, dpi);

  D2D_RECT_F front_rect = icon_rect;
  front_rect.right -= box_offset;
  front_rect.top += box_offset;

  D2D_RECT_F back_rect = icon_rect;
  back_rect.left += box_offset;
  back_rect.bottom -= box_offset;

  D2D1_ROUNDED_RECT front_rect_rounded =
      D2D1::RoundedRect(front_rect, box_corner_radius, box_corner_radius);
  D2D1_ROUNDED_RECT back_rect_rounded =
      D2D1::RoundedRect(back_rect, box_corner_radius, box_corner_radius);

  m_d2d_render_target->DrawRoundedRectangle(&back_rect_rounded, icon_brush);
  m_d2d_render_target->FillRoundedRectangle(&front_rect_rounded, bg_brush);
  m_d2d_render_target->DrawRoundedRectangle(&front_rect_rounded, icon_brush);
}

void GraphicsDirectX11::draw_titlebar_close_icon(const D2D_RECT_F& button_rect,
                                                 ID2D1SolidColorBrush* brush) {
  UINT dpi = GetDpiForWindow(m_hwnd);
  int icon_size = dpi_scale(TITLEBAR_BUTTON_ICON_SIZE, dpi);

  D2D_RECT_F icon_rect =
      get_centered_rect_in_rect(button_rect, icon_size, icon_size);

  D2D1_POINT_2F line_begin, line_end;

  line_begin = D2D1::Point2F(icon_rect.left, icon_rect.top);
  line_end = D2D1::Point2F(icon_rect.right, icon_rect.bottom);

  m_d2d_render_target->DrawLine(line_begin, line_end, brush);

  line_begin = D2D1::Point2F(icon_rect.left, icon_rect.bottom);
  line_end = D2D1::Point2F(icon_rect.right, icon_rect.top);

  m_d2d_render_target->DrawLine(line_begin, line_end, brush);
}

bool GraphicsDirectX11::init_directx() {
  if (!init_d3d_and_swapchain()) {
    deinit_swapchain();
    deinit_d3d();
    return false;
  }

  if (!init_d2d()) {
    deinit_swapchain();
    deinit_d3d();
    deinit_d2d();
    return false;
  }

  init_render_targets();

  return true;
}

void GraphicsDirectX11::deinit_directx() {
  deinit_render_targets();
  deinit_swapchain();
  deinit_d3d();
  deinit_d2d();
}

bool GraphicsDirectX11::init_d3d_and_swapchain() {
  // Setup swap chain
  DXGI_SWAP_CHAIN_DESC sd;
  ZeroMemory(&sd, sizeof(sd));
  sd.BufferCount = 2;
  sd.BufferDesc.Width = 0;
  sd.BufferDesc.Height = 0;
  sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
  sd.BufferDesc.RefreshRate.Numerator = 60;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.OutputWindow = m_hwnd;
  sd.SampleDesc.Count = 1;
  sd.SampleDesc.Quality = 0;
  sd.Windowed = TRUE;
  sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

  UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifndef NDEBUG
  createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
  D3D_FEATURE_LEVEL featureLevel;
  const D3D_FEATURE_LEVEL featureLevelArray[2] = {
      D3D_FEATURE_LEVEL_11_0,
      D3D_FEATURE_LEVEL_10_0,
  };
  HRESULT res = D3D11CreateDeviceAndSwapChain(
      nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags,
      featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &m_swap_chain, &m_device,
      &featureLevel, &m_device_context);
  if (res == DXGI_ERROR_UNSUPPORTED)  // Try high-performance WARP software
                                      // driver if hardware is not available.
    res = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags,
        featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &m_swap_chain, &m_device,
        &featureLevel, &m_device_context);

  return res == S_OK;
}

void GraphicsDirectX11::deinit_swapchain() {
  if (m_swap_chain) {
    m_swap_chain->Release();
    m_swap_chain = nullptr;
  }
}

void GraphicsDirectX11::deinit_d3d() {
  if (m_device_context) {
    m_device_context->Release();
    m_device_context = nullptr;
  }
  if (m_device) {
    m_device->Release();
    m_device = nullptr;
  }
}

bool GraphicsDirectX11::init_d2d() {
  D2D1_FACTORY_OPTIONS d2dOptions = {};
#ifndef NDEBUG
  d2dOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif
  // Create the D2D factory
  HRESULT res = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, d2dOptions,
                                  &m_d2d_factory);
  return res == S_OK;
}

void GraphicsDirectX11::deinit_d2d() {
  if (m_d2d_factory) {
    m_d2d_factory->Release();
    m_d2d_factory = nullptr;
  }
}

void GraphicsDirectX11::init_render_targets() {
  init_d3d_render_target();
  init_d2d_render_target();
}

void GraphicsDirectX11::deinit_render_targets() {
  deinit_d3d_render_target();
  deinit_d2d_render_target();
}

void GraphicsDirectX11::init_d3d_render_target() {
  ID3D11Texture2D* pBackBuffer;
  m_swap_chain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
  m_device->CreateRenderTargetView(pBackBuffer, nullptr,
                                   &m_main_render_target_view);
  pBackBuffer->Release();
}

void GraphicsDirectX11::deinit_d3d_render_target() {
  if (m_main_render_target_view) {
    m_main_render_target_view->Release();
    m_main_render_target_view = nullptr;
  }
}

void GraphicsDirectX11::init_d2d_render_target() {
  HRESULT res;

  IDXGISurface* dxgiSurface;
  res = m_swap_chain->GetBuffer(0, IID_PPV_ARGS(&dxgiSurface));

  // Create the DXGI Surface Render Target.
  float dpi = GetDpiForWindow(m_hwnd);

  D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
      D2D1_RENDER_TARGET_TYPE_DEFAULT,
      D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM,
                        D2D1_ALPHA_MODE_PREMULTIPLIED),
      dpi, dpi);

  res = m_d2d_factory->CreateDxgiSurfaceRenderTarget(dxgiSurface, &props,
                                                     &m_d2d_render_target);

  dxgiSurface->Release();
}

void GraphicsDirectX11::deinit_d2d_render_target() {
  if (m_d2d_render_target) {
    m_d2d_render_target->Release();
    m_d2d_render_target = nullptr;
  }
}

RECT GraphicsDirectX11::get_titlebar_rect() {
  RECT rect;
  GetClientRect(m_hwnd, &rect);
  rect.bottom = rect.top + m_app->menu_bar_height();
  return rect;
}

TitleBarButtonRects GraphicsDirectX11::get_title_bar_button_rects() {
  UINT dpi = GetDpiForWindow(m_hwnd);
  TitleBarButtonRects button_rects;

  int button_width = dpi_scale(TITLEBAR_BUTTON_WIDTH, dpi);
  button_rects.close = get_titlebar_rect();

  button_rects.close.left = button_rects.close.right - button_width;
  button_rects.maximize = button_rects.close;
  button_rects.maximize.left -= button_width;
  button_rects.maximize.right -= button_width;
  button_rects.minimize = button_rects.maximize;
  button_rects.minimize.left -= button_width;
  button_rects.minimize.right -= button_width;
  return button_rects;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd,
                                                             UINT msg,
                                                             WPARAM wparam,
                                                             LPARAM lparam);

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
    return true;

  GraphicsDirectX11& graphics = GraphicsDirectX11::get();
  App* app = graphics.m_app;

  TitleBarButton hovered_button =
      static_cast<TitleBarButton>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

  switch (msg) {
    case WM_NCCALCSIZE: {
      if (!wparam)
        return DefWindowProcW(hwnd, msg, wparam, lparam);
      UINT dpi = GetDpiForWindow(hwnd);

      int frame_x = GetSystemMetricsForDpi(SM_CXFRAME, dpi);
      int frame_y = GetSystemMetricsForDpi(SM_CYFRAME, dpi);
      int padding = GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi);

      NCCALCSIZE_PARAMS* params = reinterpret_cast<NCCALCSIZE_PARAMS*>(lparam);
      RECT* requested_client_rect = params->rgrc;

      requested_client_rect->right -= frame_x + padding;
      requested_client_rect->left += frame_x + padding;
      requested_client_rect->bottom -= frame_y + padding;

      if (graphics.is_window_maximized()) {
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
      LRESULT hit = DefWindowProcW(hwnd, msg, wparam, lparam);
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

      if (hovered_button == BUTTON_MAXIMIZE) {
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

      int menu_bar_width = app->menu_bar_width();  // dpi?

      RECT title_bar_rect = graphics.get_titlebar_rect();
      if (cursor_point.y < title_bar_rect.bottom &&
          cursor_point.x > title_bar_rect.left + menu_bar_width) {
        return HTCAPTION;
      }

      return HTCLIENT;
    }
    case WM_GETMINMAXINFO: {
      UINT dpi = GetDpiForWindow(hwnd);
      int menu_bar_width = app->menu_bar_width();  // dpi?
      int button_width = graphics.dpi_scale(TITLEBAR_BUTTON_WIDTH, dpi);

      MINMAXINFO* minmax = reinterpret_cast<MINMAXINFO*>(lparam);
      minmax->ptMinTrackSize.x =
          max(menu_bar_width + 3 * button_width + graphics.dpi_scale(30, dpi),
              graphics.dpi_scale(500, dpi));
      minmax->ptMinTrackSize.y = graphics.dpi_scale(400, dpi);
      return 0;
    }
    case WM_NCMOUSEMOVE: {
      POINT cursor_point;
      GetCursorPos(&cursor_point);
      ScreenToClient(hwnd, &cursor_point);

      TitleBarButtonRects rects = graphics.get_title_bar_button_rects();

      TitleBarButton new_hovered_button = BUTTON_NONE;
      if (PtInRect(&rects.close, cursor_point)) {
        new_hovered_button = BUTTON_CLOSE;
      } else if (PtInRect(&rects.minimize, cursor_point)) {
        new_hovered_button = BUTTON_MINIMIZE;
      } else if (PtInRect(&rects.maximize, cursor_point)) {
        new_hovered_button = BUTTON_MAXIMIZE;
      }
      if (new_hovered_button != hovered_button) {
        SetWindowLongPtrW(hwnd, GWLP_USERDATA,
                          static_cast<LONG_PTR>(new_hovered_button));
      }
      break;
    }
    case WM_MOUSEMOVE: {
      if (hovered_button != BUTTON_NONE) {
        RECT title_bar_rect = graphics.get_titlebar_rect();
        SetWindowLongPtrW(hwnd, GWLP_USERDATA,
                          static_cast<LONG_PTR>(BUTTON_NONE));
      }
      break;
    }
    case WM_NCLBUTTONDOWN: {
      if (hovered_button != BUTTON_NONE) {
        return 0;
      }
      break;
    }
    case WM_NCLBUTTONUP: {
      if (hovered_button == BUTTON_CLOSE) {
        app->close();
        return 0;
      } else if (hovered_button == BUTTON_MINIMIZE) {
        ShowWindow(hwnd, SW_MINIMIZE);
        return 0;
      } else if (hovered_button == BUTTON_MAXIMIZE) {
        int mode = graphics.is_window_maximized() ? SW_NORMAL : SW_MAXIMIZE;
        ShowWindow(hwnd, mode);
        return 0;
      }
      break;
    }
    case WM_NCRBUTTONUP: {
      if (wparam == HTCAPTION) {
        BOOL const isMaximized = IsZoomed(hwnd);

        MENUITEMINFOW menu_item_info;
        ZeroMemory(&menu_item_info, sizeof(menu_item_info));
        menu_item_info.cbSize = sizeof(menu_item_info);
        menu_item_info.fMask = MIIM_STATE;

        HMENU const sys_menu = GetSystemMenu(hwnd, false);

        auto set_menu_item_state = [&](UINT item, bool enabled) {
          menu_item_info.fState = enabled ? MF_ENABLED : MF_DISABLED;
          SetMenuItemInfoW(sys_menu, item, false, &menu_item_info);
        };

        set_menu_item_state(SC_RESTORE, isMaximized);
        set_menu_item_state(SC_MOVE, !isMaximized);
        set_menu_item_state(SC_SIZE, !isMaximized);
        set_menu_item_state(SC_MINIMIZE, true);
        set_menu_item_state(SC_MAXIMIZE, !isMaximized);
        set_menu_item_state(SC_CLOSE, true);

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
      if (wparam == SIZE_MINIMIZED)
        return 0;
      g_resize_width = static_cast<UINT>(LOWORD(lparam));
      g_resize_height = static_cast<UINT>(HIWORD(lparam));
      return 0;
    case WM_SYSCOMMAND:
      if ((wparam & 0xfff0) == SC_KEYMENU)  // Disable ALT application menu
        return 0;
      break;
    case WM_CLOSE:
      app->close();
      return 0;
    case WM_DESTROY: {
      PostQuitMessage(0);
      return 0;
    }
  }

  return DefWindowProcW(hwnd, msg, wparam, lparam);
}
