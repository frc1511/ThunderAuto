#pragma once

#include <ThunderLibCore/Types.hpp>
#include <vector>
#include <cstddef>

using namespace thunder::core;

using TPolyline = std::vector<Point2d>;

TPolyline CreateRoundedRectangle(Measurement2d size, units::meter_t cornerRadius, size_t numPointsPerArc = 16);

void RotatePolygon(TPolyline& polyline, CanonicalAngle angle);
void TranslatePolygon(TPolyline& polyline, Displacement2d displacement);
