#include <platform/windows/windows.h>
#include <app.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <Windows.h>

PlatformWindows::PlatformWindows() { }

PlatformWindows::~PlatformWindows() { }

std::string PlatformWindows::open_file_dialog(FileType type, const char* extension) {
  GLFWwindow* window = App::get()->get_window();
  const char* filter = extension ? ("*." + std::string(extension)).c_str() : "*";

  OPENFILENAMEA ofn;
  CHAR szFile[260] = { 0 };
  CHAR currentDir[256] = { 0 };
  ZeroMemory(&ofn, sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = glfwGetWin32Window(window);
  ofn.lpstrFile = szFile;
  ofn.nMaxFile = sizeof(szFile);
  if (GetCurrentDirectoryA(256, currentDir))
	  ofn.lpstrInitialDir = currentDir;
  ofn.lpstrFilter = filter;
  ofn.nFilterIndex = 1;
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

  if (GetOpenFileNameA(&ofn) == TRUE)
	  return ofn.lpstrFile;

  return "";
}

std::string PlatformWindows::save_file_dialog(const char* extension) {
  GLFWwindow* window = App::get()->get_window();

  const char* filter = extension ? ("*." + std::string(extension)).c_str() : "*";

  OPENFILENAMEA ofn;
  CHAR szFile[260] = { 0 };
  CHAR currentDir[256] = { 0 };
  ZeroMemory(&ofn, sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = glfwGetWin32Window(window);
  ofn.lpstrFile = szFile;
  ofn.nMaxFile = sizeof(szFile);
  if (GetCurrentDirectoryA(256, currentDir)) {
    ofn.lpstrInitialDir = currentDir;
  }
  ofn.lpstrFilter = filter;
  ofn.nFilterIndex = 1;
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

  ofn.lpstrDefExt = strchr(filter, '\0') + 1;

  if (GetSaveFileNameA(&ofn) == TRUE)
    return ofn.lpstrFile;

  return "";
}

PlatformWindows PlatformWindows::instance {};
