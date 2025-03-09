#include <ThunderAuto/curve_point.h>

#include <ThunderAuto/imgui_util.h>

void CurvePoint::set_heading(Angle heading, bool which) {
  Angle *primary_heading = &m_headings.incoming,
        *other_heading = &m_headings.outgoing;

  if (which == OUTGOING) {
    std::swap(primary_heading, other_heading);
  }

  *primary_heading = heading;

  if (!m_stop) {
    *other_heading = primary_heading->supplementary();
  }
}

ImVec2 CurvePoint::heading_control_point(const bool which) const {
  const Angle heading =
      which == INCOMING ? m_headings.incoming : m_headings.outgoing;

  const float weight = which == INCOMING ? m_heading_weights.incoming
                                         : m_heading_weights.outgoing;

  return pt_extend_at_angle(m_position, heading, weight);
}

void CurvePoint::set_heading_control_point(const ImVec2 pt, const bool which) {
  const float dx = pt.x - m_position.x;
  const float dy = pt.y - m_position.y;

  Angle *primary_heading = &m_headings.incoming,
        *other_heading = &m_headings.outgoing;

  if (which == OUTGOING) {
    std::swap(primary_heading, other_heading);
  }

  float* weight = which == INCOMING ? &m_heading_weights.incoming
                                    : &m_heading_weights.outgoing;

  *primary_heading = Angle::radians(std::atan2(dy, dx));

  if (!m_stop) {
    *other_heading = primary_heading->supplementary();
  }

  *weight = max(std::hypotf(dx, dy), 0.1f);
}

ImVec2 CurvePoint::rotation_control_point(const float robot_length) const {
  const float dist = robot_length / 2.f;

  return pt_extend_at_angle(m_position, m_rotation, dist);
}

void CurvePoint::set_rotation_control_point(const ImVec2 pt) {
  float dx = pt.x - m_position.x;
  float dy = pt.y - m_position.y;

  m_rotation = Angle::radians(std::atan2(dy, dx));
}

std::array<ImVec2, 4> CurvePoint::robot_corners(const float robot_length,
                                                const float robot_width) const {
  return ::robot_corners(m_position, m_rotation, robot_length, robot_width);
}

std::array<ImVec2, 4> robot_corners(const ImVec2 position, const Angle rotation,
                                    const float robot_length,
                                    const float robot_width) {

  const float x_dist = robot_width / 2.f;
  const float y_dist = robot_length / 2.f;

  std::array<ImVec2, 4> corners;

  for (std::size_t i = 0; i < corners.size(); i++) {
    const int x_sign = (i <= 1) ? 1 : -1;
    const int y_sign = (i == 1 || i == 2) ? 1 : -1;

    ImVec2 corner = position;
    corner = pt_extend_at_angle(corner, rotation, y_dist * y_sign);
    corner = pt_extend_at_angle(corner, rotation + 90_deg, x_dist * x_sign);

    corners[i] = corner;
  }

  return corners;
}

void to_json(nlohmann::json& json, const CurvePoint& point) {
  ImVec2 pos = point.position();
  HeadingAngles headings = point.headings();
  HeadingWeights weights = point.heading_weights();
  Angle rotation = point.rotation();
  bool stop = point.stop();
  unsigned actions = point.actions();

  json = nlohmann::json {{"x", pos.x},
                         {"y", pos.y},
                         {"h0", headings.incoming.radians()},
                         {"h1", headings.outgoing.radians()},
                         {"w0", weights.incoming},
                         {"w1", weights.outgoing},
                         {"rotation", rotation.radians()},
                         {"stop", stop},
                         {"actions", actions},
                         {"locked", point.editor_locked()},
                         {"link_index", point.link_index()}};
}

void from_json(const nlohmann::json& json, CurvePoint& point) {
  ImVec2 pos(json.at("x").get<float>(), json.at("y").get<float>());
  HeadingAngles headings {Angle::radians(json.at("h0").get<float>()),
                          Angle::radians(json.at("h1").get<float>())};
  HeadingWeights weights {json.at("w0").get<float>(),
                          json.at("w1").get<float>()};
  Angle rotation = Angle::radians(json.at("rotation").get<float>());
  bool stop = json.at("stop").get<bool>();
  unsigned actions = json.at("actions").get<unsigned>();

  point = CurvePoint(pos, headings, weights, rotation, stop, actions);

  if (json.contains("locked")) {
    point.set_editor_locked(json.at("locked").get<bool>());
  }

  if (json.contains("link_index")) {
    point.set_link_index(json.at("link_index").get<int>());
  }
}

