#pragma once

#include <ThunderAuto/Platform/Platform.hpp>

class PlatformWindows : public Platform, public Singleton<PlatformWindows> {
 public:
  std::filesystem::path openFileDialog(FileType type, const FileExtensionList& extensions) noexcept override;
  std::filesystem::path saveFileDialog(const FileExtensionList& extensions) noexcept override;

 private:
  void createFilter(char* filterBuffer, std::size_t* filterBufferIndex, const FileExtensionList& extensions);
};
