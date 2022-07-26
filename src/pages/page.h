#pragma once

#include <thunder_auto.h>

class Page {
public:
  virtual void present(bool* running) = 0;
  virtual bool is_focused() = 0;
  
protected:
  Page() = default;
  ~Page() = default;
};
