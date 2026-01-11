#pragma once

#include <ThunderAuto/Platform/Platform.hpp>

class PlatformLinux : public Platform, public Singleton<PlatformLinux> {
 public:
  std::filesystem::path openFileDialog(FileType type, const FileExtensionList& extensions) noexcept override;
  std::filesystem::path saveFileDialog(const FileExtensionList& extensions) noexcept override;

  std::filesystem::path getAppDataDirectory() noexcept override;
};
