#pragma once

#include <ThunderAuto/thunder_auto.hpp>
#include <ThunderAuto/curve.hpp>
#include <ThunderAuto/document_edit_manager.hpp>
#include <ThunderAuto/pages/page.hpp>

class ActionsPage : public Page {
  DocumentEditManager& m_history;

 public:
  inline ActionsPage(DocumentEditManager& history) : m_history(history) {}

  constexpr const char* name() const override { return "Actions"; }

  void present(bool* running) override;

 private:
  void present_action_name_input(ProjectState& state, size_t action_index);
  void present_action_id(size_t id, size_t num_actions);
  void present_action_delete_button(ProjectState& state, size_t action_index);

  void present_new_action_button(ProjectState& state);
};
