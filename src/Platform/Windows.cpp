#include <ThunderAuto/Platform/Windows.hpp>

#include <ThunderAuto/Graphics/Graphics.hpp>
#include <Windows.h>

std::filesystem::path PlatformWindows::openFileDialog(FileType type, const FileExtensionList& extensions) {
  char filter_buffer[256] = {0};
  size_t filter_buffer_index = 0;

  create_filter(filter_buffer, &filter_buffer_index, extensions);

  HWND hwnd = reinterpret_cast<HWND>(PlatformGraphics::get().get_platform_handle());

  OPENFILENAMEA ofn;
  CHAR sz_file[260] = {0};
  CHAR current_dir[256] = {0};
  ZeroMemory(&ofn, sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = hwnd;
  ofn.lpstrFile = sz_file;
  ofn.nMaxFile = sizeof(sz_file);
  if (GetCurrentDirectoryA(256, current_dir)) {
    ofn.lpstrInitialDir = current_dir;
  }
  ofn.lpstrFilter = filter_buffer;
  ofn.nFilterIndex = 1;
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_HIDEREADONLY;

  if (GetOpenFileNameA(&ofn) == TRUE) {
    return ofn.lpstrFile;
  }

  return "";
}

std::filesystem::path PlatformWindows::saveFileDialog(const FileExtensionList& extensions) {
  char filter_buffer[256] = {0};
  size_t filter_buffer_index = 0;

  create_filter(filter_buffer, &filter_buffer_index, extensions);

  HWND hwnd = reinterpret_cast<HWND>(PlatformGraphics::get().get_platform_handle());

  OPENFILENAMEA ofn;
  CHAR sz_file[260] = {0};
  CHAR current_dir[256] = {0};
  ZeroMemory(&ofn, sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = hwnd;
  ofn.lpstrFile = sz_file;
  ofn.nMaxFile = sizeof(sz_file);
  if (GetCurrentDirectoryA(256, current_dir)) {
    ofn.lpstrInitialDir = current_dir;
  }
  ofn.lpstrFilter = filter_buffer;
  ofn.nFilterIndex = 1;
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR | OFN_HIDEREADONLY;

  ofn.lpstrDefExt = strchr(filter_buffer, '\0') + 1;

  if (GetSaveFileNameA(&ofn) == TRUE)
    return ofn.lpstrFile;

  return "";
}

void PlatformWindows::create_filter(char* filter_buffer,
                                    size_t* _filter_buffer_index,
                                    const FileExtensionList& extensions) {
  size_t& filter_buffer_index = *_filter_buffer_index;

  for (auto& [name, ext] : extensions) {
    size_t name_len = std::strlen(name);
    size_t ext_len = std::strlen(ext);
    std::memcpy(filter_buffer + filter_buffer_index, name, name_len);
    filter_buffer_index += name_len;
    filter_buffer[filter_buffer_index++] = '\0';
    std::memcpy(filter_buffer + filter_buffer_index, "*.", 2);
    filter_buffer_index += 2;
    std::memcpy(filter_buffer + filter_buffer_index, ext, ext_len);
    filter_buffer_index += ext_len;
    filter_buffer[filter_buffer_index++] = '\0';
  }
  if (filter_buffer_index > 0) {
    filter_buffer[filter_buffer_index++] = '\0';
  } else {
    const char* filter = "All Files (*.*)\0*.*\0\0";
    std::memcpy(filter_buffer, filter, std::strlen(filter));
  }
}
