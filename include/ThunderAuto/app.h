#pragma once

#include <ThunderAuto/document_edit_manager.h>
#include <ThunderAuto/document_manager.h>
#include <ThunderAuto/font_library.h>
#include <ThunderAuto/platform/platform_manager.h>
#include <ThunderAuto/thunder_auto.h>

#include <ThunderAuto/popups/new_field_popup.h>
#include <ThunderAuto/popups/new_project_popup.h>
#include <ThunderAuto/popups/unsaved_popup.h>
#include <ThunderAuto/popups/welcome_popup.h>

#include <ThunderAuto/pages/actions_page.h>
#include <ThunderAuto/pages/path_editor_page.h>
#include <ThunderAuto/pages/path_manager_page.h>
#include <ThunderAuto/pages/properties_page.h>

#include <ThunderAuto/curve.h>

struct GLFWwindow;

class App {
  GLFWwindow* m_window;
  FontLibrary& m_font_lib;

  bool m_running = true;

  PlatformManager m_platform_manager {m_window};

  // The state of the application (used to manage popups and stuff).
  enum class EventState {
    NONE = 0,
    WELCOME,
    NEW_PROJECT,
    NEW_FIELD,
    OPEN_PROJECT,
    CLOSE_PROJECT,
    CLOSE_EVERYTHING,
    UNSAVED,
  };

  EventState m_event_state = EventState::WELCOME;
  EventState m_next_event_state = EventState::NONE;

  DocumentManager m_document_manager;
  DocumentEditManager m_document_edit_manager {m_document_manager.history()};

  NewFieldPopup m_new_field_popup {m_platform_manager};
  NewProjectPopup m_new_project_popup {m_platform_manager};
  UnsavedPopup m_unsaved_popup;
  WelcomePopup m_welcome_popup {m_font_lib};

  OutputCurve m_cached_curve;

  PathEditorPage m_path_editor_page {m_document_edit_manager, m_cached_curve};
  PropertiesPage m_properties_page {m_document_edit_manager, m_cached_curve,
                                    m_path_editor_page};
  PathManagerPage m_path_manager_page {m_document_edit_manager, m_cached_curve};
  ActionsPage m_actions_page {m_document_edit_manager};

public:
  inline App(GLFWwindow* window, FontLibrary& font_lib)
    : m_window(window),
      m_font_lib(font_lib) {}

  ~App() = default;

  constexpr bool is_running() const { return m_running; }

  void setup_dockspace(ImGuiID dockspace_id);

  void process_input();
  void present();

  void close();

private:
  void present_menu_bar();
  void present_file_menu();
  void present_edit_menu();
  void present_tools_menu();

  bool try_change_state(EventState event_state);

  void welcome();
  void new_project();
  void new_field();
  void open_project();

  void unsaved();

  void undo();
  void redo();
};
