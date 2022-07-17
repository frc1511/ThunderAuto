include("${THUNDER_AUTO_SRC_DIR}/platform/CMakeLists.txt")
include("${THUNDER_AUTO_SRC_DIR}/pages/CMakeLists.txt")
include("${THUNDER_AUTO_SRC_DIR}/popups/CMakeLists.txt")

add_thunder_auto_sources(
  "${THUNDER_AUTO_SRC_DIR}/main.cpp"
  "${THUNDER_AUTO_SRC_DIR}/app.cpp"
  "${THUNDER_AUTO_SRC_DIR}/project.cpp"
)
