#pragma once

#include <ThunderLibCore/Types.hpp>
#include <imgui.h>
#include <imgui_internal.h>

using namespace thunder::core;

inline ImVec2 ToImVec2(const Vec2& pt) {
  return ImVec2(static_cast<float>(pt.x), static_cast<float>(pt.y));
}

inline ImVec2 ToImVec2(const Displacement2d& pt) {
  return ImVec2(static_cast<float>(pt.x()), static_cast<float>(pt.y()));
}

inline ImVec2 ToImVec2(const Measurement2d& pt) {
  return ImVec2(static_cast<float>(pt.x()), static_cast<float>(pt.y()));
}

inline ImVec2 ToImVec2(const Point2d& pt) {
  return ImVec2(static_cast<float>(pt.x()), static_cast<float>(pt.y()));
}

inline ImRect ToImRect(const Rect& rect) {
  return ImRect(ToImVec2(rect.min), ToImVec2(rect.max));
}

inline Vec2 ToVec2(const ImVec2& pt) {
  return Vec2(static_cast<double>(pt.x), static_cast<double>(pt.y));
}

inline Displacement2d ToDisplacement2d(const ImVec2& pt) {
  return Displacement2d(static_cast<double>(pt.x), static_cast<double>(pt.y));
}

inline Measurement2d ToMeasurement2d(const ImVec2& pt) {
  return Measurement2d(static_cast<double>(pt.x), static_cast<double>(pt.y));
}

inline Point2d ToPoint2d(const ImVec2& pt) {
  return Point2d(static_cast<double>(pt.x), static_cast<double>(pt.y));
}

inline Rect ToRect(const ImRect& rect) {
  return Rect(ToVec2(rect.Min), ToVec2(rect.Max));
}
