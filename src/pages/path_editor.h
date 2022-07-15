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
  };

  using CurvePointTable = std::vector<CurvePoint>;

  CurvePointTable points {
    { 0.2f, 0.4f, 0.2f, 0.7f, 0.2f, 0.2f },
    { 0.5f, 0.3f, 0.5f, 0.6f, 0.5f, 0.2f },
    { 0.8f, 0.5f, 0.8f, 0.6f, 0.8f, 0.2f },
  };

  void present_curve_editor();
  std::vector<ImVec2> calc_curve_points() const;

  ImVec2 calc_curve_point(CurvePointTable::const_iterator pt_it, float t) const;

  float calc_curve_length() const;
  float calc_curve_part_length(CurvePointTable::const_iterator pt_it) const;

  float cached_curve_length = 0.0f;
  std::vector<ImVec2> cached_curve_points;

  enum class CurveKind {
    CUBIC_BEZIER = 0,
    CUBIC_HERMITE = 1,
  };

  CurveKind curve_kind = CurveKind::CUBIC_BEZIER;

  bool focused = false;
  bool unsaved = false;
  
  static PathEditorPage instance;
};
