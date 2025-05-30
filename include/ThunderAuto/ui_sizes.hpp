#pragma once

// Sizes for UI elements are stored in ImGuiStyle::UserSizes at these indices.
enum UISize {
  UISIZE_TITLEBAR_BUTTON_WIDTH,
  UISIZE_TITLEBAR_BUTTON_ICON_SIZE,
  UISIZE_TITLEBAR_BUTTON_MAXIMIZE_ICON_BOX_ROUNDING,
  UISIZE_TITLEBAR_BUTTON_MAXIMIZE_ICON_BOX_OFFSET,
  UISIZE_TITLEBAR_DRAG_AREA_MIN_WIDTH,
  UISIZE_TITLEBAR_FRAME_PADDING_Y,
  UISIZE_TITLEBAR_ITEM_SPACING_Y,

  UISIZE_WINDOW_MIN_WIDTH,
  UISIZE_WINDOW_MIN_HEIGHT,

  UISIZE_ACTIONS_PAGE_START_WIDTH,
  UISIZE_ACTIONS_PAGE_START_HEIGHT,
  UISIZE_PATH_EDITOR_PAGE_START_WIDTH,
  UISIZE_PATH_EDITOR_PAGE_START_HEIGHT,
  UISIZE_PATH_MANAGER_PAGE_START_WIDTH,
  UISIZE_PATH_MANAGER_PAGE_START_HEIGHT,
  UISIZE_PROPERTIES_PAGE_START_WIDTH,
  UISIZE_PROPERTIES_PAGE_START_HEIGHT,
  UISIZE_SETTINGS_PAGE_START_WIDTH,
  UISIZE_SETTINGS_PAGE_START_HEIGHT,

  UISIZE_NEW_FIELD_POPUP_START_WIDTH,
  UISIZE_NEW_FIELD_POPUP_START_HEIGHT,
  UISIZE_NEW_PROJECT_POPUP_START_WIDTH,
  UISIZE_NEW_PROJECT_POPUP_START_HEIGHT,
  UISIZE_UNSAVED_POPUP_START_WIDTH,
  UISIZE_UNSAVED_POPUP_START_HEIGHT,

  UISIZE_FIELD_NORMAL_LEFT_COLUMN_WIDTH,
  UISIZE_INDENT_SMALL,
  UISIZE_INDENT_MEDIUM,
  UISIZE_INDENT_LARGE,
};

#define GET_UISIZE(name) ImGui::GetStyle().UserSizes[UISIZE_##name]
