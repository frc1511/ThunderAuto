#include <ThunderAuto/app.hpp>
#include <ThunderAuto/font_library.hpp>
#include <ThunderAuto/thunder_auto.hpp>

#include <ThunderAuto/graphics/graphics.hpp>

static void setup_data_handler(App& app);

int _main(int argc, char** argv) {
  std::optional<std::filesystem::path> start_project_path;
  while (argc > 1) {
    std::filesystem::path path(argv[1]);
    if (path.extension() != ".thunderauto") {
      fprintf(stderr, "'%s' does not have a .thunderauto extension\n", argv[1]);
      break;
    }

    if (!std::filesystem::exists(path)) {
      fprintf(stderr, "'%s' does not exist\n", argv[1]);
      break;
    }

    start_project_path = std::filesystem::absolute(path);

    break;
  }

  int exit_code = 0;

  App app;
  PlatformGraphics::get().init(app);

  setup_data_handler(app);

  if (start_project_path.has_value()) {
    app.open_from_path(start_project_path.value().string());
  }

  //
  // Main loop.
  //
  while (app.is_running()) {
    if (PlatformGraphics::get().poll_events()) {
      app.close();
    }

    app.process_input();

    static bool was_focused = true;
    bool is_focused = false;

    auto platform_io = ImGui::GetPlatformIO();
    for (ImGuiViewport* vp : platform_io.Viewports) {
      if (vp->PlatformWindowCreated) {
        is_focused |= ImGui::GetPlatformIO().Platform_GetWindowFocus(vp);
      }
    }

    if (is_focused != was_focused) {
      app.focus_was_changed(is_focused);
      was_focused = is_focused;
    }

    if (!is_focused) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 60));
    }

    // New Frame.
    PlatformGraphics::get().begin_frame();

    ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 10.f)); // Half titlebar size

    bool running = app.is_running();

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

    ImGui::PopStyleVar(4);

    ImGuiIO* io = &ImGui::GetIO();

    // Submit the DockSpace
    if (io->ConfigFlags & ImGuiConfigFlags_DockingEnable) {
      ImGuiID dockspace_id = ImGui::GetID("DockSpace");
      app.setup_dockspace(dockspace_id);
    }

    // Frame Content
    app.present();

    ImGui::End();

    // Render frame.
    PlatformGraphics::get().end_frame();
  }

  PlatformGraphics::get().deinit();

  return exit_code;
}

#if THUNDER_AUTO_WINDOWS
#include <Windows.h>

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine,
            int nShowCmd) {
  (void)hInstance;
  (void)hPrevInstance;
  (void)lpCmdLine;
  (void)nShowCmd;

  int argc;
  LPWSTR* wc_argv = CommandLineToArgvW(GetCommandLineW(), &argc);
  if (wc_argv == nullptr) {
    fputs("GetCommandLineW() failed\n", stderr);
    return 1;
  }

  char** argv = new char*[argc];
  for (int i = 0; i < argc; ++i) {
    int len = WideCharToMultiByte(CP_UTF8, 0, wc_argv[i], -1, nullptr, 0,
                                  nullptr, nullptr);
    argv[i] = new char[len];
    WideCharToMultiByte(CP_UTF8, 0, wc_argv[i], -1, argv[i], len, nullptr,
                        nullptr);
  }

  LocalFree(wc_argv);

  int exit_code = _main(argc, argv);

  for (int i = 0; i < argc; ++i) {
    delete[] argv[i];
  }

  delete[] argv;

  return exit_code;
}

#else
int main(int argc, char** argv) { return _main(argc, argv); }
#endif

static void setup_data_handler(App& app) {
  ImGuiSettingsHandler ini_handler;
  ini_handler.UserData = reinterpret_cast<void*>(&app);

  ini_handler.ClearAllFn = [](ImGuiContext*, ImGuiSettingsHandler* handler) {
    App* app = reinterpret_cast<App*>(handler->UserData);
    app->data_clear();
  };

  ini_handler.ReadInitFn = [](ImGuiContext*, ImGuiSettingsHandler*) {};

  ini_handler.ReadOpenFn = [](ImGuiContext*, ImGuiSettingsHandler* handler,
                              const char* name) -> void* {
    App* app = reinterpret_cast<App*>(handler->UserData);
    return reinterpret_cast<void*>(app->data_should_open(name));
  };

  ini_handler.ReadLineFn = [](ImGuiContext*, ImGuiSettingsHandler* handler,
                              void*, const char* line) {
    App* app = reinterpret_cast<App*>(handler->UserData);
    app->data_read_line(line);
  };

  ini_handler.ApplyAllFn = [](ImGuiContext*, ImGuiSettingsHandler* handler) {
    App* app = reinterpret_cast<App*>(handler->UserData);
    app->data_apply();
  };

  ini_handler.WriteAllFn = [](ImGuiContext*, ImGuiSettingsHandler* handler,
                              ImGuiTextBuffer* buf) {
    App* app = reinterpret_cast<App*>(handler->UserData);
    app->data_write(handler->TypeName, buf);
  };

  ini_handler.TypeName = "ThunderAuto";
  ini_handler.TypeHash = ImHashStr(ini_handler.TypeName);

  ImGuiContext* context = ImGui::GetCurrentContext();
  context->SettingsHandlers.push_back(ini_handler);
}
