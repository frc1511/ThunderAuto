#pragma once

#include <ThunderAuto/thunder_auto.hpp>
#include <ThunderAuto/curve.hpp>
#include <ThunderAuto/document_edit_manager.hpp>
#include <ThunderAuto/pages/page.hpp>

class PathManagerPage : public Page {
  DocumentEditManager& m_history;

  OutputCurve& m_cached_curve;

  bool m_was_input_active = false;

 public:
  inline PathManagerPage(DocumentEditManager& history,
                         OutputCurve& cached_curve)
      : m_history(history), m_cached_curve(cached_curve) {}

  constexpr const char* name() const override { return "Path Manager"; }

  void present(bool* running) override;

  void duplicate_path(ProjectState& state, size_t index);
  void delete_path(ProjectState& state, size_t index);
  void reverse_path(ProjectState& state, size_t index);

 private:
  bool present_context_menu(ProjectState& state, size_t index);
  void present_new_path_button(ProjectState& state);

  static bool selectable_input(const char* label,
                               bool selected,
                               char* buf,
                               size_t buf_size,
                               bool& input_active);
};
