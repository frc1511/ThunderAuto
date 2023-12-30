#pragma once

#include <ThunderAuto/curve.h>
#include <ThunderAuto/project_settings.h>
#include <ThunderAuto/thunder_auto.h>

class ProjectState {
  std::vector<std::string> m_actions;

  std::vector<std::pair<std::string, Curve>> m_paths; // not a map on purpose

  std::size_t m_current_path_index = 0;
  int m_selected_point_index = -1;

public:
  ProjectState(std::vector<std::pair<std::string, Curve>> paths = {});

  inline const std::vector<std::string>& actions() const { return m_actions; }
  inline std::vector<std::string>& actions() { return m_actions; }

  inline const std::vector<std::pair<std::string, Curve>>& paths() const {
    return m_paths;
  }
  inline std::vector<std::pair<std::string, Curve>>& paths() { return m_paths; }

  inline std::size_t& current_path_index() { return m_current_path_index; }
  inline int& selected_point_index() { return m_selected_point_index; }

  inline Curve& current_path() {
    return m_paths.at(m_current_path_index).second;
  }
  inline const Curve& current_path() const {
    return m_paths.at(m_current_path_index).second;
  }

  inline std::string& current_path_name() {
    return m_paths.at(m_current_path_index).first;
  }
  inline const std::string& current_path_name() const {
    return m_paths.at(m_current_path_index).first;
  }

  inline CurvePoint* selected_point() {
    if (m_selected_point_index == -1) {
      return nullptr;
    }
    return &current_path().points().at(m_selected_point_index);
  }
};

void to_json(nlohmann::json& json, const ProjectState& project);
void from_json(const nlohmann::json& json, ProjectState& project);
