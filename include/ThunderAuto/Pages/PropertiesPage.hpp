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
    AUTO_MODE_ADD_STEP,
    TRAJECTORY_START_BEHAVIOR_LINK,
    TRAJECTORY_END_BEHAVIOR_LINK,
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
  bool presentPointLinkProperty(ThunderAutoTrajectorySkeletonWaypoint& point);

  // Present some rotation property with the given name and get/set rotation functions.
  bool presentRotationProperty(const char* name,
                               std::function<CanonicalAngle()> getRotation,
                               std::function<void(CanonicalAngle)> setRotation);

  bool presentTrajectoryStartBehaviorLinkProperty(ThunderAutoTrajectorySkeleton& skeleton);
  bool presentTrajectoryEndBehaviorLinkProperty(ThunderAutoTrajectorySkeleton& skeleton);

  bool presentPointStopRotationProperty(ThunderAutoTrajectorySkeletonWaypoint& point);
  bool presentTrajectoryStartRotationProperty(ThunderAutoTrajectorySkeleton& skeleton);
  bool presentTrajectoryEndRotationProperty(ThunderAutoTrajectorySkeleton& skeleton);

  bool presentPointStopActionProperty(ThunderAutoTrajectorySkeletonWaypoint& point,
                                      const ThunderAutoProjectState& state);
  bool presentTrajectoryStartActionProperty(ThunderAutoTrajectorySkeleton& skeleton,
                                            const ThunderAutoProjectState& state);
  bool presentTrajectoryEndActionProperty(ThunderAutoTrajectorySkeleton& skeleton,
                                          const ThunderAutoProjectState& state);

  void presentTrajectorySelectedRotationProperties(ThunderAutoProjectState& state);
  void presentTrajectorySelectedActionProperties(ThunderAutoProjectState& state);

  void presentTrajectoryOtherProperties(ThunderAutoProjectState& state);
  void presentTrajectorySpeedConstraintProperties(ThunderAutoProjectState& state);

  void presentAutoModeProperties(ThunderAutoProjectState& state);

  void presentAutoModeStepList(ThunderAutoProjectState& state);

  // Draw step tree. Returns true if tree was modified and the rest of the tree should not be drawn (to avoid
  // accessing bad iterators).
  bool drawAutoModeStepTreeNode(const ThunderAutoModeStepPath& path,
                                std::unique_ptr<ThunderAutoModeStep>& step,
                                const ThunderAutoModeStepTrajectoryBehaviorTreeNode& behaviorTree,
                                std::optional<frc::Pose2d> previousStepEndPose,
                                bool isFirstTrajectoryStep,
                                bool isLastTrajectoryStep,
                                ThunderAutoProjectState& state);
  bool drawAutoModeStepsTree(const ThunderAutoModeStepDirectoryPath& path,
                             std::list<std::unique_ptr<ThunderAutoModeStep>>& steps,
                             const ThunderAutoModeStepTrajectoryBehaviorTreeNode& behaviorTree,
                             std::optional<frc::Pose2d> previousStepEndPose,
                             bool isFirstTrajectoryStep,
                             bool isLastTrajectoryStep,
                             ThunderAutoProjectState& state);

  enum class AutoModeStepDragDropInsertMethod {
    BEFORE,
    AFTER,
    INTO,
  };

  bool autoModeStepDragDropTarget(
      std::variant<ThunderAutoModeStepPath, ThunderAutoModeStepDirectoryPath> closestStepOrDirectoryPath,
      AutoModeStepDragDropInsertMethod insertMethod,
      bool acceptAutoModeSteps,
      ThunderAutoProjectState& state);

  static std::vector<uint8_t> SerializeAutoModeStepPathForDragDrop(const ThunderAutoModeStepPath& path);
  static ThunderAutoModeStepPath DeserializeAutoModeStepPathFromDragDrop(void* data, size_t size);

  void presentAutoModeSelectedStepProperties(ThunderAutoProjectState& state);
  void presentAutoModeSelectedActionStepProperties(ThunderAutoModeActionStep& step,
                                                   ThunderAutoProjectState& state);
  void presentAutoModeSelectedTrajectoryStepProperties(ThunderAutoModeTrajectoryStep& step,
                                                       ThunderAutoProjectState& state);
  void presentAutoModeSelectedBoolBranchStepProperties(ThunderAutoModeBoolBranchStep& step,
                                                       ThunderAutoProjectState& state);
  void presentAutoModeSelectedSwitchBranchStepProperties(ThunderAutoModeSwitchBranchStep& step,
                                                         ThunderAutoProjectState& state);

  void presentAutoModeSpeedConstraintProperties(ThunderAutoProjectState& state);

  static bool presentRightAlignedEyeButton(int id, bool isEyeOpen);

  static bool presentActionProperty(const char* name,
                                    const char* tooltip,
                                    std::function<const std::string&()> getName,
                                    std::function<void(const std::string&)> setName,
                                    bool includeNoneOption,
                                    const ThunderAutoProjectState& state);

  static bool presentTrajectoryProperty(const char* name,
                                        const char* tooltip,
                                        std::function<const std::string&()> getName,
                                        std::function<void(const std::string&)> setName,
                                        bool includeNoneOption,
                                        const ThunderAutoProjectState& state);

  static bool presentInputText(const char* id,
                               std::function<const std::string&()> getText,
                               std::function<void(const std::string&)> setText,
                               char* workingInputBuffer,
                               size_t workingInputBufferSize,
                               bool& isShowingInput);

  bool presentSlider(const char* id,
                     double& value,
                     float speed = 1.f,
                     const char* format = "%.2f",
                     bool* isFinished = nullptr);

  static void presentColoredUnclickableButton(const char* label, ImVec2 size, const ImColor& color);

  static void presentSeparatorText(const char* text);

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
