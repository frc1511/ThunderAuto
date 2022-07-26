set(THUNDER_AUTO_PLATFORM_DIR "${THUNDER_AUTO_SRC_DIR}/platform")

add_thunder_auto_sources(
  "${THUNDER_AUTO_PLATFORM_DIR}/platform.cpp"
)

if(THUNDER_AUTO_MACOS)
  include("${THUNDER_AUTO_PLATFORM_DIR}/macos/CMakeLists.txt")
elseif(THUNDER_AUTO_WINDOWS)
  include("${THUNDER_AUTO_PLATFORM_DIR}/windows/CMakeLists.txt")
elseif(THUNDER_AUTO_LINUX)
  include("${THUNDER_AUTO_PLATFORM_DIR}/linux/CMakeLists.txt")
endif()
