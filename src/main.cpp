#include <ThunderAuto/app.h>
#include <ThunderAuto/font_library.h>
#include <ThunderAuto/thunder_auto.h>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

//
// GL/GLSL versions.
//
#if defined(IMGUI_IMPL_OPENGL_ES2)
#error "OpenGL ES 2 is not supported"
#endif

#if defined(THUNDER_AUTO_MACOS) || defined(THUNDER_AUTO_TEST_MACOS)
#define GLSL_VERSION     "#version 150"
#define GL_VERSION_MAJOR 3
#define GL_VERSION_MINOR 2
#else
#define GLSL_VERSION     "#version 130"
#define GL_VERSION_MAJOR 3
#define GL_VERSION_MINOR 0
#endif

//
// Default Window Properties.
//
#define WINDOW_WIDTH  1200
#define WINDOW_HEIGHT 700
#define WINDOW_TITLE  "1511 Auto Planner"

static void apply_imgui_style();
static FontLibrary load_fonts();

#ifdef THUNDER_AUTO_WINDOWS
#include <Windows.h>
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine,
            int nShowCmd) {
  (void)hInstance;
  (void)hPrevInstance;
  (void)lpCmdLine;
  (void)nShowCmd;
#else
int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
#endif
  int exit_code = 0;

  //
  // Initialize GLFW.
  //
  glfwSetErrorCallback([](int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
  });

  if (!glfwInit()) {
    exit(-1);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_VERSION_MAJOR);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_VERSION_MINOR);

#if defined(THUNDER_AUTO_MACOS) || defined(THUNDER_AUTO_TEST_MACOS)
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on Mac
#endif

  //
  // Initialize window.
  //
  GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT,
                                        WINDOW_TITLE, nullptr, nullptr);
  if (!window) exit(-1);

  glfwMakeContextCurrent(window);
  // VSync.
  glfwSwapInterval(true);

  //
  // Load OpenGL functions.
  //
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

  //
  // Initialize ImGui
  //
  IMGUI_CHECKVERSION();

  ImGui::CreateContext();

  {
    ImGuiIO* io = &ImGui::GetIO();
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io->ConfigWindowsMoveFromTitleBarOnly = true;
  }

  apply_imgui_style();

  FontLibrary font_lib = load_fonts();

  // Setup OpenGL backend.
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(GLSL_VERSION);

  App app(window, font_lib);

  //
  // Main loop.
  //
  while (app.is_running()) {
    glfwPollEvents();

    app.process_input();

    if (glfwWindowShouldClose(window)) {
      app.close();
    }

    //
    // New Frame.
    //
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    bool running = app.is_running();

    // clang-format off
    if (!ImGui::Begin(WINDOW_TITLE, &running,
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

    //
    // Frame Content
    //
    app.present();

    ImGui::End();

    //
    // Render frame.
    //
    ImGui::Render();

    int buf_width, buf_height;
    glfwGetFramebufferSize(window, &buf_width, &buf_height);

    glViewport(0, 0, buf_width, buf_height);
    ImVec4 clear_color = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
                 clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      GLFWwindow* backup_current_context = glfwGetCurrentContext();
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
      glfwMakeContextCurrent(backup_current_context);
    }

    glfwSwapBuffers(window);
  }

  //
  // Shutdown ImGui.
  //
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  //
  // Shutdown GLFW.
  //
  glfwDestroyWindow(window);
  glfwTerminate();

  return exit_code;
}

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
