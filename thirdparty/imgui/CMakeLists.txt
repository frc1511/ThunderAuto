cmake_minimum_required(VERSION 3.22)
project(imgui)

option(IMGUI_DIRECTX11 "Use DirectX 11" OFF)

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

if(CMAKE_SYSTEM_NAME STREQUAL "Windows" AND IMGUI_DIRECTX11)
  list(APPEND IMGUI_SRC "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_dx11.cpp")
  list(APPEND IMGUI_SRC "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_win32.cpp")
else()
  # OpenGL/GLFW
  list(APPEND IMGUI_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_opengl3.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_glfw.cpp"
  )

  if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    list(APPEND IMGUI_SRC "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_osx.mm")
  elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  endif()

  target_link_libraries(${PROJECT_NAME} PUBLIC glfw)
endif()

add_library(${PROJECT_NAME} ${IMGUI_SRC})

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/imgui/")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/")

if(CMAKE_SYSTEM_NAME STREQUAL "Windows" AND IMGUI_DIRECTX11)
  target_link_libraries(${PROJECT_NAME} PUBLIC d3d11 d3dcompiler dwmapi)

else()
  find_package(OpenGL REQUIRED)

  target_link_libraries(${PROJECT_NAME} PUBLIC OpenGL::GL)
endif()
