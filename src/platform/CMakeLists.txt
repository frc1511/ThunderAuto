set(THUNDER_AUTO_PLATFORM_DIR "${THUNDER_AUTO_SRC_DIR}/platform")

add_thunder_auto_sources(
  "${THUNDER_AUTO_PLATFORM_DIR}/platform_manager.cpp"
)

if(THUNDER_AUTO_MACOS)
  add_thunder_auto_sources("${THUNDER_AUTO_PLATFORM_DIR}/platform_macos.mm")
elseif(THUNDER_AUTO_WINDOWS)
  add_thunder_auto_sources("${THUNDER_AUTO_PLATFORM_DIR}/platform_windows.cpp")
elseif(THUNDER_AUTO_LINUX)
  add_thunder_auto_sources("${THUNDER_AUTO_PLATFORM_DIR}/platform_linux.cpp")
endif()
