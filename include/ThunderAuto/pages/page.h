#pragma once

#include <ThunderAuto/thunder_auto.h>

class Page {
public:
  virtual void present(bool* running) = 0;
  constexpr bool is_focused() const { return focused; }
  
protected:
  Page() = default;
  ~Page() = default;

  bool focused = false;
};

struct Project;

class ProjectPage : public Page {
public:
  virtual void set_project(Project* project) = 0;
};
