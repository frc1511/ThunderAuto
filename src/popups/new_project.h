#pragma once

#include <popups/popup.h>

class NewProjectPopup: public Popup {
public:
  static NewProjectPopup* get() {
    return &instance;
  }
  
  NewProjectPopup(NewProjectPopup const&) = delete;
  NewProjectPopup& operator=(NewProjectPopup const&) = delete;
  
  void present(bool* running) override;
  std::string get_name() override { return name; }
  
private:
  NewProjectPopup();
  ~NewProjectPopup();
  
  std::string name = "New Project";
  
  static NewProjectPopup instance;
};
