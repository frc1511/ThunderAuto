#include <ThunderAuto/curve.h>

const OutputCurveSettings preview_output_curve_settings {
    .length_samples = 50,
    .samples_per_meter = 25,
};

const OutputCurveSettings high_res_output_curve_settings {
    .length_samples = 1000,
    .samples_per_meter = 64,
};

const Curve default_new_curve({
    CurvePoint(ImVec2(0.f, 0.f), 180_deg, {1.f, 1.f}, 0.0_rad),
    CurvePoint(ImVec2(5.f, 0.f), 180_deg, {1.f, 1.f}, 0.0_rad),
});

void Curve::insert_point(std::size_t index, ImVec2 position) {
  assert(index <= m_points.size());

  std::vector<CurvePoint>::const_iterator it =
      std::next(m_points.cbegin(), index);

  CurvePoint point(position, 0_rad, {0.5f, 0.5f}, 0_rad);

  m_points.insert(it, point);
}

void Curve::remove_point(std::size_t index) {
  assert(index < m_points.size());

  std::vector<CurvePoint>::const_iterator it =
      std::next(m_points.cbegin(), index);

  if (index == 0)
    m_points.at(1).set_stop(false, CurvePoint::OUTGOING);
  else if (index == m_points.size() - 1)
    m_points.at(m_points.size() - 2).set_stop(false, CurvePoint::INCOMING);

  m_points.erase(it);
}

void Curve::output(OutputCurve& output,
                   const OutputCurveSettings& settings) const {

  output.total_distance = 0.f;
  output.points.clear();

  std::map<int, float> max_velocities;

  for (std::size_t i = 0; i < m_points.size() - 1; ++i) {
    const CurvePoint& begin = m_points.at(i);
    const CurvePoint& end = m_points.at(i + 1);

    const EquationFunc equation = build_bezier_curve_equation(
        begin.position(), begin.heading_control_point(CurvePoint::OUTGOING),
        end.position(), end.heading_control_point(CurvePoint::INCOMING));

    const float segment_length =
        calc_segment_length(equation, settings.length_samples);

    std::size_t segment_points =
        output_segment(equation, segment_length, settings.samples_per_meter,
                       end.rotation().radians(), i, max_velocities, output);

    std::vector<OutputCurvePoint>::iterator segment_begin =
        output.points.end() - segment_points;

    segment_begin->actions = begin.actions();

    if (begin.stop()) {
      max_velocities[output.points.size() - segment_points] = 0.f;
    }

    output.total_distance += segment_length;
  }
  // Initial rotation.
  output.points.front().rotation = m_points.front().rotation().radians();
  // Final actions.
  output.points.back().actions = m_points.back().actions();

  // Start & end velocities.
  max_velocities[0] = 0.f;
  max_velocities[output.points.size() - 1] = 0.f;

  calc_velocities(max_velocities, output);
  calc_other_values(output);
}

float Curve::calc_segment_length(const EquationFunc equation,
                                 const std::size_t samples) const {

  const float delta = 1.f / static_cast<float>(samples);

  float length = 0.f;

  ImVec2 last_pt = equation(0.f);

  for (std::size_t i = 1; i <= samples; i++) {
    const float t = i * delta;

    const ImVec2 pt = equation(t);

    const float dx = pt.x - last_pt.x;
    const float dy = pt.y - last_pt.y;

    last_pt = pt;

    length += std::hypotf(dy, dx);
  }

  return length;
}

std::size_t Curve::output_segment(const EquationFunc equation,
                                  const float length,
                                  const std::size_t samples_per_meter,
                                  const float rotation,
                                  const std::size_t segment_index,
                                  std::map<int, float>& max_velocities,
                                  OutputCurve& output) const {

  const std::size_t samples = length * static_cast<float>(samples_per_meter);

  const float delta = 1.f / static_cast<float>(samples);

  ImVec2 prev_position;
  ImVec2 next_position = equation(0.f);

  output.points.reserve(output.points.size() + samples);

  for (std::size_t i = 0; i < samples; i++) {
    const float t = i * delta;

    const float distance = output.total_distance + t * length;

    // Position.
    const ImVec2 position = next_position;
    if (i != samples - 1) {
      next_position = equation(t + delta);
    }

    float curvature = 0.f;

    // Curvature.
    if (i != 0 && i != samples - 1) {
      curvature = menger_curvature(prev_position, position, next_position);
    }

    // Slow down for high curvature.
    if (!float_eq(curvature, 0.f)) {
      // Curvature is the reciprocal of the radius.
      const float radius = 1.f / curvature;

      // Max velocity that doesn't exceed the centripetal acceleration.
      // v^2 = a_c * r
      const float max_velocity =
          std::sqrt(m_settings.max_centripetal_accel * radius);

      if (max_velocity < m_settings.max_linear_vel) {
        max_velocities[output.points.size()] = max_velocity;
      }
    }

    output.points.push_back({
        .position = position,
        .velocity = m_settings.max_linear_vel,
        .rotation = rotation,
        .distance = distance,
        .curvature = curvature,
        .segment_index = segment_index,
    });

    prev_position = position;
  }

  return samples;
}

void Curve::calc_velocities(const std::map<int, float>& max_velocities,
                            OutputCurve& curve) const {
  const float delta_distance = curve.total_distance / curve.points.size();

  for (auto [index, velocity] : max_velocities) {
    OutputCurvePoint* point = &curve.points.at(index);

    if (point->velocity < velocity) continue;
    point->velocity = velocity;

    int offset = 0;
    while (velocity < m_settings.max_linear_vel) {
      // v^2 = v_0^2 + 2ax
      velocity = std::sqrt(std::pow(velocity, 2) +
                           2 * m_settings.max_linear_accel * delta_distance);
      ++offset;

      const bool up = index + offset < (int)curve.points.size();
      const bool down = index - offset >= 0;

      if (!up && !down) {
        break;
      }

      if (up) {
        point = &curve.points.at(index + offset);

        point->velocity = std::min(velocity, point->velocity);
      }
      if (down) {
        point = &curve.points.at(index - offset);

        point->velocity = std::min(velocity, point->velocity);
      }
    }
  }
}

void Curve::calc_other_values(OutputCurve& curve) const {
  curve.points.at(0).time = 0.f;

  float last_distance = 0.f;
  float last_time = 0.f;
  for (std::size_t i = 0; i < curve.points.size(); ++i) {
    OutputCurvePoint& point = curve.points.at(i);

    // Time.
    {
      const float delta_distance = point.distance - last_distance;

      float delta_time = 0.f;
      if (!float_eq(point.velocity, 0.f)) {
        delta_time = delta_distance / point.velocity;
      }

      point.time = last_time + delta_time;

      last_distance = point.distance;
      last_time = point.time;
    }

    // Centripetal acceleration.
    {
      float accel = 0.f;

      if (!float_eq(point.curvature, 0.f)) {
        const float radius = 1.f / point.curvature;

        // a_c = v^2 / r
        accel = std::pow(point.velocity, 2) / radius;
      }

      point.centripetal_accel = accel;
    }
  }
}

void to_json(nlohmann::json& json, const Curve& curve) {
  json = nlohmann::json {
      {"points", curve.points()},
      {"settings", curve.settings()},
  };
}

void from_json(const nlohmann::json& json, Curve& curve) {
  curve.settings() = json.at("settings").get<CurveSettings>();
  curve.points() = json.at("points").get<std::vector<CurvePoint>>();
}
