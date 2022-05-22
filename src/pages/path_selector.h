#pragma once

#include <pages/page.h>

class PathSelectorPage: public Page {
public:
  static PathSelectorPage* get() {
    return &instance;
  }
  
  PathSelectorPage(PathSelectorPage const&) = delete;
  PathSelectorPage& operator=(PathSelectorPage const&) = delete;
  
  void present(bool* running) override;
  bool is_focused() override { return focused; }
  
private:
  PathSelectorPage();
  ~PathSelectorPage();
  
  bool focused = false;
  
  static PathSelectorPage instance;
};
