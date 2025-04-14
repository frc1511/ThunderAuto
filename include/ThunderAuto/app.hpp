#pragma once

#include <ThunderAuto/document_edit_manager.hpp>
#include <ThunderAuto/document_manager.hpp>
#include <ThunderAuto/font_library.hpp>
#include <ThunderAuto/platform/platform_manager.hpp>
#include <ThunderAuto/thunder_auto.hpp>

#include <ThunderAuto/popups/new_field_popup.hpp>
#include <ThunderAuto/popups/new_project_popup.hpp>
#include <ThunderAuto/popups/unsaved_popup.hpp>
#include <ThunderAuto/popups/welcome_popup.hpp>

#include <ThunderAuto/pages/actions_page.hpp>
#include <ThunderAuto/pages/path_editor_page.hpp>
#include <ThunderAuto/pages/path_manager_page.hpp>
#include <ThunderAuto/pages/properties_page.hpp>
#include <ThunderAuto/pages/settings_page.hpp>

#include <ThunderAuto/curve.hpp>

struct GLFWwindow;

class App {
  bool m_running = true;

  PlatformManager m_platform_manager;

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
    OPEN_PROJECT_ERROR,
  };

  bool m_export_popup = false;
  int m_exported_index = -1;
  bool m_export_success = true;

  bool m_project_version_popup = false;

  bool m_project_open_error_popup = false;
  OpenProjectStatus m_project_open_error = OpenProjectStatus::OK;

  EventState m_event_state = EventState::WELCOME;
  EventState m_next_event_state = EventState::NONE;

  DocumentManager m_document_manager;
  DocumentEditManager m_document_edit_manager {m_document_manager.history()};

  std::list<std::string> m_recent_projects;

  NewFieldPopup m_new_field_popup {m_platform_manager};
  NewProjectPopup m_new_project_popup {m_platform_manager};
  UnsavedPopup m_unsaved_popup;
  WelcomePopup m_welcome_popup {m_recent_projects};

  OutputCurve m_cached_curve;

  PathEditorPage m_path_editor_page {m_document_edit_manager, m_cached_curve};
  PropertiesPage m_properties_page {m_document_edit_manager, m_cached_curve,
                                    m_path_editor_page, this};
  PathManagerPage m_path_manager_page {m_document_edit_manager, m_cached_curve};
  ActionsPage m_actions_page {m_document_edit_manager};
  SettingsPage m_settings_page {m_document_manager};

  bool m_show_path_editor = true;
  bool m_show_path_manager = true;
  bool m_show_properties = true;
  bool m_show_actions = true;
  bool m_show_settings = false;

  bool m_was_unsaved = false;
  std::string m_titlebar_filename;

  // Graphics stuff

  int m_menu_bar_width = 0;

private:
  App() = default;

  static inline bool s_init = false;

public:
  static App& get() {
    static App instance;
    s_init = true;
    return instance;
  }

  // Get the singleton instance if the application has been initialized, nullptr otherwise.
  static App* get_maybe_uninitialized() {
    return s_init ? &get() : nullptr;
  }

  App(App const&) = delete;
  void operator=(App const&) = delete;

public:
  constexpr bool is_running() const { return m_running; }

  void setup_dockspace(ImGuiID dockspace_id);

  void focus_was_changed(bool focused);

  void process_input();
  void present();

  void close();

  void data_clear();
  bool data_should_open(const char* name);
  void data_read_line(const char* line);
  void data_apply();
  void data_write(const char* type_name, ImGuiTextBuffer* buf);

  void open_from_path(std::string path);

  void open_export_popup(int export_index = -1, bool export_success = true) {
    m_export_popup = true;
    m_exported_index = export_index;
    m_export_success = export_success;
  }

private:
  void present_menu_bar();
  void present_file_menu();
  void present_edit_menu();
  void present_view_menu();
  void present_path_menu();
  void present_tools_menu();

  void present_popups();
  void present_export_popup();
  void present_project_open_error_popup();
  void present_project_version_popup();

  void present_pages();

  bool try_change_state(EventState event_state);

  void welcome();
  void new_project();
  void new_field();
  void open_project();

  void save_as();

  void unsaved();

  void undo();
  void redo();

  void update_titlebar_title();

public:
  int menu_bar_width() const { return m_menu_bar_width; }
};
