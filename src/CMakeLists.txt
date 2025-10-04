include("${THUNDERAUTO_SRC_DIR}/Platform/CMakeLists.txt")
include("${THUNDERAUTO_SRC_DIR}/Pages/CMakeLists.txt")
include("${THUNDERAUTO_SRC_DIR}/Popups/CMakeLists.txt")
include("${THUNDERAUTO_SRC_DIR}/Graphics/CMakeLists.txt")

add_thunder_auto_sources(
  "${THUNDERAUTO_SRC_DIR}/main.cpp"
  "${THUNDERAUTO_SRC_DIR}/App.cpp"
  "${THUNDERAUTO_SRC_DIR}/DocumentManager.cpp"
  "${THUNDERAUTO_SRC_DIR}/DocumentEditManager.cpp"
  "${THUNDERAUTO_SRC_DIR}/HistoryManager.cpp"
  "${THUNDERAUTO_SRC_DIR}/Logger.cpp"
  "${THUNDERAUTO_SRC_DIR}/Shapes.cpp"
  "${THUNDERAUTO_SRC_DIR}/TrajectoryHelper.cpp"
)

