#pragma once

#include <pages/page.h>
#include <vector>
#include <optional>
#include <cmath>

struct Project;

#define DEG_2_RAD (M_PI / 180.0f)
#define RAD_2_DEG (180.0f / M_PI)

class PathEditorPage: public Page {
public:
  static PathEditorPage* get() {
    return &instance;
  }

  void init();
  
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
  
  static PathEditorPage instance;
};
