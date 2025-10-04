cmake_minimum_required(VERSION 3.22)

list(APPEND CMAKE_MESSAGE_CONTEXT "ThunderAuto")
set(CMAKE_MESSAGE_CONTEXT_SHOW ON)

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "In-source builds are not allowed.")
endif()

###
### Version
###

# ThunderAuto version

file(READ "ThunderAuto-Version.txt" THUNDERAUTO_VERSION_FILE_CONTENTS)

string(REGEX REPLACE "\n" ";" THUNDERAUTO_VERSION_FILE_LINES "${THUNDERAUTO_VERSION_FILE_CONTENTS}")

list(GET THUNDERAUTO_VERSION_FILE_LINES 0 THUNDERAUTO_VERSION_MAJOR)
list(GET THUNDERAUTO_VERSION_FILE_LINES 1 THUNDERAUTO_VERSION_MINOR)
list(GET THUNDERAUTO_VERSION_FILE_LINES 2 THUNDERAUTO_VERSION_PATCH)

set(THUNDERAUTO_VERSION "${THUNDERAUTO_VERSION_MAJOR}.${THUNDERAUTO_VERSION_MINOR}.${THUNDERAUTO_VERSION_PATCH}")

list(APPEND THUNDERAUTO_DEF_LIST "THUNDERAUTO_VERSION_MAJOR=${THUNDERAUTO_VERSION_MAJOR}")
list(APPEND THUNDERAUTO_DEF_LIST "THUNDERAUTO_VERSION_MINOR=${THUNDERAUTO_VERSION_MINOR}")
list(APPEND THUNDERAUTO_DEF_LIST "THUNDERAUTO_VERSION_PATCH=${THUNDERAUTO_VERSION_PATCH}")
list(APPEND THUNDERAUTO_DEF_LIST "THUNDERAUTO_VERSION_STR=\"${THUNDERAUTO_VERSION}\"")

message(STATUS "ThunderAuto version: ${THUNDERAUTO_VERSION}")

# ThunderAuto project version (independent of ThunderAuto and ThunderLib versions)

file(READ "ThunderAutoProject-Version.txt" THUNDERAUTO_PROJECT_VERSION_FILE_CONTENTS)

string(REGEX REPLACE "\n" ";" THUNDERAUTO_PROJECT_VERSION_FILE_LINES "${THUNDERAUTO_PROJECT_VERSION_FILE_CONTENTS}")

list(GET THUNDERAUTO_PROJECT_VERSION_FILE_LINES 0 THUNDERAUTO_PROJECT_VERSION_MAJOR)
list(GET THUNDERAUTO_PROJECT_VERSION_FILE_LINES 1 THUNDERAUTO_PROJECT_VERSION_MINOR)

set(THUNDERAUTO_PROJECT_VERSION "${THUNDERAUTO_PROJECT_VERSION_MAJOR}.${THUNDERAUTO_PROJECT_VERSION_MINOR}")

list(APPEND THUNDERLIB_DEF_LIST "THUNDERAUTO_PROJECT_VERSION_MAJOR=${THUNDERAUTO_PROJECT_VERSION_MAJOR}")
list(APPEND THUNDERLIB_DEF_LIST "THUNDERAUTO_PROJECT_VERSION_MINOR=${THUNDERAUTO_PROJECT_VERSION_MINOR}")
list(APPEND THUNDERLIB_DEF_LIST "THUNDERAUTO_PROJECT_VERSION_STR=\"${THUNDERAUTO_PROJECT_VERSION}\"")

message(STATUS "ThunderAuto project version: ${THUNDERAUTO_PROJECT_VERSION}")

# ThunderLib version

file(READ "ThunderLib-Version.txt" THUNDERLIB_VERSION_FILE_CONTENTS)

string(REGEX REPLACE "\n" ";" THUNDERLIB_VERSION_FILE_LINES "${THUNDERLIB_VERSION_FILE_CONTENTS}")

list(GET THUNDERLIB_VERSION_FILE_LINES 0 THUNDERLIB_VERSION_MAJOR)
list(GET THUNDERLIB_VERSION_FILE_LINES 1 THUNDERLIB_VERSION_MINOR)
list(GET THUNDERLIB_VERSION_FILE_LINES 2 THUNDERLIB_VERSION_PATCH)

set(THUNDERLIB_VERSION "${THUNDERLIB_VERSION_MAJOR}.${THUNDERLIB_VERSION_MINOR}.${THUNDERLIB_VERSION_PATCH}")

message(STATUS "ThunderLib version: ${THUNDERLIB_VERSION}")

set(THUNDERLIB_GIT_TAG "v${THUNDERLIB_VERSION}")

###
### Project
###

set(CMAKE_PROJECT_NAME ThunderAuto)
project(${CMAKE_PROJECT_NAME})

###
### Options
###

option(THUNDERAUTO_DIRECTX11 "Enable DirectX 11" OFF)
option(THUNDERAUTO_OPENGL "Enable OpenGL" OFF)

# OpenGL requires different initialization to run on macOS. By default, Windows
# builds initialize OpenGL for usage on Windows. Enable this option when you are
# cross-compiling to Windows from macOS but still want to emulate the app
# locally on macOS using wine. The resulting executable will not run on native
# Windows.
option(THUNDERAUTO_WINDOWS_TEST_OPENGL_MACOS "Enable Windows OpenGL testing on macOS environment" OFF)
if(THUNDERAUTO_WINDOWS_TEST_OPENGL_MACOS AND CMAKE_SYSTEM_NAME STREQUAL "Windows")
  message(STATUS "OpenGL Windows testing on macOS enabled")
  list(APPEND THUNDERAUTO_DEF_LIST "THUNDERAUTO_WINDOWS_TEST_OPENGL_MACOS=1")
  set(THUNDERAUTO_DIRECTX11 OFF)
  set(THUNDERAUTO_OPENGL ON)
endif()

# For development, it may be useful to use a local copy of ThunderLib instead of
# the one fetched from GitHub.
set(THUNDERLIB_DIR "" CACHE PATH "Path to local ThunderLib repository checkout")

option(BUILD_WITH_ASSERTIONS "Build with assertions" OFF) # ON for debug builds

###
### Platform
###

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(THUNDERAUTO_WINDOWS ON)
  list(APPEND THUNDERAUTO_DEF_LIST "THUNDERAUTO_WINDOWS=1")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(THUNDERAUTO_MACOS ON)
  list(APPEND THUNDERAUTO_DEF_LIST "THUNDERAUTO_MACOS=1")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(THUNDERAUTO_LINUX ON)
  list(APPEND THUNDERAUTO_DEF_LIST "THUNDERAUTO_LINUX=1")
else()
  message(FATAL_ERROR "Target operating system '${CMAKE_SYSTEM_NAME}' not supported")
endif()

if(THUNDERAUTO_DIRECTX11 AND THUNDERAUTO_OPENGL)
  message(FATAL_ERROR "DirectX 11 and OpenGL cannot be enabled at the same time")
elseif(THUNDERAUTO_DIRECTX11)
  if(NOT THUNDERAUTO_WINDOWS)
    message(FATAL_ERROR "DirectX 11 is only supported on Windows")
  endif()
  list(APPEND THUNDERAUTO_DEF_LIST "THUNDERAUTO_DIRECTX11=1")
elseif(THUNDERAUTO_WINDOWS AND NOT THUNDERAUTO_OPENGL)
  set(THUNDERAUTO_DIRECTX11 ON)
  list(APPEND THUNDERAUTO_DEF_LIST "THUNDERAUTO_DIRECTX11=1")
else()
  set(THUNDERAUTO_OPENGL ON)
  list(APPEND THUNDERAUTO_DEF_LIST "THUNDERAUTO_OPENGL=1")
endif()

###
### Languages
###

enable_language(CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)

if(THUNDERAUTO_MACOS)
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
### Build settings
###

# Default to release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(THUNDERAUTO_DEBUG ON)
  list(APPEND THUNDERAUTO_DEF_LIST "THUNDERAUTO_DEBUG")
  set(BUILD_WITH_ASSERTIONS ON)
endif()

message(STATUS "Build type: " ${CMAKE_BUILD_TYPE})

# Enable compile commands
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

###
### Project directories
###

set(THUNDERAUTO_SRC_DIR        "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(THUNDERAUTO_INC_DIR        "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(THUNDERAUTO_THIRDPARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty")
set(THUNDERAUTO_RES_DIR        "${CMAKE_CURRENT_SOURCE_DIR}/res")

###
### Executable
###

if(THUNDERAUTO_WINDOWS)
  add_executable(${CMAKE_PROJECT_NAME} WIN32)
elseif(THUNDERAUTO_MACOS)
  set(ICON_FILE "${THUNDERAUTO_RES_DIR}/macos/app_icon.icns")

  set(THUNDERAUTO_BUNDLE_DEVELOPMENT_REGION "en-US")
  set(THUNDERAUTO_BUNDLE_IDENTIFIER         "io.github.frc1511.thunderauto")
  set(THUNDERAUTO_BUNDLE_NAME               "ThunderAuto")
  set(THUNDERAUTO_COPYRIGHT                 "Copyright © 2022-2025 ThunderAuto Contributors. All rights reserved.")
  set(THUNDERAUTO_APP_CATEGORY              "public.app-category.developer-tools")
  set(THUNDERAUTO_BUNDLE_ICON_FILE          "app_icon")

  add_executable(${CMAKE_PROJECT_NAME} MACOSX_BUNDLE ${ICON_FILE})
  set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST               "${THUNDERAUTO_RES_DIR}/macos/Info.plist.in"
    XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${THUNDERAUTO_RES_DIR}/macos/ThunderAuto.entitlements"
    RESOURCE                               "${ICON_FILE}"
  )

elseif(THUNDERAUTO_LINUX)
  add_executable(${CMAKE_PROJECT_NAME})
endif()

###
### Include directories
###

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${THUNDERAUTO_INC_DIR})
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/res")

###
### Sources
###

function(add_thunder_auto_sources)
  set(THUNDERAUTO_SRC ${THUNDERAUTO_SRC} ${ARGV} PARENT_SCOPE)
endfunction()

include("${THUNDERAUTO_SRC_DIR}/CMakeLists.txt")

target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${THUNDERAUTO_SRC})

###
### Link ThunderLibCore
###

include(FetchContent)

if(THUNDERLIB_DIR)
  message(STATUS "Using local ThunderLib checkout: ${THUNDERLIB_DIR}")
  add_subdirectory(${THUNDERLIB_DIR} "${CMAKE_CURRENT_BINARY_DIR}/ThunderLib")
else()
  FetchContent_Declare(
    ThunderLib
    GIT_REPOSITORY https://github.com/petelilley/ThunderLib.git
    GIT_TAG ${THUNDERLIB_GIT_TAG}
    GIT_SHALLOW TRUE
  )
  FetchContent_MakeAvailable(ThunderLib)
endif()

target_link_libraries(${PROJECT_NAME} ThunderLibCore)

###
### Link thirdparty libraries
###

# glad

if(THUNDERAUTO_OPENGL)
  add_subdirectory("${THUNDERAUTO_THIRDPARTY_DIR}/glad")
  target_link_libraries(${PROJECT_NAME} glad)
endif()

# imgui

set(IMGUI_DEFINE_MATH_OPERATORS ON)
if(THUNDERAUTO_DIRECTX11)
  set(IMGUI_BACKEND_RENDERER "dx11")
  set(IMGUI_BACKEND_PLATFORM "win32")
else()
  set(IMGUI_BACKEND_RENDERER "opengl3")
  set(IMGUI_BACKEND_PLATFORM "glfw")
endif()

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/petelilley/imgui.git
  GIT_TAG docking
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(imgui)
target_link_libraries(${PROJECT_NAME} imgui)

# stb_image

add_subdirectory("${THUNDERAUTO_THIRDPARTY_DIR}/stb_image")
target_link_libraries(${PROJECT_NAME} stb_image)

# IconFontCppHeaders

add_subdirectory("${THUNDERAUTO_THIRDPARTY_DIR}/IconFontCppHeaders")
target_link_libraries(${PROJECT_NAME} icons_font_awesome)

###
### Link system libraries
###

if (THUNDERAUTO_DIRECTX11)
  target_link_libraries(${PROJECT_NAME} user32 gdi32 uxtheme d2d1)
endif()

if(THUNDERAUTO_MACOS)
  target_link_libraries(${PROJECT_NAME} "-framework Foundation -framework AppKit -framework UniformTypeIdentifiers")
endif()

###
### Compiler & Linker flags
###

if(MSVC)
  message(STATUS "MSVC Compiler Detected")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
  target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall -Wextra -Werror

    -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-function -Wno-unused-private-field
  )
endif()

# Cross-compiling
if(CMAKE_CXX_COMPILER MATCHES ".*w64-mingw32.*")
  message(STATUS "MinGW detected")
  target_link_options(${CMAKE_PROJECT_NAME} PRIVATE -static-libgcc -static-libstdc++)
endif()

###
### Compile definitions
###

target_compile_definitions(${PROJECT_NAME} PRIVATE ${THUNDERAUTO_DEF_LIST})

###
### Resources
###

function(build_resource RES_FILE RES_NAME)
  set(RES_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/res/${RES_NAME}.h")
  file(REMOVE ${RES_OUTPUT})
  file(READ ${RES_FILE} filedata HEX)
  string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," filedata ${filedata})
  file(APPEND ${RES_OUTPUT} "unsigned char ${RES_NAME}[] = {${filedata}};\nconst unsigned ${RES_NAME}_size = sizeof(${RES_NAME});\n")
endfunction()

build_resource("${THUNDERAUTO_RES_DIR}/images/field-2022.png"                 "field_2022_png")
build_resource("${THUNDERAUTO_RES_DIR}/images/field-2023.png"                 "field_2023_png")
build_resource("${THUNDERAUTO_RES_DIR}/images/field-2024.png"                 "field_2024_png")
build_resource("${THUNDERAUTO_RES_DIR}/images/field-2025.png"                 "field_2025_png")
build_resource("${THUNDERAUTO_RES_DIR}/fonts/Roboto/Roboto-Regular.ttf"       "Roboto_Regular_ttf")
build_resource("${THUNDERAUTO_RES_DIR}/fonts/Roboto/Roboto-Bold.ttf"          "Roboto_Bold_ttf")
build_resource("${THUNDERAUTO_RES_DIR}/fonts/Ubuntu/Ubuntu-Regular.ttf"       "Ubuntu_Regular_ttf")
build_resource("${THUNDERAUTO_RES_DIR}/fonts/Ubuntu/Ubuntu-Bold.ttf"          "Ubuntu_Bold_ttf")
build_resource("${THUNDERAUTO_RES_DIR}/fonts/Font-Awesome/fa-regular-400.ttf" "FontAwesome_Regular_ttf")
build_resource("${THUNDERAUTO_RES_DIR}/fonts/Font-Awesome/fa-solid-900.ttf"   "FontAwesome_Solid_ttf")

