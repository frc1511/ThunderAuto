#pragma once

#include <pages/page.h>
#include <thunder_auto.h>

struct Project;

class PropertiesPage: public Page {
public:
  static PropertiesPage* get() {
    return &instance;
  }
  
  PropertiesPage(PropertiesPage const&) = delete;
  PropertiesPage& operator=(PropertiesPage const&) = delete;
  
  void present(bool* running) override;
  bool is_focused() override { return focused; }

  /**
   * @brief Sets the working project.
   *
   * @param project The project to work with.
   */
  void set_project(Project* project);
  
private:
  PropertiesPage();
  ~PropertiesPage();
  
  bool focused = false;

  Project* project = nullptr;
  
  static PropertiesPage instance;
};
