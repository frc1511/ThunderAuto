#include <ThunderAuto/Pages/EditorPage.hpp>

#include <ThunderAuto/Input.hpp>
#include <ThunderAuto/Types.hpp>
#include <ThunderAuto/ColorPalette.hpp>
#include <ThunderLibCore/Math.hpp>
#include <IconsLucide.h>
#include <stb_image.h>
#include <imgui_raii.h>
#include <algorithm>
#include <limits>

static const units::meter_t kMinRotationTargetSeparation = 0.1_m;

// Colors

static const ImU32 kPointColor = ThunderAutoColorPalette::kWhite;
static const ImU32 kPointSelectedColor = ThunderAutoColorPalette::kYellowHigh;

static const ImU32 kPointPreviewColor = IM_COL32(128, 128, 128, 255);
static const ImU32 kPointLockedColor = IM_COL32(192, 192, 192, 255);

static const ImU32 kStartPointColor = ThunderAutoColorPalette::kGreenHigh;
static const ImU32 kStartPointSelectedColor = ThunderAutoColorPalette::kGreenMid;

static const ImU32 kEndPointColor = ThunderAutoColorPalette::kRedHigh;
static const ImU32 kEndPointSelectedColor = ThunderAutoColorPalette::kRedMid;

static const ImU32 kHandleColor = ThunderAutoColorPalette::kWhite;
static const ImU32 kHandleSelectedColor = ThunderAutoColorPalette::kYellowMid;

static const ImU32 kActionColor = ThunderAutoColorPalette::kOrangeMid;
static const ImU32 kActionSelectedColor = ThunderAutoColorPalette::kOrangeHigh;

ImVec2 EditorPage::ToScreenCoordinate(const Point2d& fieldCoordinate,
                                      const ThunderAutoFieldImage& fieldImage,
                                      ImRect bb) {
  ImVec2 pt = ToImVec2(fieldCoordinate) / ToImVec2(fieldImage.fieldSize());

  ImRect imageFieldBounds = ToImRect(fieldImage.imageFieldBounds());

  pt *= imageFieldBounds.Max - imageFieldBounds.Min;
  pt += imageFieldBounds.Min;

  pt = ImVec2(pt.x, 1.f - pt.y);

  pt = bb.Min + pt * bb.GetSize();

  return pt;
}

Point2d EditorPage::ToFieldCoordinate(const ImVec2& screenCoordinate,
                                      const ThunderAutoFieldImage& fieldImage,
                                      const ImRect& bb) {
  ImVec2 pt = (screenCoordinate - bb.Min) / bb.GetSize();

  pt = ImVec2(pt.x, 1.f - pt.y);

  ImRect imageFieldBounds = ToImRect(fieldImage.imageFieldBounds());

  pt -= imageFieldBounds.Min;
  pt /= imageFieldBounds.Max - imageFieldBounds.Min;

  pt *= ToImVec2(fieldImage.fieldSize());

  return ToPoint2d(pt);
}

void EditorPage::SetMouseCursorMoveDirection(CanonicalAngle angle) {
  units::degree_t angleDeg = angle.degrees();

  if ((angleDeg >= +22.5_deg && angleDeg < +67.5_deg) || (angleDeg < -112.5_deg && angleDeg >= -157.5_deg)) {
    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNESW);
  } else if ((angleDeg >= +67.5_deg && angleDeg < +112.5_deg) ||
             (angleDeg < -67.5_deg && angleDeg >= -112.5_deg)) {
    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
  }

  else if ((angleDeg > +112.5_deg && angleDeg <= +157.5_deg) ||
           (angleDeg <= -22.5_deg && angleDeg > -67.5_deg)) {
    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
  } else {
    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
  }
}

bool EditorPage::isMouseHoveringPoint(Point2d point, ImRect bb) {
  ImVec2 screenPoint = ToScreenCoordinate(point, m_settings->fieldImage, bb);

  float toleranceRadius = GET_UISIZE(DRAG_POINT_RADIUS) * 1.25f;
  ImVec2 clickTolerance(toleranceRadius, toleranceRadius);

  bool isHovering = ImGui::IsMouseHoveringRect(screenPoint - clickTolerance, screenPoint + clickTolerance);
  return isHovering;
}

#include <field_2022_png.h>
#include <field_2023_png.h>
#include <field_2024_png.h>
#include <field_2025_png.h>

void EditorPage::setupField(const ThunderAutoProjectSettings& settings) {
  m_settings = &settings;

  const ThunderAutoFieldImage& fieldImage = settings.fieldImage;
  ThunderAutoFieldImageType imageType = fieldImage.type();

  if (imageType == ThunderAutoFieldImageType::CUSTOM) {
    std::filesystem::path imagePath = fieldImage.customImagePath();
    m_fieldTexture = PlatformTexture::make(imagePath);  // will throw if error loading
  } else {
    unsigned char* imageDataBuf = nullptr;
    size_t imageDataSize = 0;
    switch (fieldImage.builtinImage()) {
      using enum ThunderAutoBuiltinFieldImage;
      case FIELD_2022:
        imageDataBuf = field_2022_png;
        imageDataSize = field_2022_png_size;
        break;
      case FIELD_2023:
        imageDataBuf = field_2023_png;
        imageDataSize = field_2023_png_size;
        break;
      case FIELD_2024:
        imageDataBuf = field_2024_png;
        imageDataSize = field_2024_png_size;
        break;
      case FIELD_2025:
        imageDataBuf = field_2025_png;
        imageDataSize = field_2025_png_size;
        break;
    }

    m_fieldTexture = PlatformTexture::make(imageDataBuf, imageDataSize);  // will throw if error loading
  }

  m_fieldAspectRatio =
      static_cast<float>(m_fieldTexture->width()) / static_cast<float>(m_fieldTexture->height());

  m_fieldOffset = ImVec2(0.f, 0.f);
  m_fieldScale = 1.0f;

  invalidateCachedTrajectory();
}

void EditorPage::resetView() {
  m_fieldOffset = ImVec2(0.f, 0.f);
  m_fieldScale = 1.f;
  // TODO: Option to focus on blue or red side?
}

void EditorPage::present(bool* running) {
  ImGui::SetNextWindowSize(ImVec2(GET_UISIZE(EDITOR_PAGE_START_WIDTH), GET_UISIZE(EDITOR_PAGE_START_HEIGHT)),
                           ImGuiCond_FirstUseEver);

  auto scopedWindow = ImGui::Scoped::Window(
      name(), running,
      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoNavInputs);
  if (!scopedWindow || !*running)
    return;

  presentEditor();
}

void EditorPage::presentEditor() {
  ImGuiWindow* win = ImGui::GetCurrentWindow();
  if (win->SkipItems)
    return;

  ImGui::SetScrollX(0);
  ImGui::SetScrollY(0);

  // Setup canvas

  const ImVec2 windowSize = ImGui::GetWindowSize();

  float dimX = windowSize.x;
  float dimY = windowSize.y;

  // Fit within the window size while maintaining aspect ratio.
  if ((dimX / dimY) > m_fieldAspectRatio)
    dimX = dimY * m_fieldAspectRatio;
  else
    dimY = dimX / m_fieldAspectRatio;

  ImVec2 fieldScreenSize(dimX, dimY);

  // Input

  processPanAndZoomInput(fieldScreenSize);
  processFieldInput();

  // Apply pan/zoom offset.
  win->DC.CursorPos += m_fieldOffset * fieldScreenSize;

  // Field bounding box, with zoom applied.
  ImRect bb(win->DC.CursorPos, win->DC.CursorPos + fieldScreenSize * m_fieldScale);

  ImGui::ItemSize(bb);
  if (!ImGui::ItemAdd(bb, 0))
    return;

  // Draw Field

  presentField(bb);

  // Draw Editor UI

  ThunderAutoProjectState state = m_history.currentState();

  ThunderAutoEditorState& editorState = state.editorState;
  switch (editorState.view) {
    using enum ThunderAutoEditorState::View;
    case TRAJECTORY:
      presentTrajectoryEditor(state, bb);
      presentPlaybackSlider();
      processPlaybackInput();
      processTrajectoryEditorInput(state, bb);
      break;
    case AUTO_MODE:
      // processAutoModeEditorInput(state);
      presentPlaybackSlider();
      processPlaybackInput();
      // presentAutoModeEditor(state);
      break;
    case NONE:
      break;
    default:
      ThunderAutoUnreachable("Unknown editor view");
  }
}

void EditorPage::processPanAndZoomInput(const ImVec2& fieldScreenSize) {
  if (!ImGui::IsWindowHovered())
    return;

  const ImGuiIO& io = ImGui::GetIO();
  const ImGuiWindow* win = ImGui::GetCurrentWindow();

  // Panning (Shift + Left-Click, or Middle-Click).

  bool leftDragging = ImGui::IsMouseDragging(ImGuiMouseButton_Left);
  bool middleDragging = ImGui::IsMouseDragging(ImGuiMouseButton_Middle);

  if (io.KeyShift || middleDragging) {
    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
  }

  if ((io.KeyShift && leftDragging) || middleDragging) {
    m_fieldOffset += io.MouseDelta / fieldScreenSize;
  }

  // Zooming.

  const float prevFieldScale = m_fieldScale;

  m_fieldScale += io.MouseWheel * 0.05f;
  m_fieldScale = std::clamp(m_fieldScale, 1.f, 4.f);

  // I CANNOT BEGIN TO EXPLAIN HOW HARD ZOOMING WAS TO FIGURE OUT.

  // Find the difference between mouse positions before and after the zoom, offset by difference.

  ImVec2 mouseScreenPos = io.MousePos - (win->DC.CursorPos + m_fieldOffset * fieldScreenSize);
  ImVec2 mouseScreenPosDiff =
      (mouseScreenPos / m_fieldScale - mouseScreenPos / prevFieldScale) * m_fieldScale;

  ImVec2 mouseFieldPosDiff = mouseScreenPosDiff / fieldScreenSize;

  m_fieldOffset += mouseFieldPosDiff;
}

void EditorPage::processFieldInput() {
  if (!ImGui::IsWindowFocused())
    return;

  if (IsCtrlDown() && IsKeyPressed(ImGuiKey_0)) {  // Ctrl+0
    resetView();
  }
  // TODO: Ctrl+1 & Ctrl+2 to focus on blue/red side of the field?
}

void EditorPage::presentField(ImRect bb) {
  ImDrawList* drawList = ImGui::GetWindowDrawList();

  // Background image.
  drawList->AddImage(m_fieldTexture->id(), bb.Min, bb.Max);
}

void EditorPage::processTrajectoryEditorInput(ThunderAutoProjectState& state, ImRect bb) {
  processTrajectoryInput(state, bb);
}

void EditorPage::presentTrajectoryEditor(ThunderAutoProjectState& state, ImRect bb) {
  // Do nothing if no trajectory selected.
  if (state.editorState.trajectoryEditorState.currentTrajectoryName.empty())
    return;

  presentTrajectory(state, bb);
  presentRobotPreview(bb);
  presentTrajectoryDragWidgets(state, bb);
}

void EditorPage::presentTrajectory(const ThunderAutoProjectState& state, ImRect bb) {
  const ThunderAutoTrajectorySkeleton& skeleton = state.currentTrajectory();

  if (!m_cachedTrajectory) {
    m_cachedTrajectory = BuildThunderAutoOutputTrajectory(skeleton, kPreviewOutputTrajectorySettings);
    resetPlayback();
  }
  ThunderAutoAssert(m_cachedTrajectory != nullptr);

  ImDrawList* drawList = ImGui::GetWindowDrawList();

  std::span<const ThunderAutoOutputTrajectoryPoint> points = m_cachedTrajectory->points;

  auto startPointIt = points.begin();
  auto endPointIt = std::next(startPointIt);

  for (; endPointIt != points.end(); ++startPointIt, ++endPointIt) {
    const ImVec2 startPointCoordinate =
        ToScreenCoordinate(startPointIt->position, m_settings->fieldImage, bb);
    const ImVec2 endPointCoordinate = ToScreenCoordinate(endPointIt->position, m_settings->fieldImage, bb);

    double hue = 0.0;
    switch (trajectoryEditorOptions.trajectoryOverlay) {
      using enum EditorPageTrajectoryOverlay;
      case VELOCITY: {
        // Make the line color the average of the two points' linear velocities.
        auto averageLinearVelocity = (startPointIt->linearVelocity + endPointIt->linearVelocity) / 2.0;
        hue = 0.7 - averageLinearVelocity / skeleton.settings().maxLinearVelocity;
        break;
      }
      case CURVATURE: {
        auto averageCurvature = (startPointIt->curvature + endPointIt->curvature) / 2.0;
        hue = 0.6 - std::clamp(averageCurvature.value(), 0.0, 10.0) / 10.0;
        break;
      }
      default:
        ThunderAutoUnreachable("Unknown trajectory overlay");
    }

    drawList->AddLine(startPointCoordinate, endPointCoordinate,
                      ImColor::HSV(static_cast<float>(hue), 1.f, 1.f), GET_UISIZE(LINE_THICKNESS));
  }
}

void EditorPage::presentRobotPreview(ImRect bb) {
  ImDrawList* drawList = ImGui::GetWindowDrawList();

  ThunderAutoAssert(m_cachedTrajectory != nullptr);

  // Find closest time.

  std::span<const ThunderAutoOutputTrajectoryPoint> points = m_cachedTrajectory->points;

  auto lowerIt = std::lower_bound(
      points.begin(), points.end(), m_playbackTime,
      [](const ThunderAutoOutputTrajectoryPoint& point, units::second_t time) { return point.time < time; });

  auto upperIt = lowerIt;

  units::second_t playbackTime = m_playbackTime;

  if (lowerIt == points.end()) {
    upperIt = std::prev(lowerIt);
    lowerIt = std::prev(upperIt);

  } else if (lowerIt == points.begin()) {
    upperIt = std::next(lowerIt);
    if (upperIt == points.end()) {
      upperIt = lowerIt;
    }
    playbackTime = lowerIt->time;

  } else {
    upperIt = lowerIt;
    lowerIt = std::prev(lowerIt);
  }

  ThunderAutoAssert(lowerIt != points.end());
  ThunderAutoAssert(upperIt != points.end());

  const ThunderAutoOutputTrajectoryPoint& lowerPoint = *lowerIt;
  const ThunderAutoOutputTrajectoryPoint& upperPoint = *upperIt;

  const units::second_t dt = upperPoint.time - lowerPoint.time;
  const double t = dt > 0.01_s ? (playbackTime - lowerPoint.time).value() / dt.value() : 0.0;

  // Interpolate between points.

  const Point2d position = Point2d(Lerp(lowerPoint.position.x, upperPoint.position.x, t),
                                   Lerp(lowerPoint.position.y, upperPoint.position.y, t));

  CanonicalAngle rotation = Lerp(lowerPoint.rotation, upperPoint.rotation, t);

  // Draw point to show which way is forward.

  ImVec2 rotationPoint = ToScreenCoordinate(
      position.extendAtAngle(rotation, m_settings->robotSize.length / 2.f), m_settings->fieldImage, bb);

  drawList->AddCircleFilled(rotationPoint, GET_UISIZE(DRAG_POINT_RADIUS) / 1.5f, kPointPreviewColor);

  // Draw the robot outline.

  // Calculate the points for the robot preview rectangle just once, unless size gets changed.
  if (m_robotRectangleSize != m_settings->robotSize ||
      m_robotRectangleCornerRadius != m_settings->robotCornerRadius) {
    m_robotRectangleSize = m_settings->robotSize;
    m_robotRectangleCornerRadius = m_settings->robotCornerRadius;

    m_baseRobotRectangle = CreateRoundedRectangle(m_robotRectangleSize, m_robotRectangleCornerRadius);
  }

  Polyline robotPreviewRectangle = m_baseRobotRectangle;

  RotatePolygon(robotPreviewRectangle, rotation);
  TranslatePolygon(robotPreviewRectangle, Displacement2d(position.x, position.y));

  std::vector<ImVec2> screenCoordinates(robotPreviewRectangle.size());
  std::transform(robotPreviewRectangle.begin(), robotPreviewRectangle.end(), screenCoordinates.begin(),
                 [&](const Point2d& fieldCoordinate) {
                   return ToScreenCoordinate(fieldCoordinate, m_settings->fieldImage, bb);
                 });

  drawList->AddPolyline(screenCoordinates.data(), static_cast<int>(screenCoordinates.size()),
                        kPointPreviewColor, true, GET_UISIZE(LINE_THICKNESS));
}

void EditorPage::presentPlaybackSlider() {
  if (m_cachedTrajectory == nullptr)
    return;

  const ImGuiStyle& style = ImGui::GetStyle();

  float sliderYOffset = style.FramePadding.y * 2.f + style.FontSizeBase + style.WindowPadding.y;

  // Bottom of the window.
  ImGui::SetCursorPosY(ImGui::GetWindowHeight() - sliderYOffset);

  // Play button.
  if (ImGui::Button(m_isPlaying ? ICON_LC_PAUSE : ICON_LC_PLAY)) {
    m_isPlaying = !m_isPlaying;
  }
  ImGui::SameLine();

  // Reset button.
  if (ImGui::Button(ICON_LC_ROTATE_CCW)) {
    m_playbackTime = 0.0_s;
  }
  ImGui::SameLine();

  // Slider.

  ThunderAutoAssert(!m_cachedTrajectory->points.empty());
  units::second_t totalTime = m_cachedTrajectory->points.back().time;

  if (m_isPlaying) {
    m_playbackTime += units::second_t(ImGui::GetIO().DeltaTime);
    m_playbackTime = units::second_t(std::fmod(m_playbackTime.value(), totalTime.value()));
  }

  {
    auto scopedItemWidth = ImGui::Scoped::ItemWidth(ImGui::GetContentRegionAvail().x);

    float playbackTime = m_playbackTime.value();

    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%.2f / %.2f s", playbackTime, totalTime.value());

    ImGui::SliderFloat("##Playback", &playbackTime, 0.0f, static_cast<float>(totalTime.value()), buffer);

    m_playbackTime = units::second_t(playbackTime);
  }
}

void EditorPage::processPlaybackInput() {
  if (!ImGui::IsWindowFocused())
    return;

  if (IsKeyPressed(ImGuiKey_Space)) {
    m_isPlaying = !m_isPlaying;
  }
}

void EditorPage::presentTrajectoryDragWidgets(const ThunderAutoProjectState& state, ImRect bb) {
  const ThunderAutoTrajectoryEditorState& editorState = state.editorState.trajectoryEditorState;
  const ThunderAutoTrajectorySkeleton& skeleton = state.currentTrajectory();

  ThunderAutoAssert(m_cachedTrajectory != nullptr);

  // Waypoints

  int i = 0;
  for (const ThunderAutoTrajectorySkeletonWaypoint& point : skeleton) {
    bool isSelected = false;
    if (editorState.trajectorySelection == ThunderAutoTrajectoryEditorState::TrajectorySelection::WAYPOINT) {
      int selectedIndex = editorState.selectionIndex;
      isSelected = (i == selectedIndex);
    }

    const bool first = (i == 0);
    const bool last = (i == static_cast<int>(skeleton.numPoints() - 1));

    i++;

    if (trajectoryEditorOptions.showRotations) {
      if (first) {
        presentTrajectoryRotationDragWidget(point.position(), skeleton.startRotation(),
                                            point.isEditorLocked(), isSelected, bb);
      } else if (last) {
        presentTrajectoryRotationDragWidget(point.position(), skeleton.endRotation(), point.isEditorLocked(),
                                            isSelected, bb);
      } else if (point.isStopped()) {
        presentTrajectoryRotationDragWidget(point.position(), point.stopRotation(), point.isEditorLocked(),
                                            isSelected, bb);
      }
    }

    if (trajectoryEditorOptions.showTangents) {
      presentTrajectoryPointHeadingDragWidget(point.position(), point.headingControlPoints(),
                                              point.isEditorLocked(), isSelected, !first, !last, bb);
    }
    presentTrajectoryPointDragWidget(point.position(), point.isEditorLocked(), isSelected, first, last, bb);
  }

  // Rotation Targets

  if (trajectoryEditorOptions.showRotations) {
    i = 0;
    const auto& rotations = skeleton.rotations();
    for (const auto& [positionInTrajectory, rotation] : rotations) {
      bool isSelected = false;
      if (editorState.trajectorySelection ==
          ThunderAutoTrajectoryEditorState::TrajectorySelection::ROTATION) {
        int selectedIndex = editorState.selectionIndex;
        isSelected = (i == selectedIndex);
      }
      i++;

      size_t pointIndex = m_cachedTrajectory->trajectoryPositionToPointIndex(positionInTrajectory);
      const ThunderAutoOutputTrajectoryPoint& point = m_cachedTrajectory->points.at(pointIndex);
      Point2d position = point.position;

      presentTrajectoryRotationDragWidget(position, rotation.angle, rotation.editorLocked, isSelected, bb);
    }
  }

  // Actions

  if (trajectoryEditorOptions.showActions) {
    i = 0;
    const auto& actions = skeleton.actions();
    for (const auto& [positionInTrajectory, action] : actions) {
      bool isSelected = false;
      if (editorState.trajectorySelection == ThunderAutoTrajectoryEditorState::TrajectorySelection::ACTION) {
        int selectedIndex = editorState.selectionIndex;
        isSelected = (i == selectedIndex);
      }
      i++;

      size_t pointIndex = m_cachedTrajectory->trajectoryPositionToPointIndex(positionInTrajectory);
      const ThunderAutoOutputTrajectoryPoint& point = m_cachedTrajectory->points.at(pointIndex);
      Point2d position = point.position;
      CanonicalAngle heading = point.heading;

      presentTrajectoryActionDragWidget(position, heading, action.action, action.editorLocked, isSelected,
                                        bb);
    }
  }
}

void EditorPage::presentTrajectoryPointDragWidget(const Point2d& position,
                                                  bool isLocked,
                                                  bool isSelected,
                                                  bool first,
                                                  bool last,
                                                  ImRect bb) {
  ImDrawList* drawList = ImGui::GetWindowDrawList();

  ImVec2 pt = ToScreenCoordinate(position, m_settings->fieldImage, bb);

  ImU32 color = kPointColor;
  ImU32 colorSelected = kPointSelectedColor;

  if (first) {
    color = kStartPointColor;
    colorSelected = kStartPointSelectedColor;
  } else if (last) {
    color = kEndPointColor;
    colorSelected = kEndPointSelectedColor;
  }

  ImU32 finalColor = isSelected ? colorSelected : color;

  if (isLocked) {
    finalColor = kPointLockedColor;
  }

  drawList->AddCircleFilled(pt, GET_UISIZE(DRAG_POINT_RADIUS), finalColor);
}

void EditorPage::presentTrajectoryPointHeadingDragWidget(
    const Point2d& position,
    const ThunderAutoTrajectorySkeletonWaypoint::HeadingControlPoints& controlPoints,
    bool isLocked,
    bool isSelected,
    bool presentIncoming,
    bool presentOutgoing,
    ImRect bb) {
  if (isLocked)
    return;

  ImDrawList* drawList = ImGui::GetWindowDrawList();

  const ImVec2 pt = ToScreenCoordinate(position, m_settings->fieldImage, bb);

  const ImU32 handleColor = isSelected ? kHandleSelectedColor : kHandleColor;
  const ImU32 pointColor = isSelected ? kPointSelectedColor : kPointColor;

  if (presentIncoming) {
    const ImVec2 controlPoint = ToScreenCoordinate(controlPoints.incomingPoint(), m_settings->fieldImage, bb);
    drawList->AddLine(pt, controlPoint, handleColor, GET_UISIZE(LINE_THICKNESS));
    drawList->AddCircleFilled(controlPoint, GET_UISIZE(DRAG_POINT_RADIUS), pointColor);
  }
  if (presentOutgoing) {
    const ImVec2 controlPoint = ToScreenCoordinate(controlPoints.outgoingPoint(), m_settings->fieldImage, bb);
    drawList->AddLine(pt, controlPoint, handleColor, GET_UISIZE(LINE_THICKNESS));
    drawList->AddCircleFilled(controlPoint, GET_UISIZE(DRAG_POINT_RADIUS), pointColor);
  }
}

void EditorPage::presentTrajectoryRotationDragWidget(const Point2d& position,
                                                     const CanonicalAngle& angle,
                                                     bool isLocked,
                                                     bool isSelected,
                                                     ImRect bb) {
  ImDrawList* drawList = ImGui::GetWindowDrawList();

  ImU32 color = kPointColor;
  if (isLocked) {
    color = kPointLockedColor;
  } else if (isSelected) {
    color = kPointSelectedColor;
  }

  // Draw position drag point.

  ImVec2 pt = ToScreenCoordinate(position, m_settings->fieldImage, bb);

  drawList->AddCircleFilled(pt, GET_UISIZE(DRAG_POINT_RADIUS) / 1.5f, color);

  // Draw rotation drag point.

  ImVec2 rotationPoint = ToScreenCoordinate(position.extendAtAngle(angle, m_settings->robotSize.length / 2.f),
                                            m_settings->fieldImage, bb);

  drawList->AddCircleFilled(rotationPoint, GET_UISIZE(DRAG_POINT_RADIUS) / 1.f, color);

  // Draw the robot outline.

  Polyline robotPreviewRectangle = m_baseRobotRectangle;

  RotatePolygon(robotPreviewRectangle, angle);
  TranslatePolygon(robotPreviewRectangle, Displacement2d(position.x, position.y));

  std::vector<ImVec2> screenCoordinates(robotPreviewRectangle.size());
  std::transform(robotPreviewRectangle.begin(), robotPreviewRectangle.end(), screenCoordinates.begin(),
                 [&](const Point2d& fieldCoordinate) {
                   return ToScreenCoordinate(fieldCoordinate, m_settings->fieldImage, bb);
                 });

  drawList->AddPolyline(screenCoordinates.data(), static_cast<int>(screenCoordinates.size()), color, true,
                        GET_UISIZE(LINE_THICKNESS));
}

void EditorPage::presentTrajectoryActionDragWidget(const Point2d& position,
                                                   CanonicalAngle trajectoryHeading,
                                                   std::string_view actionName,
                                                   bool isLocked,
                                                   bool isSelected,
                                                   ImRect bb) {
  ImDrawList* drawList = ImGui::GetWindowDrawList();

  ImU32 color = kActionColor;
  if (isLocked) {
    color = kPointLockedColor;
  } else if (isSelected) {
    color = kActionSelectedColor;
  }

  // Draw position drag point.

  ImVec2 pt = ToScreenCoordinate(position, m_settings->fieldImage, bb);

  drawList->AddCircleFilled(pt, GET_UISIZE(DRAG_POINT_RADIUS) / 1.5f, color);

  float lineLength = GET_UISIZE(DRAG_POINT_RADIUS) * 3.f;
  CanonicalAngle lineAngle = trajectoryHeading;

  float dy = static_cast<float>(gcem::cos(lineAngle.radians().value())) * (lineLength / 2.f);
  float dx = static_cast<float>(gcem::sin(lineAngle.radians().value())) * (lineLength / 2.f);

  ImVec2 lineStartPoint = pt + ImVec2(dx, dy);
  ImVec2 lineEndPoint = pt - ImVec2(dx, dy);

  drawList->AddLine(lineStartPoint, lineEndPoint, color, GET_UISIZE(LINE_THICKNESS));
}

void EditorPage::processTrajectoryInput(ThunderAutoProjectState& state, ImRect bb) {
  // Do nothing if no trajectory selected.
  if (state.editorState.trajectoryEditorState.currentTrajectoryName.empty())
    return;

  const ImGuiIO& io = ImGui::GetIO();

  ThunderAutoTrajectoryEditorState& editorState = state.editorState.trajectoryEditorState;
  ThunderAutoTrajectorySkeleton& skeleton = state.currentTrajectory();

  ThunderAutoAssert(m_cachedTrajectory != nullptr);
  ThunderAutoAssert(!m_cachedTrajectory->points.empty());

  ThunderAutoTrajectoryPosition trajectoryPositionClosestToMouse;
  ThunderAutoOutputTrajectoryPoint trajectoryPointClosestToMouse;
  bool isTrajectoryHovered = false;

  const Point2d mousePosition = ToFieldCoordinate(io.MousePos, m_settings->fieldImage, bb);

  // Find trajectory position closest to mouse

  {
    units::meter_t closestDistance{std::numeric_limits<double>::max()};
    size_t closestPointIndex = m_cachedTrajectory->points.size();

    size_t i = 0;
    for (const ThunderAutoOutputTrajectoryPoint& point : m_cachedTrajectory->points) {
      units::meter_t distance = mousePosition.distanceTo(point.position);
      if (distance < closestDistance) {
        closestDistance = distance;
        closestPointIndex = i;
      }
      i++;
    }

    ThunderAutoAssert(closestPointIndex < m_cachedTrajectory->points.size());

    trajectoryPositionClosestToMouse = m_cachedTrajectory->pointIndexToTrajectoryPosition(closestPointIndex);
    trajectoryPointClosestToMouse = m_cachedTrajectory->points.at(closestPointIndex);

    if (isMouseHoveringPoint(trajectoryPointClosestToMouse.position, bb)) {
      isTrajectoryHovered = true;
    }
  }

  // Determine hovered point

  PointType hoveredPointType = PointType::NONE;
  size_t hoveredPointIndex;

  const auto checkHoverPoint = [&](const Point2d& point, PointType type, size_t index) {
    if (hoveredPointType != PointType::NONE)
      return;

    if (isMouseHoveringPoint(point, bb)) {
      hoveredPointType = type;
      hoveredPointIndex = index;

      if (hoveredPointType == PointType::ACTION_POSITION ||
          hoveredPointType == PointType::ROTATION_POSITION) {
        SetMouseCursorMoveDirection(trajectoryPointClosestToMouse.heading);
      } else {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
      }
    }
  };

  size_t i = 0;
  for (auto pointIt = skeleton.cbegin(); pointIt != skeleton.cend(); pointIt++, i++) {
    if (hoveredPointType != PointType::NONE)
      break;

    const ThunderAutoTrajectorySkeletonWaypoint& point = *pointIt;

    if (point.isEditorLocked())
      continue;

    const bool first = (i == 0);
    const bool last = (i == (skeleton.numPoints() - 1));

    Point2d positionPoint = point.position();
    ThunderAutoTrajectorySkeletonWaypoint::HeadingControlPoints headingControlPoints =
        point.headingControlPoints();
    Point2d headingIncomingControlPoint = headingControlPoints.incomingPoint();
    Point2d headingOutgoingControlPoint = headingControlPoints.outgoingPoint();

    if (trajectoryEditorOptions.showRotations) {
      if (first) {
        Point2d rotationPoint =
            positionPoint.extendAtAngle(skeleton.startRotation(), m_settings->robotSize.length / 2.f);
        checkHoverPoint(rotationPoint, PointType::WAYPOINT_ANGLE, i);

      } else if (last) {
        Point2d rotationPoint =
            positionPoint.extendAtAngle(skeleton.endRotation(), m_settings->robotSize.length / 2.f);
        checkHoverPoint(rotationPoint, PointType::WAYPOINT_ANGLE, i);

      } else if (point.isStopped()) {
        Point2d rotationPoint =
            positionPoint.extendAtAngle(point.stopRotation(), m_settings->robotSize.length / 2.f);
        checkHoverPoint(rotationPoint, PointType::WAYPOINT_ANGLE, i);
      }
    }

    if (trajectoryEditorOptions.showTangents) {
      checkHoverPoint(headingIncomingControlPoint, PointType::WAYPOINT_HEADING_IN, i);
      checkHoverPoint(headingOutgoingControlPoint, PointType::WAYPOINT_HEADING_OUT, i);
    }

    checkHoverPoint(positionPoint, PointType::WAYPOINT_POSITION, i);
  }

  if (trajectoryEditorOptions.showRotations) {
    const auto rotations = skeleton.rotations();
    i = 0;
    for (auto rotationIt = rotations.cbegin(); rotationIt != rotations.cend(); rotationIt++, i++) {
      if (hoveredPointType != PointType::NONE)
        break;

      const auto& [positionInTrajectory, rotation] = *rotationIt;

      if (rotation.editorLocked)
        continue;

      size_t pointIndex = m_cachedTrajectory->trajectoryPositionToPointIndex(positionInTrajectory);
      const ThunderAutoOutputTrajectoryPoint& point = m_cachedTrajectory->points.at(pointIndex);
      Point2d positionPoint = point.position;
      checkHoverPoint(positionPoint, PointType::ROTATION_POSITION, i);

      Point2d rotationPoint = positionPoint.extendAtAngle(rotation.angle, m_settings->robotSize.length / 2.f);
      checkHoverPoint(rotationPoint, PointType::ROTATION_ANGLE, i);
    }
  }

  if (trajectoryEditorOptions.showActions) {
    const auto actions = skeleton.actions();
    i = 0;
    for (auto actionIt = actions.cbegin(); actionIt != actions.cend(); actionIt++, i++) {
      if (hoveredPointType != PointType::NONE)
        break;

      const auto& [positionInTrajectory, action] = *actionIt;

      if (action.editorLocked)
        continue;

      size_t pointIndex = m_cachedTrajectory->trajectoryPositionToPointIndex(positionInTrajectory);
      const ThunderAutoOutputTrajectoryPoint& point = m_cachedTrajectory->points.at(pointIndex);
      Point2d positionPoint = point.position;
      checkHoverPoint(positionPoint, PointType::ACTION_POSITION, i);
    }
  }

  // Context menus

  if (auto scopedPopup = ImGui::Scoped::PopupContextItem("TrajectoryEditorContextMenu_Field")) {
    if (auto scopedMenu = ImGui::Scoped::Menu(ICON_LC_PLUS "  Insert Point Here")) {
      if (ImGui::MenuItem("At Beginning")) {
        Point2d newWaypointPosition = m_contextMenuOpenData.mousePosition;
        CanonicalAngle newWaypointHeading = newWaypointPosition.angleTo(skeleton.front().position());

        state.currentTrajectoryPrependWaypoint(newWaypointPosition, newWaypointHeading);
        m_history.addState(state);
        invalidateCachedTrajectory();
      }
      if (ImGui::MenuItem("At End")) {
        Point2d newWaypointPosition = m_contextMenuOpenData.mousePosition;
        CanonicalAngle newWaypointHeading =
            newWaypointPosition.angleTo(skeleton.back().position()).supplementary();

        state.currentTrajectoryAppendWaypoint(newWaypointPosition, newWaypointHeading);
        m_history.addState(state);
        invalidateCachedTrajectory();
      }
      {
        auto scopedDisabled =
            ImGui::Scoped::Disabled(editorState.trajectorySelection !=
                                    ThunderAutoTrajectoryEditorState::TrajectorySelection::WAYPOINT);

        bool doInsertPoint = false;
        size_t newWaypointIndex;

        if (ImGui::MenuItem("Before Selected")) {
          doInsertPoint = true;
          newWaypointIndex = editorState.selectionIndex;
        }
        if (ImGui::MenuItem("After Selected")) {
          newWaypointIndex = editorState.selectionIndex + 1;
          doInsertPoint = true;
        }

        if (doInsertPoint) {
          Point2d newWaypointPosition = m_contextMenuOpenData.mousePosition;
          CanonicalAngle newWaypointHeading = 0_deg;  // TODO: Better heading
          state.currentTrajectoryInsertWaypoint(newWaypointIndex, newWaypointPosition, newWaypointHeading);
          m_history.addState(state);
          invalidateCachedTrajectory();
        }
      }
    }
    ImGui::Separator();
    if (ImGui::MenuItem(ICON_LC_ROTATE_CCW "  Reset View")) {
      resetView();
    }
  } else if (auto scopedPopup = ImGui::Scoped::PopupContextItem("TrajectoryEditorContextMenu_Waypoint")) {
    {
      auto scopedDisabled = ImGui::Scoped::Disabled(skeleton.numPoints() <= 2);

      if (ImGui::MenuItem(ICON_LC_TRASH "  Delete Waypoint")) {
        ThunderAutoAssert(editorState.trajectorySelection ==
                          ThunderAutoTrajectoryEditorState::TrajectorySelection::WAYPOINT);
        state.currentTrajectoryDeleteSelectedItem();
        m_history.addState(state);
        invalidateCachedTrajectory();
      }
    }
  } else if (auto scopedPopup = ImGui::Scoped::PopupContextItem("TrajectoryEditorContextMenu_Rotation")) {
    if (ImGui::MenuItem(ICON_LC_TRASH "  Delete Rotation")) {
      ThunderAutoAssert(editorState.trajectorySelection ==
                        ThunderAutoTrajectoryEditorState::TrajectorySelection::ROTATION);
      state.currentTrajectoryDeleteSelectedItem();
      m_history.addState(state);
      invalidateCachedTrajectory();
    }
  } else if (auto scopedPopup = ImGui::Scoped::PopupContextItem("TrajectoryEditorContextMenu_Action")) {
    if (ImGui::MenuItem(ICON_LC_TRASH "  Delete Action")) {
      ThunderAutoAssert(editorState.trajectorySelection ==
                        ThunderAutoTrajectoryEditorState::TrajectorySelection::ACTION);
      state.currentTrajectoryDeleteSelectedItem();
      m_history.addState(state);
    }
  } else if (auto scopedPopup = ImGui::Scoped::PopupContextItem("TrajectoryEditorContextMenu_Trajectory")) {
    if (ImGui::MenuItem(ICON_LC_PLUS "  Insert Waypoint Here")) {
      const ThunderAutoOutputTrajectoryPoint& point = m_contextMenuOpenData.trajectoryPointClosestToMouse;
      const size_t newWaypointIndex =
          static_cast<size_t>(std::floor(m_contextMenuOpenData.trajectoryPositionClosestToMouse)) + 1;
      state.currentTrajectoryInsertWaypoint(newWaypointIndex, point.position, point.heading);
      m_history.addState(state);
      invalidateCachedTrajectory();

    } else if (ImGui::MenuItem(ICON_LC_PLUS "  Insert Rotation Target Here")) {
      const ThunderAutoTrajectoryPosition newRotationPosition =
          m_contextMenuOpenData.trajectoryPositionClosestToMouse;
      const CanonicalAngle newRotationAngle = 0_deg;
      state.currentTrajectoryInsertRotation(newRotationPosition, newRotationAngle);
      m_history.addState(state);
      invalidateCachedTrajectory();

    } else if (auto scopedMenu = ImGui::Scoped::Menu(ICON_LC_PLUS "  Insert Action Here")) {
      std::span<const std::string> availableActions = state.actionsOrder;

      int selectedActionIndex = -1;

      if (auto scopedCombo = ImGui::Scoped::Combo("##Action Name", "")) {
        for (size_t actionIndex = 0; actionIndex < availableActions.size(); actionIndex++) {
          if (ImGui::Selectable(availableActions[actionIndex].c_str(), false)) {
            selectedActionIndex = static_cast<int>(actionIndex);
          }
        }
      }

      if (selectedActionIndex >= 0) {
        const ThunderAutoTrajectoryPosition newActionPosition =
            m_contextMenuOpenData.trajectoryPositionClosestToMouse;
        const std::string& newActionName = availableActions[selectedActionIndex];
        state.currentTrajectoryInsertAction(newActionPosition, newActionName);
        m_history.addState(state);
        ImGui::CloseCurrentPopup();
      }
    }
  }
  // If not showing any context menus, show tooltips instead.
  else if (ImGui::IsWindowFocused() && m_dragPoint == PointType::NONE &&
           trajectoryEditorOptions.showTooltip) {
    if (hoveredPointType == PointType::ACTION_POSITION) {
      ThunderAutoPositionedTrajectoryItemList<ThunderAutoTrajectoryAction>& actions = skeleton.actions();
      auto actionIt = std::next(actions.begin(), hoveredPointIndex);

      ImGui::SetTooltip("%s", actionIt->second.action.c_str());

    } else if (isTrajectoryHovered) {
      auto scopedTooltip = ImGui::Scoped::Tooltip();

      const ThunderAutoOutputTrajectoryPoint& point = trajectoryPointClosestToMouse;

      ImGui::Text("Time: %.2f s", point.time());
      ImGui::Text("Distance traveled: %.2f m", point.distance());
      ImGui::Text("Position: (%.2f m, %.2f m)", point.position.x(), point.position.y());
      ImGui::Text("Linear Velocity: %.2f m/s", point.linearVelocity());
      ImGui::Text("Centripetal Accel: %.2f m/s²", point.centripetalAcceleration());
      ImGui::Text("Rotation: %.2f deg", point.rotation.degrees()());
      ImGui::Text("Angular Velocity: %.2f deg/s", units::degrees_per_second_t(point.angularVelocity)());
      ImGui::Text("Curvature: %.2f", point.curvature());
      ImGui::Text("Radius of Curvature: %.2f m", gcem::pow(point.curvature(), -1));
    }
  }

  auto openContextMenu = [&](const char* id) {
    m_contextMenuOpenData.trajectoryPositionClosestToMouse = trajectoryPositionClosestToMouse;
    m_contextMenuOpenData.trajectoryPointClosestToMouse = trajectoryPointClosestToMouse;
    m_contextMenuOpenData.mousePosition = mousePosition;
    ImGui::OpenPopup(id);
  };

  // Keyboard input

  if (ImGui::IsWindowFocused() && m_dragPoint == PointType::NONE) {
    const bool isDeletePressed = IsKeyPressed(ImGuiKey_Delete) || IsKeyPressed(ImGuiKey_Backspace);

    // const bool isItemSelected = (editorState.trajectorySelection ==
    //                                       ThunderAutoTrajectoryEditorState::TrajectorySelection::NONE);

    const bool isWaypointDeleteBlocked = (editorState.trajectorySelection ==
                                          ThunderAutoTrajectoryEditorState::TrajectorySelection::WAYPOINT) &&
                                         skeleton.numPoints() <= 2;

    if (isDeletePressed && !isWaypointDeleteBlocked) {
      if (state.currentTrajectoryDeleteSelectedItem()) {
        m_history.addState(state);
        invalidateCachedTrajectory();
      }
    }

    const bool isTabPressed = IsKeyPressed(ImGuiKey_Tab);

    if (isTabPressed) {
      if (io.KeyShift) {
        if (state.currentTrajectoryIncrementSelectedItemIndex(false)) {
          m_history.addState(state, false);
          invalidateCachedTrajectory();
        }
      } else {
        if (state.currentTrajectoryIncrementSelectedItemIndex(true)) {
          m_history.addState(state, false);
          invalidateCachedTrajectory();
        }
      }
    }
  }

  // Handle clicking

  if (!ImGui::IsWindowHovered()) {
    if (m_dragPoint != PointType::NONE) {
      // End drag.

      if (m_dragPoint == PointType::ROTATION_POSITION || m_dragPoint == PointType::WAYPOINT_POSITION ||
          m_dragPoint == PointType::WAYPOINT_HEADING_IN || m_dragPoint == PointType::WAYPOINT_HEADING_OUT) {
        std::unique_ptr<ThunderAutoPartialOutputTrajectory> trajectoryPositionData =
            BuildThunderAutoPartialOutputTrajectory(skeleton, kPreviewOutputTrajectorySettings);

        skeleton.separateRotations(kMinRotationTargetSeparation, trajectoryPositionData.get());
      }

      if (m_dragPoint == PointType::WAYPOINT_POSITION) {
        state.trajectoryUpdateLinkedWaypointsFromSelected();
      }
      m_history.addState(state);
      m_history.finishLongEdit();

      m_isPlaying = m_wasPlaying;

      invalidateCachedTrajectory();
    }

    m_dragPoint = PointType::NONE;
    m_clickedPoint = PointType::NONE;

    return;
  }

  bool isLeftDoubleClicked = ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
  bool isLeftClicked = !isLeftDoubleClicked && ImGui::IsMouseClicked(ImGuiMouseButton_Left);
  bool isRightClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Right);
  bool isHoveringRightClickablePoint = false;
  if (hoveredPointType == PointType::WAYPOINT_POSITION || hoveredPointType == PointType::ROTATION_POSITION ||
      hoveredPointType == PointType::ACTION_POSITION) {
    isHoveringRightClickablePoint = true;
  }

  if (isLeftClicked || (isRightClicked && isHoveringRightClickablePoint)) {
    ThunderAutoTrajectoryEditorState::TrajectorySelection previousTrajectorySelection =
        editorState.trajectorySelection;
    size_t previousSelectionIndex = editorState.selectionIndex;

    editorState.trajectorySelection = ToTrajectorySelection(hoveredPointType);
    editorState.selectionIndex = hoveredPointIndex;

    m_clickedPoint = hoveredPointType;

    if ((editorState.trajectorySelection != previousTrajectorySelection) ||
        (editorState.selectionIndex != previousSelectionIndex)) {
      m_history.addState(state, false);
    }

    if (isRightClicked) {
      if (hoveredPointType == PointType::WAYPOINT_POSITION) {
        openContextMenu("TrajectoryEditorContextMenu_Waypoint");
      } else if (hoveredPointType == PointType::ROTATION_POSITION) {
        openContextMenu("TrajectoryEditorContextMenu_Rotation");
      } else if (hoveredPointType == PointType::ACTION_POSITION) {
        openContextMenu("TrajectoryEditorContextMenu_Action");
      }
    }
  } else if (isRightClicked) {
    if (isTrajectoryHovered) {
      openContextMenu("TrajectoryEditorContextMenu_Trajectory");
    } else {
      openContextMenu("TrajectoryEditorContextMenu_Field");
    }
  } else if (isLeftDoubleClicked && !isTrajectoryHovered) {
    // Add new point to the end of the trajectory.

    Point2d newWaypointPosition = mousePosition;
    CanonicalAngle newWaypointHeading =
        newWaypointPosition.angleTo(skeleton.back().position()).supplementary();

    ThunderAutoLogger::Info("Appending waypoint at ({}, {}), {} deg (double click)", newWaypointPosition.x(),
                            newWaypointPosition.y(), newWaypointHeading.degrees()());

    state.currentTrajectoryAppendWaypoint(newWaypointPosition, newWaypointHeading);
    m_history.addState(state);
    invalidateCachedTrajectory();
  }

  // Handle dragging

  if (m_clickedPoint != PointType::NONE || m_dragPoint != PointType::NONE) {
    // Dragging.
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
      // TODO: Update linked waypoins while dragging? Maybe once links are improved so operation is not O(n^2)

      if (m_clickedPoint != PointType::NONE) {
        // Begin drag.
        m_history.startLongEdit();

        m_wasPlaying = m_isPlaying;
        m_isPlaying = false;
        m_playbackTime = 0.0_s;

        m_dragPoint = m_clickedPoint;
      }

      const Point2d dragPosition = ToFieldCoordinate(io.MousePos, m_settings->fieldImage, bb);

      switch (m_dragPoint) {
        using enum PointType;
        case WAYPOINT_POSITION: {
          ThunderAutoTrajectorySkeletonWaypoint& waypoint = skeleton.getPoint(editorState.selectionIndex);
          waypoint.setPosition(dragPosition);
          ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
          break;
        }
        case WAYPOINT_ANGLE: {
          ThunderAutoTrajectorySkeletonWaypoint& waypoint = skeleton.getPoint(editorState.selectionIndex);
          CanonicalAngle angle = waypoint.position().angleTo(dragPosition);
          if (editorState.selectionIndex == 0) {
            skeleton.setStartRotation(angle);
          } else if (editorState.selectionIndex == skeleton.numPoints() - 1) {
            skeleton.setEndRotation(angle);
          } else {
            ThunderAutoAssert(waypoint.isStopped());
            waypoint.setStopRotation(angle);
          }
          ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
          break;
        }
        case WAYPOINT_HEADING_IN: {
          ThunderAutoTrajectorySkeletonWaypoint& waypoint = skeleton.getPoint(editorState.selectionIndex);
          waypoint.setIncomingHeadingControlPoint(dragPosition);
          ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
          break;
        }
        case WAYPOINT_HEADING_OUT: {
          ThunderAutoTrajectorySkeletonWaypoint& waypoint = skeleton.getPoint(editorState.selectionIndex);
          waypoint.setOutgoingHeadingControlPoint(dragPosition);
          ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
          break;
        }
        case ROTATION_POSITION: {
          ThunderAutoPositionedTrajectoryItemList<ThunderAutoTrajectoryRotation>& rotations =
              skeleton.rotations();
          auto rotationIt = std::next(rotations.begin(), editorState.selectionIndex);

          ThunderAutoTrajectoryPosition newPosition = trajectoryPositionClosestToMouse;

          auto newRotationIt = rotations.move(rotationIt, newPosition);
          editorState.selectionIndex = std::distance(rotations.begin(), newRotationIt);
          SetMouseCursorMoveDirection(trajectoryPointClosestToMouse.heading);
          break;
        }
        case ROTATION_ANGLE: {
          ThunderAutoPositionedTrajectoryItemList<ThunderAutoTrajectoryRotation>& rotations =
              skeleton.rotations();
          auto rotationIt = std::next(rotations.begin(), editorState.selectionIndex);
          ThunderAutoTrajectoryPosition positionInTrajectory = rotationIt->first;
          size_t rotationCenterPointIndex =
              m_cachedTrajectory->trajectoryPositionToPointIndex(positionInTrajectory);
          const ThunderAutoOutputTrajectoryPoint& rotationCenterPoint =
              m_cachedTrajectory->points.at(rotationCenterPointIndex);
          CanonicalAngle newAngle = rotationCenterPoint.position.angleTo(dragPosition);
          rotationIt->second.angle = newAngle;
          ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
          break;
        }
        case ACTION_POSITION: {
          ThunderAutoPositionedTrajectoryItemList<ThunderAutoTrajectoryAction>& actions = skeleton.actions();
          auto actionIt = std::next(actions.begin(), editorState.selectionIndex);
          auto newActionIt = actions.move(actionIt, trajectoryPositionClosestToMouse);
          editorState.selectionIndex = std::distance(actions.begin(), newActionIt);
          SetMouseCursorMoveDirection(trajectoryPointClosestToMouse.heading);
          break;
        }
        case NONE:
        default:
          ThunderAutoUnreachable("Invalid drag point type");
      }

      m_history.addState(state);

      m_clickedPoint = PointType::NONE;

      invalidateCachedTrajectory();
    }

    // Release.
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
      if (m_dragPoint != PointType::NONE) {
        // End drag.

        if (m_dragPoint == PointType::ROTATION_POSITION || m_dragPoint == PointType::WAYPOINT_POSITION ||
            m_dragPoint == PointType::WAYPOINT_HEADING_IN || m_dragPoint == PointType::WAYPOINT_HEADING_OUT) {
          std::unique_ptr<ThunderAutoPartialOutputTrajectory> trajectoryPositionData =
              BuildThunderAutoPartialOutputTrajectory(skeleton, kPreviewOutputTrajectorySettings);

          skeleton.separateRotations(kMinRotationTargetSeparation, trajectoryPositionData.get());
        }

        if (m_dragPoint == PointType::WAYPOINT_POSITION) {
          state.trajectoryUpdateLinkedWaypointsFromSelected();
        }
        m_history.addState(state);
        m_history.finishLongEdit();

        m_isPlaying = m_wasPlaying;

        invalidateCachedTrajectory();
      }

      m_dragPoint = PointType::NONE;
      m_clickedPoint = PointType::NONE;
    }
  }
}

// hi ishan!!!
