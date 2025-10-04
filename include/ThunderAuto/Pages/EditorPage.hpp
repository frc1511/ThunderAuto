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
  } m_contextMenuOpenData;

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

  // bool m_showTangents = true;
  // bool m_showRotation = true;
  // bool m_showTooltip = true;

  // Vec2 m_fieldSize;
  double m_fieldAspectRatio = 1.0;
  std::unique_ptr<Texture> m_fieldTexture;

  ImVec2 m_fieldOffset;
  float m_fieldScale = 1.f;

  units::second_t m_playbackTime = 0.0_s;
  double m_isPlaying = false;
  double m_wasPlaying = false;

  EditorPageTrajectoryOverlay m_trajectoryOverlay = EditorPageTrajectoryOverlay::VELOCITY;

  Polyline m_baseRobotRectangle;
  Measurement2d m_robotRectangleSize;
  units::meter_t m_robotRectangleCornerRadius;

 public:
  explicit EditorPage(DocumentEditManager& history) noexcept : m_history(history) {}

  /**
   * Loads the configured field image, prepares the editor to present the current project state.
   *
   * @param settings The project settings
   */
  void setupField(const ThunderAutoProjectSettings& settings);

  /**
   * Invalidate the cached trajectory, forcing it to be rebuilt on next access.
   * This should be called when the state is changed (e.g. undo, redo, etc.).
   */
  void invalidateCachedTrajectory() noexcept { m_cachedTrajectory.reset(); }

  const char* name() const noexcept override { return "Editor"; }

  /**
   * Reset the zoom and offset of the field.
   */
  void resetView();

  void resetPlayback() noexcept { m_playbackTime = 0.0_s; }

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
  void presentEditor();

  void processPanAndZoomInput(const ImVec2& fieldScreenSize);
  void processFieldInput();
  void presentField(ImRect bb);

  // Trajectory Editor

  void processTrajectoryEditorInput(ThunderAutoProjectState& state, ImRect bb);
  void presentTrajectoryEditor(ThunderAutoProjectState& state, ImRect bb);

  // Auto Mode Editor

  // void processAutoModeEditorInput(ThunderAutoProjectState& state);
  // void presentAutoModeEditor(ThunderAutoProjectState& state);

  // General Editor Stuff

  // void presentContextMenus(ThunderAutoProjectState& state);

  void presentTrajectory(const ThunderAutoProjectState& state, ImRect bb);
  void presentRobotPreview(ImRect bb);

  void presentPlaybackSlider();
  void processPlaybackInput();

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

  void presentTrajectoryContextMenus(ThunderAutoProjectState& state);

  // void handleInput(ThunderAutoProjectState& state, Rect bb);
  // void handlePointInput(ThunderAutoProjectState& state, Rect bb);
  // void handleCurveInput(ThunderAutoProjectState& state, Rect bb);

  // Utility functions

  // void insertWaypoint(ThunderAutoProjectState& state,
  //                     Point2d position,
  //                     CanonicalAngle heading,
  //                     size_t index) const;
  // void insertRotation(ThunderAutoProjectState& state,
  //                     ThunderAutoTrajectoryPosition position,
  //                     CanonicalAngle angle) const;
  // void insertAction(ThunderAutoProjectState& state,
  //                   ThunderAutoTrajectoryPosition position,
  //                   const std::string& name) const;

  // void deleteSelectedWaypoint(ThunderAutoProjectState& state) const;
  // void deleteSelectedRotation(ThunderAutoProjectState& state) const;
  // void deleteSelectedAction(ThunderAutoProjectState& state) const;

  bool isMouseHoveringPoint(Point2d point, ImRect bb);

  static void SetMouseCursorMoveDirection(CanonicalAngle angle);

  static ImVec2 ToScreenCoordinate(const Point2d& fieldCoordinate,
                                   const ThunderAutoFieldImage& fieldImage,
                                   ImRect bb);
  static Point2d ToFieldCoordinate(const ImVec2& screenCoordinate,
                                   const ThunderAutoFieldImage& fieldImage,
                                   const ImRect& bb);
};
