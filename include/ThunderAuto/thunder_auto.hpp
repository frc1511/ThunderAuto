#pragma once

#include <algorithm>
#include <iomanip>
#include <cmath>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <numbers>
#include <numeric>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

using namespace std::literals;
using std::max;
using std::min;

#include <nlohmann/json.hpp>

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_raii.h>

#if THUNDER_AUTO_WINDOWS
#include <Windows.h>
#endif

#if THUNDER_AUTO_DIRECTX11
#include <d3d11.h>
#include <d2d1.h>
#include <tchar.h>
#include <wrl/client.h>
#include <windowsx.h>
#else // THUNDER_AUTO_OPENGL
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#if THUNDER_AUTO_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif
#endif

