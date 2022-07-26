#pragma once

#include <popups/popup.h>
#include <platform/platform.h>
#include <project.h>
#include <thunder_auto.h>

class NewProjectPopup: public Popup {
public:
  static NewProjectPopup* get() {
    return &instance;
  }
  
  NewProjectPopup(NewProjectPopup const&) = delete;
  NewProjectPopup& operator=(NewProjectPopup const&) = delete;
  
  void present(bool* running) override;
  std::string get_name() override { return name; }
  
  inline std::optional<ProjectSettings> get_project_settings() const { return (has_project) ? project : std::optional<ProjectSettings>(); }

  constexpr bool is_showing_new_field_popup() const { return show_new_field_popup; }
  constexpr void set_is_showing_new_field_popup(bool show) { show_new_field_popup = show; }
  
private:
  NewProjectPopup();
  ~NewProjectPopup();

  std::string name = "New Project";
  
  bool has_project = false;
  ProjectSettings project;

  bool show_new_field_popup = false;
  
  static NewProjectPopup instance;
};
