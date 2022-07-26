#pragma once

#include <pages/page.h>
#include <thunder_auto.h>

class PropertiesPage: public Page {
public:
  static PropertiesPage* get() {
    return &instance;
  }
  
  PropertiesPage(PropertiesPage const&) = delete;
  PropertiesPage& operator=(PropertiesPage const&) = delete;
  
  void present(bool* running) override;
  bool is_focused() override { return focused; }
  
private:
  PropertiesPage();
  ~PropertiesPage();
  
  bool focused = false;
  
  static PropertiesPage instance;
};
