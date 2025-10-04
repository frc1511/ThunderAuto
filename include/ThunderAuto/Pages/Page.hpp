#pragma once

#include <ThunderAuto/UISizes.hpp>
#include <imgui.h>
#include <imgui_raii.h>

class Page {
 protected:
  Page() = default;

 public:
  virtual ~Page() = default;

  virtual const char* name() const noexcept = 0;
  virtual void present(bool* running) = 0;
};
