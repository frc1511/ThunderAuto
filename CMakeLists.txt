cmake_minimum_required(VERSION 3.22)

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "In-source builds are not allowed.")
endif()

###
### Version
###

set(THUNDER_AUTO_VERSION_MAJOR "2026")
set(THUNDER_AUTO_VERSION_MINOR "0")
set(THUNDER_AUTO_VERSION_PATCH "0")

set(THUNDER_AUTO_VERSION "${THUNDER_AUTO_VERSION_MAJOR}.${THUNDER_AUTO_VERSION_MINOR}.${THUNDER_AUTO_VERSION_PATCH}")
set(THUNDER_AUTO_VERSION_SHORT "${THUNDER_AUTO_VERSION_MAJOR}.${THUNDER_AUTO_VERSION_MINOR}")

list(APPEND THUNDER_AUTO_DEF_LIST "THUNDER_AUTO_VERSION=${THUNDER_AUTO_VERSION_SHORT}")
list(APPEND THUNDER_AUTO_DEF_LIST "THUNDER_AUTO_VERSION_MAJOR=${THUNDER_AUTO_VERSION_MAJOR}")
list(APPEND THUNDER_AUTO_DEF_LIST "THUNDER_AUTO_VERSION_MINOR=${THUNDER_AUTO_VERSION_MINOR}")
list(APPEND THUNDER_AUTO_DEF_LIST "THUNDER_AUTO_VERSION_PATCH=${THUNDER_AUTO_VERSION_PATCH}")
list(APPEND THUNDER_AUTO_DEF_LIST "THUNDER_AUTO_VERSION_STR=\"${THUNDER_AUTO_VERSION}\"")

###
### Project
###

set(CMAKE_PROJECT_NAME ThunderAuto)
project(${CMAKE_PROJECT_NAME} VERSION ${THUNDER_AUTO_VERSION})

###
### Options
###

option(THUNDER_AUTO_DIRECTX11 "Enable DirectX 11" OFF)
option(THUNDER_AUTO_OPENGL "Enable OpenGL" OFF)

# OpenGL requires different initialization to run on macOS. By default, Windows
# builds initialize OpenGL for usage on Windows. Enable this option when you are
# cross-compiling to Windows from macOS but still want to emulate the app
# locally on macOS using wine. The resulting executable will not run on native
# Windows.
option(THUNDER_AUTO_WINDOWS_TEST_OPENGL_MACOS "Enable Windows OpenGL testing on macOS environment" OFF)
if(THUNDER_AUTO_WINDOWS_TEST_OPENGL_MACOS AND CMAKE_SYSTEM_NAME STREQUAL "Windows")
  message(STATUS "OpenGL Windows testing on macOS enabled")
  list(APPEND THUNDER_AUTO_DEF_LIST "THUNDER_AUTO_WINDOWS_TEST_OPENGL_MACOS=1")
  set(THUNDER_AUTO_DIRECTX11 OFF)
  set(THUNDER_AUTO_OPENGL ON)
endif()

# For development, it may be useful to use a local copy of ThunderLib instead of
# the one fetched from GitHub.
set(THUNDER_LIB_DIR "" CACHE PATH "Path to local ThunderLib repository checkout")

###
### Platform
###

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(THUNDER_AUTO_WINDOWS ON)
  list(APPEND THUNDER_AUTO_DEF_LIST "THUNDER_AUTO_WINDOWS=1")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  if(NOT XCODE)
    message(FATAL_ERROR "Xcode is required to build ThunderAuto on macOS")
  endif()
  set(THUNDER_AUTO_MACOS ON)
  list(APPEND THUNDER_AUTO_DEF_LIST "THUNDER_AUTO_MACOS=1")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(THUNDER_AUTO_LINUX ON)
  list(APPEND THUNDER_AUTO_DEF_LIST "THUNDER_AUTO_LINUX=1")
else()
  message(FATAL_ERROR "Target operating system '${CMAKE_SYSTEM_NAME}' not supported")
endif()

if(THUNDER_AUTO_DIRECTX11 AND THUNDER_AUTO_OPENGL)
  message(FATAL_ERROR "DirectX 11 and OpenGL cannot be enabled at the same time")
elseif(THUNDER_AUTO_DIRECTX11)
  if(NOT THUNDER_AUTO_WINDOWS)
    message(FATAL_ERROR "DirectX 11 is only supported on Windows")
  endif()
  list(APPEND THUNDER_AUTO_DEF_LIST "THUNDER_AUTO_DIRECTX11=1")
elseif(THUNDER_AUTO_WINDOWS AND NOT THUNDER_AUTO_OPENGL)
  set(THUNDER_AUTO_DIRECTX11 ON)
  list(APPEND THUNDER_AUTO_DEF_LIST "THUNDER_AUTO_DIRECTX11=1")
else()
  set(THUNDER_AUTO_OPENGL ON)
  list(APPEND THUNDER_AUTO_DEF_LIST "THUNDER_AUTO_OPENGL=1")
endif()

###
### Languages
###

enable_language(CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)

if(THUNDER_AUTO_MACOS)
  enable_language(OBJC)
  set(CMAKE_OBJC_STANDARD 11)
  set(CMAKE_OBJC_STANDARD_REQUIRED YES)
  set(CMAKE_OBJC_EXTENSIONS OFF)
  
  enable_language(OBJCXX)
  set(CMAKE_OBJCXX_STANDARD 20)
  set(CMAKE_OBJCXX_STANDARD_REQUIRED YES)
  set(CMAKE_OBJCXX_EXTENSIONS OFF)
endif()

###
### Build mode
###

# Default to release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
  set(THUNDER_AUTO_DEBUG ON)
  list(APPEND THUNDER_AUTO_DEF_LIST "THUNDER_AUTO_DEBUG")
endif()

###
### Project directories.
###

set(THUNDER_AUTO_SRC_DIR        "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(THUNDER_AUTO_INC_DIR        "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(THUNDER_AUTO_THIRDPARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty")
set(THUNDER_AUTO_RES_DIR        "${CMAKE_CURRENT_SOURCE_DIR}/res")

###
### Executable.
###

if(THUNDER_AUTO_WINDOWS)
  add_executable(${CMAKE_PROJECT_NAME} WIN32)
elseif(THUNDER_AUTO_MACOS)
  set(ICON_FILE "${THUNDER_AUTO_RES_DIR}/macos/app_icon.icns")

  set(THUNDER_AUTO_BUNDLE_IDENTIFIER    "io.github.petelilley.thunderauto")
  set(THUNDER_AUTO_BUNDLE_NAME          "ThunderAuto")
  set(THUNDER_AUTO_VERSION              "${THUNDER_AUTO_VERSION}")
  set(THUNDER_AUTO_LONG_VERSION_STRING  "${THUNDER_AUTO_VERSION}")
  set(THUNDER_AUTO_SHORT_VERSION_STRING "${THUNDER_AUTO_VERSION_SHORT}")
  set(THUNDER_AUTO_COPYRIGHT            "Copyright © 2022-2025 Peter P Lilley III. All rights reserved.")
  set(THUNDER_AUTO_APP_CATEGORY         "public.app-category.developer-tools")
  set(THUNDER_AUTO_BUNDLE_ICON_FILE     "app_icon")

  add_executable(${CMAKE_PROJECT_NAME} MACOSX_BUNDLE ${ICON_FILE})
  set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST               "${THUNDER_AUTO_RES_DIR}/macos/Info.plist.in"
    XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${THUNDER_AUTO_RES_DIR}/macos/ThunderAuto.entitlements"
    RESOURCE                               "${ICON_FILE}"
  )

elseif(THUNDER_AUTO_LINUX)
  add_executable(${CMAKE_PROJECT_NAME})
endif()

###
### Include directories.
###

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${THUNDER_AUTO_INC_DIR})
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/res")

###
### Precompiled headers.
###

function(add_thunder_auto_pch)
  set(THUNDER_AUTO_PCH ${THUNDER_AUTO_PCH} ${ARGV} PARENT_SCOPE)
endfunction()

include("${THUNDER_AUTO_INC_DIR}/ThunderAuto/CMakeLists.txt")

target_precompile_headers(${CMAKE_PROJECT_NAME} PRIVATE ${THUNDER_AUTO_PCH})

###
### Sources.
###

function(add_thunder_auto_sources)
  set(THUNDER_AUTO_SRC ${THUNDER_AUTO_SRC} ${ARGV} PARENT_SCOPE)
endfunction()

include("${THUNDER_AUTO_SRC_DIR}/CMakeLists.txt")

target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${THUNDER_AUTO_SRC})

###
### Link ThunderLib.
###

include(FetchContent)

if(THUNDER_LIB_DIR)
  message(STATUS "Using local ThunderLib checkout: ${THUNDER_LIB_DIR}")
  add_subdirectory(${THUNDER_LIB_DIR} "${CMAKE_CURRENT_BINARY_DIR}/thunderlib")
else()
  FetchContent_Declare(
    ThunderLib
    GIT_REPOSITORY https://github.com/petelilley/ThunderLib.git
    GIT_TAG main
    GIT_SHALLOW TRUE
  )
  FetchContent_MakeAvailable(ThunderLib)
endif()

target_link_libraries(${PROJECT_NAME} ThunderLib)

###
### Link thirdparty libraries.
###

# glad

if(THUNDER_AUTO_OPENGL)
  add_subdirectory("${THUNDER_AUTO_THIRDPARTY_DIR}/glad")
  target_link_libraries(${PROJECT_NAME} glad)
endif()

# imgui

set(IMGUI_DEFINE_MATH_OPERATORS ON)
if(THUNDER_AUTO_DIRECTX11)
  set(IMGUI_BACKEND_RENDERER "dx11")
  set(IMGUI_BACKEND_PLATFORM "win32")
else()
  set(IMGUI_BACKEND_RENDERER "opengl3")
  set(IMGUI_BACKEND_PLATFORM "glfw")
endif()

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/petelilley/imgui.git
  GIT_TAG master
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(imgui)
target_link_libraries(${PROJECT_NAME} imgui)

# stb_image

add_subdirectory("${THUNDER_AUTO_THIRDPARTY_DIR}/stb_image")
target_link_libraries(${PROJECT_NAME} stb_image)

# IconFontCppHeaders

add_subdirectory("${THUNDER_AUTO_THIRDPARTY_DIR}/IconFontCppHeaders")
target_link_libraries(${PROJECT_NAME} icons_font_awesome)

###
### Link system libraries.
###

if (THUNDER_AUTO_DIRECTX11)
  target_link_libraries(${PROJECT_NAME} user32 gdi32 uxtheme d2d1)
endif()

if(THUNDER_AUTO_MACOS)
  target_link_libraries(${PROJECT_NAME} "-framework Foundation -framework AppKit -framework UniformTypeIdentifiers")
endif()

###
### Compiler & Linker flags.
###

if(MSVC)
  message(STATUS "MSVC Compiler Detected")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
  target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall -Wextra) # -Werror
endif()

# Cross-compiling
if(CMAKE_CXX_COMPILER MATCHES ".*w64-mingw32.*")
  message(STATUS "MinGW detected")
  target_link_options(${CMAKE_PROJECT_NAME} PRIVATE -static-libgcc -static-libstdc++)
endif()

###
### Compile definitions.
###

target_compile_definitions(${PROJECT_NAME} PRIVATE ${THUNDER_AUTO_DEF_LIST})

###
### Resources.
###

function(build_resource RES_FILE RES_NAME)
  set(RES_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/res/${RES_NAME}.h")
  file(REMOVE ${RES_OUTPUT})
  file(READ ${RES_FILE} filedata HEX)
  string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," filedata ${filedata})
  file(APPEND ${RES_OUTPUT} "unsigned char ${RES_NAME}[] = {${filedata}};\nconst unsigned ${RES_NAME}_size = sizeof(${RES_NAME});\n")
endfunction()

build_resource("${THUNDER_AUTO_RES_DIR}/images/field-2022.png"                 "field_2022_png")
build_resource("${THUNDER_AUTO_RES_DIR}/images/field-2023.png"                 "field_2023_png")
build_resource("${THUNDER_AUTO_RES_DIR}/images/field-2024.png"                 "field_2024_png")
build_resource("${THUNDER_AUTO_RES_DIR}/images/field-2025.png"                 "field_2025_png")
build_resource("${THUNDER_AUTO_RES_DIR}/fonts/Roboto/Roboto-Regular.ttf"       "Roboto_Regular_ttf")
build_resource("${THUNDER_AUTO_RES_DIR}/fonts/Roboto/Roboto-Bold.ttf"          "Roboto_Bold_ttf")
build_resource("${THUNDER_AUTO_RES_DIR}/fonts/Ubuntu/Ubuntu-Regular.ttf"       "Ubuntu_Regular_ttf")
build_resource("${THUNDER_AUTO_RES_DIR}/fonts/Ubuntu/Ubuntu-Bold.ttf"          "Ubuntu_Bold_ttf")
build_resource("${THUNDER_AUTO_RES_DIR}/fonts/Font-Awesome/fa-regular-400.ttf" "FontAwesome_Regular_ttf")
build_resource("${THUNDER_AUTO_RES_DIR}/fonts/Font-Awesome/fa-solid-900.ttf"   "FontAwesome_Solid_ttf")
