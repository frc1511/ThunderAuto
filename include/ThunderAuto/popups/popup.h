#pragma once

#include <ThunderAuto/thunder_auto.h>

class Popup {
public:
  virtual void present(bool* running) = 0;
  virtual const char* name() = 0;

protected:
  Popup() = default;
  ~Popup() = default;
};
