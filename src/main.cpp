#include <thunder_auto.h>
#include <app.h>

#include <imgui.h>

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 700

// Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
# error "OpenGL ES 2 is not supported"
#elif defined(__APPLE__)
# define GLSL_VERSION "#version 150" // GL 3.2 + GLSL 150
#else
# define GLSL_VERSION "#version 130" // GL 3.0 + GLSL 130
#endif

static void glfwErrorCallback(int error, const char* description) {
  std::cerr << "ERROR: GLFW error " << error << ": " << description << '\n';
}

static void set_imgui_style();

#ifdef THUNDER_AUTO_WINDOWS
#include <Windows.h>
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
#else
int main(int argc, char** argv) {
#endif
    int exit_code = 0;
  
  // --- Initialize GLFW ---
  
  glfwSetErrorCallback(glfwErrorCallback);
  if (!glfwInit()) {
    exit(-1);
  }
  
  // --- Initialize OpenGL ---
  
  // Decide GL+GLSL versions
#ifdef __APPLE__
  // GL 3.2 + GLSL 150
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
  // GL 3.0 + GLSL 130
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif
  
  // --- Initialize window ---
  
  GLFWwindow* window;
  if (!(window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "1511 Auto Planner", nullptr, nullptr))) {
    exit(-1);
  }
  
  glfwMakeContextCurrent(window);
  // VSync.
  glfwSwapInterval(true);

  // Load OpenGL functions.
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
  
  glfwSetWindowUserPointer(window, App::get());
  
  glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
    App::get()->handle_keyboard(key, scancode, action, mods);
  });

  App::get()->init(window);
  
  // --- Initialize ImGui ---
  
  IMGUI_CHECKVERSION();
  
  ImGui::CreateContext();
  
  ImGuiIO* io = &ImGui::GetIO();
  io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
  
  set_imgui_style();
  
  // Setup OpenGL backend.
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(GLSL_VERSION);
  
  // --- The main loop ---
  
  while (App::get()->is_running()) {
    if (glfwWindowShouldClose(window)) {
      App::get()->close();
    }
    
    glfwPollEvents();
    
    // --- Begin the next frame ---
    
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
    
    bool main_running = App::get()->is_running();
    
    if (!ImGui::Begin("1511 Auto Planner", &main_running,
                  ImGuiWindowFlags_MenuBar
                | ImGuiWindowFlags_NoDocking
                | ImGuiWindowFlags_NoTitleBar
                | ImGuiWindowFlags_NoCollapse
                | ImGuiWindowFlags_NoResize
                | ImGuiWindowFlags_NoMove
                | ImGuiWindowFlags_NoBringToFrontOnFocus
                | ImGuiWindowFlags_NoNavFocus)) {
      ImGui::End();
      App::get()->close();
      break;
    }
    
    ImGui::PopStyleVar(3);
    
    ImGuiIO* io = &ImGui::GetIO();
    
    // Submit the DockSpace
    if (io->ConfigFlags & ImGuiConfigFlags_DockingEnable) {
      ImGuiID dockspaceId = ImGui::GetID("DockSpace");
      ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    }
    
    // --- Run the application ---
    
    App::get()->present();
    
    ImGui::End();
    
    // --- Render ---
    
    ImGui::Render();
    
    int buf_width, buf_height;
    glfwGetFramebufferSize(window, &buf_width, &buf_height);
    
    glViewport(0, 0, buf_width, buf_height);
    ImVec4 clear_color = ImVec4(0.05f, 0.07f, 0.09f, 1.0f);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
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
  
  // --- De-initialize ImGui ---
  
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  // --- De-initialize GLFW window ---
  
  glfwDestroyWindow(window);
  glfwTerminate();
  
  return exit_code;
}

static void set_imgui_style() {
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
  ImVec4 yellow_high  = ImVec4(1.00f, 0.95f, 0.00f, 1.00f);
  ImVec4 yellow_low   = ImVec4(1.00f, 0.95f, 0.00f, 0.70f);
  ImVec4 red_very_low = ImVec4(0.93f, 0.11f, 0.14f, 0.20f);
  ImVec4 red_low      = ImVec4(0.93f, 0.11f, 0.14f, 0.50f);
  ImVec4 red_mid      = ImVec4(0.93f, 0.11f, 0.14f, 0.65f);
  ImVec4 red_high     = ImVec4(0.93f, 0.11f, 0.14f, 0.75f);
  ImVec4 grey_low     = ImVec4(0.10f, 0.12f, 0.15f, 1.00f);
  ImVec4 grey_mid     = ImVec4(0.13f, 0.15f, 0.17f, 1.00f);
  ImVec4 grey_high    = ImVec4(0.14f, 0.16f, 0.18f, 1.00f);
  ImVec4 bg           = ImVec4(0.05f, 0.07f, 0.09f, 1.00f);
  ImVec4 border       = ImVec4(0.17f, 0.18f, 0.21f, 1.00f);
  ImVec4 title_high   = ImVec4(0.09f, 0.11f, 0.13f, 1.00f);
  ImVec4 title_low    = ImVec4(0.07f, 0.09f, 0.11f, 1.00f);
  
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
  
  ImGuiIO* io = &ImGui::GetIO();
  
  // Fonts.
  //io->Fonts->AddFontFromFileTTF("Roboto-Regular.ttf", 15.0f);
  //io->Fonts->AddFontFromFileTTF("Roboto-Regular.ttf", 16.0f);
  //ImFont* default_font = io->Fonts->AddFontFromFileTTF("Roboto-Bold.ttf", 15.0f);
  //io->FontDefault = default_font;
  //io->Fonts->AddFontFromFileTTF("Roboto-Bold.ttf", 16.0f);
  
  ImFont* default_font = io->Fonts->AddFontFromFileTTF("Comic Sans MS.ttf", 16.0f);
  //io->Fonts->AddFontFromFileTTF("Comic Sans MS Bold.ttf", 16.0f);

  io->ConfigWindowsMoveFromTitleBarOnly = true;
}
