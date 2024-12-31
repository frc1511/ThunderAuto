#pragma once

#include <ThunderAuto/thunder_auto.h>

enum class FileType {
  FILE,
  DIRECTORY,
};

using FileExtensionList = std::unordered_map<const char*, const char*>;

class PlatformImpl {
public:
  PlatformImpl() = default;
  virtual ~PlatformImpl() = default;

  virtual std::string open_file_dialog(FileType type,
                                       const FileExtensionList& extensions) = 0;

  virtual std::string save_file_dialog(const FileExtensionList& extensions) = 0;
};
