#pragma once

#include <pages/page.h>
#include <thunder_auto.h>
#include <pages/path_editor.h>

struct Project;

class PathManagerPage: public Page {
public:
  static PathManagerPage* get() {
    return &instance;
  }
  
  PathManagerPage(PathManagerPage const&) = delete;
  PathManagerPage& operator=(PathManagerPage const&) = delete;
  
  void present(bool* running) override;
  bool is_focused() override { return focused; }

  /**
   * @brief Sets the working project.
   *
   * @param project The project to work with.
   */
  void set_project(Project* project);

  PathEditorPage::CurvePointTable& get_selected_path() const;

  const std::string& get_selected_path_name() const;
  
private:
  PathManagerPage();
  ~PathManagerPage();
  
  bool focused = false;

  std::size_t selected = 0;

  Project* project = nullptr;
  
  static PathManagerPage instance;
};
