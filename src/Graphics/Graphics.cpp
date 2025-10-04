#include <ThunderAuto/Graphics/Graphics.hpp>

#include <ThunderAuto/Logger.hpp>
#include <ThunderAuto/Error.hpp>

#include <ThunderAuto/UISizes.hpp>
#include <ThunderLibCore/Error.hpp>

#if THUNDERAUTO_DIRECTX11
#include "DX11Graphics.hpp"
#elif THUNDERAUTO_OPENGL
#include "OpenGLGraphics.hpp"
#else
#error "Unknown graphics API"
#endif

void Graphics::applyUIConfig() {
  ImGuiIO& io = ImGui::GetIO();

  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

  io.ConfigWindowsMoveFromTitleBarOnly = false;
}

void Graphics::applyUIStyle(bool darkMode) {
  UNUSED(darkMode);

  //
  // My janky Rolling Thunder ImGui theme...
  //

  ImGui::StyleColorsDark();

  ImGuiStyle& style = ImGui::GetStyle();

  // Colors.
  ImVec4 yellowHigh = ImVec4(1.00f, 0.95f, 0.00f, 1.00f);
  ImVec4 yellowLow = ImVec4(1.00f, 0.95f, 0.00f, 0.70f);
  ImVec4 redVeryLow = ImVec4(0.93f, 0.11f, 0.14f, 0.20f);
  ImVec4 redLow = ImVec4(0.93f, 0.11f, 0.14f, 0.50f);
  ImVec4 redMid = ImVec4(0.93f, 0.11f, 0.14f, 0.65f);
  ImVec4 redHigh = ImVec4(0.93f, 0.11f, 0.14f, 0.75f);
  ImVec4 greyLow = ImVec4(0.10f, 0.12f, 0.15f, 1.00f);
  ImVec4 greyMid = ImVec4(0.13f, 0.15f, 0.17f, 1.00f);
  ImVec4 greyHigh = ImVec4(0.14f, 0.16f, 0.18f, 1.00f);
  ImVec4 bg = ImVec4(0.05f, 0.07f, 0.09f, 1.00f);
  ImVec4 border = ImVec4(0.17f, 0.18f, 0.21f, 1.00f);
  ImVec4 titleHigh = ImVec4(0.09f, 0.11f, 0.13f, 1.00f);
  ImVec4 titleLow = ImVec4(0.07f, 0.09f, 0.11f, 1.00f);

  style.Colors[ImGuiCol_WindowBg] = bg;
  style.Colors[ImGuiCol_PopupBg] = bg;
  style.Colors[ImGuiCol_ChildBg] = bg;

  style.Colors[ImGuiCol_Border] = border;

  style.Colors[ImGuiCol_TitleBg] = titleLow;
  style.Colors[ImGuiCol_TitleBgCollapsed] = titleLow;
  style.Colors[ImGuiCol_TitleBgActive] = titleHigh;

  style.Colors[ImGuiCol_MenuBarBg] = titleHigh;

  style.Colors[ImGuiCol_Header] = greyLow;
  style.Colors[ImGuiCol_HeaderHovered] = greyMid;
  style.Colors[ImGuiCol_HeaderActive] = greyHigh;

  style.Colors[ImGuiCol_FrameBg] = greyLow;
  style.Colors[ImGuiCol_FrameBgHovered] = greyMid;
  style.Colors[ImGuiCol_FrameBgActive] = greyHigh;

  style.Colors[ImGuiCol_Button] = redLow;
  style.Colors[ImGuiCol_ButtonHovered] = redMid;
  style.Colors[ImGuiCol_ButtonActive] = redHigh;

  style.Colors[ImGuiCol_CheckMark] = redHigh;
  style.Colors[ImGuiCol_SliderGrab] = redLow;
  style.Colors[ImGuiCol_SliderGrabActive] = redHigh;

  style.Colors[ImGuiCol_SeparatorHovered] = yellowLow;
  style.Colors[ImGuiCol_SeparatorActive] = yellowHigh;

  style.Colors[ImGuiCol_ResizeGrip] = redVeryLow;
  style.Colors[ImGuiCol_ResizeGripHovered] = redMid;
  style.Colors[ImGuiCol_ResizeGripActive] = redHigh;

  style.Colors[ImGuiCol_Tab] = greyMid;
  style.Colors[ImGuiCol_TabHovered] = redMid;
  style.Colors[ImGuiCol_TabSelected] = redHigh;
  style.Colors[ImGuiCol_TabDimmed] = greyLow;
  style.Colors[ImGuiCol_TabDimmedSelected] = redLow;

  style.Colors[ImGuiCol_DockingPreview] = redLow;

  double dpiscale = PlatformGraphics::get().dpiScale();
  updateUIScale(dpiscale);
}

void Graphics::updateUIScale(double scale) {
  ImGuiStyle& style = ImGui::GetStyle();
  ImGuiStyle styleold = style;  // Backup colors

  // ScaleAllSizes will change the original size, so reset all style config
  style = ImGuiStyle();
  style.PopupRounding = 2;
  style.WindowRounding = 2;
  style.ChildRounding = 2;
  style.FrameRounding = 2;
  style.ScrollbarRounding = 12;
  style.GrabRounding = 2;
  style.TabRounding = 2;
  style.WindowMenuButtonPosition = ImGuiDir_None;
  style.WindowTitleAlign.x = 0.5f;

  // Titlebar sizes
  style.UserSizes[UISIZE_TITLEBAR_BUTTON_WIDTH] = 47.f;
  style.UserSizes[UISIZE_TITLEBAR_BUTTON_ICON_SIZE] = 10.f;
  style.UserSizes[UISIZE_TITLEBAR_BUTTON_MAXIMIZE_ICON_BOX_ROUNDING] = 1.f;
  style.UserSizes[UISIZE_TITLEBAR_BUTTON_MAXIMIZE_ICON_BOX_OFFSET] = 2.f;
  style.UserSizes[UISIZE_TITLEBAR_DRAG_AREA_MIN_WIDTH] = 30.f;
  style.UserSizes[UISIZE_TITLEBAR_FRAME_PADDING_Y] = 10.f;
  style.UserSizes[UISIZE_TITLEBAR_ITEM_SPACING_Y] = 20.f;
  // Window sizes
  style.UserSizes[UISIZE_WINDOW_MIN_WIDTH] = 500.f;
  style.UserSizes[UISIZE_WINDOW_MIN_HEIGHT] = 400.f;
  // Page sizes
  style.UserSizes[UISIZE_ACTIONS_PAGE_START_WIDTH] = 300.f;
  style.UserSizes[UISIZE_ACTIONS_PAGE_START_HEIGHT] = 400.f;
  style.UserSizes[UISIZE_EDITOR_PAGE_START_WIDTH] = 800.f;
  style.UserSizes[UISIZE_EDITOR_PAGE_START_HEIGHT] = 600.f;
  style.UserSizes[UISIZE_TRAJECTORY_MANAGER_PAGE_START_WIDTH] = 300.f;
  style.UserSizes[UISIZE_TRAJECTORY_MANAGER_PAGE_START_HEIGHT] = 600.f;
  style.UserSizes[UISIZE_PROPERTIES_PAGE_START_WIDTH] = 300.f;
  style.UserSizes[UISIZE_PROPERTIES_PAGE_START_HEIGHT] = 600.f;
  style.UserSizes[UISIZE_PROJECT_SETTINGS_PAGE_START_WIDTH] = 500.f;
  style.UserSizes[UISIZE_PROJECT_SETTINGS_PAGE_START_HEIGHT] = 350.f;
  // Popup sizes
  style.UserSizes[UISIZE_WELCOME_POPUP_WIDTH] = 630.f;
  style.UserSizes[UISIZE_WELCOME_POPUP_HEIGHT] = 235.f;
  style.UserSizes[UISIZE_WELCOME_POPUP_RECENT_PROJECT_COLUMN_WIDTH] = 225.f;
  style.UserSizes[UISIZE_WELCOME_POPUP_WINDOW_PADDING] = 20.f;
  style.UserSizes[UISIZE_NEW_FIELD_POPUP_START_WIDTH] = 800.f;
  style.UserSizes[UISIZE_NEW_FIELD_POPUP_START_HEIGHT] = 600.f;
  style.UserSizes[UISIZE_NEW_PROJECT_POPUP_START_WIDTH] = 500.f;
  style.UserSizes[UISIZE_NEW_PROJECT_POPUP_START_HEIGHT] = 250.f;
  style.UserSizes[UISIZE_UNSAVED_POPUP_START_WIDTH] = 125.f;
  style.UserSizes[UISIZE_UNSAVED_POPUP_START_HEIGHT] = 115.f;
  style.UserSizes[UISIZE_PROJECT_VERSION_POPUP_START_WIDTH] = 550.f;
  style.UserSizes[UISIZE_PROJECT_VERSION_POPUP_START_HEIGHT] = 300.f;
  style.UserSizes[UISIZE_PROJECT_OPEN_ERROR_POPUP_START_WIDTH] = 800.f;
  style.UserSizes[UISIZE_PROJECT_OPEN_ERROR_POPUP_START_HEIGHT] = 96.f;
  style.UserSizes[UISIZE_PROJECT_SAVE_ERROR_POPUP_START_WIDTH] = 800.f;
  style.UserSizes[UISIZE_PROJECT_SAVE_ERROR_POPUP_START_HEIGHT] = 96.f;
  style.UserSizes[UISIZE_CSV_EXPORT_POPUP_START_WIDTH] = 550.f;
  style.UserSizes[UISIZE_CSV_EXPORT_POPUP_START_HEIGHT] = 300.f;
  style.UserSizes[UISIZE_NEW_TRAJECTORY_POPUP_START_WIDTH] = 500.f;
  style.UserSizes[UISIZE_NEW_TRAJECTORY_POPUP_START_HEIGHT] = 250.f;
  style.UserSizes[UISIZE_RENAME_TRAJECTORY_POPUP_START_WIDTH] = 500.f;
  style.UserSizes[UISIZE_RENAME_TRAJECTORY_POPUP_START_HEIGHT] = 100.f;
  style.UserSizes[UISIZE_DUPLICATE_TRAJECTORY_POPUP_START_WIDTH] = 500.f;
  style.UserSizes[UISIZE_DUPLICATE_TRAJECTORY_POPUP_START_HEIGHT] = 100.f;
  style.UserSizes[UISIZE_LINK_TRAJECTORY_POINT_POPUP_START_WIDTH] = 500.f;
  style.UserSizes[UISIZE_LINK_TRAJECTORY_POINT_POPUP_START_HEIGHT] = 250.f;
  style.UserSizes[UISIZE_NEW_ACTION_POPUP_START_WIDTH] = 500.f;
  style.UserSizes[UISIZE_NEW_ACTION_POPUP_START_HEIGHT] = 250.f;
  style.UserSizes[UISIZE_RENAME_ACTION_POPUP_START_WIDTH] = 500.f;
  style.UserSizes[UISIZE_RENAME_ACTION_POPUP_START_HEIGHT] = 100.f;
  style.UserSizes[UISIZE_RECURSIVE_ACTION_ERROR_POPUP_START_WIDTH] = 500.f;
  style.UserSizes[UISIZE_RECURSIVE_ACTION_ERROR_POPUP_START_HEIGHT] = 200.f;
  // Editor page sizes
  style.UserSizes[UISIZE_DRAG_POINT_RADIUS] = 5.f;
  style.UserSizes[UISIZE_LINE_THICKNESS] = 2.f;
  // Trajectory Manager page sizes
  // Properties page sizes
  style.UserSizes[UISIZE_PROPERTIES_PAGE_TRAJECTORY_ITEM_LIST_CHILD_WINDOW_START_SIZE_Y] = 200.f;
  style.UserSizes[UISIZE_PROPERTIES_PAGE_ACTIONS_LIST_CHILD_WINDOW_START_SIZE_Y] = 50.f;
  // Actions page sizes
  style.UserSizes[UISIZE_ACTIONS_PAGE_ACTIONS_GROUP_CHILD_WINDOW_START_SIZE_Y] = 75.f;
  // New Action popup sizes
  style.UserSizes[UISIZE_NEW_ACTION_POPUP_CHILD_WINDOW_START_SIZE_Y] = 105.f;
  // Other sizes
  style.UserSizes[UISIZE_SELECTABLE_LIST_ITEM_SPACING_Y] = 10.f;
  style.UserSizes[UISIZE_FIELD_NORMAL_LEFT_COLUMN_WIDTH] = 135.f;
  style.UserSizes[UISIZE_INDENT_SMALL] = 10.f, style.UserSizes[UISIZE_INDENT_MEDIUM] = 20.f,
  style.UserSizes[UISIZE_INDENT_LARGE] = 30.f;

  style.ScaleAllSizes(scale);
  std::memcpy(style.Colors, styleold.Colors,
              sizeof(style.Colors));  // Restore colors

  loadFonts(scale);

  PlatformGraphics::get().windowSetSize(DEFAULT_WINDOW_WIDTH * scale, DEFAULT_WINDOW_HEIGHT * scale);
  PlatformGraphics::get().windowMoveToCenter();
}

#include <Ubuntu_Bold_ttf.h>
#include <Ubuntu_Regular_ttf.h>
// #include <Roboto_Regular_ttf.h>
// #include <Roboto_Bold_ttf.h>
#include <FontAwesome_Regular_ttf.h>
#include <FontAwesome_Solid_ttf.h>

#include <IconsFontAwesome5.h>

static void mergeFontAwesomeIconsWithFont(double size) {
  ImGuiIO* io = &ImGui::GetIO();
  ImFontConfig fontCfg;
  fontCfg.FontDataOwnedByAtlas = false;
  fontCfg.MergeMode = true;
  fontCfg.PixelSnapH = true;

  ImFont* font;

  static const ImWchar icon_ranges[] = {ICON_MIN_FA, ICON_MAX_16_FA, 0};
  // font = io->Fonts->AddFontFromMemoryTTF(FontAwesome_Regular_ttf, FontAwesome_Regular_ttf_size, size,
  //                                        &fontCfg, icon_ranges);
  // ThunderAutoAssert(font != nullptr);
  font = io->Fonts->AddFontFromMemoryTTF(FontAwesome_Solid_ttf, FontAwesome_Solid_ttf_size, size, &fontCfg,
                                         icon_ranges);
  ThunderAutoAssert(font != nullptr);
}

void Graphics::loadFonts(double scale) {
  FontLibrary& fontLib = FontLibrary::get();

  ImGuiIO* io = &ImGui::GetIO();

  io->Fonts->Clear();  // Clear previous fonts.

  // Tell ImGui not to free fonts from memory.
  ImFontConfig fontCfg;
  fontCfg.FontDataOwnedByAtlas = false;

  static const ImWchar* glyphRanges = io->Fonts->GetGlyphRangesDefault();

  // Regular font.
  fontLib.regularFont =
      io->Fonts->AddFontFromMemoryTTF(reinterpret_cast<void*>(Ubuntu_Regular_ttf), Ubuntu_Regular_ttf_size,
                                      15.0f * scale, &fontCfg, glyphRanges);
  ThunderAutoAssert(fontLib.regularFont != nullptr);

  mergeFontAwesomeIconsWithFont(15.0 * scale);

  fontLib.bigFont = io->Fonts->AddFontFromMemoryTTF(
      reinterpret_cast<void*>(Ubuntu_Bold_ttf), Ubuntu_Bold_ttf_size, 30.0f * scale, &fontCfg, glyphRanges);
  ThunderAutoAssert(fontLib.bigFont != nullptr);

  fontLib.boldFont = io->Fonts->AddFontFromMemoryTTF(
      reinterpret_cast<void*>(Ubuntu_Bold_ttf), Ubuntu_Bold_ttf_size, 15.0f * scale, &fontCfg, glyphRanges);
  ThunderAutoAssert(fontLib.boldFont != nullptr);

  mergeFontAwesomeIconsWithFont(15.0 * scale);
}

Graphics& PlatformGraphics::get() {
#if THUNDERAUTO_DIRECTX11
  return GraphicsDirectX11::get();
#else  // THUNDERAUTO_OPENGL
  return GraphicsOpenGL::get();
#endif
}
