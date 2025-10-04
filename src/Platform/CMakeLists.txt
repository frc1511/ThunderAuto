set(THUNDERAUTO_PLATFORM_DIR "${THUNDERAUTO_SRC_DIR}/Platform")

add_thunder_auto_sources(
  "${THUNDERAUTO_PLATFORM_DIR}/PlatformManager.cpp"
  "${THUNDERAUTO_PLATFORM_DIR}/Platform.cpp"
)

if(THUNDERAUTO_MACOS)
  add_thunder_auto_sources("${THUNDERAUTO_PLATFORM_DIR}/MacOS.mm")
elseif(THUNDERAUTO_WINDOWS)
  add_thunder_auto_sources("${THUNDERAUTO_PLATFORM_DIR}/Windows.cpp")
elseif(THUNDERAUTO_LINUX)
  add_thunder_auto_sources("${THUNDERAUTO_PLATFORM_DIR}/Linux.cpp")
endif()

