#pragma once

#include <ThunderAuto/Platform/Platform.hpp>
#include <memory>
#include <string>

class PlatformManager final {
  std::unique_ptr<PlatformImpl> m_impl;

 public:
  PlatformManager();

  void openURL(const std::string& url);
  std::string openFileDialog(FileType type, const FileExtensionList& extensions);
  std::string saveFileDialog(const FileExtensionList& extensions);
};
