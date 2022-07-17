#pragma once

#include <popups/popup.h>
#include <platform/platform.h>
#include <project.h>
#include <string>
#include <optional>

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
  
private:
  NewProjectPopup();
  ~NewProjectPopup();
  
  std::string name = "New Project";
  
  Platform* platform = nullptr;
  
  bool has_project = false;
  ProjectSettings project;
  
  static NewProjectPopup instance;
};
