#include <ThunderAuto/app.h>
#include <ThunderAuto/font_library.h>
#include <ThunderAuto/thunder_auto.h>

#include <ThunderAuto/graphics.h>

#include <thread>

static void apply_imgui_style();
static FontLibrary load_fonts();
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

  Graphics::get().init();

  apply_imgui_style();
  FontLibrary font_lib = load_fonts();

  App app(font_lib);

  setup_data_handler(app);

  if (start_project_path.has_value()) {
    app.open_from_path(start_project_path.value().string());
  }

  //
  // Main loop.
  //
  while (app.is_running()) {
    if (Graphics::get().poll_events()) {
      app.close();
    }

    app.process_input();

    bool is_focused = false;

    auto platform_io = ImGui::GetPlatformIO();
    for (ImGuiViewport* vp : platform_io.Viewports) {
      if (vp->PlatformWindowCreated) {
        is_focused |= ImGui::GetPlatformIO().Platform_GetWindowFocus(vp);
      }
    }

    if (!is_focused) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1000/60));
      continue;
    }

    // New Frame.
    Graphics::get().begin_frame();

    ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    bool running = app.is_running();

    // clang-format off
    if (!ImGui::Begin("ThunderAuto", &running,
                      ImGuiWindowFlags_MenuBar
                    | ImGuiWindowFlags_NoDocking
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
      ImGuiID dockspace_id = ImGui::GetID("DockSpace");
      app.setup_dockspace(dockspace_id);
    }

    // Frame Content
    app.present();

    ImGui::End();

    // Render frame.
    Graphics::get().end_frame();
  }

  Graphics::get().deinit();

  return exit_code;
}

#if TH_WINDOWS
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

#include <Ubuntu_Bold_ttf.h>
#include <Ubuntu_Regular_ttf.h>
// #include <Roboto_Regular_ttf.h>
// #include <Roboto_Bold_ttf.h>
#include <FontAwesome_Regular_ttf.h>
#include <FontAwesome_Solid_ttf.h>

#include <IconsFontAwesome5.h>

static void apply_imgui_style() {

  //
  // My janky Rolling Thunder ImGui theme...
  //

  ImGui::StyleColorsDark();

  ImGuiStyle& style = ImGui::GetStyle();
  style.PopupRounding = 2;
  style.WindowRounding = 2;
  style.ChildRounding = 2;
  style.FrameRounding = 2;
  style.ScrollbarRounding = 12;
  style.GrabRounding = 2;
  style.TabRounding = 2;
  style.WindowMenuButtonPosition = ImGuiDir_None;
  style.WindowTitleAlign.x = 0.5f;

  // Colors.
  ImVec4 yellow_high = ImVec4(1.00f, 0.95f, 0.00f, 1.00f);
  ImVec4 yellow_low = ImVec4(1.00f, 0.95f, 0.00f, 0.70f);
  ImVec4 red_very_low = ImVec4(0.93f, 0.11f, 0.14f, 0.20f);
  ImVec4 red_low = ImVec4(0.93f, 0.11f, 0.14f, 0.50f);
  ImVec4 red_mid = ImVec4(0.93f, 0.11f, 0.14f, 0.65f);
  ImVec4 red_high = ImVec4(0.93f, 0.11f, 0.14f, 0.75f);
  ImVec4 grey_low = ImVec4(0.10f, 0.12f, 0.15f, 1.00f);
  ImVec4 grey_mid = ImVec4(0.13f, 0.15f, 0.17f, 1.00f);
  ImVec4 grey_high = ImVec4(0.14f, 0.16f, 0.18f, 1.00f);
  ImVec4 bg = ImVec4(0.05f, 0.07f, 0.09f, 1.00f);
  ImVec4 border = ImVec4(0.17f, 0.18f, 0.21f, 1.00f);
  ImVec4 title_high = ImVec4(0.09f, 0.11f, 0.13f, 1.00f);
  ImVec4 title_low = ImVec4(0.07f, 0.09f, 0.11f, 1.00f);

  style.Colors[ImGuiCol_WindowBg] = bg;
  style.Colors[ImGuiCol_PopupBg] = bg;
  style.Colors[ImGuiCol_ChildBg] = bg;

  style.Colors[ImGuiCol_Border] = border;

  style.Colors[ImGuiCol_TitleBg] = title_low;
  style.Colors[ImGuiCol_TitleBgCollapsed] = title_low;
  style.Colors[ImGuiCol_TitleBgActive] = title_high;

  style.Colors[ImGuiCol_MenuBarBg] = title_high;

  style.Colors[ImGuiCol_Header] = grey_low;
  style.Colors[ImGuiCol_HeaderHovered] = grey_mid;
  style.Colors[ImGuiCol_HeaderActive] = grey_high;

  style.Colors[ImGuiCol_FrameBg] = grey_low;
  style.Colors[ImGuiCol_FrameBgHovered] = grey_mid;
  style.Colors[ImGuiCol_FrameBgActive] = grey_high;

  style.Colors[ImGuiCol_Button] = red_low;
  style.Colors[ImGuiCol_ButtonHovered] = red_mid;
  style.Colors[ImGuiCol_ButtonActive] = red_high;

  style.Colors[ImGuiCol_CheckMark] = red_high;
  style.Colors[ImGuiCol_SliderGrab] = red_low;
  style.Colors[ImGuiCol_SliderGrabActive] = red_high;

  style.Colors[ImGuiCol_SeparatorHovered] = yellow_low;
  style.Colors[ImGuiCol_SeparatorActive] = yellow_high;

  style.Colors[ImGuiCol_ResizeGrip] = red_very_low;
  style.Colors[ImGuiCol_ResizeGripHovered] = red_mid;
  style.Colors[ImGuiCol_ResizeGripActive] = red_high;

  style.Colors[ImGuiCol_Tab] = grey_mid;
  style.Colors[ImGuiCol_TabHovered] = red_mid;
  style.Colors[ImGuiCol_TabActive] = red_high;
  style.Colors[ImGuiCol_TabUnfocused] = grey_low;
  style.Colors[ImGuiCol_TabUnfocusedActive] = red_low;

  style.Colors[ImGuiCol_DockingPreview] = red_low;
}

static FontLibrary load_fonts() {
  FontLibrary font_lib;

  ImGuiIO* io = &ImGui::GetIO();

  // Tell ImGui not to free fonts from memory.
  ImFontConfig font_cfg;
  font_cfg.FontDataOwnedByAtlas = false;

  static const ImWchar* glyph_ranges = io->Fonts->GetGlyphRangesDefault();

  // Regular font.
  font_lib.regular_font = io->Fonts->AddFontFromMemoryTTF(
      reinterpret_cast<void*>(Ubuntu_Regular_ttf), Ubuntu_Regular_ttf_size,
      15.0f, &font_cfg, glyph_ranges);

  // FontAwesome icons...
  font_cfg.MergeMode = true;
  font_cfg.PixelSnapH = true;

  static const ImWchar icon_ranges[] = {ICON_MIN_FA, ICON_MAX_16_FA, 0};
  io->Fonts->AddFontFromMemoryTTF(FontAwesome_Regular_ttf,
                                  FontAwesome_Regular_ttf_size, 15.0f,
                                  &font_cfg, icon_ranges);
  io->Fonts->AddFontFromMemoryTTF(FontAwesome_Solid_ttf,
                                  FontAwesome_Solid_ttf_size, 15.0f, &font_cfg,
                                  icon_ranges);

  // Additional fonts.
  font_cfg.MergeMode = false;
  font_cfg.PixelSnapH = false;

  font_lib.big_font = io->Fonts->AddFontFromMemoryTTF(
      reinterpret_cast<void*>(Ubuntu_Bold_ttf), Ubuntu_Bold_ttf_size, 30.0f,
      &font_cfg, glyph_ranges);

  io->ConfigWindowsMoveFromTitleBarOnly = true;

  return font_lib;
}

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

