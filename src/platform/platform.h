#pragma once

#include <string>

class Platform {
public:
  virtual std::string open_file_dialog() = 0;
  
};
