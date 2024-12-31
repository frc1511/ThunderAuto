set(TH_PLATFORM_DIR "${TH_SRC_DIR}/platform")

add_thunder_auto_sources(
  "${TH_PLATFORM_DIR}/platform_manager.cpp"
)

if(TH_MACOS)
  add_thunder_auto_sources("${TH_PLATFORM_DIR}/platform_macos.mm")
elseif(TH_WINDOWS)
  add_thunder_auto_sources("${TH_PLATFORM_DIR}/platform_windows.cpp")
elseif(TH_LINUX)
  add_thunder_auto_sources("${TH_PLATFORM_DIR}/platform_linux.cpp")
endif()
