#pragma once

#include <ThunderAuto/platform/platform.hpp>

class PlatformMacOS : public PlatformImpl {
public:
  std::string open_file_dialog(FileType type,
                               const FileExtensionList& extensions) override;

  std::string save_file_dialog(const FileExtensionList& extensions) override;
};
