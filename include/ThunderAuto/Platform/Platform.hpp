#pragma once

#include <unordered_map>
#include <filesystem>
#include <string>

enum class FileType {
  FILE,
  DIRECTORY,
};

extern const char* const kThunderAutoFileExtension;
extern const std::pair<const char*, const char*> kThunderAutoFileFilter;

using FileExtensionList = std::unordered_map<const char*, const char*>;

class PlatformImpl {
 public:
  PlatformImpl() = default;
  virtual ~PlatformImpl() = default;

  void openURL(const std::string& url);

  virtual std::filesystem::path openFileDialog(FileType type, const FileExtensionList& extensions) = 0;
  virtual std::filesystem::path saveFileDialog(const FileExtensionList& extensions) = 0;
};
