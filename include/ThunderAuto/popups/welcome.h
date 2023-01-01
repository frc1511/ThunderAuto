#pragma once

#include <ThunderAuto/popups/popup.h>

class WelcomePopup: public Popup {
public:
  static WelcomePopup* get() {
    return &instance;
  }
  
  WelcomePopup(WelcomePopup const&) = delete;
  WelcomePopup& operator=(WelcomePopup const&) = delete;
  
  void present(bool* running) override;
  std::string get_name() override { return name; }
  
  constexpr bool is_showing_new_project_popup() const { return new_project; }
  constexpr void set_showing_new_project_popup(bool show) { new_project = show; }

  constexpr bool is_opening_project() const { return opening_project; }
  constexpr void set_opening_project(bool opening) { opening_project = opening; }

private:
  WelcomePopup();
  ~WelcomePopup();
  
  std::string name = "##Welcome!";

  bool new_project = false;
  bool opening_project = false;
  
  static WelcomePopup instance;
};
