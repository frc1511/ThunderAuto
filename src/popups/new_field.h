#pragma once

#include <popups/popup.h>
#include <platform/platform.h>
#include <field.h>
#include <string>
#include <optional>

class NewFieldPopup: public Popup {
public:
  static NewFieldPopup* get() {
    return &instance;
  }
  
  NewFieldPopup(NewFieldPopup const&) = delete;
  NewFieldPopup& operator=(NewFieldPopup const&) = delete;
  
  void present(bool* running) override;
  std::string get_name() override { return name; }

  std::optional<Field> get_field() { return field; }
  
private:
  NewFieldPopup();
  ~NewFieldPopup();

  void present_field_setup();
  
  std::string name = "New Field";

  Platform* platform = nullptr;
  
  bool selected_img = false;

  unsigned int field_tex;
  float field_aspect_ratio = 1;
  bool img_load_failed = false;

  std::optional<Field> field = std::nullopt;
  
  static NewFieldPopup instance;
};
