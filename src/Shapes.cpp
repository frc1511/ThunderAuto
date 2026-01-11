#include <ThunderAuto/Shapes.hpp>
#include <gcem.hpp>
#include <algorithm>
#include <numbers>

TPolyline CreateRoundedRectangle(Measurement2d size,
                                units::meter_t cornerRadius,
                                size_t numPointsPerArc /* = 16*/) {
  cornerRadius = std::min(cornerRadius, std::min(size.width / 2.f, size.length / 2.f));

  TPolyline points;

  const units::meter_t halfWidth = size.width / 2.f;
  const units::meter_t halfLength = size.length / 2.f;

  const units::radian_t cornerAngle{std::numbers::pi_v<float> / 2.f};

  units::meter_t centerX, centerY;

  // Top right (0 -> 90 degrees)
  centerX = halfLength - cornerRadius;
  centerY = halfWidth - cornerRadius;
  for (size_t i = 0; i <= numPointsPerArc; i++) {
    units::radian_t angle = cornerAngle * (static_cast<float>(i) / numPointsPerArc);
    units::meter_t x = centerX + cornerRadius * gcem::cos(angle.value());
    units::meter_t y = centerY + cornerRadius * gcem::sin(angle.value());
    points.push_back(Point2d(x, y));
  }

  // Top left (90 -> 180 degrees)
  centerX = -halfLength + cornerRadius;
  centerY = halfWidth - cornerRadius;
  for (size_t i = 0; i <= numPointsPerArc; i++) {
    units::radian_t offsetAngle = units::radian_t(std::numbers::pi_v<float> * 0.5);
    units::radian_t angle = offsetAngle + cornerAngle * (static_cast<float>(i) / numPointsPerArc);
    units::meter_t x = centerX + cornerRadius * gcem::cos(angle.value());
    units::meter_t y = centerY + cornerRadius * gcem::sin(angle.value());
    points.push_back(Point2d(x, y));
  }

  // Bottom left (180 -> 270 degrees)
  centerX = -halfLength + cornerRadius;
  centerY = -halfWidth + cornerRadius;
  for (size_t i = 0; i <= numPointsPerArc; i++) {
    units::radian_t offsetAngle = units::radian_t(std::numbers::pi_v<float>);
    units::radian_t angle = offsetAngle + cornerAngle * (static_cast<float>(i) / numPointsPerArc);
    units::meter_t x = centerX + cornerRadius * gcem::cos(angle.value());
    units::meter_t y = centerY + cornerRadius * gcem::sin(angle.value());
    points.push_back(Point2d(x, y));
  }

  // Bottom right (270 -> 360 degrees)
  centerX = halfLength - cornerRadius;
  centerY = -halfWidth + cornerRadius;
  for (size_t i = 0; i <= numPointsPerArc; i++) {
    units::radian_t offsetAngle = units::radian_t(std::numbers::pi_v<float> * 1.5);
    units::radian_t angle = offsetAngle + cornerAngle * (static_cast<float>(i) / numPointsPerArc);
    units::meter_t x = centerX + cornerRadius * gcem::cos(angle.value());
    units::meter_t y = centerY + cornerRadius * gcem::sin(angle.value());
    points.push_back(Point2d(x, y));
  }

  return points;
}

void RotatePolygon(TPolyline& points, CanonicalAngle angle) {
  double cosAngle = angle.cos();
  double sinAngle = angle.sin();

  std::for_each(points.begin(), points.end(), [&](Point2d& point) {
    units::meter_t x = point.x * cosAngle - point.y * sinAngle;
    units::meter_t y = point.x * sinAngle + point.y * cosAngle;
    point = Point2d(x, y);
  });
}

void TranslatePolygon(TPolyline& points, Displacement2d displacement) {
  std::for_each(points.begin(), points.end(), [&displacement](Point2d& point) { point += displacement; });
}
