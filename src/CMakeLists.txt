include("${THUNDER_AUTO_SRC_DIR}/platform/CMakeLists.txt")
include("${THUNDER_AUTO_SRC_DIR}/pages/CMakeLists.txt")
include("${THUNDER_AUTO_SRC_DIR}/popups/CMakeLists.txt")
include("${THUNDER_AUTO_SRC_DIR}/graphics/CMakeLists.txt")

add_thunder_auto_sources(
  "${THUNDER_AUTO_SRC_DIR}/main.cpp"
  "${THUNDER_AUTO_SRC_DIR}/app.cpp"
  "${THUNDER_AUTO_SRC_DIR}/angle.cpp"
  "${THUNDER_AUTO_SRC_DIR}/curve.cpp"
  "${THUNDER_AUTO_SRC_DIR}/curve_math.cpp"
  "${THUNDER_AUTO_SRC_DIR}/curve_point.cpp"
  "${THUNDER_AUTO_SRC_DIR}/curve_settings.cpp"
  "${THUNDER_AUTO_SRC_DIR}/document_manager.cpp"
  "${THUNDER_AUTO_SRC_DIR}/field.cpp"
  "${THUNDER_AUTO_SRC_DIR}/file_types.cpp"
  "${THUNDER_AUTO_SRC_DIR}/history_manager.cpp"
  "${THUNDER_AUTO_SRC_DIR}/macro_util.cpp"
  "${THUNDER_AUTO_SRC_DIR}/project_settings.cpp"
  "${THUNDER_AUTO_SRC_DIR}/project_state.cpp"
)

