#include <ThunderAuto/App.hpp>
#include <ThunderAuto/Logger.hpp>
#include <ThunderAuto/FontLibrary.hpp>
#include <ThunderAuto/Graphics/Graphics.hpp>
#include <imgui.h>
#include <imgui_internal.h>

// Returns the path to the project file to open if provided as the first command line argument.
static std::optional<std::filesystem::path> GetStartProjectPath(int argc, char** argv) {
  if (argc <= 1) {
    return std::nullopt;
  }

  std::filesystem::path path(argv[1]);
  if (path.extension() != ".thunderauto") {
    ThunderAutoLogger::Error("File '{}' does not have a .thunderauto extension", argv[1]);
    return std::nullopt;

  } else if (!std::filesystem::exists(path)) {
    ThunderAutoLogger::Error("Project file '{}' does not exist", argv[1]);
    return std::nullopt;
  }

  if (argc > 2) {
    ThunderAutoLogger::Warn("Received more than one argument, ignoring all but the first file path");
  }

  std::filesystem::path startProjectPath = std::filesystem::absolute(path);
  return startProjectPath;
}

// Tell ImGui to use App to load and save data from/to the imgui.ini file.
static void SetupDataHandler(App& app) {
  ImGuiSettingsHandler iniHandler;
  iniHandler.UserData = reinterpret_cast<void*>(&app);

  iniHandler.ClearAllFn = [](ImGuiContext*, ImGuiSettingsHandler* handler) {
    App* app = reinterpret_cast<App*>(handler->UserData);
    app->dataClear();
  };

  iniHandler.ReadInitFn = [](ImGuiContext*, ImGuiSettingsHandler*) {};

  iniHandler.ReadOpenFn = [](ImGuiContext*, ImGuiSettingsHandler* handler, const char* name) -> void* {
    App* app = reinterpret_cast<App*>(handler->UserData);
    return reinterpret_cast<void*>(app->dataShouldOpen(name));
  };

  iniHandler.ReadLineFn = [](ImGuiContext*, ImGuiSettingsHandler* handler, void*, const char* line) {
    App* app = reinterpret_cast<App*>(handler->UserData);
    app->dataReadLine(line);
  };

  iniHandler.ApplyAllFn = [](ImGuiContext*, ImGuiSettingsHandler* handler) {
    App* app = reinterpret_cast<App*>(handler->UserData);
    app->dataApply();
  };

  iniHandler.WriteAllFn = [](ImGuiContext*, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf) {
    App* app = reinterpret_cast<App*>(handler->UserData);
    app->dataWrite(handler->TypeName, buf);
  };

  iniHandler.TypeName = "ThunderAuto";
  iniHandler.TypeHash = ImHashStr(iniHandler.TypeName);

  ImGuiContext* context = ImGui::GetCurrentContext();
  context->SettingsHandlers.push_back(iniHandler);
}

// The real main function that handles all the important stuff.
static int main2(int argc, char** argv) {
  std::optional<std::filesystem::path> startProjectPath = GetStartProjectPath(argc, argv);
  int exitCode = 0;

  App app;
  getPlatformGraphics().init(app);

  SetupDataHandler(app);

  if (startProjectPath.has_value()) {
    app.openFromPath(startProjectPath.value().string());
  }

  //
  // Main loop.
  //
  while (app.isRunning()) {
    if (getPlatformGraphics().pollEvents()) {
      getPlatformGraphics().setMainWindowShouldClose(false);
      app.close();
    }

    app.processInput();

    static bool wasFocused = true;
    bool isFocused = false;

    ImGuiPlatformIO& platformIO = ImGui::GetPlatformIO();
    for (ImGuiViewport* vp : platformIO.Viewports) {
      if (vp->PlatformWindowCreated) {
        void* nativeWindowHandle = vp->PlatformHandle;
        isFocused |= getPlatformGraphics().isWindowFocused(nativeWindowHandle);
      }
    }

    if (isFocused != wasFocused) {
      app.focusWasChanged(isFocused);
      wasFocused = isFocused;
    }

    if (!isFocused) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60));
    }

    // New Frame.
    getPlatformGraphics().beginFrame();

    ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    // ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 10.f));  // Half titlebar size

    bool running = app.isRunning();

    // clang-format off
    if (!ImGui::Begin("ThunderAuto", &running,
                      ImGuiWindowFlags_NoDocking
                    | ImGuiWindowFlags_NoTitleBar
                    | ImGuiWindowFlags_NoCollapse
                    | ImGuiWindowFlags_NoResize
                    | ImGuiWindowFlags_NoMove
                    | ImGuiWindowFlags_NoBringToFrontOnFocus
                    | ImGuiWindowFlags_NoNavFocus)) {
      // clang-format on

      ImGui::End();
      app.close();
      break;
    }

    ImGui::PopStyleVar(3);

    ImGuiIO* io = &ImGui::GetIO();

    // Submit the DockSpace
    if (io->ConfigFlags & ImGuiConfigFlags_DockingEnable) {
      ImGuiID dockspaceID = ImGui::GetID("DockSpace");
      app.setupDockspace(dockspaceID);
    }

    // Frame Content
    app.present();

    ImGui::End();

    // Render frame.
    getPlatformGraphics().endFrame();
  }

  getPlatformGraphics().deinit();

  return exitCode;
}

// Wrapper around main2 to catch exceptions and log them.
static int main1(int argc, char** argv) {
  int exitCode;

  try {
    exitCode = main2(argc, argv);

  } catch (std::exception& e) {
    ThunderAutoLogger::Error("Uncaught exception: {}", e.what());
    ThunderAutoLogger::Critical("Terminating now");
    std::terminate();

  } catch (...) {
    ThunderAutoLogger::Error("Uncaught unknown exception");
    ThunderAutoLogger::Critical("Terminating now");
    std::terminate();
  }

  ThunderAutoLogger::Info("Exiting with code {}", exitCode);

  return exitCode;
}

//
// Platform-specific entry points
//

#if THUNDERAUTO_WINDOWS
#include <Windows.h>

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
  (void)hInstance;
  (void)hPrevInstance;
  (void)lpCmdLine;
  (void)nShowCmd;

  int argc;
  LPWSTR* wcArgv = CommandLineToArgvW(GetCommandLineW(), &argc);
  if (wcArgv == nullptr) {
    fputs("GetCommandLineW() failed\n", stderr);
    return 1;
  }

  char** argv = new char*[argc];
  for (int i = 0; i < argc; ++i) {
    int len = WideCharToMultiByte(CP_UTF8, 0, wcArgv[i], -1, nullptr, 0, nullptr, nullptr);
    argv[i] = new char[len];
    WideCharToMultiByte(CP_UTF8, 0, wcArgv[i], -1, argv[i], len, nullptr, nullptr);
  }

  LocalFree(wcArgv);

  int exitCode = main1(argc, argv);

  for (int i = 0; i < argc; ++i) {
    delete[] argv[i];
  }

  delete[] argv;

  return exitCode;
}

#else
int main(int argc, char** argv) {
  return main1(argc, argv);
}
#endif
