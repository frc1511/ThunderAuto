#pragma once

#include <ThunderAuto/UISizes.hpp>
#include <imgui.h>
#include <imgui_raii.h>

class Popup {
 protected:
  Popup() = default;

 public:
  virtual ~Popup() = default;

  virtual void present(bool* running) = 0;
  virtual const char* name() const noexcept = 0;
};
