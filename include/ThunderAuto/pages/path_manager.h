#pragma once

#include <ThunderAuto/pages/page.h>
#include <ThunderAuto/thunder_auto.h>
#include <ThunderAuto/pages/path_editor.h>

struct Project;

class PathManagerPage: public ProjectPage {
public:
  static PathManagerPage* get() {
    return &instance;
  }
  
  PathManagerPage(PathManagerPage const&) = delete;
  PathManagerPage& operator=(PathManagerPage const&) = delete;
  
  void present(bool* running) override;

  /**
   * @brief Sets the working project.
   *
   * @param project The project to work with.
   */
  void set_project(Project* project) override;

  PathEditorPage::CurvePointTable& get_selected_path() const;

  const std::string& get_selected_path_name() const;
  
private:
  PathManagerPage();
  ~PathManagerPage();
  
  std::size_t selected = 0;

  Project* project = nullptr;
  
  static PathManagerPage instance;
};
