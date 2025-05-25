#include <ThunderAuto/graphics/graphics.hpp>

#if THUNDER_AUTO_DIRECTX11
#include "graphics_directx11.hpp"
#elif THUNDER_AUTO_OPENGL
#include "graphics_opengl.hpp"
#else
#error "Unknown graphics API"
#endif

Graphics& PlatformGraphics::get() {
#if THUNDER_AUTO_DIRECTX11
  return GraphicsDirectX11::get();
#else  // THUNDER_AUTO_OPENGL
  return GraphicsOpenGL::get();
#endif
}
