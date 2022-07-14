#pragma once

#include <pages/page.h>
#include <vector>

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

  struct SplinePoint {
    float x;
    float y;
    float cx;
    float cy;
  };

  using SplinePointTable = std::vector<SplinePoint>;

  SplinePointTable points {
    { 0.2f, 0.4f, 0.3f, 0.7f },
    { 0.5f, 0.3f, 0.6f, 0.6f },
    { 0.8f, 0.5f, 0.9f, 0.6f },
  };

  void present_spline_editor();

  std::vector<ImVec2> calc_cubic_hermite_spline_point() const;

  bool focused = false;
  bool unsaved = false;
  
  static PathEditorPage instance;
};
