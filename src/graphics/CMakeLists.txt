set(THUNDER_AUTO_GRAPHICS_DIR "${THUNDER_AUTO_SRC_DIR}/graphics")

add_thunder_auto_sources(
  "${THUNDER_AUTO_GRAPHICS_DIR}/graphics.cpp"
  "${THUNDER_AUTO_GRAPHICS_DIR}/texture.cpp"
)

if(THUNDER_AUTO_DIRECTX11)
  add_thunder_auto_sources("${THUNDER_AUTO_GRAPHICS_DIR}/graphics_directx11.cpp")
  add_thunder_auto_sources("${THUNDER_AUTO_GRAPHICS_DIR}/texture_directx11.cpp")
elseif(THUNDER_AUTO_OPENGL)
  add_thunder_auto_sources("${THUNDER_AUTO_GRAPHICS_DIR}/graphics_opengl.cpp")
  add_thunder_auto_sources("${THUNDER_AUTO_GRAPHICS_DIR}/texture_opengl.cpp")
endif()
