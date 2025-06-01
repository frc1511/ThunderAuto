#pragma once

#include <ThunderAuto/thunder_auto.hpp>
#include <ThunderAuto/angle.hpp>

struct HeadingControlPoints {
  ImVec2 incoming_pt, outgoing_pt;
};

struct HeadingAngles {
  Angle incoming;
  Angle outgoing;
};

struct HeadingWeights {
  float incoming = 1.f, outgoing = 1.f;
};

class CurvePoint {
  // Robot coordinates on the field, in meters.
  ImVec2 m_position;

  // Robot headings (incoming and outgoing), in radians.
  //
  // The incoming and outgoing headings are supplementary unless the point is a
  // stop point.
  HeadingAngles m_headings;
  HeadingWeights m_heading_weights;

  // The rotation of the robot, in radians.
  Angle m_rotation;

  bool m_stop = false;

  // Percent of the previous segment that the robot should be rotating.
  float m_previous_segment_rotation_time_percent = 1.f;

  // Bit-field of actions to perform at this point.
  uint32_t m_actions = 0;

  bool m_editor_locked = false;
  int m_link_index = -1;

public:
  inline CurvePoint()
    : CurvePoint({0, 0}, 0_deg, {}, 0_deg) {}

  inline CurvePoint(ImVec2 pos, Angle heading, HeadingWeights heading_weights,
                    Angle rotation, bool stop = false, uint32_t actions = 0)
    : CurvePoint(pos, {heading, heading.supplementary()}, heading_weights,
                 rotation, stop, actions) {}

  inline CurvePoint(ImVec2 pos, HeadingAngles headings,
                    HeadingWeights heading_weights, Angle rotation,
                    bool stop = false, uint32_t actions = 0)
    : m_position(pos),
      m_headings(headings),
      m_heading_weights(heading_weights),
      m_rotation(rotation),
      m_stop(stop),
      m_actions(actions) {

    // Headings should be supplementary if the point is not a stop point.
    assert(m_stop || m_headings.incoming.is_supplementary(m_headings.outgoing));
  }

  inline CurvePoint(ImVec2 pos, HeadingControlPoints heading_control_points,
                    ImVec2 rotation_control_point, bool stop = false,
                    uint32_t actions = 0)
    : CurvePoint(pos, 0_deg, {}, 0_deg, stop, actions) {

    set_heading_control_points(heading_control_points);
    set_rotation_control_point(rotation_control_point);
  }

  inline ImVec2 position() const { return m_position; }
  inline void set_position(ImVec2 pos) { m_position = pos; }

  enum {
    INCOMING = 0,
    OUTGOING = 1
  };

  inline HeadingAngles headings() const { return m_headings; }
  inline void set_headings(HeadingAngles headings) { m_headings = headings; }

  inline Angle heading(bool which) const {
    return which == INCOMING ? m_headings.incoming : m_headings.outgoing;
  }
  void set_heading(Angle heading, bool which);

  inline HeadingWeights heading_weights() const { return m_heading_weights; }
  inline void set_heading_weights(HeadingWeights heading_weights) {
    m_heading_weights = heading_weights;
  }

  inline float heading_weight(bool which) const {
    return which == INCOMING ? m_heading_weights.incoming
                             : m_heading_weights.outgoing;
  }

  inline void set_heading_weight(float weight, bool which) {
    (which == INCOMING ? m_heading_weights.incoming
                       : m_heading_weights.outgoing) = weight;
  }

  inline Angle rotation() const { return m_rotation; }
  inline void set_rotation(Angle rotation) { m_rotation = rotation; }

  inline bool stop() const { return m_stop; }
  inline void set_stop(bool stop, bool which_to_keep = INCOMING) {
    m_stop = stop;
    if (!m_stop) {
      if (which_to_keep == INCOMING)
        m_headings.outgoing = m_headings.incoming.supplementary();
      else // OUTGOING
        m_headings.incoming = m_headings.outgoing.supplementary();
    }
  }

  inline float previous_segment_rotation_time_percent() const {
    return m_previous_segment_rotation_time_percent;
  }
  inline void set_previous_segment_rotation_time_percent(float percent) {
    m_previous_segment_rotation_time_percent = percent;
  }

  inline bool editor_locked() const { return m_editor_locked; }
  inline void set_editor_locked(bool locked) { m_editor_locked = locked; }

  inline int link_index() const { return m_link_index; }
  inline void set_link_index(int index) { m_link_index = index; }

  inline uint32_t actions() const { return m_actions; }
  inline void set_actions(uint32_t actions) { m_actions = actions; }
  inline void add_actions(uint32_t actions) { m_actions |= actions; }
  inline void remove_actions(uint32_t actions) { m_actions &= ~actions; }

  inline void remove_action(std::size_t action_bit,
                            bool shift_down_higher = false) {
    assert(action_bit <= 31);

    if (action_bit == 31) {
      remove_actions(1U << 31);
      return;
    }

    // Bitwise magic mua ha ha ha ha...

    const uint32_t lower_mask = (1U << action_bit) - 1;
    const uint32_t higher_mask = ~((1U << (action_bit + 1)) - 1);

    const uint32_t result =
        (m_actions & lower_mask) |
        ((m_actions & higher_mask) >> uint32_t(shift_down_higher));

    m_actions = result;
  }

  inline void translate(float dx, float dy) { translate(ImVec2 {dx, dy}); }

  inline void translate(ImVec2 delta) { m_position += delta; }

  inline HeadingControlPoints heading_control_points() const {
    return {heading_control_point(INCOMING), heading_control_point(OUTGOING)};
  }

  inline void set_heading_control_points(HeadingControlPoints control_points) {
    set_heading_control_points(control_points.incoming_pt,
                               control_points.outgoing_pt);
  }

  inline void set_heading_control_points(ImVec2 incoming_pt,
                                         ImVec2 outgoing_pt) {
    set_heading_control_point(incoming_pt, INCOMING);
    set_heading_control_point(outgoing_pt, OUTGOING);
  }

  ImVec2 heading_control_point(bool which) const;

  void set_heading_control_point(ImVec2 control_point, bool which);

  ImVec2 rotation_control_point(float robot_length) const;

  void set_rotation_control_point(ImVec2 control_point);

  std::array<ImVec2, 4> robot_corners(float robot_length,
                                      float robot_width) const;
};

std::array<ImVec2, 4> robot_corners(ImVec2 position, Angle rotation,
                                    float robot_length, float robot_width);

void to_json(wpi::json& json, const CurvePoint& point);
void from_json(const wpi::json& json, CurvePoint& point);

