#pragma once

#include <ThunderAuto/thunder_auto.hpp>
#include <ThunderAuto/curve.hpp>
#include <ThunderAuto/project_settings.hpp>

class ProjectState {
  std::vector<std::string> m_actions;

  std::vector<std::pair<std::string, Curve>> m_paths; // not a map on purpose

  std::size_t m_current_path_index = 0;
  int m_selected_point_index = -1;

  std::vector<std::string> m_waypoint_links;

public:
  explicit ProjectState(std::vector<std::pair<std::string, Curve>> paths = {});

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

  inline std::vector<std::string>& waypoint_links() { return m_waypoint_links; }
  inline const std::vector<std::string>& waypoint_links() const {
    return m_waypoint_links;
  }

  // Update linked waypoints with the same link as the selected point.
  void update_linked_waypoints_from_selected();
  void update_point_from_linked_waypoints(int point_index);
  inline void update_selected_from_linked_waypoints() {
    update_point_from_linked_waypoints(m_selected_point_index);
  }

  bool export_path_to_csv(std::size_t path_index,
                          const ProjectSettings& settings) const;
  inline bool export_current_path_to_csv(const ProjectSettings& settings) const {
    return export_path_to_csv(m_current_path_index, settings);
  }
  bool export_all_paths_to_csv(const ProjectSettings& settings) const;
};

void to_json(wpi::json& json, const ProjectState& project);
void from_json(const wpi::json& json, ProjectState& project);

