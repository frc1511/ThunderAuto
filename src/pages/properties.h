#pragma once

#include <pages/page.h>
#include <thunder_auto.h>

struct Project;

class PropertiesPage: public ProjectPage {
public:
  static PropertiesPage* get() {
    return &instance;
  }
  
  PropertiesPage(PropertiesPage const&) = delete;
  PropertiesPage& operator=(PropertiesPage const&) = delete;
  
  void present(bool* running) override;

  /**
   * @brief Sets the working project.
   *
   * @param project The project to work with.
   */
  void set_project(Project* project) override;

  inline std::vector<std::string> get_action_names() const { return actions; }

  inline void set_action_names(std::vector<std::string> names) { actions = names; }
  
private:
  PropertiesPage();
  ~PropertiesPage();
  
  Project* project = nullptr;

  std::vector<std::string> actions;
  
  static PropertiesPage instance;
};
