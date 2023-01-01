#pragma once

#include <ThunderAuto/pages/page.h>
#include <ThunderAuto/thunder_auto.h>

struct Project;

class SettingsPage: public ProjectPage {
public:
  static SettingsPage* get() {
    return &instance;
  }
  
  SettingsPage(SettingsPage const&) = delete;
  SettingsPage& operator=(SettingsPage const&) = delete;
  
  void present(bool* running) override;

  /**
   * @brief Sets the working project.
   *
   * @param project The project to work with.
   */
  void set_project(Project* project) override;

  void reset();
  
private:
  SettingsPage();
  ~SettingsPage();
  
  Project* project = nullptr;

  int controller = 0;
  float max_accel = 0.0f;
  float max_vel = 0.0f;
  float robot_length = 0.0f;
  float robot_width = 0.0f;
  
  static SettingsPage instance;
};