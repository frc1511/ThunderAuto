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
  
  constexpr bool is_unsaved() const { return unsaved; }
  inline void set_unsaved(bool _unsaved) { unsaved = _unsaved; }
  
private:
  PathEditorPage();
  ~PathEditorPage();
  
  bool focused = false;
  bool unsaved = false;
  
  static PathEditorPage instance;
};
