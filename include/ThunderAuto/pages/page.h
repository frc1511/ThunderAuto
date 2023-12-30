#pragma once

#include <ThunderAuto/thunder_auto.h>

class Page {
public:
  virtual const char* name() const = 0;

  virtual void present(bool* running) = 0;

protected:
  Page() = default;
  ~Page() = default;
};
