#pragma once

#include <ThunderAuto/curve.h>
#include <ThunderAuto/document_edit_manager.h>
#include <ThunderAuto/pages/page.h>
#include <ThunderAuto/thunder_auto.h>

class PathManagerPage : public Page {
  DocumentEditManager& m_history;

  OutputCurve& m_cached_curve;

public:
  inline PathManagerPage(DocumentEditManager& history,
                         OutputCurve& cached_curve)
    : m_history(history),
      m_cached_curve(cached_curve) {}

  constexpr const char* name() const override { return "Path Manager"; }

  void present(bool* running) override;

private:
  bool selectable_input(const char* label, bool selected, char* buf,
                        std::size_t buf_size, bool& input_active);

  void duplicate_path(ProjectState& state, std::size_t index);
  void delete_path(ProjectState& state, std::size_t index);
};
