#include "PlatformLinux.hpp"
#include <nfd.h>
#include <vector>
#include <cstring>

std::filesystem::path PlatformLinux::openFileDialog(FileType type,
                                                    const FileExtensionList& extensions) noexcept {
  const size_t numExtensions = extensions.size();

  std::vector<nfdu8filteritem_t> filterItems(numExtensions);
  size_t index = 0;
  for (auto& [name, ext] : extensions) {
    filterItems[index].name = name;
    filterItems[index].spec = ext;
    index++;
  }

  nfdopendialogu8args_t args;
  std::memset(&args, 0, sizeof(nfdopendialogu8args_t));

  args.filterList = filterItems.data();
  args.filterCount = static_cast<nfdfiltersize_t>(numExtensions);

  nfdu8char_t* outPathStr = nullptr;
  nfdresult_t result = NFD_OpenDialogU8_With(&outPathStr, &args);

  std::filesystem::path outPath;

  if (outPathStr != nullptr) {
    if (result == NFD_OKAY) {
      outPath = std::filesystem::path(outPathStr);
    }
    NFD_FreePathU8(outPathStr);
  }

  return outPath;
}

std::filesystem::path PlatformLinux::saveFileDialog(const FileExtensionList& extensions) noexcept {
  const size_t numExtensions = extensions.size();

  std::vector<nfdu8filteritem_t> filterItems(numExtensions);
  size_t index = 0;
  for (auto& [name, ext] : extensions) {
    filterItems[index].name = name;
    filterItems[index].spec = ext;
    index++;
  }

  nfdsavedialogu8args_t args;
  std::memset(&args, 0, sizeof(nfdsavedialogu8args_t));

  args.filterList = filterItems.data();
  args.filterCount = static_cast<nfdfiltersize_t>(numExtensions);

  nfdu8char_t* outPathStr = nullptr;
  nfdresult_t result = NFD_SaveDialogU8_With(&outPathStr, &args);

  std::filesystem::path outPath;
  if (outPathStr != nullptr) {
    if (result == NFD_OKAY) {
      outPath = std::filesystem::path(outPathStr);
    }
    NFD_FreePathU8(outPathStr);
  }

  return outPath;
}
