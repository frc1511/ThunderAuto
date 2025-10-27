#pragma once

#include <ThunderAuto/Graphics/Graphics.hpp>

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
  ID3D11DeviceContext* m_deviceContext = nullptr;
  IDXGISwapChain* m_swapChain = nullptr;
  ID3D11RenderTargetView* m_mainRenderTargetView = nullptr;

  ID2D1Factory* m_d2dFactory = nullptr;
  ID2D1RenderTarget* m_d2dRenderTarget = nullptr;

  HWND m_hwnd = nullptr;
  WNDCLASSEXW m_wc = {};

  bool m_swapChainOccluded = false;

  App* m_app = nullptr;

  bool m_init = false;

 public:
  void init(App& app) override;
  void deinit() override;

  bool isInitialized() const override { return m_init; }

  bool pollEvents() override;

  void beginFrame() override;
  void endFrame() override;

  float getDPIScale() const;

  ImVec2 getMainWindowSize() const override;
  void setMainWindowSize(int width, int height) override;

  ImVec2 getMainWindowPosition() const override;
  void setMainWindowPosition(int x, int y) override;
  void moveMainWindowToCenter() override;

  void setMainWindowTitle(const char* title) override;
  void setMainWindowShouldClose(bool value) override;
  void focusMainWindow() override;
  bool isMainWindowFocused() override;
  bool isWindowFocused(void* platformHandle) override;
  bool isMainWindowMaximized() override;

  void* getPlatformHandle() override { return reinterpret_cast<void*>(m_hwnd); }

  ID3D11Device* device() { return m_device; }
  ID3D11DeviceContext* context() { return m_deviceContext; }

 private:
  void drawTitlebarButtons();
  void drawTitlebarButtonHover(const D2D_RECT_F& button_rect, ID2D1SolidColorBrush* brush);
  void drawTitlebarMinimizeIcon(const D2D_RECT_F& button_rect, ID2D1SolidColorBrush* icon_brush);
  void drawTitlebarMaximizeIcon(const D2D_RECT_F& button_rect,
                                   ID2D1SolidColorBrush* bg_brush,
                                   ID2D1SolidColorBrush* icon_brush);
  void drawTitlebarCloseIcon(const D2D_RECT_F& button_rect, ID2D1SolidColorBrush* icon_brush);

  // DirectX helper functions.

  bool initDirectX();
  void deinitDirectX();

  bool initD3DAndSwapChain();
  void deinitSwapChain();
  void deinitD3D();

  bool initD2D();
  void deinitD2D();

  void initRenderTargets();
  void deinitRenderTargets();

  void initD3DRenderTarget();
  void deinitD3DRenderTarget();

  void initD2DRenderTarget();
  void deinitD2DRenderTarget();

  // Window helper functions.

  RECT getTitlebarRect();
  TitleBarButtonRects getTitlebarButtonRects();

  friend LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
};
