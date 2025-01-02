#pragma once

#include <ThunderAuto/curve.h>
#include <ThunderAuto/document_edit_manager.h>
#include <ThunderAuto/pages/page.h>
#include <ThunderAuto/project_settings.h>
#include <ThunderAuto/texture.h>
#include <ThunderAuto/thunder_auto.h>

class PathEditorPage : public Page {
  DocumentEditManager& m_history;
  const ProjectSettings* m_settings = nullptr;

  enum class PointType {
    NONE = 0,
    POSITION,
    HEADING_IN,
    HEADING_OUT,
    ROTATION,
  };

  PointType m_drag_point = PointType::NONE;
  PointType m_clicked_point = PointType::NONE;

  bool m_show_context_menu = false;
  ImVec2 m_context_menu_position;
  std::size_t m_context_menu_hovered_curve_point_index = 0;

  OutputCurve& m_cached_curve;

  bool m_show_tangents = true;
  bool m_show_rotation = true;
  bool m_show_tooltip = true;

  float m_field_aspect_ratio = 1.f;
  Texture m_field_texture;

  ImVec2 m_field_offset;
  float m_field_scale = 1.f;

public:
  inline PathEditorPage(DocumentEditManager& history, OutputCurve& cached_curve)
    : m_history(history),
      m_cached_curve(cached_curve) {}

  void setup_field(const ProjectSettings& settings);

  constexpr const char* name() const override { return "Path Editor"; }

  void present(bool* running) override;

  // Whether the heading tangent lines are visible.
  constexpr void set_show_tangents(bool show_tangents) {
    m_show_tangents = show_tangents;
  }

  // Whether the rotation widgets are visible.
  constexpr void set_show_rotation(bool show_rotation) {
    m_show_rotation = show_rotation;
  }

  // Whether the curve tooltip is visible.
  constexpr void set_show_tooltip(bool show_tooltip) {
    m_show_tooltip = show_tooltip;
  }

  enum class CurveOverlay {
    VELOCITY = 0,
    CURVATURE = 1,
  };

  constexpr void set_curve_overlay(CurveOverlay curve_overlay) {
    m_curve_overlay = curve_overlay;
  }

private:
  void present_curve_editor();

  void pan_and_zoom();

  void present_context_menus(ProjectState& state);

  void present_field(ImRect bb);

  void present_curve(ImRect bb);

  void present_point_widget(const CurvePoint& point, bool selected, bool first,
                            bool last, ImRect bb);
  void present_point_rotation_widget(const CurvePoint& point, bool selected,
                                     ImRect bb);
  void present_point_heading_widget(const CurvePoint& point, bool incoming,
                                    bool outgoing, bool selected, ImRect bb);

  void handle_input(ProjectState& state, ImRect bb);

  void handle_point_input(ProjectState& state, ImRect bb);
  void handle_curve_input(ProjectState& state, ImRect bb);

  void insert_point(ProjectState& state, std::size_t index, ImVec2 position);
  void remove_selected_point(ProjectState& state);

private:
  CurveOverlay m_curve_overlay = CurveOverlay::VELOCITY;
};

