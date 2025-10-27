#include "PlatformLinux.hpp"
#include <nfd.h>
#include <vector>

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
  args.filterList = filterItems.data();
  args.filterCount = static_cast<nfdfiltersize_t>(numExtensions);

  nfdu8char_t* outPath = nullptr;
  nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);
  if (result == NFD_OKAY) {
    return std::filesystem::path(outPath);
  }

  // Error or cancel.
  return "";
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
  args.filterList = filterItems.data();
  args.filterCount = static_cast<nfdfiltersize_t>(numExtensions);

  nfdu8char_t* outPath = nullptr;
  nfdresult_t result = NFD_SaveDialogU8_With(&outPath, &args);
  if (result == NFD_OKAY) {
    return std::filesystem::path(outPath);
  }

  // Error or cancel.
  return "";
}
