#pragma once

#include <pages/page.h>
#include <vector>
#include <optional>

class PathEditorPage: public Page {
public:
  static PathEditorPage* get() {
    return &instance;
  }
  
  PathEditorPage(PathEditorPage const&) = delete;
  PathEditorPage& operator=(PathEditorPage const&) = delete;
  
  void present(bool* running) override;
  bool is_focused() override { return focused; }
  
  constexpr bool is_unsaved() const { return unsaved; }
  inline void set_unsaved(bool _unsaved) { unsaved = _unsaved; }
  
private:
  PathEditorPage();
  ~PathEditorPage();

  struct CurvePoint {
    float px;
    float py;
    float c0x;
    float c0y;
    float c1x;
    float c1y;
    float ax;
    float ay;
  };

  using CurvePointTable = std::vector<CurvePoint>;

  CurvePointTable points {
    { 0.2f, 0.4f, 0.2f, 0.7f, 0.2f, 0.2f, 0.2f, 0.35f },
    { 0.5f, 0.3f, 0.5f, 0.6f, 0.5f, 0.2f, 0.55f, 0.3f },
    { 0.8f, 0.5f, 0.8f, 0.6f, 0.8f, 0.2f, 0.8f, 0.55f },
  };

  void present_curve_editor();
  std::vector<ImVec2> calc_curve_points() const;

  ImVec2 calc_curve_point(CurvePointTable::const_iterator pt_it, float t) const;

  std::vector<float> calc_curve_lengths() const;
  float calc_curve_part_length(CurvePointTable::const_iterator pt_it) const;
  std::vector<float> calc_curvature() const;

  std::pair<CurvePointTable::const_iterator, float> find_curve_point(float x, float y) const;

  std::vector<float> cached_curve_lengths;
  std::vector<ImVec2> cached_curve_points;
  std::vector<float> cached_curvatures;

  enum class CurveKind {
    CUBIC_BEZIER = 0,
    CUBIC_HERMITE = 1,
  };

  CurveKind curve_kind = CurveKind::CUBIC_BEZIER;

  bool updated = true;

  bool focused = false;
  bool unsaved = false;

  bool show_handles = true;
  
  static PathEditorPage instance;
};
