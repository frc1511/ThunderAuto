#pragma once

#include <ThunderLibCore/Types.hpp>
#include <vector>
#include <cstddef>

using namespace thunder::core;

using Polyline = std::vector<Point2d>;

Polyline CreateRoundedRectangle(Measurement2d size, units::meter_t cornerRadius, size_t numPointsPerArc = 16);

void RotatePolygon(Polyline& polyline, CanonicalAngle angle);
void TranslatePolygon(Polyline& polyline, Displacement2d displacement);
