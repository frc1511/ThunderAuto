#pragma once

#include <cmath>

// Windows missed this one D:
#ifndef M_PI
#define M_PI 3.14159265359
#endif

// Windows also missed this one D:
#ifndef M_PI_2
#define M_PI_2 1.57079632679
#endif

#define DEG_2_RAD (M_PI / 180.0f)
#define RAD_2_DEG (180.0f / M_PI)

#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <optional>
#include <vector>
#include <limits>
#include <utility>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

#define FILE_EXTENSION "thunderauto"
