#pragma once

#include <ThunderAuto/thunder_auto.hpp>

#include <ThunderAuto/curve_math.hpp>
#include <ThunderAuto/curve_point.hpp>
#include <ThunderAuto/curve_settings.hpp>

enum OutputCurvePointFlags {
  OUTPUT_CURVE_POINT_FLAG_ROTATION_DONE = 1 << 0,
};

struct OutputCurvePoint {
  float time;
  ImVec2 position;
  float velocity;
  Angle rotation;
  float angular_velocity;
  uint32_t actions;

  float distance;
  float curvature;
  float centripetal_accel;

  uint32_t flags;

  std::size_t segment_index;
};

struct OutputCurveSegment {
  // float distance;
  float time;
  Angle begin_rotation;
  Angle end_rotation;
  size_t begin_index;
  size_t end_index;
  float rotation_time_percent;
};

struct OutputCurve {
  float total_distance;
  std::vector<OutputCurvePoint> points;
};

struct OutputCurveSettings {
  // Number of samples per curve segment. This is used to initially calculate
  // the length of the curve.
  std::size_t length_samples;

  // Number of samples per meter of the curve. This affects the resolution of
  // the output curve.
  std::size_t samples_per_meter;
};

extern const OutputCurveSettings preview_output_curve_settings;
extern const OutputCurveSettings high_res_output_curve_settings;

class Curve {
  CurveSettings m_settings;
  std::vector<CurvePoint> m_points;

public:
  inline explicit Curve(CurveSettings settings = {})
    : m_settings(settings) {}

  inline explicit Curve(std::initializer_list<CurvePoint> points,
                        CurveSettings settings = {})
    : m_settings(settings),
      m_points(points) {}

  constexpr CurveSettings& settings() { return m_settings; }
  constexpr const CurveSettings& settings() const { return m_settings; }

  constexpr std::vector<CurvePoint>& points() { return m_points; }
  constexpr const std::vector<CurvePoint>& points() const { return m_points; }

  void insert_point(std::size_t index, ImVec2 position);
  void remove_point(std::size_t index);

  // Builds the curve from the given points.
  void output(OutputCurve& output, const OutputCurveSettings& settings) const;

private:
  float calc_segment_length(EquationFunc equation, std::size_t samples) const;

  std::size_t output_segment(EquationFunc equation, float length,
                             std::size_t samples_per_meter,
                             std::size_t segment_index,
                             std::map<int, float>& max_velocities,
                             OutputCurve& output) const;

  void calc_linear_velocities(const std::map<int, float>& max_velocities,
                              OutputCurve& output) const;
  void calc_angular_velocities(const std::vector<OutputCurveSegment>& segments,
                               OutputCurve& output) const;

  // The rest of the values (times, centripetal accelerations, etc.).
  void calc_other_values(std::vector<OutputCurveSegment>& segments,
                         OutputCurve& output) const;
};

extern const Curve default_new_curve;

void to_json(wpi::json& json, const Curve& curve);
void from_json(const wpi::json& json, Curve& curve);

