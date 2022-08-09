#include <project.h>
#include <pages/path_editor.h>
#include <pages/path_manager.h>
#include <pages/properties.h>
#include <pages/settings.h>

ProjectManager::ProjectManager() { }

ProjectManager::~ProjectManager() { }

void ProjectManager::new_project(ProjectSettings _settings) {
  project.settings = _settings;
  project.paths.clear();
  project.paths.emplace_back("the_path", PathEditorPage::CurvePointTable({
    { 8.124f, 1.78f, 4.73853f, 4.73853f, 1.44372f, 1.70807f, 4.73853, false, true, false },
    { 4.0f,   1.5f,  2.0944f,  2.0944f,  2.0f,     2.0f,     2.0944,  false, false, true },
  }));

  working_project = true;
  unsaved = false;

  PathManagerPage::get()->set_project(&project);
  PathEditorPage::get()->set_project(&project);
  PropertiesPage::get()->set_project(&project);
  SettingsPage::get()->set_project(&project);

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

  auto count = [&]() -> std::ptrdiff_t {
    std::ptrdiff_t n = 0;

    bool quoted = *file_iter == '"';
    if (quoted) {
      ++n;
      ++file_iter;
    }

    auto cond = [&]() {
      if (quoted) return *file_iter != '"';
      return *file_iter != '\n' && *file_iter != ',' && *file_iter != '}';
    };

    while (file_iter != file_str.end() && cond()) {
      n++;
      file_iter++;
    }
    return n;
  };

  auto get_str = [&]() -> std::string {
    std::string::const_iterator start = file_iter;
    std::ptrdiff_t n = count();
    return std::string(start, start + n);
  };

  project.settings.field.img_type = static_cast<Field::ImageType>(std::stoi(get_str())); ++file_iter;
  std::string img_str = get_str(); file_iter += 2;
  img_str.erase(img_str.cbegin());
  if (project.settings.field.img_type == Field::ImageType::CUSTOM) {
    project.settings.field.img = img_str;
  }
  else {
    project.settings.field.img = static_cast<Field::BuiltinImage>(std::stoi(img_str));
  }
  project.settings.field.min.x = std::stof(get_str()); ++file_iter;
  project.settings.field.min.y = std::stof(get_str()); ++file_iter;
  project.settings.field.max.x = std::stof(get_str()); ++file_iter;
  project.settings.field.max.y = std::stof(get_str()); ++file_iter;
  project.settings.drive_ctrl = static_cast<DriveController>(std::stoi(get_str())); ++file_iter;
  project.settings.max_accel = std::stof(get_str()); ++file_iter;
  project.settings.max_vel = std::stof(get_str()); ++file_iter;
  project.settings.robot_length = std::stof(get_str()); ++file_iter;
  project.settings.robot_width = std::stof(get_str()); ++file_iter;

  project.paths.clear();
  do {
    std::string name(get_str()); ++file_iter;
    PathEditorPage::CurvePointTable points;
    while (file_iter != file_str.cend() && *file_iter == '{') {
        ++file_iter;
        PathEditorPage::CurvePoint point;
        point.px = std::stof(get_str()); ++file_iter;
        point.py = std::stof(get_str()); ++file_iter;
        point.h0 = std::stof(get_str()); ++file_iter;
        point.h1 = std::stof(get_str()); ++file_iter;
        point.w0 = std::stof(get_str()); ++file_iter;
        point.w1 = std::stof(get_str()); ++file_iter;
        point.rotation = std::stof(get_str()); ++file_iter;
        point.stop = static_cast<bool>(std::stoi(get_str())); ++file_iter;
        point.begin = static_cast<bool>(std::stoi(get_str())); ++file_iter;
        point.end = static_cast<bool>(std::stoi(get_str())); ++file_iter;

        points.push_back(point);
    }
    project.paths.emplace_back(name, points);
    if (*file_iter == '\n') ++file_iter;
    else break;

  } while (true);

  PathManagerPage::get()->set_project(&project);
  PathEditorPage::get()->set_project(&project);
  PropertiesPage::get()->set_project(&project);
  SettingsPage::get()->set_project(&project);
  working_project = true;
}

void ProjectManager::save_project() {
  const ProjectSettings& settings = project.settings;
  const Field& field = settings.field;

std::cout << project.settings.path << '\n';
  std::ofstream file(project.settings.path);
  file.clear();

  file << static_cast<std::size_t>(settings.field.img_type) << ',';
  if (field.img_type == Field::ImageType::CUSTOM) {
    file << '"' << std::get<std::string>(field.img).c_str() << '"' << ',';
  }
  else {
    file << '"' << static_cast<std::size_t>(std::get<Field::BuiltinImage>(field.img)) << '"' << ',';
  }
  file << field.min.x << ',' << field.min.y << ',' << field.max.x << ',' << field.max.y << '\n';
  file << static_cast<std::size_t>(settings.drive_ctrl) << ',';
  file << settings.max_accel << ',';
  file << settings.max_vel << '\n';
  file << settings.robot_length << ',';
  file << settings.robot_width << '\n';

  for (decltype(project.paths)::const_iterator it(project.paths.cbegin()); it != project.paths.cend(); ++it) {
    auto [name, points] = *it;

    file << name << ',';
    for (const PathEditorPage::CurvePoint& pt : points) {
      file << '{'
          << pt.px << ','
          << pt.py << ','
          << pt.h0 << ','
          << pt.h1 << ','
          << pt.w0 << ','
          << pt.w1 << ','
          << pt.rotation << ','
          << static_cast<std::size_t>(pt.stop) << ','
          << static_cast<std::size_t>(pt.begin) << ','
          << static_cast<std::size_t>(pt.end) << '}';
    }
    if (it != project.paths.cend() - 1) {
      file << '\n';
    }
  }
}

void ProjectManager::save_project_as(std::string path) {
  project.settings.path = path;
  save_project();
}

void ProjectManager::close_project() {
  working_project = false;
}

ProjectManager ProjectManager::instance {};
