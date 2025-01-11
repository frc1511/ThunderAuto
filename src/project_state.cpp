#include <ThunderAuto/project_state.h>

ProjectState::ProjectState(std::vector<std::pair<std::string, Curve>> paths)
  : m_paths(paths) {

  if (m_paths.empty()) {
    m_paths.emplace_back("match_winning_auto", default_new_curve);
  }

  m_current_path_index = 0;
  m_selected_point_index = -1;
}

void ProjectState::update_linked_waypoints_from_selected() {
  if (m_selected_point_index == -1) return;

  const CurvePoint& selected_point =
      current_path().points().at(m_selected_point_index);
  if (selected_point.link_index() == -1) return;

  for (std::size_t i = 0; i < m_paths.size(); ++i) {
    Curve& curve = m_paths.at(i).second;

    for (std::size_t j = 0; j < curve.points().size(); ++j) {
      CurvePoint& point = curve.points().at(j);

      if (point.link_index() == selected_point.link_index()) {
        if (i == m_current_path_index &&
            j == std::size_t(m_selected_point_index)) {
          continue;
        }

        point.set_position(selected_point.position());
      }
    }
  }
}

void ProjectState::update_point_from_linked_waypoints(int point_index) {
  if (point_index == -1) return;

  CurvePoint& selected_point = current_path().points().at(point_index);
  if (selected_point.link_index() == -1) return;

  for (std::size_t i = 0; i < m_paths.size(); ++i) {
    const Curve& curve = m_paths.at(i).second;

    for (std::size_t j = 0; j < curve.points().size(); ++j) {
      const CurvePoint& point = curve.points().at(j);

      if (point.link_index() == selected_point.link_index()) {
        if (i == m_current_path_index &&
            j == std::size_t(m_selected_point_index)) {
          continue;
        }

        selected_point.set_position(point.position());
        break;
      }
    }
  }
}

void ProjectState::export_path_to_csv(std::size_t path_index,
                          const ProjectSettings& settings) const {

  const auto& [path_name, curve] = m_paths.at(path_index);

  // Build the high resolution output curve.
  OutputCurve output;
  curve.output(output, high_res_output_curve_settings);

  // Write to file.
  std::filesystem::path path =
      settings.path.parent_path() / (path_name + ".csv");

  std::ofstream file(path);
  if (!file.is_open()) {
    puts("Failed to open CSV file");
    return;
  }

  file << "time,x_pos,y_pos,velocity,rotation,action\n";

  for (const OutputCurvePoint& point : output.points) {
    file << point.time << ",";
    file << point.position.x << ",";
    file << point.position.y << ",";
    file << point.velocity << ",";
    file << point.rotation << ",";
    file << point.actions << "\n";
  }

  printf("Exported to %s\n", path.string().c_str());
}

void ProjectState::export_all_paths_to_csv(const ProjectSettings& settings) const {
  for (std::size_t i = 0; i < m_paths.size(); ++i) {
    export_path_to_csv(i, settings);
  }
}

void to_json(nlohmann::json& json, const ProjectState& project) {
  json = nlohmann::json {
      {"paths", project.paths()},
      {"actions", project.actions()},
      {"waypoint_links", project.waypoint_links()},
  };
}

void from_json(const nlohmann::json& json, ProjectState& project) {
  project.paths() =
      json.at("paths").get<std::vector<std::pair<std::string, Curve>>>();

  project.actions() = json.at("actions").get<std::vector<std::string>>();

  if (json.contains("waypoint_links")) {
    project.waypoint_links() =
        json.at("waypoint_links").get<std::vector<std::string>>();
  }

  assert(!project.paths().empty());

  project.current_path_index() = 0;
  project.selected_point_index() = -1;
}

