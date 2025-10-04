#include <ThunderAuto/graphics/graphics.hpp>

#include <Windows.h>
#include <d3d11.h>
#include <d2d1.h>

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

class GraphicsDirectX11 final : public Graphics, public Singleton<GraphicsDirectX11> {
  ID3D11Device* m_device = nullptr;
  ID3D11DeviceContext* m_device_context = nullptr;
  IDXGISwapChain* m_swap_chain = nullptr;
  ID3D11RenderTargetView* m_main_render_target_view = nullptr;

  ID2D1Factory* m_d2d_factory = nullptr;
  ID2D1RenderTarget* m_d2d_render_target = nullptr;

  HWND m_hwnd = nullptr;
  WNDCLASSEXW m_wc = {};

  bool m_swap_chain_occluded = false;

  App* m_app = nullptr;

  bool m_init = false;

 public:
  void init(App& app) override;
  void deinit() override;

  bool is_initialized() const override { return m_init; }

  bool poll_events() override;

  void begin_frame() override;
  void end_frame() override;

  float dpi_scale() const;

  ImVec2 window_size() const override;
  void window_set_size(int width, int height) override;

  ImVec2 window_position() const override;
  void window_set_position(int x, int y) override;
  void window_move_to_center() override;

  void window_set_title(const char* title) override;
  void window_set_should_close(bool value) override;
  void window_focus() override;
  bool is_window_focused() override;
  bool is_window_maximized() override;

  void* get_platform_handle() override { return reinterpret_cast<void*>(m_hwnd); }

  ID3D11Device* device() { return m_device; }
  ID3D11DeviceContext* context() { return m_device_context; }

 private:
  void draw_titlebar_buttons();
  void draw_titlebar_button_hover(const D2D_RECT_F& button_rect, ID2D1SolidColorBrush* brush);
  void draw_titlebar_minimize_icon(const D2D_RECT_F& button_rect, ID2D1SolidColorBrush* icon_brush);
  void draw_titlebar_maximize_icon(const D2D_RECT_F& button_rect,
                                   ID2D1SolidColorBrush* bg_brush,
                                   ID2D1SolidColorBrush* icon_brush);
  void draw_titlebar_close_icon(const D2D_RECT_F& button_rect, ID2D1SolidColorBrush* icon_brush);

  // DirectX helper functions.

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

  friend LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
};
