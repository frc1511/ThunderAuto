#pragma once

#include <ThunderAuto/Platform/Platform.hpp>

class PlatformWindows : public PlatformImpl {
 public:
  std::filesystem::path openFileDialog(FileType type, const FileExtensionList& extensions) override;
  std::filesystem::path saveFileDialog(const FileExtensionList& extensions) override;

 private:
  void createFilter(char* filterBuffer, std::size_t* filterBufferIndex, const FileExtensionList& extensions);
};
