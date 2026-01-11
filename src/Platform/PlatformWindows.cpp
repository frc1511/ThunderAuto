#include "PlatformWindows.hpp"

#include <ThunderAuto/Graphics/Graphics.hpp>
#include <Windows.h>
#include <ShlObj.h>

std::filesystem::path PlatformWindows::openFileDialog(FileType type,
                                                      const FileExtensionList& extensions) noexcept {
  char filterBuffer[256] = {0};
  size_t filterBufferIndex = 0;

  createFilter(filterBuffer, &filterBufferIndex, extensions);

  HWND hwnd = reinterpret_cast<HWND>(getPlatformGraphics().getPlatformHandle());

  OPENFILENAMEA ofn;
  CHAR szFile[260] = {0};
  CHAR currentDir[256] = {0};
  ZeroMemory(&ofn, sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = hwnd;
  ofn.lpstrFile = szFile;
  ofn.nMaxFile = sizeof(szFile);
  if (GetCurrentDirectoryA(256, currentDir)) {
    ofn.lpstrInitialDir = currentDir;
  }
  ofn.lpstrFilter = filterBuffer;
  ofn.nFilterIndex = 1;
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_HIDEREADONLY;

  if (GetOpenFileNameA(&ofn) == TRUE) {
    return ofn.lpstrFile;
  }

  return "";
}

std::filesystem::path PlatformWindows::saveFileDialog(const FileExtensionList& extensions) noexcept {
  char filterBuffer[256] = {0};
  size_t filterBufferIndex = 0;

  createFilter(filterBuffer, &filterBufferIndex, extensions);

  HWND hwnd = reinterpret_cast<HWND>(getPlatformGraphics().getPlatformHandle());

  OPENFILENAMEA ofn;
  CHAR szFile[260] = {0};
  CHAR currentDir[256] = {0};
  ZeroMemory(&ofn, sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = hwnd;
  ofn.lpstrFile = szFile;
  ofn.nMaxFile = sizeof(szFile);
  if (GetCurrentDirectoryA(256, currentDir)) {
    ofn.lpstrInitialDir = currentDir;
  }
  ofn.lpstrFilter = filterBuffer;
  ofn.nFilterIndex = 1;
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR | OFN_HIDEREADONLY;

  ofn.lpstrDefExt = strchr(filterBuffer, '\0') + 1;

  if (GetSaveFileNameA(&ofn) == TRUE)
    return ofn.lpstrFile;

  return "";
}

std::filesystem::path PlatformWindows::getAppDataDirectory() noexcept {
  char path[MAX_PATH];
  if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, path))) {
    return std::filesystem::path(path) / "ThunderAuto";
  }
  return "";
}

void PlatformWindows::createFilter(char* filterBuffer,
                                   size_t* filterBufferIndexPtr,
                                   const FileExtensionList& extensions) {
  size_t& filterBufferIndex = *filterBufferIndexPtr;

  for (auto& [name, ext] : extensions) {
    size_t name_len = std::strlen(name);
    size_t ext_len = std::strlen(ext);
    std::memcpy(filterBuffer + filterBufferIndex, name, name_len);
    filterBufferIndex += name_len;
    filterBuffer[filterBufferIndex++] = '\0';
    std::memcpy(filterBuffer + filterBufferIndex, "*.", 2);
    filterBufferIndex += 2;
    std::memcpy(filterBuffer + filterBufferIndex, ext, ext_len);
    filterBufferIndex += ext_len;
    filterBuffer[filterBufferIndex++] = '\0';
  }
  if (filterBufferIndex > 0) {
    filterBuffer[filterBufferIndex++] = '\0';
  } else {
    const char* filter = "All Files (*.*)\0*.*\0\0";
    std::memcpy(filterBuffer, filter, std::strlen(filter));
  }
}
