#include <ThunderAuto/project_state.h>

ProjectState::ProjectState(std::vector<std::pair<std::string, Curve>> paths)
  : m_paths(paths) {

  if (m_paths.empty()) {
    m_paths.emplace_back("match_winning_auto", default_new_curve);
  }
}

void to_json(nlohmann::json& json, const ProjectState& project) {
  json = nlohmann::json {
      {"paths", project.paths()},
      {"actions", project.actions()},
  };
}

void from_json(const nlohmann::json& json, ProjectState& project) {
  project.paths() =
      json.at("paths").get<std::vector<std::pair<std::string, Curve>>>();

  project.actions() = json.at("actions").get<std::vector<std::string>>();

  assert(!project.paths().empty());
}
