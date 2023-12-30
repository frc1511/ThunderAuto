#pragma once

#include <ThunderAuto/platform/platform.h>

class PlatformLinux : public PlatformImpl {
public:
  PlatformLinux(GLFWwindow* window);

  std::string open_file_dialog(FileType type,
                               const FileExtensionList& extensions) override;

  std::string save_file_dialog(const FileExtensionList& extensions) override;
};
