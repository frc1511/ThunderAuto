cmake_minimum_required(VERSION 3.22)

set(TH_VERSION_MAJOR "2025")
set(TH_VERSION_MINOR "4")
set(TH_VERSION_PATCH "0")

set(TH_VERSION "${TH_VERSION_MAJOR}.${TH_VERSION_MINOR}.${TH_VERSION_PATCH}")
set(TH_VERSION_SHORT "${TH_VERSION_MAJOR}.${TH_VERSION_MINOR}")

list(APPEND TH_DEF_LIST "TH_VERSION=${TH_VERSION_SHORT}")
list(APPEND TH_DEF_LIST "TH_VERSION_MAJOR=${TH_VERSION_MAJOR}")
list(APPEND TH_DEF_LIST "TH_VERSION_MINOR=${TH_VERSION_MINOR}")
list(APPEND TH_DEF_LIST "TH_VERSION_PATCH=${TH_VERSION_PATCH}")
list(APPEND TH_DEF_LIST "TH_VERSION_STR=\"${TH_VERSION}\"")

project(ThunderAuto LANGUAGES C CXX VERSION ${TH_VERSION})

option(TH_WINDOWS_TEST_OPENGL_MACOS "Enable Windows OpenGL testing on macOS environment" OFF)
if(TH_WINDOWS_TEST_OPENGL_MACOS AND CMAKE_SYSTEM_NAME STREQUAL "Windows")
  message(STATUS "OpenGL Windows testing on macOS enabled")
  list(APPEND TH_DEF_LIST "TH_WINDOWS_TEST_OPENGL_MACOS=1")
endif()

option(TH_DIRECTX11 "Enable DirectX 11" OFF)
option(TH_OPENGL "Enable OpenGL" OFF)

#
# Project configuration.
#

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)

# Disable in-source builds.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "In-source builds are not allowed.")
endif()

# Compiler flags.
if(MSVC)
  message(STATUS "MSVC Compiler Detected")
else()
  add_compile_options(-Wall -Wextra -pedantic)
endif()

# For cross-compiling with MinGW.
if(CMAKE_CXX_COMPILER MATCHES ".*w64-mingw32.*")
  message(STATUS "MinGW detected")
  add_link_options(-static-libgcc -static-libstdc++)
endif()

# Default to release.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

# Debug build
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  # compile_commands.json
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
  set(TH_DEBUG ON)
  list(APPEND TH_DEF_LIST "TH_DEBUG")
endif()

#
# Platform.
#

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(TH_WINDOWS ON)
  list(APPEND TH_DEF_LIST "TH_WINDOWS=1")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(TH_MACOS ON)
  list(APPEND TH_DEF_LIST "TH_MACOS=1")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(TH_LINUX ON)
  list(APPEND TH_DEF_LIST "TH_LINUX=1")
else()
  message(FATAL_ERROR "Target operating system '${CMAKE_SYSTEM_NAME}' not supported")
endif()

if(TH_DIRECTX11 AND TH_OPENGL)
  message(FATAL_ERROR "DirectX 11 and OpenGL cannot be enabled at the same time")
elseif(TH_DIRECTX11)
  if(NOT TH_WINDOWS)
    message(FATAL_ERROR "DirectX 11 is only supported on Windows")
  endif()
  list(APPEND TH_DEF_LIST "TH_DIRECTX11=1")
elseif(TH_WINDOWS AND NOT TH_OPENGL)
  set(TH_DIRECTX11 ON)
  list(APPEND TH_DEF_LIST "TH_DIRECTX11=1")
else()
  set(TH_OPENGL ON)
  list(APPEND TH_DEF_LIST "TH_OPENGL=1")
endif()

if(TH_MACOS)
  enable_language(OBJC)
  set(CMAKE_OBJC_STANDARD 11)
  set(CMAKE_OBJC_STANDARD_REQUIRED YES)
  set(CMAKE_OBJC_EXTENSIONS OFF)
  
  enable_language(OBJCXX)
  set(CMAKE_OBJCXX_STANDARD 20)
  set(CMAKE_OBJCXX_STANDARD_REQUIRED YES)
  set(CMAKE_OBJCXX_EXTENSIONS OFF)
endif()

#
# Sources.
#

set(TH_SRC_DIR        "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(TH_INC_DIR        "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(TH_THIRDPARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty")
set(TH_RES_DIR        "${CMAKE_CURRENT_SOURCE_DIR}/res")

function(add_thunder_auto_sources)
  set(TH_SRC ${TH_SRC} ${ARGV} PARENT_SCOPE)
endfunction()

# Add sources.
include("${TH_SRC_DIR}/CMakeLists.txt")

#
# Build resources.
#

function(build_thunder_auto_resource RES_FILE RES_NAME)
  set(RES_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/res/${RES_NAME}.h")
  file(REMOVE ${RES_OUTPUT})
  file(READ ${RES_FILE} filedata HEX)
  string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," filedata ${filedata})
  file(APPEND ${RES_OUTPUT} "unsigned char ${RES_NAME}[] = {${filedata}};\nconst unsigned ${RES_NAME}_size = sizeof(${RES_NAME});\n")
endfunction()

build_thunder_auto_resource("${TH_RES_DIR}/images/field-2022.png"           "field_2022_png")
build_thunder_auto_resource("${TH_RES_DIR}/images/field-2023.png"           "field_2023_png")
build_thunder_auto_resource("${TH_RES_DIR}/images/field-2024.png"           "field_2024_png")
build_thunder_auto_resource("${TH_RES_DIR}/images/field-2025.png"           "field_2025_png")
build_thunder_auto_resource("${TH_RES_DIR}/fonts/Roboto/Roboto-Regular.ttf" "Roboto_Regular_ttf")
build_thunder_auto_resource("${TH_RES_DIR}/fonts/Roboto/Roboto-Bold.ttf"    "Roboto_Bold_ttf")
build_thunder_auto_resource("${TH_RES_DIR}/fonts/Ubuntu/Ubuntu-Regular.ttf" "Ubuntu_Regular_ttf")
build_thunder_auto_resource("${TH_RES_DIR}/fonts/Ubuntu/Ubuntu-Bold.ttf"    "Ubuntu_Bold_ttf")
build_thunder_auto_resource("${TH_RES_DIR}/fonts/Font-Awesome/fa-regular-400.ttf" "FontAwesome_Regular_ttf")
build_thunder_auto_resource("${TH_RES_DIR}/fonts/Font-Awesome/fa-solid-900.ttf" "FontAwesome_Solid_ttf")

#
# Thirdparty libraries.
#

if(TH_OPENGL)
  add_subdirectory("${TH_THIRDPARTY_DIR}/glad")
endif()

set(IMGUI_DEFINE_MATH_OPERATORS ON)
if(TH_DIRECTX11)
  set(IMGUI_BACKEND_RENDERER "dx11")
  set(IMGUI_BACKEND_PLATFORM "win32")
else()
  set(IMGUI_BACKEND_RENDERER "opengl3")
  set(IMGUI_BACKEND_PLATFORM "glfw")
endif()
add_subdirectory("${TH_THIRDPARTY_DIR}/imgui")
add_subdirectory("${TH_THIRDPARTY_DIR}/fmt")
add_subdirectory("${TH_THIRDPARTY_DIR}/stb_image")
add_subdirectory("${TH_THIRDPARTY_DIR}/IconFontCppHeaders")
add_subdirectory("${TH_THIRDPARTY_DIR}/json")

#
# Build executable.
#

if(TH_WINDOWS)
  add_executable(${PROJECT_NAME} WIN32 ${TH_SRC})
elseif(TH_MACOS)
  set(ICON_FILE "${TH_RES_DIR}/macos/app_icon.icns")

  set(TH_BUNDLE_IDENTIFIER    "io.github.petelilley.thunderauto")
  set(TH_BUNDLE_NAME          "ThunderAuto")
  set(TH_VERSION              "${TH_VERSION}")
  set(TH_LONG_VERSION_STRING  "${TH_VERSION}")
  set(TH_SHORT_VERSION_STRING "${TH_VERSION_SHORT}")
  set(TH_COPYRIGHT            "Copyright © 2022-2024 Peter P Lilley III. All rights reserved.")
  set(TH_APP_CATEGORY         "public.app-category.developer-tools")
  set(TH_BUNDLE_ICON_FILE     "app_icon")

  add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${TH_SRC} ${ICON_FILE})
  set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST          "${TH_RES_DIR}/macos/Info.plist.in"
    XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${TH_RES_DIR}/macos/ThunderAuto.entitlements"
    RESOURCE                          "${ICON_FILE}"
  )
  # set_source_files_properties(${ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

elseif(TH_LINUX)
  add_executable(${PROJECT_NAME} ${TH_SRC})
endif()

#
# Include directories.
#

target_include_directories(${PROJECT_NAME} PRIVATE ${TH_INC_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/res")

#
# Link libraries.
#

if (TH_DIRECTX11)
  target_link_libraries(${PROJECT_NAME} user32 gdi32 uxtheme)
else()
  target_link_libraries(${PROJECT_NAME} glad)
endif()
target_link_libraries(${PROJECT_NAME} fmt)
target_link_libraries(${PROJECT_NAME} imgui)
target_link_libraries(${PROJECT_NAME} stb_image)
target_link_libraries(${PROJECT_NAME} icons_font_awesome)
target_link_libraries(${PROJECT_NAME} nlohmann_json)

if(TH_MACOS)
  target_link_libraries(${PROJECT_NAME} "-framework Foundation -framework AppKit -framework UniformTypeIdentifiers")
endif()

#
# Compile definitions.
#

target_compile_definitions(${PROJECT_NAME} PRIVATE ${TH_DEF_LIST})
