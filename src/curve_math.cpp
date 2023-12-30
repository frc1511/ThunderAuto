#include <ThunderAuto/curve_math.h>

EquationFunc build_bezier_curve_equation(const ImVec2& pt_begin,
                                         const ImVec2& control_pt_begin,
                                         const ImVec2& pt_end,
                                         const ImVec2& control_pt_end) {

  // B(t) = (1-t)^3 * P0 + 3(1-t)^2t * P1 + 3(1-t)t^2 * P2 + t^3 * P3
  const auto B = [](float P0, float P1, float P2, float P3, float t) -> float {
    // (1-t)^3 * P0
    const float B_0 = std::pow(1 - t, 3) * P0;

    // 3(1-t)^2t * P1
    const float B_1 = 3 * std::pow(1 - t, 2) * t * P1;

    // 3(1-t)t^2 * P2
    const float B_2 = 3 * (1 - t) * std::pow(t, 2) * P2;

    // t^3 * P3
    const float B_3 = std::pow(t, 3) * P3;

    return B_0 + B_1 + B_2 + B_3;
  };

  const EquationFunc eq = [=](float t) -> ImVec2 {
    const float x =
        B(pt_begin.x, control_pt_begin.x, control_pt_end.x, pt_end.x, t);
    const float y =
        B(pt_begin.y, control_pt_begin.y, control_pt_end.y, pt_end.y, t);

    return ImVec2(x, y);
  };

  return eq;
}

float menger_curvature(ImVec2 a, ImVec2 b, ImVec2 c) {
  // Side lengths.
  const float ab = std::hypotf(a.x - b.x, a.y - b.y);
  const float ac = std::hypotf(a.x - c.x, a.y - c.y);
  const float bc = std::hypotf(b.x - c.x, b.y - c.y);

  // Semi-perimeter.
  const float s = (ab + ac + bc) / 2.f;

  // Heron's formula.
  float radicand = s * (s - ab) * (s - ac) * (s - bc);

  // The radicand can never actually be negative or zero, although floating
  // point math sometimes says it is when really it's just very very close to
  // zero.
  if (radicand <= 0.f) return 0.f;

  const float A = std::sqrt(radicand);

  // Menger's curvature formula.
  return (4 * A) / (ab * ac * bc);
}
