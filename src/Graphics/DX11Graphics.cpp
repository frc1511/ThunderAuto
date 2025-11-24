#include "DX11Graphics.hpp"

#include <wrl/client.h>
#include <windowsx.h>

#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

static UINT g_resizeWidth = 0, g_resizeHeight = 0;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

void GraphicsDirectX11::init(App& app) {
  if (m_init)
    return;

  m_app = &app;

  //
  // Imgui.
  //
  IMGUI_CHECKVERSION();

  ImGui::CreateContext();

  ImGui_ImplWin32_EnableDpiAwareness();

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

  const DWORD ws = WS_THICKFRAME | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_VISIBLE;

  m_hwnd =
      CreateWindowExW(WS_EX_APPWINDOW, m_wc.lpszClassName, L"" DEFAULT_WINDOW_TITLE, ws, 100, 100,
                      DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, nullptr, nullptr, m_wc.hInstance, nullptr);

  // Initialize DirectX
  if (!initDirectX()) {
    UnregisterClassW(m_wc.lpszClassName, m_wc.hInstance);
    exit(1);
  }

  ShowWindow(m_hwnd, SW_SHOWDEFAULT);
  UpdateWindow(m_hwnd);

  //
  // More ImGui setup.
  //

  apply_ui_config();
  apply_ui_style();

  ImGui_ImplWin32_Init(m_hwnd);
  ImGui_ImplDX11_Init(m_device, m_deviceContext);

  m_init = true;
}

void GraphicsDirectX11::deinit() {
  if (!m_init)
    return;

  ImGui_ImplDX11_Shutdown();
  ImGui_ImplWin32_Shutdown();

  ImGui::DestroyContext();

  deinitDirectX();
  DestroyWindow(m_hwnd);
  UnregisterClassW(m_wc.lpszClassName, m_wc.hInstance);
}

bool GraphicsDirectX11::pollEvents() {
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

  if (m_swapChainOccluded && m_swapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED) {
    Sleep(10);
    return false;
  }
  m_swapChainOccluded = false;

  if (g_resizeWidth != 0 && g_resizeHeight != 0) {
    deinitRenderTargets();
    m_swapChain->ResizeBuffers(0, g_resizeWidth, g_resizeHeight, DXGI_FORMAT_UNKNOWN, 0);
    g_resizeWidth = g_resizeHeight = 0;
    initRenderTargets();
  }
  return false;
}

void GraphicsDirectX11::beginFrame() {
  if (!m_init)
    return;

  ImGui_ImplDX11_NewFrame();
  ImGui_ImplWin32_NewFrame();

  ImGui::NewFrame();
}

void GraphicsDirectX11::endFrame() {
  if (!m_init)
    return;

  ImGui::Render();

  const ImVec4 clearColor = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];

  const float clearColorWithAlpha[4] = {clearColor.x * clearColor.w, clearColor.y * clearColor.w,
                                        clearColor.z * clearColor.w, clearColor.w};
  m_deviceContext->OMSetRenderTargets(1, &m_mainRenderTargetView, nullptr);
  m_deviceContext->ClearRenderTargetView(m_mainRenderTargetView, clearColorWithAlpha);
  ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

  // Update and Render additional Platform Windows
  if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
  }

  // Render custom minimize/maximize/close buttons in the title bar.
  m_d2dRenderTarget->BeginDraw();
  drawTitlebarButtons();
  m_d2dRenderTarget->EndDraw();

  HRESULT hr = m_swapChain->Present(1, 0);  // Present with vsync
  m_swapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
}

float GraphicsDirectX11::getDPIScale() const {
  if (!m_hwnd)
    return 1.f;

  return ImGui_ImplWin32_GetDpiScaleForHwnd(m_hwnd);
}

ImVec2 GraphicsDirectX11::getMainWindowSize() const {
  if (!m_hwnd)
    return ImVec2(0, 0);

  RECT rect;
  GetClientRect(m_hwnd, &rect);
  return ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));
}

void GraphicsDirectX11::setMainWindowSize(int width, int height) {
  if (!m_hwnd)
    return;

  RECT rect;
  GetClientRect(m_hwnd, &rect);
  int x = rect.left;
  int y = rect.top;

  SetWindowPos(m_hwnd, nullptr, x, y, width, height, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
}

ImVec2 GraphicsDirectX11::getMainWindowPosition() const {
  if (!m_hwnd)
    return ImVec2(0, 0);

  RECT rect;
  GetWindowRect(m_hwnd, &rect);
  return ImVec2((float)(rect.left), (float)(rect.top));
}

void GraphicsDirectX11::setMainWindowPosition(int x, int y) {
  if (!m_hwnd)
    return;

  RECT rect;
  GetClientRect(m_hwnd, &rect);
  int width = rect.right - rect.left;
  int height = rect.bottom - rect.top;

  SetWindowPos(m_hwnd, nullptr, x, y, width, height, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
}

void GraphicsDirectX11::moveMainWindowToCenter() {
  if (!m_hwnd)
    return;

  int screenWidth = GetSystemMetrics(SM_CXSCREEN);
  int screenHeight = GetSystemMetrics(SM_CYSCREEN);

  auto [width, height] = getMainWindowSize();

  int x = (screenWidth - width) / 2;
  int y = (screenHeight - height) / 2;

  setMainWindowPosition(x, y);
}

void GraphicsDirectX11::setMainWindowTitle(const char* title) {
  if (!m_hwnd)
    return;

  SetWindowTextA(m_hwnd, title);
}

void GraphicsDirectX11::setMainWindowShouldClose(bool value) {
  if (!m_hwnd)
    return;

  if (value) {
    PostMessageW(m_hwnd, WM_CLOSE, 0, 0);
  } else {
    // ShowWindow(m_hwnd, SW_SHOW);
  }
}

void GraphicsDirectX11::focusMainWindow() {
  if (!m_hwnd)
    return;

  SetForegroundWindow(m_hwnd);
  SetFocus(m_hwnd);
}

bool GraphicsDirectX11::isMainWindowFocused() {
  if (!m_hwnd)
    return true;

  return GetFocus() == m_hwnd;
}

bool GraphicsDirectX11::isWindowFocused(void* platformHandle) override {
  if (!platformHandle)
    return false;

  HWND hwnd = reinterpret_cast<HWND>(platformHandle);
  return GetFocus() == hwnd;
}

bool GraphicsDirectX11::isMainWindowMaximized() {
  if (!m_hwnd)
    return false;

  WINDOWPLACEMENT placement;
  placement.length = sizeof(WINDOWPLACEMENT);
  if (GetWindowPlacement(m_hwnd, &placement)) {
    return placement.showCmd == SW_MAXIMIZE;
  }

  return false;
}

static inline D2D_RECT_F to_d2d_rect(const RECT& rect) {
  return D2D1::RectF((float)rect.left, (float)rect.top, (float)rect.right, (float)rect.bottom);
}

static D2D_RECT_F get_centered_rect_in_rect(const D2D_RECT_F& outerRect,
                                            float centerWidth,
                                            float centerHeight) {
  float outerWidth = outerRect.right - outerRect.left;
  float outerHeight = outerRect.bottom - outerRect.top;

  centerWidth = min(centerWidth, outerWidth);
  centerHeight = min(centerHeight, outerHeight);

  float padding_x = (outerWidth - centerWidth) / 2.f;
  float padding_y = (outerHeight - centerHeight) / 2.f;

  D2D_RECT_F center;
  center.left = outerRect.left + padding_x;
  center.top = outerRect.top + padding_y;
  center.right = center.left + centerWidth;
  center.bottom = center.top + centerHeight;

  return center;
}

void GraphicsDirectX11::drawTitlebarButtons() {
  HRESULT hr;
  D2D1_BRUSH_PROPERTIES brushProps;
  brushProps.opacity = 1.f;

  const ImGuiStyle& style = ImGui::GetStyle();

  Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> titlebarBgBrush;
  D2D1_COLOR_F* titlebarColor = (D2D1_COLOR_F*)&style.Colors[ImGuiCol_MenuBarBg];
  hr = m_d2dRenderTarget->CreateSolidColorBrush(titlebarColor, &brushProps, &titlebarBgBrush);
  if (FAILED(hr))
    return;

  Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> buttonHoveredBgBrush;
  D2D1_COLOR_F* buttonHoveredColor = (D2D1_COLOR_F*)&style.Colors[ImGuiCol_HeaderHovered];
  hr = m_d2dRenderTarget->CreateSolidColorBrush(buttonHoveredColor, &brushProps, &buttonHoveredBgBrush);
  if (FAILED(hr))
    return;

  Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> close_buttonHoveredBgBrush;
  D2D1_COLOR_F closeButtonHoveredColor = D2D1::ColorF(0.8f, 0.f, 0.f, 1.f);
  hr = m_d2dRenderTarget->CreateSolidColorBrush(&closeButtonHoveredColor, &brushProps,
                                                &close_buttonHoveredBgBrush);
  if (FAILED(hr))
    return;

  Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> buttonIconBrush;
  D2D1_COLOR_F buttonIconColor = *(D2D1_COLOR_F*)&style.Colors[ImGuiCol_Text];
  if (!isMainWindowFocused()) {
    buttonIconColor.a = .5f;
  }
  hr = m_d2dRenderTarget->CreateSolidColorBrush(&buttonIconColor, &brushProps, &buttonIconBrush);
  if (FAILED(hr))
    return;

  TitleBarButtonRects buttonRects = getTitlebarButtonRects();

  // Draw background color when hovering a button.

  TitleBarButton hoveredButton = static_cast<TitleBarButton>(GetWindowLongPtrW(m_hwnd, GWLP_USERDATA));

  if (hoveredButton != BUTTON_NONE) {
    auto rect = to_d2d_rect(buttonRects.rects[hoveredButton]);
    auto brush =
        (hoveredButton == BUTTON_CLOSE) ? close_buttonHoveredBgBrush.Get() : buttonHoveredBgBrush.Get();
    drawTitlebarButtonHover(rect, brush);
  }

  // Draw the icons on the buttons.

  auto minimizeRect = to_d2d_rect(buttonRects.minimize);
  auto maximizeRect = to_d2d_rect(buttonRects.maximize);
  auto closeRect = to_d2d_rect(buttonRects.close);

  drawTitlebarMinimizeIcon(minimizeRect, buttonIconBrush.Get());
  drawTitlebarMaximizeIcon(maximizeRect, titlebarBgBrush.Get(), buttonIconBrush.Get());
  drawTitlebarCloseIcon(closeRect, buttonIconBrush.Get());
}

void GraphicsDirectX11::drawTitlebarButtonHover(const D2D_RECT_F& buttonRect, ID2D1SolidColorBrush* brush) {
  m_d2dRenderTarget->FillRectangle(&buttonRect, brush);
}

void GraphicsDirectX11::drawTitlebarMinimizeIcon(const D2D_RECT_F& buttonRect, ID2D1SolidColorBrush* brush) {
  float iconSize = GET_UISIZE(TITLEBAR_BUTTON_ICON_SIZE);

  D2D_RECT_F iconRect = get_centered_rect_in_rect(buttonRect, iconSize, 0);

  D2D1_POINT_2F lineBegin = D2D1::Point2F(iconRect.left, iconRect.top);
  D2D1_POINT_2F lineEnd = D2D1::Point2F(iconRect.right, iconRect.top);

  m_d2dRenderTarget->DrawLine(lineBegin, lineEnd, brush);
}

void GraphicsDirectX11::drawTitlebarMaximizeIcon(const D2D_RECT_F& buttonRect,
                                                 ID2D1SolidColorBrush* bgBrush,
                                                 ID2D1SolidColorBrush* iconBrush) {
  float iconSize = GET_UISIZE(TITLEBAR_BUTTON_ICON_SIZE);

  D2D_RECT_F iconRect = get_centered_rect_in_rect(buttonRect, iconSize, iconSize);

  int boxCornerRadius = GET_UISIZE(TITLEBAR_BUTTON_MAXIMIZE_ICON_BOX_ROUNDING);

  if (!isMainWindowMaximized()) {
    D2D1_ROUNDED_RECT icon_rect_rounded = D2D1::RoundedRect(iconRect, boxCornerRadius, boxCornerRadius);
    m_d2dRenderTarget->DrawRoundedRectangle(&icon_rect_rounded, iconBrush);
    return;
  }

  float boxOffset = GET_UISIZE(TITLEBAR_BUTTON_MAXIMIZE_ICON_BOX_OFFSET);

  D2D_RECT_F frontRect = iconRect;
  frontRect.right -= boxOffset;
  frontRect.top += boxOffset;

  D2D_RECT_F backRect = iconRect;
  backRect.left += boxOffset;
  backRect.bottom -= boxOffset;

  D2D1_ROUNDED_RECT frontRectRounded = D2D1::RoundedRect(frontRect, boxCornerRadius, boxCornerRadius);
  D2D1_ROUNDED_RECT backRectRounded = D2D1::RoundedRect(backRect, boxCornerRadius, boxCornerRadius);

  m_d2dRenderTarget->DrawRoundedRectangle(&backRectRounded, iconBrush);
  m_d2dRenderTarget->FillRoundedRectangle(&frontRectRounded, bgBrush);
  m_d2dRenderTarget->DrawRoundedRectangle(&frontRectRounded, iconBrush);
}

void GraphicsDirectX11::drawTitlebarCloseIcon(const D2D_RECT_F& buttonRect, ID2D1SolidColorBrush* brush) {
  float iconSize = GET_UISIZE(TITLEBAR_BUTTON_ICON_SIZE);

  D2D_RECT_F iconRect = get_centered_rect_in_rect(buttonRect, iconSize, iconSize);

  D2D1_POINT_2F lineBegin, lineEnd;

  lineBegin = D2D1::Point2F(iconRect.left, iconRect.top);
  lineEnd = D2D1::Point2F(iconRect.right, iconRect.bottom);

  m_d2dRenderTarget->DrawLine(lineBegin, lineEnd, brush);

  lineBegin = D2D1::Point2F(iconRect.left, iconRect.bottom);
  lineEnd = D2D1::Point2F(iconRect.right, iconRect.top);

  m_d2dRenderTarget->DrawLine(lineBegin, lineEnd, brush);
}

bool GraphicsDirectX11::initDirectX() {
  if (!initD3DAndSwapChain()) {
    deinitSwapChain();
    deinitD3D();
    return false;
  }

  if (!initD2D()) {
    deinitSwapChain();
    deinitD3D();
    deinitD2D();
    return false;
  }

  initRenderTargets();

  return true;
}

void GraphicsDirectX11::deinitDirectX() {
  deinitRenderTargets();
  deinitSwapChain();
  deinitD3D();
  deinitD2D();
}

bool GraphicsDirectX11::initD3DAndSwapChain() {
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
  HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags,
                                              featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &m_swapChain,
                                              &m_device, &featureLevel, &m_deviceContext);
  if (res ==
      DXGI_ERROR_UNSUPPORTED)  // Try high-performance WARP software driver if hardware is not available.
    res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags,
                                        featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &m_swapChain, &m_device,
                                        &featureLevel, &m_deviceContext);

  return res == S_OK;
}

void GraphicsDirectX11::deinitSwapChain() {
  if (m_swapChain) {
    m_swapChain->Release();
    m_swapChain = nullptr;
  }
}

void GraphicsDirectX11::deinitD3D() {
  if (m_deviceContext) {
    m_deviceContext->Release();
    m_deviceContext = nullptr;
  }
  if (m_device) {
    m_device->Release();
    m_device = nullptr;
  }
}

bool GraphicsDirectX11::initD2D() {
  D2D1_FACTORY_OPTIONS d2dOptions = {};
#ifndef NDEBUG
  d2dOptions.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif
  // Create the D2D factory
  HRESULT res = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, d2dOptions, &m_d2dFactory);
  return res == S_OK;
}

void GraphicsDirectX11::deinitD2D() {
  if (m_d2dFactory) {
    m_d2dFactory->Release();
    m_d2dFactory = nullptr;
  }
}

void GraphicsDirectX11::initRenderTargets() {
  initD3DRenderTarget();
  initD2DRenderTarget();
}

void GraphicsDirectX11::deinitRenderTargets() {
  deinitD3DRenderTarget();
  deinitD2DRenderTarget();
}

void GraphicsDirectX11::initD3DRenderTarget() {
  ID3D11Texture2D* pBackBuffer;
  m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
  m_device->CreateRenderTargetView(pBackBuffer, nullptr, &m_mainRenderTargetView);
  pBackBuffer->Release();
}

void GraphicsDirectX11::deinitD3DRenderTarget() {
  if (m_mainRenderTargetView) {
    m_mainRenderTargetView->Release();
    m_mainRenderTargetView = nullptr;
  }
}

void GraphicsDirectX11::initD2DRenderTarget() {
  HRESULT res;

  IDXGISurface* dxgiSurface;
  res = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiSurface));

  D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
      D2D1_RENDER_TARGET_TYPE_DEFAULT,
      D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED), USER_DEFAULT_SCREEN_DPI,
      USER_DEFAULT_SCREEN_DPI);

  res = m_d2dFactory->CreateDxgiSurfaceRenderTarget(dxgiSurface, &props, &m_d2dRenderTarget);

  dxgiSurface->Release();
}

void GraphicsDirectX11::deinitD2DRenderTarget() {
  if (m_d2dRenderTarget) {
    m_d2dRenderTarget->Release();
    m_d2dRenderTarget = nullptr;
  }
}

RECT GraphicsDirectX11::getTitlebarRect() {
  RECT rect;
  GetClientRect(m_hwnd, &rect);
  rect.bottom = rect.top + m_app->menu_bar_height();
  return rect;
}

TitleBarButtonRects GraphicsDirectX11::getTitlebarButtonRects() {
  UINT dpi = GetDpiForWindow(m_hwnd);
  TitleBarButtonRects buttonRects;

  int button_width = (int)GET_UISIZE(TITLEBAR_BUTTON_WIDTH);
  buttonRects.close = getTitlebarRect();

  buttonRects.close.left = buttonRects.close.right - button_width;
  buttonRects.maximize = buttonRects.close;
  buttonRects.maximize.left -= button_width;
  buttonRects.maximize.right -= button_width;
  buttonRects.minimize = buttonRects.maximize;
  buttonRects.minimize.left -= button_width;
  buttonRects.minimize.right -= button_width;
  return buttonRects;
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

  TitleBarButton hoveredButton = static_cast<TitleBarButton>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

  switch (msg) {
    case WM_NCCALCSIZE: {
      if (!wparam)
        return DefWindowProcW(hwnd, msg, wparam, lparam);
      UINT dpi = GetDpiForWindow(hwnd);

      int frameX = GetSystemMetricsForDpi(SM_CXFRAME, dpi);
      int frameY = GetSystemMetricsForDpi(SM_CYFRAME, dpi);
      int padding = GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi);

      NCCALCSIZE_PARAMS* params = reinterpret_cast<NCCALCSIZE_PARAMS*>(lparam);
      RECT* requestedClientRect = params->rgrc;

      requestedClientRect->right -= frameX + padding;
      requestedClientRect->left += frameX + padding;
      requestedClientRect->bottom -= frameY + padding;

      if (graphics.isMainWindowMaximized()) {
        requestedClientRect->top += padding;
      }

      return 0;
    }
    case WM_CREATE: {
      SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER);
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

      if (hoveredButton == BUTTON_MAXIMIZE) {
        // Show SnapLayout on Windows 11
        return HTMAXBUTTON;
      }

      UINT dpi = GetDpiForWindow(hwnd);
      int frameY = GetSystemMetricsForDpi(SM_CYFRAME, dpi);
      int padding = GetSystemMetricsForDpi(SM_CXPADDEDBORDER, dpi);
      POINT cursorPoint;
      cursorPoint.x = GET_X_LPARAM(lparam);
      cursorPoint.y = GET_Y_LPARAM(lparam);
      ScreenToClient(hwnd, &cursorPoint);
      if (cursorPoint.y > 0 && cursorPoint.y < frameY + padding) {
        return HTTOP;
      }

      int menuBarWidth = app->menuBarWidth();  // dpi?

      RECT titlebarRect = graphics.getTitlebarRect();
      if (cursorPoint.y < titlebarRect.bottom && cursorPoint.x > titlebarRect.left + menuBarWidth) {
        return HTCAPTION;
      }

      return HTCLIENT;
    }
    case WM_GETMINMAXINFO: {
      int menuBarWidth = app->menuBarWidth();

      int buttonWidth = (int)GET_UISIZE(TITLEBAR_BUTTON_WIDTH);

      int dragAreaWidth = (int)GET_UISIZE(TITLEBAR_DRAG_AREA_MIN_WIDTH);
      int minWidth = (int)GET_UISIZE(WINDOW_MIN_WIDTH);
      int minHeight = (int)GET_UISIZE(WINDOW_MIN_HEIGHT);

      MINMAXINFO* minmax = reinterpret_cast<MINMAXINFO*>(lparam);
      minmax->ptMinTrackSize.x = max(menuBarWidth + 3 * buttonWidth + dragAreaWidth, minWidth);
      minmax->ptMinTrackSize.y = minHeight;
      return 0;
    }
    case WM_NCMOUSEMOVE: {
      POINT cursorPoint;
      GetCursorPos(&cursorPoint);
      ScreenToClient(hwnd, &cursorPoint);

      TitleBarButtonRects rects = graphics.getTitlebarButtonRects();

      TitleBarButton newHoveredButton = BUTTON_NONE;
      if (PtInRect(&rects.close, cursorPoint)) {
        newHoveredButton = BUTTON_CLOSE;
      } else if (PtInRect(&rects.minimize, cursorPoint)) {
        newHoveredButton = BUTTON_MINIMIZE;
      } else if (PtInRect(&rects.maximize, cursorPoint)) {
        newHoveredButton = BUTTON_MAXIMIZE;
      }
      if (newHoveredButton != hoveredButton) {
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, static_cast<LONG_PTR>(newHoveredButton));
      }
      break;
    }
    case WM_MOUSEMOVE: {
      if (hoveredButton != BUTTON_NONE) {
        RECT titlebarRect = graphics.getTitlebarRect();
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, static_cast<LONG_PTR>(BUTTON_NONE));
      }
      break;
    }
    case WM_NCLBUTTONDOWN: {
      if (hoveredButton != BUTTON_NONE) {
        return 0;
      }
      break;
    }
    case WM_NCLBUTTONUP: {
      if (hoveredButton == BUTTON_CLOSE) {
        app->close();
        return 0;
      } else if (hoveredButton == BUTTON_MINIMIZE) {
        ShowWindow(hwnd, SW_MINIMIZE);
        return 0;
      } else if (hoveredButton == BUTTON_MAXIMIZE) {
        int mode = graphics.isMainWindowMaximized() ? SW_NORMAL : SW_MAXIMIZE;
        ShowWindow(hwnd, mode);
        return 0;
      }
      break;
    }
    case WM_NCRBUTTONUP: {
      if (wparam == HTCAPTION) {
        BOOL const isMaximized = IsZoomed(hwnd);

        MENUITEMINFOW menuItemInfo;
        ZeroMemory(&menuItemInfo, sizeof(menuItemInfo));
        menuItemInfo.cbSize = sizeof(menuItemInfo);
        menuItemInfo.fMask = MIIM_STATE;

        HMENU const sys_menu = GetSystemMenu(hwnd, false);

        auto setMenuItemState = [&](UINT item, bool enabled) {
          menuItemInfo.fState = enabled ? MF_ENABLED : MF_DISABLED;
          SetMenuItemInfoW(sys_menu, item, false, &menuItemInfo);
        };

        setMenuItemState(SC_RESTORE, isMaximized);
        setMenuItemState(SC_MOVE, !isMaximized);
        setMenuItemState(SC_SIZE, !isMaximized);
        setMenuItemState(SC_MINIMIZE, true);
        setMenuItemState(SC_MAXIMIZE, !isMaximized);
        setMenuItemState(SC_CLOSE, true);

        BOOL const result = TrackPopupMenu(sys_menu, TPM_RETURNCMD, GET_X_LPARAM(lparam),
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
      g_resizeWidth = static_cast<UINT>(LOWORD(lparam));
      g_resizeHeight = static_cast<UINT>(HIWORD(lparam));
      return 0;
    case WM_SYSCOMMAND:
      if ((wparam & 0xfff0) == SC_KEYMENU)  // Disable ALT application menu
        return 0;
      break;
    case WM_DPICHANGED: {
      RECT* rect = (RECT*)lparam;
      SetWindowPos(hwnd, NULL, rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top,
                   SWP_NOZORDER);
      float scale = static_cast<float>(LOWORD(wparam)) / USER_DEFAULT_SCREEN_DPI;
      graphics.update_ui_scale(scale);
      return 0;
    }
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
