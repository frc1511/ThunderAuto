#pragma once

#include <ThunderAuto/thunder_auto.hpp>
#include <ThunderAuto/singleton.hpp>
#include <ThunderAuto/app.hpp>

#if THUNDER_AUTO_DIRECTX11
enum TitleBarButton {
  BUTTON_NONE = -1,
  BUTTON_MINIMIZE,
  BUTTON_MAXIMIZE,
  BUTTON_CLOSE,
};
union TitleBarButtonRects {
  RECT rects[3];
  struct {
    RECT minimize;
    RECT maximize;
    RECT close;
  };
};
#endif

class Graphics : public Singleton<Graphics> {
#if THUNDER_AUTO_DIRECTX11
  ID3D11Device* m_device = nullptr;
  ID3D11DeviceContext* m_device_context = nullptr;
  IDXGISwapChain* m_swap_chain = nullptr;
  ID3D11RenderTargetView* m_main_render_target_view = nullptr;

  ID2D1Factory* m_d2d_factory = nullptr;
  ID2D1RenderTarget* m_d2d_render_target = nullptr;

  HWND m_hwnd;
  WNDCLASSEXW m_wc = {};

  bool m_swap_chain_occluded = false;

#else
  GLFWwindow* m_window = nullptr;
#endif

  App* m_app = nullptr;

 public:
#if THUNDER_AUTO_DIRECTX11
  ID3D11Device* device() { return m_device; }
  ID3D11DeviceContext* context() { return m_device_context; }
#endif

#if THUNDER_AUTO_WINDOWS
  HWND hwnd();
#endif

  void init(App& app);
  void deinit();

  bool poll_events();  // Returns whether the window should close.

  void begin_frame();
  void end_frame();

  ImVec2 window_size() const;
  ImVec2 window_pos() const;

  bool is_maximized();
  bool is_focused();

  void window_set_size(int width, int height);
  void window_set_pos(int x, int y);
  void window_set_title(const char* title);
  void window_focus();
  void window_set_should_close(bool value);

 private:
#ifdef THUNDER_AUTO_DIRECTX11
  void draw_titlebar_buttons();
  void draw_titlebar_button_hover(const D2D_RECT_F& button_rect,
                                  ID2D1SolidColorBrush* brush);
  void draw_titlebar_minimize_icon(const D2D_RECT_F& button_rect,
                                   ID2D1SolidColorBrush* icon_brush);
  void draw_titlebar_maximize_icon(const D2D_RECT_F& button_rect,
                                   ID2D1SolidColorBrush* bg_brush,
                                   ID2D1SolidColorBrush* icon_brush);
  void draw_titlebar_close_icon(const D2D_RECT_F& button_rect,
                                ID2D1SolidColorBrush* icon_brush);

  // DirectX 11 helper functions.

  bool init_directx();
  void deinit_directx();

  bool init_d3d_and_swapchain();
  void deinit_swapchain();
  void deinit_d3d();

  bool init_d2d();
  void deinit_d2d();

  void init_render_targets();
  void deinit_render_targets();

  void init_d3d_render_target();
  void deinit_d3d_render_target();

  void init_d2d_render_target();
  void deinit_d2d_render_target();

  // Window helper functions.

  RECT get_titlebar_rect();
  TitleBarButtonRects get_title_bar_button_rects();

  static inline int dpi_scale(int value, UINT dpi) {
    return static_cast<int>(value * dpi / 96.f);
  }

  friend LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
#endif
};
