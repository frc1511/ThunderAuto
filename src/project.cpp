#include <project.h>
#include <pages/path_editor.h>
#include <pages/path_manager.h>

ProjectManager::ProjectManager() { }

ProjectManager::~ProjectManager() { }

void ProjectManager::new_project(ProjectSettings _settings) {
  project.settings = _settings;
  project.points = PathEditorPage::CurvePointTable({
    { 0.9f, 0.5f, -M_PI_2, 0.3f, 0.3f, 0.0f },
    { 0.5f, 0.3f, +M_PI_2, 0.3f, 0.3f, 0.0f },
    { 0.1f, 0.4f, +M_PI_2, 0.3f, 0.3f, 0.0f },
  });

  working_project = true;
  unsaved = false;

  PathEditorPage::get()->set_project(&project);

  save_project();
}

void ProjectManager::open_project(std::string path) {
  project.settings.path = path;

  std::string file_str;
  {
    std::ifstream file(path);
    if (!file) return;
    file_str = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
  }

  std::string::const_iterator file_iter = file_str.cbegin();

  auto count = [&]() -> std::size_t {
    std::size_t n = 0;
    while (file_iter != file_str.end() && *file_iter != '\n' && *file_iter != ',' && *file_iter != '}') {
      n++;
      file_iter++;
    }
    return n;
  };

  auto get_str = [&]() -> std::string {
    std::string::const_iterator start = file_iter;
    std::size_t n = count();
    return std::string(start, start + n);
  };

  project.settings.field.img_path = get_str(); ++file_iter;
  project.settings.field.min.x = std::stof(get_str()); ++file_iter;
  project.settings.field.min.y = std::stof(get_str()); ++file_iter;
  project.settings.field.max.x = std::stof(get_str()); ++file_iter;
  project.settings.field.max.y = std::stof(get_str()); ++file_iter;
  project.settings.drive_controller = static_cast<DriveController>(std::stoi(get_str())); ++file_iter;
  project.settings.max_acceleration = std::stof(get_str()); ++file_iter;
  project.settings.max_deceleration = std::stof(get_str()); ++file_iter;
  project.settings.max_velocity = std::stof(get_str()); ++file_iter;

  project.points.clear();
  while (file_iter != file_str.cend() && *file_iter == '{') {
      ++file_iter;
      PathEditorPage::CurvePoint point;
      point.px = std::stof(get_str()); ++file_iter;
      point.py = std::stof(get_str()); ++file_iter;
      point.heading = std::stof(get_str()); ++file_iter;
      point.w0 = std::stof(get_str()); ++file_iter;
      point.w1 = std::stof(get_str()); ++file_iter;
      point.rotation = std::stof(get_str()); ++file_iter;

      project.points.push_back(point);
  }

  PathEditorPage::get()->set_project(&project);
  working_project = true;
}

void ProjectManager::save_project() {
  const ProjectSettings& settings = project.settings;
  const Field& field = settings.field;

  std::ofstream file(project.settings.path);

  file << field.img_path << ',' << field.min.x << ',' << field.min.y << ',' << field.max.x << ',' << field.max.y << '\n';
  file << static_cast<int>(settings.drive_controller) << ',';
  file << settings.max_acceleration << ',';
  file << settings.max_deceleration << ',';
  file << settings.max_velocity << '\n';
  
  for (const PathEditorPage::CurvePoint& pt : project.points) {
    file << '{' << pt.px << "," << pt.py << "," << pt.heading << "," << pt.w0 << "," << pt.w1 << "," << pt.rotation << '}';
  }
  file << '\n';
}

void ProjectManager::save_project_as(std::string path) {
  project.settings.path = path;
  save_project();
}

void ProjectManager::close_project() {
  working_project = false;
}

ProjectManager ProjectManager::instance {};
