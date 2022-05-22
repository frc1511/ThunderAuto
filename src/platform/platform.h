#pragma once

#include <string>

class Platform {
public:
  virtual std::string open_file_dialog() = 0;
  virtual std::string save_file_dialog() = 0;
};
