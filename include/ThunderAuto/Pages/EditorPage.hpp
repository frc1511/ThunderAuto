#pragma once

#include <ThunderAuto/DocumentEditManager.hpp>
#include <ThunderAuto/Pages/Page.hpp>
#include <ThunderAuto/Graphics/Texture.hpp>
#include <ThunderAuto/Shapes.hpp>
#include <ThunderAuto/Error.hpp>
#include <ThunderLibCore/Auto/ThunderAutoProject.hpp>
#include <ThunderLibCore/Auto/ThunderAutoOutputTrajectory.hpp>
#include <ThunderLibCore/Types.hpp>
#include <units/length.h>
#include <units/time.h>
#include <imgui_internal.h>
#include <string_view>
#include <string>
#include <memory>

using namespace thunder::core;

enum class EditorPageTrajectoryOverlay {
  VELOCITY = 0,
  CURVATURE = 1,
};

// TODO: Add auto mode support (currently only Trajectory support is implemented).
class EditorPage : public Page {
  DocumentEditManager& m_history;
  DocumentEditManager::StateUpdateSubscriberID m_stateUpdateSubscriberID;

  const ThunderAutoProjectSettings* m_settings = nullptr;

  enum class PointType {
    NONE = 0,

    WAYPOINT_POSITION,
    WAYPOINT_ANGLE,
    WAYPOINT_HEADING_IN,
    WAYPOINT_HEADING_OUT,

    ROTATION_POSITION,
    ROTATION_ANGLE,

    ACTION_POSITION,
  };

  PointType m_clickedPoint = PointType::NONE;
  PointType m_dragPoint = PointType::NONE;

  // Save mouse position data when a context menu is opened because mouse will move when user selects an
  // option.
  struct {
    ThunderAutoTrajectoryPosition trajectoryPositionClosestToMouse;
    ThunderAutoOutputTrajectoryPoint trajectoryPointClosestToMouse;
    Point2d mousePosition;
  } m_trajectoryContextMenuOpenData;

  struct {
    std::string trajectoryName;
  } m_autoModeContextMenuOpenData;

  static ThunderAutoTrajectoryEditorState::TrajectorySelection ToTrajectorySelection(PointType pointType) {
    switch (pointType) {
      using enum ThunderAutoTrajectoryEditorState::TrajectorySelection;
      case PointType::NONE:
        return NONE;
      case PointType::WAYPOINT_POSITION:
      case PointType::WAYPOINT_ANGLE:
      case PointType::WAYPOINT_HEADING_IN:
      case PointType::WAYPOINT_HEADING_OUT:
        return WAYPOINT;
      case PointType::ROTATION_POSITION:
      case PointType::ROTATION_ANGLE:
        return ROTATION;
      case PointType::ACTION_POSITION:
        return ACTION;
      default:
        ThunderAutoUnreachable("Invalid PointType");
    }
  }

  std::unique_ptr<ThunderAutoOutputTrajectory> m_cachedTrajectory;
  std::vector<std::unique_ptr<ThunderAutoOutputTrajectory>> m_cachedAutoModeTrajectories;

  double m_fieldAspectRatio = 1.0;
  std::unique_ptr<Texture> m_fieldTexture;

  ImVec2 m_fieldOffset;
  float m_fieldScale = 1.f;

  units::second_t m_playbackTime = 0.0_s;
  double m_isPlaying = false;
  double m_wasPlaying = false;

  EditorPageTrajectoryOverlay m_trajectoryOverlay = EditorPageTrajectoryOverlay::VELOCITY;

  TPolyline m_baseRobotRectangle;
  Measurement2d m_robotRectangleSize;
  units::meter_t m_robotRectangleCornerRadius;

 public:
  explicit EditorPage(DocumentEditManager& history) noexcept
      : m_history(history),
        m_stateUpdateSubscriberID(
            history.registerStateUpdateSubscriber(std::bind(&EditorPage::onStateUpdated, this))) {}

  ~EditorPage() { m_history.unregisterStateUpdateSubscriber(m_stateUpdateSubscriberID); }

  /**
   * Loads the configured field image, prepares the editor to present the current project state.
   *
   * @param settings The project settings
   */
  void setupField(const ThunderAutoProjectSettings& settings);

  const char* name() const noexcept override { return "Editor"; }

  /**
   * Reset the zoom and offset of the field.
   */
  void resetView();

  void present(bool* running) override;

  struct TrajectoryEditorOptions {
    bool showTangents = true;
    bool showRotations = true;
    bool showActions = true;
    bool showTooltip = true;
    EditorPageTrajectoryOverlay trajectoryOverlay;
  } trajectoryEditorOptions = {};

  struct AutoModeEditorOptions {
    // TODO
  } autoModeEditorOptions = {};

 private:
  void invalidateCachedTrajectories() noexcept {
    m_cachedTrajectory.reset();
    m_cachedAutoModeTrajectories.clear();
  }

 private:
  void presentEditor();

  void onStateUpdated();

  void processPanAndZoomInput(const ImVec2& fieldScreenSize);
  void processFieldInput();
  void presentField(ImRect bb);

  // Trajectory Editor

  void processTrajectoryEditorInput(ThunderAutoProjectState& state, ImRect bb);
  void presentTrajectoryEditor(ThunderAutoProjectState& state, ImRect bb);

  void presentTrajectory(const ThunderAutoProjectState& state, ImRect bb);
  void presentTrajectoryRobotPreview(ImRect bb);

  void presentTrajectoryDragWidgets(const ThunderAutoProjectState& state, ImRect bb);

  void presentTrajectoryPointDragWidget(const Point2d& position,
                                        bool isLocked,
                                        bool isSelected,
                                        bool first,
                                        bool last,
                                        ImRect bb);

  void presentTrajectoryPointHeadingDragWidget(
      const Point2d& position,
      const ThunderAutoTrajectorySkeletonWaypoint::HeadingControlPoints& controlPoints,
      bool isLocked,
      bool isSelected,
      bool presentIncoming,
      bool presentOutgoing,
      ImRect bb);

  void presentTrajectoryRotationDragWidget(const Point2d& position,
                                           const CanonicalAngle& angle,
                                           bool isLocked,
                                           bool isSelected,
                                           ImRect bb);

  void presentTrajectoryActionDragWidget(const Point2d& position,
                                         CanonicalAngle trajectoryHeading,
                                         std::string_view actionName,
                                         bool isLocked,
                                         bool isSelected,
                                         ImRect bb);

  void processTrajectoryInput(ThunderAutoProjectState& state, ImRect bb);

  // Auto Mode Editor

  void processAutoModeEditorInput(ThunderAutoProjectState& state, ImRect bb);
  void presentAutoModeEditor(ThunderAutoProjectState& state, ImRect bb);

  bool presentAutoModeStepList(const ThunderAutoModeStepDirectoryPath& path,
                               const ThunderAutoMode::StepDirectory& steps,
                               size_t& trajectoryIndex,
                               bool& clickWasCaptured,
                               bool isActive,
                               ThunderAutoProjectState& state,
                               ImRect bb);
  bool presentAutoModeStep(const ThunderAutoModeStepPath& path,
                           const ThunderAutoModeStep& step,
                           size_t& trajectoryIndex,
                           bool& clickWasCaptured,
                           bool isActive,
                           ThunderAutoProjectState& state,
                           ImRect bb);

  bool presentAutoModeTrajectoryStep(const ThunderAutoModeStepPath& path,
                                     const ThunderAutoModeTrajectoryStep& step,
                                     size_t& trajectoryIndex,
                                     bool& clickWasCaptured,
                                     bool isActive,
                                     ThunderAutoProjectState& state,
                                     ImRect bb);

  // General Editor Stuff

  void presentPlaybackSlider(const ThunderAutoProjectState& state);
  void processPlaybackInput();

  void drawRobot(const Point2d& position,
                 const CanonicalAngle& rotation,
                 float positionPointRadius,
                 float rotationPointRadius,
                 ImU32 color,
                 ImRect bb);
  void drawRobot(const frc::Pose2d& pose,
                 float positionPointRadius,
                 float rotationPointRadius,
                 ImU32 color,
                 ImRect bb);

  // Utility functions

  bool IsMouseHoveringPoint(Point2d point, ImRect bb);

  static void SetMouseCursorMoveDirection(CanonicalAngle angle);

  static ImVec2 ToScreenCoordinate(const Point2d& fieldCoordinate,
                                   const ThunderAutoFieldImage& fieldImage,
                                   ImRect bb);
  static Point2d ToFieldCoordinate(const ImVec2& screenCoordinate,
                                   const ThunderAutoFieldImage& fieldImage,
                                   const ImRect& bb);
};
