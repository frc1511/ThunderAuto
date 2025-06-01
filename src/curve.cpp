#include <ThunderAuto/curve.hpp>

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

  // Orient the new point roughly in the direction of the curve.

  ImVec2 prev_position, next_position;

  if (index != 0) {
    prev_position = m_points.at(index - 1).position();
  } else {
    prev_position = position;
  }
  if (index != m_points.size()) {
    next_position = m_points.at(index).position();
  } else {
    next_position = position;
  }

  float heading_rad = std::atan2(next_position.y - prev_position.y,
                                 next_position.x - prev_position.x) +
                      std::numbers::pi_v<float>;

  Angle heading = Angle::radians(heading_rad);

  // Construct point and insert.

  CurvePoint point(position, heading, {0.5f, 0.5f}, 0_rad);

  std::vector<CurvePoint>::const_iterator it =
      std::next(m_points.cbegin(), index);

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
  std::vector<OutputCurveSegment> segments;

  for (std::size_t i = 0; i < m_points.size() - 1; ++i) {
    const CurvePoint& begin = m_points.at(i);
    const CurvePoint& end = m_points.at(i + 1);

    const EquationFunc equation = build_bezier_curve_equation(
        begin.position(), begin.heading_control_point(CurvePoint::OUTGOING),
        end.position(), end.heading_control_point(CurvePoint::INCOMING));

    const float segment_length =
        calc_segment_length(equation, settings.length_samples);

    std::size_t segment_points =
        output_segment(equation, segment_length, settings.samples_per_meter, i,
                       max_velocities, output);

    segments.push_back({
        // .distance = segment_length,
        .time = 0.f,
        .begin_rotation = begin.rotation(),
        .end_rotation = end.rotation(),
        .begin_index = output.points.size() - segment_points,
        .end_index = output.points.size() - 1,
        .rotation_time_percent = end.previous_segment_rotation_time_percent(),
    });

    std::vector<OutputCurvePoint>::iterator segment_begin =
        output.points.end() - segment_points;

    segment_begin->actions = begin.actions();

    if (begin.stop()) {
      max_velocities[output.points.size() - segment_points] = 0.f;
    }

    output.total_distance += segment_length;
  }

  // Initial rotation.
  output.points.front().rotation = m_points.front().rotation();
  output.points.front().rotation.set_bounds(Angle::Bounds::NEG_180_TO_POS_180);

  // Final actions.
  output.points.back().actions = m_points.back().actions();

  // Start & end velocities.
  max_velocities[0] = 0.f;
  max_velocities[output.points.size() - 1] = 0.f;

  calc_linear_velocities(max_velocities, output);
  calc_other_values(segments, output);
  calc_angular_velocities(segments, output);
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
                                  const std::size_t segment_index,
                                  std::map<int, float>& max_velocities,
                                  OutputCurve& output) const {

  const std::size_t samples =
      std::size_t(length * float(samples_per_meter)) - 1;

  const float delta = 1.f / float(samples + 1);

  ImVec2 prev_position;
  ImVec2 next_position = equation(0.f);

  output.points.reserve(output.points.size() + samples);

  for (std::size_t i = 0; i < samples; i++) {
    const float t = i * delta;

    const float distance = output.total_distance + t * length;

    // Position.
    const ImVec2 position = next_position;
    if (i != samples - 1) {
      next_position = equation(t);
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
        .rotation = Angle::radians(0, Angle::Bounds::NEG_180_TO_POS_180),
        .distance = distance,
        .curvature = curvature,
        .segment_index = segment_index,
    });

    prev_position = position;
  }

  return samples;
}

void Curve::calc_linear_velocities(const std::map<int, float>& max_velocities,
                                   OutputCurve& curve) const {
  const float delta_distance = curve.total_distance / curve.points.size();

  for (auto [index, velocity] : max_velocities) {
    OutputCurvePoint* point = &curve.points.at(index);

    if (point->velocity < velocity) continue;
    point->velocity = velocity;

    int offset = 0;
    while (velocity < m_settings.max_linear_vel) {
      // v^2 = v_0^2 + 2ax
      velocity = std::sqrtf(std::pow(velocity, 2) +
                            2.f * m_settings.max_linear_accel * delta_distance);
      ++offset;

      const bool up = index + offset < (int)curve.points.size();
      const bool down = index - offset >= 0;

      if (!up && !down) {
        break;
      }

      if (up) {
        point = &curve.points.at(index + offset);

        point->velocity = min(velocity, point->velocity);
      }
      if (down) {
        point = &curve.points.at(index - offset);

        point->velocity = min(velocity, point->velocity);
      }
    }
  }
}

void Curve::calc_angular_velocities(
    const std::vector<OutputCurveSegment>& segments, OutputCurve& curve) const {

  for (const OutputCurveSegment& segment : segments) {
    const float begin_rotation_rad = segment.begin_rotation.radians();
    const float end_rotation_rad = segment.end_rotation.radians();
    float rotation_delta_rad = end_rotation_rad - begin_rotation_rad;

    {
      const float pi = std::numbers::pi_v<float>;

      if (rotation_delta_rad > pi) {
        rotation_delta_rad -= 2.f * pi;
      } else if (rotation_delta_rad < -pi) {
        rotation_delta_rad += 2.f * pi;
      }
    }

    assert(segment.rotation_time_percent > 0.f &&
           segment.rotation_time_percent <= 1.f);
    const float segment_time = segment.time * segment.rotation_time_percent;

    // Calculate minimum acceleration needed to reach target angle.
    float a = 4.f * abs(rotation_delta_rad) / std::pow(segment_time, 2);
    float v_max = std::sqrt(a * abs(rotation_delta_rad));
    float t_accel = segment_time / 2.f;

    if (rotation_delta_rad < 0.f) {
      a = -a;
      v_max = -v_max;
    }

    const float d_accel =
        (abs(a) > 0.0001f) ? (v_max * v_max) / (2.f * a) : 0.f;

    bool rotation_done = false;
    const float time_start = curve.points.at(segment.begin_index).time;
    for (size_t i = segment.begin_index; i <= segment.end_index; ++i) {
      OutputCurvePoint& point = curve.points.at(i);
      float t = point.time - time_start;

      if (t > t_accel * 2) {
        point.rotation.set_radians(end_rotation_rad);
        point.angular_velocity = 0.f;
        if (!rotation_done) {
          point.flags |= OUTPUT_CURVE_POINT_FLAG_ROTATION_DONE;
          rotation_done = true;
        }
      } else {
        if (t < t_accel) { // Accelerating.
          point.rotation.set_radians(begin_rotation_rad + 0.5f * a * t * t);
          point.angular_velocity = a * t;
        } else { // Decelerating.
          t -= t_accel;

          point.rotation.set_radians(begin_rotation_rad + d_accel + v_max * t -
                                     0.5f * a * t * t);
          point.angular_velocity = v_max - a * t;
        }
      }

      assert(std::isfinite(point.rotation.radians()));
    }
  }
}

void Curve::calc_other_values(std::vector<OutputCurveSegment>& segments,
                              OutputCurve& curve) const {
  curve.points.at(0).time = 0.f;

  float last_distance = 0.f;
  float last_time = 0.f;
  float last_velocity = 0.f;

  for (std::size_t i = 0; i < curve.points.size(); ++i) {
    OutputCurvePoint& point = curve.points.at(i);

    auto segment_it = std::find_if(
        segments.begin(), segments.end(), [i](const OutputCurveSegment& s) {
          return (i >= s.begin_index && i <= s.end_index);
        });
    assert(segment_it != segments.end());
    OutputCurveSegment& segment = *segment_it;

    // Time.
    {
      const float delta_distance = point.distance - last_distance;

      float delta_time = 0.0001f;
      if (!float_eq(last_velocity, 0.f)) {
        delta_time = delta_distance / last_velocity;
      }

      point.time = last_time + delta_time;
      segment.time += delta_time;

      last_distance = point.distance;
      last_time = point.time;
      last_velocity = point.velocity;
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

void to_json(wpi::json& json, const Curve& curve) {
  json = wpi::json {
      {"points", curve.points()},
      {"settings", curve.settings()},
  };
}

void from_json(const wpi::json& json, Curve& curve) {
  if (json.contains("settings")) {
    curve.settings() = json.at("settings").get<CurveSettings>();
  }

  if (json.contains("points")) {
    curve.points() = json.at("points").get<std::vector<CurvePoint>>();
  }
}

