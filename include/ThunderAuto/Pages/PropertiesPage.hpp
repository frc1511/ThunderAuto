#pragma once

#include <ThunderAuto/DocumentEditManager.hpp>
#include <ThunderAuto/Pages/Page.hpp>
#include <ThunderAuto/Pages/EditorPage.hpp>
#include <ThunderLibCore/Auto/ThunderAutoProject.hpp>

using namespace thunder::core;

class App;

class PropertiesPage : public Page {
  DocumentEditManager& m_history;

  EditorPage& m_editorPage;

  const ThunderAutoProjectSettings* m_settings = nullptr;

 public:
  PropertiesPage(DocumentEditManager& history, EditorPage& editorPage)
      : m_history(history), m_editorPage(editorPage) {}

  void setup(const ThunderAutoProjectSettings& settings) { m_settings = &settings; }

  const char* name() const noexcept override { return "Properties"; }

  void present(bool* running) override;

  enum class Event {
    NONE = 0,
    TRAJECTORY_POINT_LINK,
  };

  Event lastPresentEvent() const noexcept { return m_event; }

 private:
  void presentTrajectoryProperties(ThunderAutoProjectState& state);

  void presentTrajectoryItemList(ThunderAutoProjectState& state);
  void presentTrajectorySelectedItemProperties(ThunderAutoProjectState& state);

  void presentTrajectorySelectedPointProperties(ThunderAutoProjectState& state);

  bool presentPointPositionProperties(ThunderAutoTrajectorySkeletonWaypoint& point);
  bool presentPointHeadingProperties(ThunderAutoTrajectorySkeletonWaypoint& point, bool isEndPoint);
  bool presentPointWeightProperties(ThunderAutoTrajectorySkeletonWaypoint& point,
                                    bool showIncomingWeight,
                                    bool showOutgoingWeight);
  bool presentPointVelocityOverrideProperty(ThunderAutoTrajectorySkeletonWaypoint& point,
                                            bool isFirstPoint,
                                            bool isLastPoint,
                                            units::meters_per_second_t maxLinearVelocitySetting);

  // Present some rotation property with the given name and get/set rotation functions.
  bool presentRotationProperty(const char* name,
                               std::function<CanonicalAngle()> getRotation,
                               std::function<void(CanonicalAngle)> setRotation);

  bool presentPointStopRotationProperty(ThunderAutoTrajectorySkeletonWaypoint& point);
  bool presentTrajectoryStartRotationProperty(ThunderAutoTrajectorySkeleton& skeleton);
  bool presentTrajectoryEndRotationProperty(ThunderAutoTrajectorySkeleton& skeleton);

  // Present some action list property with the given name and add/remove action functions.
  bool presentActionsProperty(const char* name,
                              const char* tooltip,
                              const std::unordered_set<std::string>& actions,
                              std::function<bool(const std::string&)> removeAction,
                              std::function<bool(const std::string&)> addAction,
                              std::span<const std::string> availableActions);

  bool presentPointStopActionsProperty(ThunderAutoTrajectorySkeletonWaypoint& point,
                                       const ThunderAutoProjectState& state);
  bool presentTrajectoryStartActionsProperty(ThunderAutoTrajectorySkeleton& skeleton,
                                             const ThunderAutoProjectState& state);
  bool presentTrajectoryEndActionsProperty(ThunderAutoTrajectorySkeleton& skeleton,
                                           const ThunderAutoProjectState& state);

  void presentTrajectorySelectedRotationProperties(ThunderAutoProjectState& state);
  void presentTrajectorySelectedActionProperties(ThunderAutoProjectState& state);

  void presentTrajectorySpeedConstraintProperties(ThunderAutoProjectState& state);

  // void presentAutoModeProperties(ThunderAutoProjectState& state);

  bool presentSlider(const char* id,
                     double& value,
                     float speed = 1.f,
                     const char* format = "%.2f",
                     bool* isFinished = nullptr);

  template <typename T>
  struct TrajectoryItemSelection {
    T item;
    bool editorLocked;
    ThunderAutoTrajectoryEditorState::TrajectorySelection trajectorySelection;
    size_t selectionIndex;
  };

  static std::map<ThunderAutoTrajectoryPosition, TrajectoryItemSelection<CanonicalAngle>>
  GetAllRotationSelections(const ThunderAutoTrajectorySkeleton& skeleton);

  static std::multimap<ThunderAutoTrajectoryPosition, TrajectoryItemSelection<std::string>>
  GetAllActionSelections(const ThunderAutoTrajectorySkeleton& skeleton);

 private:
  Event m_event = Event::NONE;
};
