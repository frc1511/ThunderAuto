#pragma once

#include <ThunderAuto/Platform/Platform.hpp>

class PlatformMacOS : public Platform, public Singleton<PlatformMacOS> {
 public:
  std::filesystem::path openFileDialog(FileType type, const FileExtensionList& extensions) noexcept override;
  std::filesystem::path saveFileDialog(const FileExtensionList& extensions) noexcept override;
};
