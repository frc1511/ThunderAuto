#pragma once

#include <string>

enum class FileType {
  FILE,
  DIRECTORY,
};

class Platform {
public:
  
  virtual std::string open_file_dialog(FileType type, const char* extension = nullptr) = 0;
  virtual std::string save_file_dialog(const char* extension = nullptr) = 0;
};
