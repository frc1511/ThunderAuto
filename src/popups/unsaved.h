#pragma once

#include <popups/popup.h>

class UnsavedPopup: public Popup {
public:
  static UnsavedPopup* get() {
    return &instance;
  }
  
  UnsavedPopup(UnsavedPopup const&) = delete;
  UnsavedPopup& operator=(UnsavedPopup const&) = delete;
  
  void present(bool* running) override;
  std::string get_name() override { return name; }
  
  enum Result {
    CANCEL = 0,
    SAVE,
    DONT_SAVE,
  };
  
  constexpr Result get_result() const { return result; }
  
private:
  UnsavedPopup();
  ~UnsavedPopup();
  
  std::string name = "Do you want to save the changes made to this document?";
  
  Result result = CANCEL;
  
  static UnsavedPopup instance;
};
