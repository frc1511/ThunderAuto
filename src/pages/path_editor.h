#pragma once

#include <pages/page.h>

class PathEditorPage: public Page {
public:
  static PathEditorPage* get() {
    return &instance;
  }
  
  PathEditorPage(PathEditorPage const&) = delete;
  PathEditorPage& operator=(PathEditorPage const&) = delete;
  
  void present(bool* running) override;
  bool is_focused() override { return focused; }
  
private:
  PathEditorPage();
  ~PathEditorPage();
  
  bool focused = false;
  
  static PathEditorPage instance;
};
