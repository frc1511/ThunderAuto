#pragma once

#include <imgui.h>
#include <string>

class Popup {
public:
  virtual void present(bool* running) = 0;
  virtual std::string get_name() = 0;
  
protected:
  Popup() = default;
  ~Popup() = default;
};