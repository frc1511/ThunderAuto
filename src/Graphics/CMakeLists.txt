set(THUNDERAUTO_GRAPHICS_DIR "${THUNDERAUTO_SRC_DIR}/Graphics")

add_thunder_auto_sources(
  "${THUNDERAUTO_GRAPHICS_DIR}/Graphics.cpp"
  "${THUNDERAUTO_GRAPHICS_DIR}/Texture.cpp"
)

if(THUNDERAUTO_DIRECTX11)
  add_thunder_auto_sources("${THUNDERAUTO_GRAPHICS_DIR}/DX11Graphics.cpp")
  add_thunder_auto_sources("${THUNDERAUTO_GRAPHICS_DIR}/DX11Texture.cpp")
elseif(THUNDERAUTO_OPENGL)
  add_thunder_auto_sources("${THUNDERAUTO_GRAPHICS_DIR}/OpenGLGraphics.cpp")
  add_thunder_auto_sources("${THUNDERAUTO_GRAPHICS_DIR}/OpenGLTexture.cpp")
endif()

