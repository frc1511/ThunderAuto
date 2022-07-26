#pragma once

#include <pages/page.h>
#include <thunder_auto.h>
#include <imgui_internal.h>

struct Project;

class PathEditorPage: public Page {
public:
  static PathEditorPage* get() {
    return &instance;
  }
  
  PathEditorPage(PathEditorPage const&) = delete;
  PathEditorPage& operator=(PathEditorPage const&) = delete;
  
  void present(bool* running) override;
  bool is_focused() override { return focused; }

  void set_project(Project* project);

  struct CurvePoint {
    float px;
    float py;
    float heading;
    float w0;
    float w1;
    float rotation;

    ImVec2 get_tangent_pt(bool first) const;
    void set_tangent_pt(bool first, float x, float y);

    ImVec2 get_rot_pt(bool reverse = false) const;
    void set_rot_pt(float x, float y);

    void translate(float dx, float dy);
  };

  using CurvePointTable = std::vector<CurvePoint>;

  std::optional<CurvePointTable::iterator> get_selected_point();

  void delete_point();

  constexpr void update() { updated = true; }

  void set_show_tangents(bool show) { show_tangents = show; }

  enum class CurveKind : std::size_t {
    CUBIC_BEZIER = 0,
    CUBIC_HERMITE = 1,
  };

  void set_curve_kind(CurveKind kind) { curve_kind = kind; }
  
private:
  PathEditorPage();
  ~PathEditorPage();

  void present_curve_editor();
  std::vector<ImVec2> calc_curve_points() const;

  ImVec2 calc_curve_point(CurvePointTable::const_iterator pt_it, float t) const;

  std::vector<float> calc_curve_lengths() const;
  float calc_curve_part_length(CurvePointTable::const_iterator pt_it) const;
  std::vector<float> calc_curvature() const;

  std::pair<CurvePointTable::const_iterator, float> find_curve_point(float x, float y) const;

  Project* project = nullptr;

  std::vector<float> cached_curve_lengths;
  std::vector<ImVec2> cached_curve_points;
  std::vector<float> cached_curvatures;

  CurvePointTable::iterator selected_pt;

  CurveKind curve_kind = CurveKind::CUBIC_BEZIER;

  bool updated = true;
  bool focused = false;

  bool show_tangents = true;

  float field_aspect_ratio;
  unsigned int field_tex;

  ImVec2 to_field_coord(ImVec2 pt) const;
  ImVec2 to_draw_coord(ImVec2 pt) const;

  ImVec2 adjust_field_coord(ImVec2 pt) const;
  ImVec2 un_adjust_field_coord(ImVec2 pt) const;
  
  ImRect bb;
  
  static PathEditorPage instance;
};
