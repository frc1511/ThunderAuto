#pragma once

#include <ThunderAuto/platform/platform.h>
#include <ThunderAuto/thunder_auto.h>

class PlatformManager {
  GLFWwindow* m_window;
  std::unique_ptr<PlatformImpl> m_impl;

public:
  PlatformManager(GLFWwindow* window);

  std::string open_file_dialog(FileType type,
                               const FileExtensionList& extensions);

  std::string save_file_dialog(const FileExtensionList& extensions);
};
