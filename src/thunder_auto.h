#pragma once

#include <cmath>

// Windows missed this one D:
#ifndef M_PI
#define M_PI 3.14159265359f
#endif

// Windows also missed this one D:
#ifndef M_PI_2
#define M_PI_2 1.57079632679f
#endif

#define DEG_2_RAD (M_PI / 180.0f)
#define RAD_2_DEG (180.0f / M_PI)

#include <string>
#include <string_view>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <optional>
#include <vector>
#include <limits>
#include <utility>
#include <filesystem>
#include <numeric>
#include <variant>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

#if THUNDER_AUTO_WINDOWS
# define FILE_FILTER "ThunderAuto Project (*.thunderauto)\0*.thunderauto\0"
#else
# define FILE_FILTER "thunderauto"
#endif
