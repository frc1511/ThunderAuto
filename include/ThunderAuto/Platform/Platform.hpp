#pragma once

#include <ThunderAuto/Singleton.hpp>
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

class Platform : public SingletonBase {
 public:
  Platform() = default;
  virtual ~Platform() = default;

  void openURL(const std::string& url) noexcept;

  virtual std::filesystem::path openFileDialog(FileType type, const FileExtensionList& extensions) noexcept = 0;
  virtual std::filesystem::path saveFileDialog(const FileExtensionList& extensions) noexcept = 0;

  virtual std::filesystem::path getAppDataDirectory() noexcept = 0;
};

Platform& getPlatform() noexcept;