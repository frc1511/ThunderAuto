#pragma once

#include <algorithm>
#include <cmath>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <numbers>
#include <numeric>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>
#include <list>

#include <fmt/core.h>
#include <nlohmann/json.hpp>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

inline bool float_eq(float x, float y) { return std::fabs(x - y) < 1e-6; }

#if TH_WINDOWS
#include <Windows.h>
#endif

#if TH_DIRECTX11
#include <d3d11.h>
#include <tchar.h>
#include <wrl/client.h>
#else // TH_OPENGL
#include <glad/glad.h>
// This order
#include <GLFW/glfw3.h>
#endif // TH_DIRECTX11

