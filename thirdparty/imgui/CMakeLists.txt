cmake_minimum_required(VERSION 3.1)
project(imgui_docking)

set(IMGUI_SRC
  "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imconfig.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_demo.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_internal.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imstb_rectpack.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imstb_textedit.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/imgui/imstb_truetype.h"
)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  list(APPEND IMGUI_SRC "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_win32.cpp")
  list(APPEND IMGUI_SRC "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_win32.h")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  list(APPEND IMGUI_SRC "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_osx.mm")
  list(APPEND IMGUI_SRC "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_osx.h")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
endif()

list(APPEND IMGUI_SRC
  "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_opengl3.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_opengl3.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_opengl3_loader.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_glfw.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_glfw.h"
)

find_package(OpenGL REQUIRED)

add_library(${PROJECT_NAME} ${IMGUI_SRC})

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/imgui/")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/")

target_link_libraries(${PROJECT_NAME} PUBLIC glfw)
target_link_libraries(${PROJECT_NAME} PUBLIC OpenGL::GL)
