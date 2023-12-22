cmake_minimum_required(VERSION 3.5)

set(THUNDER_AUTO_VERSION_MAJOR "0")
set(THUNDER_AUTO_VERSION_MINOR "3")
set(THUNDER_AUTO_VERSION_PATCH "0")
set(THUNDER_AUTO_VERSION "${THUNDER_AUTO_VERSION_MAJOR}.${THUNDER_AUTO_VERSION_MINOR}.${THUNDER_AUTO_VERSION_PATCH}")
set(THUNDER_AUTO_VERSION_SHORT "${THUNDER_AUTO_VERSION_MAJOR}.${THUNDER_AUTO_VERSION_MINOR}")

list(APPEND THUNDER_AUTO_DEF_LIST "THUNDER_AUTO_VERSION=${THUNDER_AUTO_VERSION_SHORT}")
list(APPEND THUNDER_AUTO_DEF_LIST "THUNDER_AUTO_VERSION_MAJOR=${THUNDER_AUTO_VERSION_MAJOR}")
list(APPEND THUNDER_AUTO_DEF_LIST "THUNDER_AUTO_VERSION_MINOR=${THUNDER_AUTO_VERSION_MINOR}")
list(APPEND THUNDER_AUTO_DEF_LIST "THUNDER_AUTO_VERSION_PATCH=${THUNDER_AUTO_VERSION_PATCH}")
list(APPEND THUNDER_AUTO_DEF_LIST "THUNDER_AUTO_VERSION_STR=\"${THUNDER_AUTO_VERSION}\"")

set(THUNDER_AUTO_YEAR 2023)

project(ThunderAuto LANGUAGES C CXX VERSION ${THUNDER_AUTO_VERSION})

option(THUNDER_AUTO_TEST_MACOS "Enable testing on macOS environment" OFF)
if(THUNDER_AUTO_TEST_MACOS)
  message(STATUS "Testing on macOS enabled")
  list(APPEND THUNDER_AUTO_DEF_LIST "THUNDER_AUTO_TEST_MACOS=1")
endif()

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
  add_compile_options(/W4)
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
  set(THUNDER_AUTO_DEBUG ON)
  list(APPEND THUNDER_AUTO_DEF_LIST "THUNDER_AUTO_DEBUG")
endif()

#
# Platform.
#

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(THUNDER_AUTO_WINDOWS ON)
  list(APPEND THUNDER_AUTO_DEF_LIST "THUNDER_AUTO_WINDOWS=1")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(THUNDER_AUTO_MACOS ON)
  list(APPEND THUNDER_AUTO_DEF_LIST "THUNDER_AUTO_MACOS=1")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(THUNDER_AUTO_LINUX ON)
  list(APPEND THUNDER_AUTO_DEF_LIST "THUNDER_AUTO_LINUX=1")
else()
  message(FATAL_ERROR "Target operating system '${CMAKE_SYSTEM_NAME}' not supported")
endif()

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

#
# Sources.
#

set(THUNDER_AUTO_SRC_DIR        "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(THUNDER_AUTO_INC_DIR        "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(THUNDER_AUTO_THIRDPARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty")
set(THUNDER_AUTO_RES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/res")

function(add_thunder_auto_sources)
  set(THUNDER_AUTO_SRC ${THUNDER_AUTO_SRC} ${ARGV} PARENT_SCOPE)
endfunction()

# Add sources.
include("${THUNDER_AUTO_SRC_DIR}/CMakeLists.txt")

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

build_thunder_auto_resource("${THUNDER_AUTO_RES_DIR}/images/field-2022.png"           "field_2022_png")
build_thunder_auto_resource("${THUNDER_AUTO_RES_DIR}/images/field-2023.png"           "field_2023_png")
build_thunder_auto_resource("${THUNDER_AUTO_RES_DIR}/fonts/Roboto/Roboto-Regular.ttf" "Roboto_Regular_ttf")
build_thunder_auto_resource("${THUNDER_AUTO_RES_DIR}/fonts/Roboto/Roboto-Bold.ttf"    "Roboto_Bold_ttf")
build_thunder_auto_resource("${THUNDER_AUTO_RES_DIR}/fonts/Ubuntu/Ubuntu-Regular.ttf" "Ubuntu_Regular_ttf")
build_thunder_auto_resource("${THUNDER_AUTO_RES_DIR}/fonts/Ubuntu/Ubuntu-Bold.ttf"    "Ubuntu_Bold_ttf")
build_thunder_auto_resource("${THUNDER_AUTO_RES_DIR}/fonts/Font-Awesome/fa-regular-400.ttf" "FontAwesome_Regular_ttf")
build_thunder_auto_resource("${THUNDER_AUTO_RES_DIR}/fonts/Font-Awesome/fa-solid-900.ttf" "FontAwesome_Solid_ttf")

#
# Thirdparty libraries.
#

option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
option(GLFW_BUILD_DOCS OFF)
option(GLFW_INSTALL OFF)
add_subdirectory("${THUNDER_AUTO_THIRDPARTY_DIR}/fmt")
add_subdirectory("${THUNDER_AUTO_THIRDPARTY_DIR}/glfw")
add_subdirectory("${THUNDER_AUTO_THIRDPARTY_DIR}/imgui")
add_subdirectory("${THUNDER_AUTO_THIRDPARTY_DIR}/stb_image")
add_subdirectory("${THUNDER_AUTO_THIRDPARTY_DIR}/glad")
add_subdirectory("${THUNDER_AUTO_THIRDPARTY_DIR}/IconFontCppHeaders")
add_subdirectory("${THUNDER_AUTO_THIRDPARTY_DIR}/json")

#
# Build executable.
#

if(THUNDER_AUTO_WINDOWS)
  add_executable(${PROJECT_NAME} WIN32 ${THUNDER_AUTO_SRC})
elseif(THUNDER_AUTO_MACOS)
  list(APPEND THUNDER_AUTO_RESOURCES "${THUNDER_AUTO_RES_DIR}/macos/Assets.xcassets")

  set(THUNDER_AUTO_BUNDLE_IDENTIFIER    "io.github.petelilley.thunderauto")
  set(THUNDER_AUTO_BUNDLE_NAME          "ThunderAuto")
  set(THUNDER_AUTO_VERSION              "${THUNDER_AUTO_VERSION}")
  set(THUNDER_AUTO_LONG_VERSION_STRING  "${THUNDER_AUTO_VERSION}")
  set(THUNDER_AUTO_SHORT_VERSION_STRING "${THUNDER_AUTO_VERSION_SHORT}")
  set(THUNDER_AUTO_COPYRIGHT            "Copyright © ${THUNDER_AUTO_YEAR} Peter P Lilley III. All rights reserved.")
  set(THUNDER_AUTO_APP_CATEGORY         "public.app-category.developer-tools")
  set(THUNDER_AUTO_BUNDLE_ICON_FILE     "bundle_icon.icns")

  add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${THUNDER_AUTO_SRC} ${THUNDER_AUTO_RESOURCES})
  set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST          "${THUNDER_AUTO_RES_DIR}/macos/Info.plist.in"
    RESOURCE                          "${THUNDER_AUTO_RESOURCES}"
  )
elseif(THUNDER_AUTO_LINUX)
  add_executable(${PROJECT_NAME} ${THUNDER_AUTO_SRC})
endif()

#
# Include directories.
#

target_include_directories(${PROJECT_NAME} PRIVATE ${THUNDER_AUTO_INC_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/res")

#
# Link libraries.
#

target_link_libraries(${PROJECT_NAME} fmt)
target_link_libraries(${PROJECT_NAME} glfw)
target_link_libraries(${PROJECT_NAME} imgui)
target_link_libraries(${PROJECT_NAME} stb_image)
target_link_libraries(${PROJECT_NAME} glad)
target_link_libraries(${PROJECT_NAME} icons_font_awesome)
target_link_libraries(${PROJECT_NAME} nlohmann_json)

if(THUNDER_AUTO_MACOS)
  target_link_libraries(${PROJECT_NAME} "-framework Foundation -framework AppKit -framework UniformTypeIdentifiers")
endif()

#
# Compile definitions.
#

target_compile_definitions(${PROJECT_NAME} PRIVATE ${THUNDER_AUTO_DEF_LIST})
