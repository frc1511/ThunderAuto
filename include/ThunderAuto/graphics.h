#pragma once

#include <ThunderAuto/thunder_auto.h>

class Graphics {
#ifdef TH_DIRECTX11
  ID3D11Device* m_device = nullptr;
  ID3D11DeviceContext* m_device_context = nullptr;
  IDXGISwapChain* m_swap_chain = nullptr;
  ID3D11RenderTargetView* m_main_render_target_view = nullptr;
#endif

  GLFWwindow* m_window = nullptr;

private:
  Graphics() = default;

public:
  static Graphics& get() {
    static Graphics instance;
    return instance;
  }

  Graphics(Graphics const&) = delete;
  void operator=(Graphics const&) = delete;

#if TH_DIRECTX11
  ID3D11Device* device() { return m_device; }
  ID3D11DeviceContext* context() { return m_device_context; }
#endif

#if TH_WINDOWS
  HWND hwnd();
#endif

  void init();
  void deinit();

  bool poll_events(); // Returns whether the window should close.

  void begin_frame();
  void end_frame();

  ImVec2 window_size() const;
  ImVec2 window_pos() const;

  void window_set_size(int width, int height);
  void window_set_pos(int x, int y);
  void window_set_title(const char* title);
  void window_focus();
  void window_set_should_close(bool value);

private:
#ifdef TH_DIRECTX11 // DirectX 11 helper functions.
  bool init_device();
  void deinit_device();
  void init_render_target();
  void deinit_render_target();
#endif
};

