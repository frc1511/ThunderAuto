#pragma once

#include <ThunderAuto/platform/platform.h>

class PlatformWindows : public PlatformImpl {
public:
  std::string open_file_dialog(FileType type,
                               const FileExtensionList& extensions) override;

  std::string save_file_dialog(const FileExtensionList& extensions) override;

private:
  void create_filter(char* filter_buffer, std::size_t* filter_buffer_index,
                     const FileExtensionList& extensions);
};
