#pragma once

#include <ThunderAuto/Platform/Platform.hpp>

class PlatformLinux : public PlatformImpl {
 public:
  std::filesystem::path openFileDialog(FileType type, const FileExtensionList& extensions) override;
  std::filesystem::path saveFileDialog(const FileExtensionList& extensions) override;
};
