#pragma once

#include <pages/page.h>

class PathManagerPage: public Page {
public:
  static PathManagerPage* get() {
    return &instance;
  }
  
  PathManagerPage(PathManagerPage const&) = delete;
  PathManagerPage& operator=(PathManagerPage const&) = delete;
  
  void present(bool* running) override;
  bool is_focused() override { return focused; }
  
private:
  PathManagerPage();
  ~PathManagerPage();
  
  bool focused = false;
  
  static PathManagerPage instance;
};
