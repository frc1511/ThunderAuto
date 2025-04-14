#pragma once

#include <ThunderAuto/thunder_auto.hpp>

using EquationFunc = std::function<ImVec2(float)>;

//
// Builds a parametric function representing a Bezier curve.
//
EquationFunc build_bezier_curve_equation(const ImVec2& pt_begin,
                                         const ImVec2& control_pt_begin,
                                         const ImVec2& pt_end,
                                         const ImVec2& control_pt_end);

//
// Calculates the Menger curvature of three points (1/R).
//
float menger_curvature(ImVec2 a, ImVec2 b, ImVec2 c);
