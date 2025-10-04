#include <ThunderAuto/DocumentManager.hpp>

#include <ThunderAuto/TrajectoryHelper.hpp>
#include <ThunderAuto/Logger.hpp>
#include <ThunderAuto/Error.hpp>

void DocumentManager::newProject(ThunderAutoProjectSettings settings) noexcept {
  if (m_open)
    close();

  ThunderAutoLogger::Info("New project: {}", settings.name);

  m_settings = settings;

  ThunderAutoProjectState startState;
  startState.trajectories["NewTrajectory"] = kDefaultNewTrajectory;
  startState.editorState.view = ThunderAutoEditorState::View::TRAJECTORY;
  startState.editorState.trajectoryEditorState.currentTrajectoryName = "NewTrajectory";

  m_history.reset(startState, true);
  m_open = true;
}

ThunderAutoProjectVersion DocumentManager::openProject(const std::filesystem::path& path) {
  if (m_open)
    close();

  ThunderAutoLogger::Info("Open project: {}", path.string());

  ThunderAutoProjectVersion version;
  std::unique_ptr<ThunderAutoProject> project = LoadThunderAutoProject(path, &version);
  ThunderAutoAssert(project, "LoadThunderAutoProject returned nullptr but did not throw an error");

  m_settings = project->settings();
  m_history.reset(project->state());
  m_open = true;

  return version;
}

void DocumentManager::save() {
  if (!m_open)
    return;

  ThunderAutoLogger::Info("Save project: {}", m_settings.projectPath.string());

  SaveThunderAutoProject(m_settings, m_history.currentState());

  m_history.markSaved();
}

void DocumentManager::close() noexcept {
  ThunderAutoLogger::Info("Close project");

  m_open = false;
  m_settings = {};
}
