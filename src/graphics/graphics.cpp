#include <ThunderAuto/graphics/graphics.hpp>

#include <ThunderAuto/ui_sizes.hpp>

#if THUNDER_AUTO_DIRECTX11
#include "graphics_directx11.hpp"
#elif THUNDER_AUTO_OPENGL
#include "graphics_opengl.hpp"
#else
#error "Unknown graphics API"
#endif

void Graphics::apply_ui_config() {
  ImGuiIO& io = ImGui::GetIO();

  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

  io.ConfigWindowsMoveFromTitleBarOnly = false;
}

void Graphics::apply_ui_style(bool dark_mode) {
  UNUSED(dark_mode);

  //
  // My janky Rolling Thunder ImGui theme...
  //

  ImGui::StyleColorsDark();

  ImGuiStyle& style = ImGui::GetStyle();

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
  style.Colors[ImGuiCol_TabSelected] = red_high;
  style.Colors[ImGuiCol_TabDimmed] = grey_low;
  style.Colors[ImGuiCol_TabDimmedSelected] = red_low;

  style.Colors[ImGuiCol_DockingPreview] = red_low;

  float dpiscale = PlatformGraphics::get().dpi_scale();
  update_ui_scale(dpiscale);
}

void Graphics::update_ui_scale(float scale) {
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
  style.UserSizes[UISIZE_PATH_EDITOR_PAGE_START_WIDTH] = 800.f;
  style.UserSizes[UISIZE_PATH_EDITOR_PAGE_START_HEIGHT] = 600.f;
  style.UserSizes[UISIZE_PATH_MANAGER_PAGE_START_WIDTH] = 300.f;
  style.UserSizes[UISIZE_PATH_MANAGER_PAGE_START_HEIGHT] = 600.f;
  style.UserSizes[UISIZE_PROPERTIES_PAGE_START_WIDTH] = 300.f;
  style.UserSizes[UISIZE_PROPERTIES_PAGE_START_HEIGHT] = 600.f;
  style.UserSizes[UISIZE_SETTINGS_PAGE_START_WIDTH] = 300.f;
  style.UserSizes[UISIZE_SETTINGS_PAGE_START_HEIGHT] = 120.f;
  // Popup sizes
  style.UserSizes[UISIZE_NEW_FIELD_POPUP_START_WIDTH] = 370.f;
  style.UserSizes[UISIZE_NEW_FIELD_POPUP_START_HEIGHT] = 250.f;
  style.UserSizes[UISIZE_NEW_PROJECT_POPUP_START_WIDTH] = 400.f;
  style.UserSizes[UISIZE_NEW_PROJECT_POPUP_START_HEIGHT] = 250.f;
  style.UserSizes[UISIZE_UNSAVED_POPUP_START_WIDTH] = 125.f;
  style.UserSizes[UISIZE_UNSAVED_POPUP_START_HEIGHT] = 115.f;
  // Other sizes
  style.UserSizes[UISIZE_FIELD_NORMAL_LEFT_COLUMN_WIDTH] = 135.f;
  style.UserSizes[UISIZE_INDENT_SMALL] = 10.f,
  style.UserSizes[UISIZE_INDENT_MEDIUM] = 20.f,
  style.UserSizes[UISIZE_INDENT_LARGE] = 30.f;

  style.ScaleAllSizes(scale);
  std::memcpy(style.Colors, styleold.Colors,
              sizeof(style.Colors));  // Restore colors

  load_fonts(scale);

  PlatformGraphics::get().window_set_size(DEFAULT_WINDOW_WIDTH * scale,
                                          DEFAULT_WINDOW_HEIGHT * scale);
  PlatformGraphics::get().window_move_to_center();
}

#include <Ubuntu_Bold_ttf.h>
#include <Ubuntu_Regular_ttf.h>
// #include <Roboto_Regular_ttf.h>
// #include <Roboto_Bold_ttf.h>
#include <FontAwesome_Regular_ttf.h>
#include <FontAwesome_Solid_ttf.h>

#include <IconsFontAwesome5.h>

void Graphics::load_fonts(float scale) {
  FontLibrary& font_lib = FontLibrary::get();

  ImGuiIO* io = &ImGui::GetIO();

  io->Fonts->Clear();  // Clear previous fonts.

  // Tell ImGui not to free fonts from memory.
  ImFontConfig font_cfg;
  font_cfg.FontDataOwnedByAtlas = false;

  static const ImWchar* glyph_ranges = io->Fonts->GetGlyphRangesDefault();

  // Regular font.
  font_lib.regular_font = io->Fonts->AddFontFromMemoryTTF(
      reinterpret_cast<void*>(Ubuntu_Regular_ttf), Ubuntu_Regular_ttf_size,
      15.0f * scale, &font_cfg, glyph_ranges);
  assert(font_lib.regular_font != nullptr);

  // FontAwesome icons...
  font_cfg.MergeMode = true;
  font_cfg.PixelSnapH = true;

  ImFont* font;

  static const ImWchar icon_ranges[] = {ICON_MIN_FA, ICON_MAX_16_FA, 0};
  font = io->Fonts->AddFontFromMemoryTTF(FontAwesome_Regular_ttf,
                                         FontAwesome_Regular_ttf_size,
                                         15.0f * scale, &font_cfg, icon_ranges);
  assert(font != nullptr);
  font = io->Fonts->AddFontFromMemoryTTF(FontAwesome_Solid_ttf,
                                         FontAwesome_Solid_ttf_size,
                                         15.0f * scale, &font_cfg, icon_ranges);
  assert(font != nullptr);

  // Additional fonts.
  font_cfg.MergeMode = false;
  font_cfg.PixelSnapH = false;

  font_lib.big_font = io->Fonts->AddFontFromMemoryTTF(
      reinterpret_cast<void*>(Ubuntu_Bold_ttf), Ubuntu_Bold_ttf_size,
      30.0f * scale, &font_cfg, glyph_ranges);
  assert(font_lib.big_font != nullptr);

  font_lib.bold_font = io->Fonts->AddFontFromMemoryTTF(
      reinterpret_cast<void*>(Ubuntu_Bold_ttf), Ubuntu_Bold_ttf_size,
      15.0f * scale, &font_cfg, glyph_ranges);
  assert(font_lib.bold_font != nullptr);
}

Graphics& PlatformGraphics::get() {
#if THUNDER_AUTO_DIRECTX11
  return GraphicsDirectX11::get();
#else  // THUNDER_AUTO_OPENGL
  return GraphicsOpenGL::get();
#endif
}
