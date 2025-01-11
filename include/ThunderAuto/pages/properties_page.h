#pragma once

#include <ThunderAuto/curve.h>
#include <ThunderAuto/document_edit_manager.h>
#include <ThunderAuto/pages/page.h>
#include <ThunderAuto/pages/path_editor_page.h>
#include <ThunderAuto/project_settings.h>
#include <ThunderAuto/thunder_auto.h>

class PropertiesPage : public Page {
  DocumentEditManager& m_history;

  OutputCurve& m_cached_curve;

  PathEditorPage& m_path_editor_page;

  const ProjectSettings* m_settings = nullptr;

public:
  inline PropertiesPage(DocumentEditManager& history, OutputCurve& cached_curve,
                        PathEditorPage& path_editor_page)
    : m_history(history),
      m_cached_curve(cached_curve),
      m_path_editor_page(path_editor_page) {}

  constexpr void setup(const ProjectSettings& settings) {
    m_settings = &settings;
  }

  constexpr const char* name() const override { return "Properties"; }

  void present(bool* running) override;

private:
  void present_point_properties(ProjectState& current_state);
  void present_path_properties(ProjectState& current_state);
  void present_velocity_properties(ProjectState& current_state);
  void present_editor_properties();

  void present_link_popup(ProjectState& current_state, std::size_t point_index,
                          bool reset = false);

  bool edit_point_position(CurvePoint& point);
  bool edit_point_headings(CurvePoint& point, bool incoming, bool outgoing);
  bool edit_point_heading_weights(CurvePoint& point, bool incoming,
                                  bool outgoing);
  bool edit_point_rotation(CurvePoint& point);
  bool edit_point_stop(CurvePoint& point);
  bool edit_point_actions(CurvePoint& point);

  bool present_slider(const char* id, float& value, float speed = 1.f,
                      const char* format = "%.2f");
};

