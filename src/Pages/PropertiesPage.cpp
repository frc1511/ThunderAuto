#include <ThunderAuto/Pages/PropertiesPage.hpp>

#include <ThunderAuto/ImGuiScopedField.hpp>
#include <ThunderAuto/FontLibrary.hpp>
#include <ThunderAuto/ColorPalette.hpp>
#include <ThunderAuto/Platform/Platform.hpp>
#include <ThunderAuto/App.hpp>
#include <IconsFontAwesome5.h>
#include <imgui_raii.h>
#include <fmt/format.h>
#include <limits>

static const ImU32 kWarningTextColor = IM_COL32(255, 242, 0, 255);

static const float kFieldPositionSliderSpeed = 0.1f;
static const float kAngleSliderSpeed = 1.f;
static const float kVelocitySliderSpeed = 0.025f;
static const float kTrajectoryPositionSliderSpeed = 0.025f;

void PropertiesPage::present(bool* running) {
  m_event = Event::NONE;

  ImGui::SetNextWindowSize(
      ImVec2(GET_UISIZE(PROPERTIES_PAGE_START_WIDTH), GET_UISIZE(PROPERTIES_PAGE_START_HEIGHT)),
      ImGuiCond_FirstUseEver);
  ImGui::Scoped scopedWindow = ImGui::Scoped::Window(name(), running);
  if (!scopedWindow || !*running)
    return;

  ThunderAutoProjectState state = m_history.currentState();

  ThunderAutoEditorState& editorState = state.editorState;
  switch (editorState.view) {
    using enum ThunderAutoEditorState::View;
    case TRAJECTORY:
      presentTrajectoryProperties(state);
      break;
    case AUTO_MODE:
      presentAutoModeProperties(state);
      break;
    case NONE:
      break;
    default:
      ThunderAutoUnreachable("Unknown editor view");
  }
}

void PropertiesPage::presentTrajectoryProperties(ThunderAutoProjectState& state) {
  ThunderAutoTrajectoryEditorState& editorState = state.editorState.trajectoryEditorState;
  if (editorState.currentTrajectoryName.empty())
    return;

  presentTrajectoryItemList(state);
  presentTrajectorySelectedItemProperties(state);
  presentTrajectoryOtherProperties(state);
  presentTrajectorySpeedConstraintProperties(state);
}

void PropertiesPage::presentTrajectoryItemList(ThunderAutoProjectState& state) {
  presentSeparatorText("Trajectory Items");

  auto scopedID = ImGui::Scoped::ID("Trajectory Items List");

  if (auto scopedTabBar = ImGui::Scoped::TabBar("TrajectoryItems")) {
    const char* const childWindowName = "Trajectory Items Child Window";

    ThunderAutoTrajectoryEditorState& editorState = state.editorState.trajectoryEditorState;
    ThunderAutoTrajectorySkeleton& skeleton = state.currentTrajectory();

    // Don't push id on tabs so that child window is the same for all tabs (user can change the size of one,
    // changes it for all).

    if (auto scopedTabItem = ImGui::Scoped::TabItem("Points", nullptr, ImGuiTabItemFlags_NoPushId)) {
      auto scopedChildWindow = ImGui::Scoped::ChildWindow(
          childWindowName,
          ImVec2(0.f, GET_UISIZE(PROPERTIES_PAGE_TRAJECTORY_ITEM_LIST_CHILD_WINDOW_START_SIZE_Y)),
          ImGuiChildFlags_ResizeY | ImGuiChildFlags_Borders);
      auto scopedPadding =
          ImGui::Scoped::StyleVarY(ImGuiStyleVar_ItemSpacing, GET_UISIZE(SELECTABLE_LIST_ITEM_SPACING_Y));

      size_t pointIndex = 0;
      for (auto pointIt = skeleton.begin(); pointIt != skeleton.end(); pointIt++, pointIndex++) {
        auto scopedID = ImGui::Scoped::ID(pointIndex);

        std::string selectableTitle = fmt::format("{}:    ({:.2f} m, {:.2f} m)", pointIndex,
                                                  pointIt->position().x(), pointIt->position().y());

        const bool isPointLinked = pointIt->isLinked();

        if (isPointLinked) {
          selectableTitle += fmt::format("  {}  {}", ICON_FA_ARROW_RIGHT, pointIt->linkName());
        }

        const bool isPointSelected = (editorState.trajectorySelection ==
                                      ThunderAutoTrajectoryEditorState::TrajectorySelection::WAYPOINT) &&
                                     (editorState.selectionIndex == pointIndex);

        if (ImGui::Selectable(selectableTitle.c_str(), isPointSelected, ImGuiSelectableFlags_AllowOverlap)) {
          editorState.trajectorySelection = ThunderAutoTrajectoryEditorState::TrajectorySelection::WAYPOINT;
          editorState.selectionIndex = pointIndex;
          m_history.addState(state, false);
        }

        const bool isPointLocked = pointIt->isEditorLocked();

        // Right-click the point.
        if (auto scopedContextMenu = ImGui::Scoped::PopupContextItem()) {
          if (!isPointSelected) {
            editorState.trajectorySelection = ThunderAutoTrajectoryEditorState::TrajectorySelection::WAYPOINT;
            editorState.selectionIndex = pointIndex;
            m_history.addState(state, false);
          }

          if (ImGui::MenuItem(ICON_FA_TRASH "  Delete Point")) {
            state.currentTrajectoryDeleteSelectedItem();
            m_history.addState(state);
            m_editorPage.invalidateCachedTrajectory();
            m_editorPage.resetPlayback();
            break;
          }

          if (isPointLinked) {
            if (ImGui::MenuItem(ICON_FA_LINK "  Edit Link")) {
              m_event = Event::TRAJECTORY_POINT_LINK;
            }
            if (ImGui::MenuItem(ICON_FA_UNLINK "  Remove Link")) {
              pointIt->removeLink();
              m_history.addState(state);
            }
          } else {
            if (ImGui::MenuItem(ICON_FA_LINK "  Link")) {
              m_event = Event::TRAJECTORY_POINT_LINK;
            }
          }

          const char* lockedMenuItemText =
              isPointLocked ? ICON_FA_UNLOCK "  Unlock in Editor" : ICON_FA_LOCK "  Lock in Editor";
          if (ImGui::MenuItem(lockedMenuItemText)) {
            state.currentTrajectoryToggleEditorLockedForSelectedItem();
            m_history.addState(state);
          }
        }

        if (isPointLocked) {
          ImGui::SameLine();

          const ImGuiStyle& style = ImGui::GetStyle();
          const float lockedButtonWidthNeeded = ImGui::CalcTextSize(ICON_FA_LOCK).x + style.ItemSpacing.x;
          const float lockedButtonCursorOffset = ImGui::GetContentRegionAvail().x - lockedButtonWidthNeeded;
          if (lockedButtonCursorOffset > 0) {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + lockedButtonCursorOffset);
          }

          auto scopedButtonColor = ImGui::Scoped::StyleColor(ImGuiCol_Button, 0);
          auto scopedButtonHoveredColor = ImGui::Scoped::StyleColor(ImGuiCol_ButtonHovered, 0);
          auto scopedButtonActiveColor = ImGui::Scoped::StyleColor(ImGuiCol_ButtonActive, 0);

          if (ImGui::SmallButton(ICON_FA_LOCK)) {
            if (!isPointSelected) {
              editorState.trajectorySelection =
                  ThunderAutoTrajectoryEditorState::TrajectorySelection::WAYPOINT;
              editorState.selectionIndex = pointIndex;
              m_history.addState(state, false);
            }

            state.currentTrajectoryToggleEditorLockedForSelectedItem();
            m_history.addState(state);
          }
          if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
            ImGui::SetTooltip("Unlock in Editor");
          }
        }
      }
    }
    if (auto scopedTabItem = ImGui::Scoped::TabItem("Rotations", nullptr, ImGuiTabItemFlags_NoPushId)) {
      auto scopedChildWindow = ImGui::Scoped::ChildWindow(
          childWindowName,
          ImVec2(0.f, GET_UISIZE(PROPERTIES_PAGE_TRAJECTORY_ITEM_LIST_CHILD_WINDOW_START_SIZE_Y)),
          ImGuiChildFlags_ResizeY | ImGuiChildFlags_Borders);
      auto scopedPadding =
          ImGui::Scoped::StyleVarY(ImGuiStyleVar_ItemSpacing, GET_UISIZE(SELECTABLE_LIST_ITEM_SPACING_Y));

      std::map<ThunderAutoTrajectoryPosition, TrajectoryItemSelection<CanonicalAngle>> rotations =
          GetAllRotationSelections(skeleton);

      size_t i = 0;
      for (const auto& [position, selection] : rotations) {
        auto scopedID = ImGui::Scoped::ID(i++);

        const CanonicalAngle& angle = selection.item;
        std::string selectableTitle =
            fmt::format("{:.2f}:    {:.0f} deg", (double)position, angle.degrees().value());

        const bool isRotationSelected = (editorState.trajectorySelection == selection.trajectorySelection) &&
                                        (editorState.selectionIndex == selection.selectionIndex);

        if (ImGui::Selectable(selectableTitle.c_str(), isRotationSelected,
                              ImGuiSelectableFlags_AllowOverlap)) {
          editorState.trajectorySelection = selection.trajectorySelection;
          editorState.selectionIndex = selection.selectionIndex;
          m_history.addState(state, false);
        }

        // Right-click the rotation.
        if (auto scopedContextMenu = ImGui::Scoped::PopupContextItem()) {
          if (!isRotationSelected) {
            editorState.trajectorySelection = selection.trajectorySelection;
            editorState.selectionIndex = selection.selectionIndex;
            m_history.addState(state, false);
          }

          const std::string deleteMenuItemText = fmt::format(
              "{}  Delete {}", ICON_FA_TRASH, TrajectorySelectionToString(selection.trajectorySelection));

          if (ImGui::MenuItem(deleteMenuItemText.c_str())) {
            state.currentTrajectoryDeleteSelectedItem();
            m_history.addState(state);
            m_editorPage.invalidateCachedTrajectory();
            m_editorPage.resetPlayback();
            break;
          }

          const bool locked = selection.editorLocked;
          const char* lockedMenuItemText =
              locked ? ICON_FA_UNLOCK "  Unlock in Editor" : ICON_FA_LOCK "  Lock in Editor";
          if (ImGui::MenuItem(lockedMenuItemText)) {
            state.currentTrajectoryToggleEditorLockedForSelectedItem();
            m_history.addState(state);
          }
        }

        if (selection.editorLocked) {
          ImGui::SameLine();

          const ImGuiStyle& style = ImGui::GetStyle();
          const float lockedButtonWidthNeeded = ImGui::CalcTextSize(ICON_FA_LOCK).x + style.ItemSpacing.x;
          const float lockedButtonCursorOffset = ImGui::GetContentRegionAvail().x - lockedButtonWidthNeeded;
          if (lockedButtonCursorOffset > 0) {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + lockedButtonCursorOffset);
          }

          auto scopedButtonColor = ImGui::Scoped::StyleColor(ImGuiCol_Button, 0);
          auto scopedButtonHoveredColor = ImGui::Scoped::StyleColor(ImGuiCol_ButtonHovered, 0);
          auto scopedButtonActiveColor = ImGui::Scoped::StyleColor(ImGuiCol_ButtonActive, 0);

          if (ImGui::SmallButton(ICON_FA_LOCK)) {
            if (!isRotationSelected) {
              editorState.trajectorySelection = selection.trajectorySelection;
              editorState.selectionIndex = selection.selectionIndex;
              m_history.addState(state, false);
            }

            state.currentTrajectoryToggleEditorLockedForSelectedItem();
            m_history.addState(state);
          }
          if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
            ImGui::SetTooltip("Unlock in Editor");
          }
        }
      }
    }
    if (auto scopedTabItem = ImGui::Scoped::TabItem("Actions", nullptr, ImGuiTabItemFlags_NoPushId)) {
      auto scopedChildWindow = ImGui::Scoped::ChildWindow(
          childWindowName,
          ImVec2(0.f, GET_UISIZE(PROPERTIES_PAGE_TRAJECTORY_ITEM_LIST_CHILD_WINDOW_START_SIZE_Y)),
          ImGuiChildFlags_ResizeY | ImGuiChildFlags_Borders);
      auto scopedPadding =
          ImGui::Scoped::StyleVarY(ImGuiStyleVar_ItemSpacing, GET_UISIZE(SELECTABLE_LIST_ITEM_SPACING_Y));

      std::multimap<ThunderAutoTrajectoryPosition, TrajectoryItemSelection<std::string>> actions =
          GetAllActionSelections(skeleton);

      for (const auto& [position, selection] : actions) {
        const std::string& action = selection.item;
        std::string selectableTitle = fmt::format("{:.2f}:    {}", (double)position, action);

        const bool isActionSelected = (editorState.trajectorySelection == selection.trajectorySelection) &&
                                      (editorState.selectionIndex == selection.selectionIndex);

        if (ImGui::Selectable(selectableTitle.c_str(), isActionSelected, ImGuiSelectableFlags_AllowOverlap)) {
          editorState.trajectorySelection = selection.trajectorySelection;
          editorState.selectionIndex = selection.selectionIndex;
          m_history.addState(state, false);
        }

        const bool isFirstPoint = (selection.trajectorySelection ==
                                   ThunderAutoTrajectoryEditorState::TrajectorySelection::WAYPOINT) &&
                                  (selection.selectionIndex == 0);
        const bool isLastPoint = (selection.trajectorySelection ==
                                  ThunderAutoTrajectoryEditorState::TrajectorySelection::WAYPOINT) &&
                                 (selection.selectionIndex == skeleton.numPoints() - 1);

        if (!isFirstPoint && !isLastPoint) {
          // Right-click the action.
          if (auto scopedContextMenu = ImGui::Scoped::PopupContextItem()) {
            if (!isActionSelected) {
              editorState.trajectorySelection = selection.trajectorySelection;
              editorState.selectionIndex = selection.selectionIndex;
              m_history.addState(state, false);
            }

            const std::string deleteMenuItemText = fmt::format(
                "{}  Delete {}", ICON_FA_TRASH, TrajectorySelectionToString(selection.trajectorySelection));

            if (ImGui::MenuItem(deleteMenuItemText.c_str())) {
              state.currentTrajectoryDeleteSelectedItem();
              m_history.addState(state);
              m_editorPage.invalidateCachedTrajectory();
              m_editorPage.resetPlayback();
              break;
            }

            const bool locked = selection.editorLocked;
            const char* lockedMenuItemText =
                locked ? ICON_FA_UNLOCK "  Unlock in Editor" : ICON_FA_LOCK "  Lock in Editor";
            if (ImGui::MenuItem(lockedMenuItemText)) {
              state.currentTrajectoryToggleEditorLockedForSelectedItem();
              m_history.addState(state);
            }
          }

          if (selection.editorLocked) {
            ImGui::SameLine();

            const ImGuiStyle& style = ImGui::GetStyle();
            const float lockedButtonWidthNeeded = ImGui::CalcTextSize(ICON_FA_LOCK).x + style.ItemSpacing.x;
            const float lockedButtonCursorOffset = ImGui::GetContentRegionAvail().x - lockedButtonWidthNeeded;
            if (lockedButtonCursorOffset > 0) {
              ImGui::SetCursorPosX(ImGui::GetCursorPosX() + lockedButtonCursorOffset);
            }

            auto scopedButtonColor = ImGui::Scoped::StyleColor(ImGuiCol_Button, 0);
            auto scopedButtonHoveredColor = ImGui::Scoped::StyleColor(ImGuiCol_ButtonHovered, 0);
            auto scopedButtonActiveColor = ImGui::Scoped::StyleColor(ImGuiCol_ButtonActive, 0);

            if (ImGui::SmallButton(ICON_FA_LOCK)) {
              if (!isActionSelected) {
                editorState.trajectorySelection = selection.trajectorySelection;
                editorState.selectionIndex = selection.selectionIndex;
                m_history.addState(state, false);
              }

              state.currentTrajectoryToggleEditorLockedForSelectedItem();
              m_history.addState(state);
            }
            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
              ImGui::SetTooltip("Unlock in Editor");
            }
          }
        }
      }
    }
  }

  ImGui::Spacing();
}

void PropertiesPage::presentTrajectorySelectedItemProperties(ThunderAutoProjectState& state) {
  ThunderAutoTrajectoryEditorState& editorState = state.editorState.trajectoryEditorState;

  presentSeparatorText("Selected Trajectory Item");

  auto scopedID = ImGui::Scoped::ID("Selected Trajectory Item Properties");

  switch (editorState.trajectorySelection) {
    using enum ThunderAutoTrajectoryEditorState::TrajectorySelection;
    case NONE:
      ImGui::Text("No item selected");
      break;
    case WAYPOINT:
      presentTrajectorySelectedPointProperties(state);
      break;
    case ROTATION:
      presentTrajectorySelectedRotationProperties(state);
      break;
    case ACTION:
      presentTrajectorySelectedActionProperties(state);
      break;
    default:
      ThunderAutoUnreachable("Invalid trajectory selection");
  }
}

void PropertiesPage::presentTrajectorySelectedPointProperties(ThunderAutoProjectState& state) {
  ThunderAutoTrajectorySkeleton& skeleton = state.currentTrajectory();
  ThunderAutoTrajectoryEditorState& editorState = state.editorState.trajectoryEditorState;

  const bool isFirstPoint = (editorState.selectionIndex == 0);
  const bool isLastPoint = (editorState.selectionIndex == skeleton.numPoints() - 1);
  ThunderAutoAssert(!(isFirstPoint && isLastPoint));

  ThunderAutoTrajectorySkeletonWaypoint& point = skeleton.getPoint(editorState.selectionIndex);

  bool changed = false;

  // Position
  bool positionChanged = presentPointPositionProperties(point);
  if (positionChanged) {
    changed = true;
    state.trajectoryUpdateLinkedWaypointsFromSelected();
  }

  ImGui::Separator();

  // Heading
  changed |= presentPointHeadingProperties(point, isFirstPoint || isLastPoint);

  const bool showIncomingWeights = !isFirstPoint;
  const bool showOutgoingWeights = !isLastPoint;

  // Heading weights.
  changed |= presentPointWeightProperties(point, showIncomingWeights, showOutgoingWeights);

  // Start/End rotations & actions
  if (isFirstPoint) {
    ImGui::Separator();
    changed |= presentTrajectoryStartRotationProperty(skeleton);
    changed |= presentTrajectoryStartActionProperty(skeleton, state);

  } else if (isLastPoint) {
    ImGui::Separator();
    changed |= presentTrajectoryEndRotationProperty(skeleton);
    changed |= presentTrajectoryEndActionProperty(skeleton, state);
  }

  ImGui::Separator();

  // Max velocity override.
  changed |= presentPointVelocityOverrideProperty(point, isFirstPoint, isLastPoint,
                                                  skeleton.settings().maxLinearVelocity);

  if (!isFirstPoint && !isLastPoint && point.isStopped()) {
    changed |= presentPointStopRotationProperty(point);
    changed |= presentPointStopActionProperty(point, state);
  }

  if (changed) {
    m_history.addState(state);
    m_editorPage.invalidateCachedTrajectory();
    m_editorPage.resetPlayback();
  }
}

bool PropertiesPage::presentPointPositionProperties(ThunderAutoTrajectorySkeletonWaypoint& point) {
  Point2d position = point.position();

  double x = position.x();
  double y = position.y();

  bool changed = false;
  {
    auto scopedField = ImGui::ScopedField::Builder("Position").build();
    auto scopedSpacingX = ImGui::Scoped::StyleVarX(ImGuiStyleVar_ItemSpacing, 0.f);

    const float lineHeight = ImGui::GetStyle().FontSizeBase + GImGui->Style.FramePadding.y * 2.0f;
    const float sliderWidth = (ImGui::GetContentRegionAvail().x - lineHeight * 2.f) / 2.f;

    const ImVec2 buttonSize = {lineHeight + 3.f, lineHeight};

    presentColoredUnclickableButton("X", buttonSize, ThunderAutoColorPalette::kRed);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
      ImGui::SetTooltip("X Position");
    }

    ImGui::SameLine();

    ImGui::SetNextItemWidth(sliderWidth);
    changed |= presentSlider("##X Position", x, kFieldPositionSliderSpeed, "%.2f m");

    ImGui::SameLine();

    presentColoredUnclickableButton("Y", buttonSize, ThunderAutoColorPalette::kGreen);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
      ImGui::SetTooltip("Y Position");
    }

    ImGui::SameLine();

    ImGui::SetNextItemWidth(sliderWidth);
    changed |= presentSlider("##Y Position", y, kFieldPositionSliderSpeed, "%.2f m");
  }

  position = Point2d(units::meter_t{x}, units::meter_t{y});
  point.setPosition(position);

  return changed;
}

bool PropertiesPage::presentPointHeadingProperties(ThunderAutoTrajectorySkeletonWaypoint& point,
                                                   bool isEndPoint) {
  ThunderAutoTrajectorySkeletonWaypoint::HeadingAngles headings = point.headings();

  bool changed = false;

  if (!isEndPoint && point.isStopped()) {
    auto scopedField = ImGui::ScopedField::Builder("Headings").build();
    auto scopedSpacingX = ImGui::Scoped::StyleVarX(ImGuiStyleVar_ItemSpacing, 0.f);

    double incomingHeading = headings.incomingAngle().supplementary().degrees().value();
    double outgoingHeading = headings.outgoingAngle().degrees().value();
    const double lastIncomingHeading = incomingHeading, lastOutgoingHeading = outgoingHeading;

    const float lineHeight = ImGui::GetStyle().FontSizeBase + GImGui->Style.FramePadding.y * 2.0f;
    const float sliderWidth = (ImGui::GetContentRegionAvail().x - lineHeight * 2.f) / 2.f;

    const ImVec2 buttonSize = {lineHeight + 3.f, lineHeight};

    presentColoredUnclickableButton(ICON_FA_ARROW_LEFT, buttonSize, ThunderAutoColorPalette::kBlue);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
      ImGui::SetTooltip("Incoming Heading");
    }

    ImGui::SameLine();

    ImGui::SetNextItemWidth(sliderWidth);
    changed |= presentSlider("##Incoming Heading", incomingHeading, kAngleSliderSpeed, "%.2f°");

    ImGui::SameLine();

    presentColoredUnclickableButton(ICON_FA_ARROW_RIGHT, buttonSize, ThunderAutoColorPalette::kOrange);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
      ImGui::SetTooltip("Outgoing Heading");
    }

    ImGui::SameLine();

    ImGui::SetNextItemWidth(sliderWidth);
    changed |= presentSlider("##Outgoing Heading", outgoingHeading, kAngleSliderSpeed, "%.2f°");

    if (incomingHeading != lastIncomingHeading) {
      CanonicalAngle angle = CanonicalAngle(units::degree_t(incomingHeading)).supplementary();
      headings.setIncomingAngle(angle, false);
    }
    if (outgoingHeading != lastOutgoingHeading) {
      CanonicalAngle angle{units::degree_t(outgoingHeading)};
      headings.setOutgoingAngle(angle, false);
    }
  } else {
    auto scopedField = ImGui::ScopedField::Builder("Heading").build();

    double heading = headings.outgoingAngle().degrees().value();
    changed |= presentSlider("##Heading", heading, kAngleSliderSpeed, "%.2f°");

    headings.setOutgoingAngle(CanonicalAngle(units::degree_t(heading)), true);
  }

  point.setHeadings(headings);

  return changed;
}

bool PropertiesPage::presentPointWeightProperties(ThunderAutoTrajectorySkeletonWaypoint& point,
                                                  bool showIncomingWeight,
                                                  bool showOutgoingWeight) {
  ThunderAutoTrajectorySkeletonWaypoint::HeadingWeights weights = point.headingWeights();

  bool changed = false;

  if (showIncomingWeight && showOutgoingWeight) {
    auto scopedField = ImGui::ScopedField::Builder("Weights").build();
    auto scopedSpacingX = ImGui::Scoped::StyleVarX(ImGuiStyleVar_ItemSpacing, 0.f);

    double incomingWeight = weights.incomingWeight();
    double outgoingWeight = weights.outgoingWeight();

    const float lineHeight = ImGui::GetStyle().FontSizeBase + GImGui->Style.FramePadding.y * 2.0f;
    const float sliderWidth = (ImGui::GetContentRegionAvail().x - lineHeight * 2.f) / 2.f;

    const ImVec2 buttonSize = {lineHeight + 3.f, lineHeight};

    presentColoredUnclickableButton(ICON_FA_ARROW_LEFT, buttonSize, ThunderAutoColorPalette::kBlue);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
      ImGui::SetTooltip("Incoming Weight");
    }

    ImGui::SameLine();

    ImGui::SetNextItemWidth(sliderWidth);
    changed |= presentSlider("##Incoming Weight", incomingWeight, kFieldPositionSliderSpeed, "%.2f m");

    ImGui::SameLine();

    presentColoredUnclickableButton(ICON_FA_ARROW_RIGHT, buttonSize, ThunderAutoColorPalette::kOrange);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
      ImGui::SetTooltip("Outgoing Weight");
    }

    ImGui::SameLine();

    ImGui::SetNextItemWidth(sliderWidth);
    changed |= presentSlider("##Outgoing Weight", outgoingWeight, kFieldPositionSliderSpeed, "%.2f m");

    weights.setIncomingWeight(incomingWeight);
    weights.setOutgoingWeight(outgoingWeight);

  } else if (showIncomingWeight) {
    auto scopedField = ImGui::ScopedField::Builder("Incoming Weight").build();

    double weight = weights.incomingWeight();
    changed |= presentSlider("##Heading Weight", weight, kFieldPositionSliderSpeed, "%.2f m");

    weights.setIncomingWeight(weight);

  } else if (showOutgoingWeight) {
    auto scopedField = ImGui::ScopedField::Builder("Outgoing Weight").build();

    double weight = weights.outgoingWeight();
    changed |= presentSlider("##Heading Weight", weight, kFieldPositionSliderSpeed, "%.2f m");

    weights.setOutgoingWeight(weight);
  }

  point.setHeadingWeights(weights);

  return changed;
}

bool PropertiesPage::presentPointVelocityOverrideProperty(
    ThunderAutoTrajectorySkeletonWaypoint& point,
    bool isFirstPoint,
    bool isLastPoint,
    units::meters_per_second_t maxLinearVelocitySetting) {
  bool changed = false;

  auto scopedField =
      ImGui::ScopedField::Builder("Velocity Override").tooltip("Maximum velocity at this point").build();

  ImGui::SetNextItemWidth(ImGui::CalcItemWidth());

  bool doOverrideVelocity = point.hasMaxVelocityOverride();
  if (ImGui::Checkbox("##Do Velocity Override", &doOverrideVelocity)) {
    if (doOverrideVelocity) {
      point.setMaxVelocityOverride(maxLinearVelocitySetting);
    } else {
      point.resetMaxVelocityOverride();
    }

    changed = true;
  }

  ImGui::SameLine();
  ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
  {
    auto scopedDisabled = ImGui::Scoped::Disabled(!doOverrideVelocity);

    double velocity = maxLinearVelocitySetting.value();
    if (doOverrideVelocity) {
      velocity = point.maxVelocityOverride().value().value();
    } else if (isFirstPoint || isLastPoint) {
      velocity = 0;
    }

    changed |= presentSlider("##Velocity Override", velocity, kVelocitySliderSpeed, "%.2f m/s");
    velocity = std::min(velocity, maxLinearVelocitySetting.value());

    if (doOverrideVelocity) {
      point.setMaxVelocityOverride(units::meters_per_second_t(velocity));

      if (point.maxVelocityOverride().value() > 0_mps && (isFirstPoint || isLastPoint)) {
        auto scopedYellowText = ImGui::Scoped::StyleColor(ImGuiCol_Text, kWarningTextColor);
        ImGui::TextWrapped("Warning: Trajectory does not %s at 0 m/s", isFirstPoint ? "start" : "end");
      }
    }
  }

  return changed;
}

bool PropertiesPage::presentRotationProperty(const char* name,
                                             std::function<CanonicalAngle()> getRotation,
                                             std::function<void(CanonicalAngle)> setRotation) {
  bool changed = false;

  auto scopedField = ImGui::ScopedField::Builder(name).build();

  double angle = getRotation().degrees().value();
  const bool lastAngle = angle;

  std::string sliderName = fmt::format("##{}", name);
  changed |= presentSlider(sliderName.c_str(), angle, kAngleSliderSpeed, "%.2f°");

  if (angle != lastAngle) {
    setRotation(CanonicalAngle(units::degree_t(angle)));
  }

  return changed;
}

bool PropertiesPage::presentPointStopRotationProperty(ThunderAutoTrajectorySkeletonWaypoint& point) {
  ThunderAutoAssert(point.isStopped());

  auto getRotation = [&]() { return point.stopRotation(); };
  auto setRotation = [&](CanonicalAngle angle) { point.setStopRotation(angle); };

  return presentRotationProperty("Stop Rotation", getRotation, setRotation);
}

bool PropertiesPage::presentTrajectoryStartRotationProperty(ThunderAutoTrajectorySkeleton& skeleton) {
  auto getRotation = [&]() { return skeleton.startRotation(); };
  auto setRotation = [&](CanonicalAngle angle) { skeleton.setStartRotation(angle); };

  return presentRotationProperty("Start Rotation", getRotation, setRotation);
}

bool PropertiesPage::presentTrajectoryEndRotationProperty(ThunderAutoTrajectorySkeleton& skeleton) {
  auto getRotation = [&]() { return skeleton.endRotation(); };
  auto setRotation = [&](CanonicalAngle angle) { skeleton.setEndRotation(angle); };

  return presentRotationProperty("End Rotation", getRotation, setRotation);
}

bool PropertiesPage::presentActionProperty(const char* name,
                                           const char* tooltip,
                                           std::function<const std::string&()> getActionName,
                                           std::function<void(const std::string&)> setActionName,
                                           std::span<const std::string> availableActionNames) {
  auto scopedField = ImGui::ScopedField::Builder(name).tooltip(tooltip).build();

  std::string currentActionName = getActionName();
  bool changed = false;

  if (auto scopedCombo = ImGui::Scoped::Combo("##Action Name", currentActionName.c_str())) {
    if (ImGui::Selectable("<none>", currentActionName.empty()) && !currentActionName.empty()) {
      setActionName("");
      changed = true;
    }

    if (!availableActionNames.empty()) {
      ImGui::Separator();
    }

    for (const std::string& action : availableActionNames) {
      bool isActionSelected = action == currentActionName;
      if (ImGui::Selectable(action.c_str(), isActionSelected) && !isActionSelected) {
        setActionName(action);
        changed = true;
        break;
      }
    }
  }

  return changed;
}

bool PropertiesPage::presentPointStopActionProperty(ThunderAutoTrajectorySkeletonWaypoint& point,
                                                    const ThunderAutoProjectState& state) {
  ThunderAutoAssert(point.isStopped());

  auto getActionName = [&]() -> const std::string& { return point.stopAction(); };
  auto setActionName = [&](const std::string& actionName) { point.setStopAction(actionName); };

  return presentActionProperty("Stop Action", "Action to perform before the robot resumes driving",
                               getActionName, setActionName, state.actionsOrder);
}

bool PropertiesPage::presentTrajectoryStartActionProperty(ThunderAutoTrajectorySkeleton& skeleton,
                                                          const ThunderAutoProjectState& state) {
  auto getActionName = [&]() -> const std::string& { return skeleton.startAction(); };
  auto setActionName = [&](const std::string& actionName) { skeleton.setStartAction(actionName); };

  return presentActionProperty("Start Action", "Action to perform before the robot starts driving",
                               getActionName, setActionName, state.actionsOrder);
}

bool PropertiesPage::presentTrajectoryEndActionProperty(ThunderAutoTrajectorySkeleton& skeleton,
                                                        const ThunderAutoProjectState& state) {
  auto getActionName = [&]() -> const std::string& { return skeleton.endAction(); };
  auto setActionName = [&](const std::string& actionName) { skeleton.setEndAction(actionName); };

  return presentActionProperty("End Action", "Action to perform after the robot has finished driving",
                               getActionName, setActionName, state.actionsOrder);
}

void PropertiesPage::presentTrajectorySelectedRotationProperties(ThunderAutoProjectState& state) {
  ThunderAutoTrajectorySkeleton& skeleton = state.currentTrajectory();
  ThunderAutoTrajectoryEditorState& editorState = state.editorState.trajectoryEditorState;

  ThunderAutoPositionedTrajectoryItemList<ThunderAutoTrajectoryRotation>& rotations = skeleton.rotations();

  auto it = std::next(rotations.begin(), editorState.selectionIndex);
  ThunderAutoAssert(it != rotations.cend());

  bool changed = false;

  {
    auto scopedField = ImGui::ScopedField::Builder("Rotation").build();

    double angle = it->second.angle.degrees().value();
    const bool lastAngle = angle;

    changed |= presentSlider("##Rotation", angle, kAngleSliderSpeed, "%.2f°");

    if (angle != lastAngle) {
      it->second.angle = CanonicalAngle(units::degree_t(angle));
    }
  }

  {
    auto scopedField = ImGui::ScopedField::Builder("Trajectory Position").build();

    double position = it->first;
    const double lastPosition = position;

    bool isFinished;

    changed |=
        presentSlider("##Trajectory Position", position, kTrajectoryPositionSliderSpeed, "%.2f", &isFinished);
    position = std::clamp(position, 0.0, static_cast<double>(skeleton.numPoints() - 1));

    if (position != lastPosition) {
      auto newRotationIt = rotations.move(it, position);
      editorState.selectionIndex = std::distance(rotations.begin(), newRotationIt);
    }

    if (isFinished) {
      std::unique_ptr<ThunderAutoPartialOutputTrajectory> trajectoryPositionData =
          BuildThunderAutoPartialOutputTrajectory(skeleton, kPreviewOutputTrajectorySettings);

      skeleton.separateRotations(0.1_m, trajectoryPositionData.get());

      m_history.modifyLastState(state);
    }
  }

  if (changed) {
    m_history.addState(state);
    m_editorPage.invalidateCachedTrajectory();
    m_editorPage.resetPlayback();
  }
}

void PropertiesPage::presentTrajectorySelectedActionProperties(ThunderAutoProjectState& state) {
  ThunderAutoTrajectorySkeleton& skeleton = state.currentTrajectory();
  ThunderAutoTrajectoryEditorState& editorState = state.editorState.trajectoryEditorState;

  ThunderAutoPositionedTrajectoryItemList<ThunderAutoTrajectoryAction>& actions = skeleton.actions();

  auto it = std::next(actions.begin(), editorState.selectionIndex);
  ThunderAutoAssert(it != actions.cend());

  bool changed = false;

  {
    auto scopedField = ImGui::ScopedField::Builder("Action").build();

    if (auto scopedCombo = ImGui::Scoped::Combo("##Action", it->second.action.c_str())) {
      for (const std::string& action : state.actionsOrder) {
        if (ImGui::Selectable(action.c_str(), false)) {
          it->second.action = action;
          changed = true;
        }
      }
    }
    if (auto scopedDragTarget = ImGui::Scoped::DragDropTarget()) {
      if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Action")) {
        std::string payloadActionName = reinterpret_cast<const char*>(payload->Data);
        it->second.action = payloadActionName;
        changed = true;
      }
    }
  }

  {
    auto scopedField = ImGui::ScopedField::Builder("Trajectory Position").build();

    double position = it->first;
    const double lastPosition = position;

    changed |= presentSlider("##Trajectory Position", position, kTrajectoryPositionSliderSpeed, "%.2f");
    position = std::clamp(position, 0.0, static_cast<double>(skeleton.numPoints() - 1));

    if (position != lastPosition) {
      auto newActionIt = actions.move(it, position);
      editorState.selectionIndex = std::distance(actions.begin(), newActionIt);
    }
  }

  if (changed) {
    m_history.addState(state);
    m_editorPage.invalidateCachedTrajectory();
    m_editorPage.resetPlayback();
  }
}

void PropertiesPage::presentTrajectoryOtherProperties(ThunderAutoProjectState& state) {
  presentSeparatorText("Trajectory Properties");

  auto scopedID = ImGui::Scoped::ID("Trajectory Other Properties");

  ThunderAutoTrajectorySkeleton& skeleton = state.currentTrajectory();

  bool changed = false;

  changed |= presentTrajectoryStartRotationProperty(skeleton);
  changed |= presentTrajectoryEndRotationProperty(skeleton);

  changed |= presentTrajectoryStartActionProperty(skeleton, state);
  changed |= presentTrajectoryEndActionProperty(skeleton, state);

  if (changed) {
    m_history.addState(state);
    m_editorPage.invalidateCachedTrajectory();
    m_editorPage.resetPlayback();
  }
}

void PropertiesPage::presentTrajectorySpeedConstraintProperties(ThunderAutoProjectState& state) {
  presentSeparatorText("Trajectory Speed Constraints");

  auto scopedID = ImGui::Scoped::ID("Trajectory Speed Constraint Properties");

  ThunderAutoTrajectorySkeleton& skeleton = state.currentTrajectory();
  ThunderAutoTrajectorySkeletonSettings& settings = skeleton.settings();

  bool changed = false;

  // Linear Acceleration.
  {
    auto scopedField = ImGui::ScopedField::Builder("Linear Accel").build();

    double maxLinearAcceleration = settings.maxLinearAcceleration.value();

    changed |=
        presentSlider("##Linear Acceleration", maxLinearAcceleration, kVelocitySliderSpeed, "%.2f m/s²");

    maxLinearAcceleration = std::clamp(maxLinearAcceleration, 0.1, 25.0);
    settings.maxLinearAcceleration = units::meters_per_second_squared_t{maxLinearAcceleration};
  }

  // Linear Velocity.
  {
    auto scopedField = ImGui::ScopedField::Builder("Linear Velocity").build();

    double maxLinearVelocity = settings.maxLinearVelocity.value();

    changed |= presentSlider("##Linear Velocity", maxLinearVelocity, kVelocitySliderSpeed, "%.2f m/s");

    maxLinearVelocity = std::clamp(maxLinearVelocity, 0.1, 25.0);
    settings.maxLinearVelocity = units::meters_per_second_t{maxLinearVelocity};
  }

  // Centripetal Acceleration.
  {
    auto scopedField = ImGui::ScopedField::Builder("Centripetal Accel").build();

    double maxCentripetalAcceleration = settings.maxCentripetalAcceleration.value();

    changed |= presentSlider("##Centripetal Acceleration", maxCentripetalAcceleration, 0.25f, "%.2f m/s²");

    maxCentripetalAcceleration =
        std::clamp(maxCentripetalAcceleration, 0.1, std::numeric_limits<double>::infinity());
    settings.maxCentripetalAcceleration = units::meters_per_second_squared_t{maxCentripetalAcceleration};
  }

  // Angular Velocity.
  {
    auto scopedField = ImGui::ScopedField::Builder("Angular Velocity").build();

    double maxAngularVelocity = units::degrees_per_second_t{settings.maxAngularVelocity}.value();

    changed |= presentSlider("##Angular Velocity", maxAngularVelocity, kAngleSliderSpeed, "%.2f °/s");

    maxAngularVelocity = std::clamp(maxAngularVelocity, 1.0, 720.0);
    settings.maxAngularVelocity = units::degrees_per_second_t{maxAngularVelocity};
  }

  // Angular Acceleration.
  {
    auto scopedField = ImGui::Scoped::StyleVarY(ImGuiStyleVar_ItemSpacing, 0.f);
    auto scopedField2 = ImGui::ScopedField::Builder("Angular Accel").build();

    double maxAngularAcceleration =
        units::degrees_per_second_squared_t{settings.maxAngularAcceleration}.value();

    changed |=
        presentSlider("##Angular Acceleration", maxAngularAcceleration, kAngleSliderSpeed, "%.2f °/s²");

    maxAngularAcceleration = std::clamp(maxAngularAcceleration, 1.0, 1440.0);
    settings.maxAngularAcceleration = units::degrees_per_second_squared_t{maxAngularAcceleration};
  }

  if (changed) {
    m_history.addState(state);
    m_editorPage.invalidateCachedTrajectory();
    m_editorPage.resetPlayback();
  }

  ImGui::Spacing();
}

void PropertiesPage::presentAutoModeProperties(ThunderAutoProjectState& state) {
  ThunderAutoModeEditorState& editorState = state.editorState.autoModeEditorState;
  if (editorState.currentAutoModeName.empty())
    return;

  presentAutoModeStepList(state);
  presentAutoModeSelectedStepProperties(state);
  presentAutoModeSpeedConstraintProperties(state);
}

void PropertiesPage::presentAutoModeStepList(ThunderAutoProjectState& state) {
  presentSeparatorText("Auto Mode Steps");

  auto scopedID = ImGui::Scoped::ID("Auto Mode Steps List");

  ThunderAutoModeStepPath rootPath;
  rootPath /= ThunderAutoModeStepPath::Node{};

  {
    auto scopedChildWindow = ImGui::Scoped::ChildWindow(
        "Auto Mode Steps Child Window",
        ImVec2(0.f, GET_UISIZE(PROPERTIES_PAGE_AUTO_MODE_STEP_LIST_CHILD_WINDOW_START_SIZE_Y)),
        ImGuiChildFlags_ResizeY | ImGuiChildFlags_Borders);

    ThunderAutoMode& autoMode = state.currentAutoMode();

    (void)drawAutoModeStepsTree(autoMode.steps, rootPath, state);
  }

  // Dropping trajectories and actions into the child window adds them as steps to the end of the auto mode.
  autoModeStepDragDropTarget(rootPath, AutoModeStepDragDropInsertMethod::INTO, false, state);

  if (ImGui::Button("+ Add Step", ImVec2(ImGui::GetContentRegionAvail().x, 0.f))) {
    m_event = Event::AUTO_MODE_ADD_STEP;
  }
}

bool PropertiesPage::drawAutoModeStepTreeNode(std::unique_ptr<ThunderAutoModeStep>& step,
                                              const ThunderAutoModeStepPath& stepPath,
                                              ThunderAutoProjectState& state) {
  // Space in between steps to allow for drag-and-drop.
  {
    auto scopedPadding = ImGui::Scoped::StyleVarY(ImGuiStyleVar_ItemSpacing, 0.f);

    const float spacingX = std::max(ImGui::GetContentRegionAvail().x, 1.f);
    const float spacingY = GET_UISIZE(SELECTABLE_LIST_ITEM_SPACING_Y) / 3.f;
    (void)ImGui::InvisibleButton("Drag Separator", ImVec2(spacingX, spacingY));

    autoModeStepDragDropTarget(stepPath, AutoModeStepDragDropInsertMethod::BEFORE, true, state);
  }

  // Draw the tree node for the step.

  ImGuiTreeNodeFlags treeNodeFlags =
      ImGuiTreeNodeFlags_DrawLinesFull | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanFullWidth;
  std::string treeNodeLabel;

  switch (step->type()) {
    using enum ThunderAutoModeStepType;
    case ACTION: {
      const ThunderAutoModeActionStep& actionStep = reinterpret_cast<const ThunderAutoModeActionStep&>(*step);

      treeNodeLabel = fmt::format(ICON_FA_PAPERCLIP "  {}",
                                  actionStep.actionName.empty() ? "<none>" : actionStep.actionName);
      treeNodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;
      break;
    }
    case TRAJECTORY: {
      const ThunderAutoModeTrajectoryStep& trajectoryStep =
          reinterpret_cast<const ThunderAutoModeTrajectoryStep&>(*step);

      treeNodeLabel =
          fmt::format(ICON_FA_ROUTE "  {}",
                      trajectoryStep.trajectoryName.empty() ? "<none>" : trajectoryStep.trajectoryName);
      treeNodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;
      break;
    }
    case BRANCH_BOOL: {
      const ThunderAutoModeBoolBranchStep& branchStep =
          reinterpret_cast<const ThunderAutoModeBoolBranchStep&>(*step);

      treeNodeLabel = fmt::format(ICON_FA_TOGGLE_ON "  {}",
                                  branchStep.conditionName.empty() ? "<none>" : branchStep.conditionName);
      break;
    }
    case BRANCH_SWITCH: {
      const ThunderAutoModeSwitchBranchStep& branchStep =
          reinterpret_cast<const ThunderAutoModeSwitchBranchStep&>(*step);

      treeNodeLabel = fmt::format(ICON_FA_LIST_OL "  {}",
                                  branchStep.conditionName.empty() ? "<none>" : branchStep.conditionName);
      break;
    }
    default:
      ThunderAutoUnreachable("Invalid auto mode step type");
  }

  ThunderAutoModeEditorState& editorState = state.editorState.autoModeEditorState;
  bool isStepSelected = (editorState.selectedStepPath == stepPath);

  ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, 0.f);
  auto scopedTreeNode = ImGui::Scoped::TreeNodeEx(
      treeNodeLabel.c_str(), treeNodeFlags | (isStepSelected ? ImGuiTreeNodeFlags_Selected : 0));
  ImGui::PopStyleVar();

  if (ImGui::IsItemActivated() && !isStepSelected) {
    editorState.selectedStepPath = stepPath;
    m_history.addState(state, false);
  }

  // Right-click the step.
  if (auto popup = ImGui::Scoped::PopupContextItem()) {
    if (ImGui::MenuItem(ICON_FA_TRASH_ALT "  Delete")) {
      ThunderAutoLogger::Info("Deleting auto mode step at \"{}\"", ThunderAutoModeStepPathToString(stepPath));
      state.currentAutoModeDeleteStep(stepPath);
      m_history.addState(state);
      return true;
    }

    // Cases can be added to switch branches.
    if (step->type() == ThunderAutoModeStepType::BRANCH_SWITCH) {
      ThunderAutoModeSwitchBranchStep& branchStep = reinterpret_cast<ThunderAutoModeSwitchBranchStep&>(*step);

      if (auto scopedMenu = ImGui::Scoped::Menu(ICON_FA_PLUS "  Add Case")) {
        static int newCaseValue = 0;
        ImGui::InputInt("##Case Value", &newCaseValue);
        ImGui::SameLine();
        bool isNewCaseValueUsed = branchStep.caseBranches.contains(newCaseValue);
        auto scopedDisabled = ImGui::Scoped::Disabled(isNewCaseValueUsed);
        if (ImGui::Button("Add")) {
          ThunderAutoLogger::Info("Adding case {} to switch branch at \"{}\"", newCaseValue,
                                  ThunderAutoModeStepPathToString(stepPath));
          branchStep.caseBranches[newCaseValue] = std::list<std::unique_ptr<ThunderAutoModeStep>>{};
          m_history.addState(state);

          newCaseValue = 0;
          ImGui::CloseCurrentPopup();
        }
        if (isNewCaseValueUsed && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
          ImGui::SetTooltip("Case value already exists");
        }
      }
    }
  }

  // Steps can be dragged.
  if (auto scopedDragSource = ImGui::Scoped::DragDropSource()) {
    const void* payloadData = stepPath.path.data();
    const size_t payloadSize = stepPath.path.size() * sizeof(ThunderAutoModeStepPath::Node);
    ImGui::SetDragDropPayload("Auto Mode Step", payloadData, payloadSize);
    ImGui::Text("%s", treeNodeLabel.c_str());
  }

  // Draw any child steps.
  switch (step->type()) {
    using enum ThunderAutoModeStepType;
    case ACTION:
      break;
    case TRAJECTORY:
      break;
    case BRANCH_BOOL: {
      ThunderAutoModeBoolBranchStep& branchStep = reinterpret_cast<ThunderAutoModeBoolBranchStep&>(*step);

      if (scopedTreeNode) {
        if (auto scopedTrueBranchTreeNode = ImGui::Scoped::TreeNodeEx("TRUE", treeNodeFlags)) {
          ThunderAutoModeStepPath::Node trueChildNode(
              ThunderAutoModeStepPath::Node::DirectoryType::BOOL_TRUE);
          ThunderAutoModeStepPath childStepPath = stepPath / trueChildNode;

          autoModeStepDragDropTarget(childStepPath, AutoModeStepDragDropInsertMethod::INTO, true, state);

          bool shouldStop = drawAutoModeStepsTree(branchStep.trueBranch, childStepPath, state);
          if (shouldStop) {
            return true;
          }
        }
        if (auto scopedFalseBranchTreeNode = ImGui::Scoped::TreeNodeEx("FALSE", treeNodeFlags)) {
          ThunderAutoModeStepPath::Node falseChildNode(
              ThunderAutoModeStepPath::Node::DirectoryType::BOOL_ELSE);
          ThunderAutoModeStepPath childStepPath = stepPath / falseChildNode;

          autoModeStepDragDropTarget(childStepPath, AutoModeStepDragDropInsertMethod::INTO, true, state);

          bool shouldStop = drawAutoModeStepsTree(branchStep.elseBranch, childStepPath, state);
          if (shouldStop) {
            return true;
          }
        }
      }
      break;
    }
    case BRANCH_SWITCH: {
      ThunderAutoModeSwitchBranchStep& branchStep = reinterpret_cast<ThunderAutoModeSwitchBranchStep&>(*step);

      if (scopedTreeNode) {
        for (auto& [caseValue, caseBranch] : branchStep.caseBranches) {
          if (auto scopedCaseBranchTreeNode =
                  ImGui::Scoped::TreeNodeEx(fmt::format("CASE {}", caseValue).c_str(), treeNodeFlags)) {
            ThunderAutoModeStepPath::Node caseChildNode(
                ThunderAutoModeStepPath::Node::DirectoryType::SWITCH_CASE);
            caseChildNode.caseBranchValue = caseValue;
            ThunderAutoModeStepPath childStepPath = stepPath / caseChildNode;

            autoModeStepDragDropTarget(childStepPath, AutoModeStepDragDropInsertMethod::INTO, true, state);

            bool shouldStop = drawAutoModeStepsTree(caseBranch, childStepPath, state);
            if (shouldStop) {
              return true;
            }
          }

          // Right-click the case branch.
          if (auto popup = ImGui::Scoped::PopupContextItem()) {
            if (ImGui::MenuItem(ICON_FA_TRASH_ALT "  Delete Case")) {
              ThunderAutoLogger::Info("Deleting case {} from switch branch at \"{}\"", caseValue,
                                      ThunderAutoModeStepPathToString(stepPath));
              branchStep.caseBranches.erase(caseValue);
              m_history.addState(state);
              break;
            }
          }
        }
        if (auto scopedFalseBranchTreeNode = ImGui::Scoped::TreeNodeEx("DEFAULT", treeNodeFlags)) {
          ThunderAutoModeStepPath::Node defaultChildNode(
              ThunderAutoModeStepPath::Node::DirectoryType::SWITCH_DEFAULT);
          ThunderAutoModeStepPath childStepPath = stepPath / defaultChildNode;

          autoModeStepDragDropTarget(childStepPath, AutoModeStepDragDropInsertMethod::INTO, true, state);

          bool shouldStop = drawAutoModeStepsTree(branchStep.defaultBranch, childStepPath, state);
          if (shouldStop) {
            return true;
          }
        }
      }
      break;
    }
    default:
      ThunderAutoUnreachable("Invalid auto mode step type");
  }

  return false;
}

bool PropertiesPage::drawAutoModeStepsTree(std::list<std::unique_ptr<ThunderAutoModeStep>>& steps,
                                           const ThunderAutoModeStepPath& parentPath,
                                           ThunderAutoProjectState& state) {
  ThunderAutoModeStepPath path = parentPath;
  size_t stepIndex = 0;
  for (auto& step : steps) {
    path.lastNode().stepIndex = stepIndex;

    /**
     * TODO:
     * This current method of differentiating steps for ImGui using just their index is not ideal because the
     * IDs of tree nodes can shift when steps are added/removed, causing nodes to close/open unexpectedly. A
     * better method would be to assign each step a unique ID upon creation and use that instead.
     */
    auto scopedID = ImGui::Scoped::ID(stepIndex++);

    bool shouldStop = drawAutoModeStepTreeNode(step, path, state);
    if (shouldStop) {
      return true;
    }
  }

  // Space at the bottom to allow for drag-and-drop.
  {
    auto scopedID = ImGui::Scoped::ID("Bottom Drag Target");
    auto scopedPadding = ImGui::Scoped::StyleVarY(ImGuiStyleVar_ItemSpacing, 0.f);

    const float spacingX = std::max(ImGui::GetContentRegionAvail().x, 1.f);
    const float spacingY = GET_UISIZE(SELECTABLE_LIST_ITEM_SPACING_Y) / 3.f;
    (void)ImGui::InvisibleButton("Drag Separator", ImVec2(spacingX, spacingY));

    AutoModeStepDragDropInsertMethod insertMethod = AutoModeStepDragDropInsertMethod::AFTER;
    if (steps.empty()) {
      insertMethod = AutoModeStepDragDropInsertMethod::INTO;
    }

    autoModeStepDragDropTarget(path, insertMethod, true, state);
  }

  return false;
}

void PropertiesPage::autoModeStepDragDropTarget(const ThunderAutoModeStepPath& stepPath,
                                                AutoModeStepDragDropInsertMethod insertMethod,
                                                bool acceptAutoModeSteps,
                                                ThunderAutoProjectState& state) {
  ThunderAutoAssert(!stepPath.path.empty());

  using enum AutoModeStepDragDropInsertMethod;

  if (auto scopedDragTarget = ImGui::Scoped::DragDropTarget()) {
    const ImGuiPayload* payload = nullptr;
    if (acceptAutoModeSteps && (payload = ImGui::AcceptDragDropPayload("Auto Mode Step"))) {
      ThunderAutoModeStepPath::Node* payloadPathNodes =
          reinterpret_cast<ThunderAutoModeStepPath::Node*>(payload->Data);
      size_t payloadPathNumNodes = payload->DataSize / sizeof(ThunderAutoModeStepPath::Node);

      ThunderAutoModeStepPath payloadPath;
      payloadPath.path.assign(payloadPathNodes, payloadPathNodes + payloadPathNumNodes);

      ThunderAutoLogger::Info(
          "Drag and drop auto mode step from \"{}\" to {} \"{}\"",
          ThunderAutoModeStepPathToString(payloadPath),
          (insertMethod == BEFORE ? "before" : (insertMethod == AFTER ? "after" : "into")),
          ThunderAutoModeStepPathToString(stepPath));

      if (stepPath.hasParentPath(payloadPath)) {
        ThunderAutoLogger::Warn("Cannot move a step under one of its own children");
        return;
      }

      bool isInSameLocation = (payloadPath == stepPath);
      if (!isInSameLocation) {
        switch (insertMethod) {
          case INTO:
            if (payloadPath.isInSameDirectoryAs(stepPath)) {
              isInSameLocation = true;
            }
            break;
          case AFTER:
            isInSameLocation = (payloadPath == (stepPath.parentPath() / (stepPath.lastNode().next())));
            break;
          case BEFORE:
            if (stepPath.lastNode().stepIndex > 0) {
              isInSameLocation = (payloadPath == (stepPath.parentPath() / (stepPath.lastNode().prev())));
            }
            break;
          default:
            ThunderAutoUnreachable("Invalid insert method");
        }
      }

      if (isInSameLocation) {
        ThunderAutoLogger::Warn("Ignoring move to the same location");
        return;
      }

      // Move the step.
      if (insertMethod == INTO) {
        state.currentAutoModeMoveStepIntoDirectory(payloadPath, stepPath);
      } else if (insertMethod == BEFORE) {
        state.currentAutoModeMoveStepBeforeOther(payloadPath, stepPath);
      } else if (insertMethod == AFTER) {
        state.currentAutoModeMoveStepAfterOther(payloadPath, stepPath);
      }
      m_history.addState(state);

    } else {
      std::unique_ptr<ThunderAutoModeStep> newStep;

      // Make a new step from the dropped payload.
      if ((payload = ImGui::AcceptDragDropPayload("Action"))) {
        std::string payloadActionName = reinterpret_cast<const char*>(payload->Data);
        auto actionStep = std::make_unique<ThunderAutoModeActionStep>();
        actionStep->actionName = payloadActionName;
        newStep = std::move(actionStep);
      } else if ((payload = ImGui::AcceptDragDropPayload("Trajectory"))) {
        std::string payloadTrajectoryName = reinterpret_cast<const char*>(payload->Data);
        auto trajectoryStep = std::make_unique<ThunderAutoModeTrajectoryStep>();
        trajectoryStep->trajectoryName = payloadTrajectoryName;
        newStep = std::move(trajectoryStep);
      }

      // Add the new step.
      if (newStep) {
        if (insertMethod == INTO) {
          state.currentAutoModeInsertStepInDirectory(stepPath, std::move(newStep));
        } else if (insertMethod == BEFORE) {
          state.currentAutoModeInsertStepBeforeOther(stepPath, std::move(newStep));
        } else if (insertMethod == AFTER) {
          state.currentAutoModeInsertStepAfterOther(stepPath, std::move(newStep));
        }
        m_history.addState(state);
      }
    }
  }
}

void PropertiesPage::presentAutoModeSelectedStepProperties(ThunderAutoProjectState& state) {
  ThunderAutoModeEditorState& editorState = state.editorState.autoModeEditorState;

  presentSeparatorText("Selected Auto Mode Step");

  auto scopedID = ImGui::Scoped::ID("Selected Step Properties");

  // TODO: Implement

  ImGui::Text("No step selected");
}

void PropertiesPage::presentAutoModeSpeedConstraintProperties(ThunderAutoProjectState& state) {
  presentSeparatorText("Auto Mode Speed Constraints");

  auto scopedID = ImGui::Scoped::ID("Auto Mode Speed Constraint Properties");

  // TODO: Implement
}

std::map<ThunderAutoTrajectoryPosition, PropertiesPage::TrajectoryItemSelection<CanonicalAngle>>
PropertiesPage::GetAllRotationSelections(const ThunderAutoTrajectorySkeleton& skeleton) {
  std::map<ThunderAutoTrajectoryPosition, PropertiesPage::TrajectoryItemSelection<CanonicalAngle>>
      rotationSelections;

  PropertiesPage::TrajectoryItemSelection<CanonicalAngle> startRotationSelection{
      .item = skeleton.startRotation(),
      .editorLocked = skeleton.front().isEditorLocked(),
      .trajectorySelection = ThunderAutoTrajectoryEditorState::TrajectorySelection::WAYPOINT,
      .selectionIndex = 0,
  };

  rotationSelections.emplace(ThunderAutoTrajectoryPosition(0.0), startRotationSelection);

  const ThunderAutoPositionedTrajectoryItemList<ThunderAutoTrajectoryRotation>& rotationTargets =
      skeleton.rotations();

  size_t rotationIndex = 0;
  for (const auto& [position, rotation] : rotationTargets) {
    PropertiesPage::TrajectoryItemSelection<CanonicalAngle> rotationSelection{
        .item = rotation.angle,
        .editorLocked = rotation.editorLocked,
        .trajectorySelection = ThunderAutoTrajectoryEditorState::TrajectorySelection::ROTATION,
        .selectionIndex = rotationIndex,
    };

    rotationSelections.emplace(position, rotationSelection);

    rotationIndex++;
  }

  if (skeleton.numPoints() > 2) {
    size_t waypointIndex = 1;
    for (auto waypointIt = std::next(skeleton.begin()); waypointIt != std::prev(skeleton.end());
         ++waypointIt, ++waypointIndex) {
      if (waypointIt->isStopped()) {
        PropertiesPage::TrajectoryItemSelection<CanonicalAngle> rotationSelection{
            .item = waypointIt->stopRotation(),
            .editorLocked = waypointIt->isEditorLocked(),
            .trajectorySelection = ThunderAutoTrajectoryEditorState::TrajectorySelection::WAYPOINT,
            .selectionIndex = waypointIndex,
        };

        rotationSelections.emplace(ThunderAutoTrajectoryPosition(static_cast<double>(waypointIndex)),
                                   rotationSelection);
      }
    }
  }

  PropertiesPage::TrajectoryItemSelection<CanonicalAngle> endRotationSelection{
      .item = skeleton.startRotation(),
      .editorLocked = skeleton.back().isEditorLocked(),
      .trajectorySelection = ThunderAutoTrajectoryEditorState::TrajectorySelection::WAYPOINT,
      .selectionIndex = skeleton.numPoints() - 1,
  };

  rotationSelections.emplace(ThunderAutoTrajectoryPosition(static_cast<double>(skeleton.numPoints() - 1)),
                             endRotationSelection);

  return rotationSelections;
}

std::multimap<ThunderAutoTrajectoryPosition, PropertiesPage::TrajectoryItemSelection<std::string>>
PropertiesPage::GetAllActionSelections(const ThunderAutoTrajectorySkeleton& skeleton) {
  std::multimap<ThunderAutoTrajectoryPosition, PropertiesPage::TrajectoryItemSelection<std::string>>
      actionSelections;

  if (skeleton.hasStartAction()) {
    PropertiesPage::TrajectoryItemSelection<std::string> startActionSelection{
        .item = "<start> " + skeleton.startAction(),
        .editorLocked = skeleton.front().isEditorLocked(),
        .trajectorySelection = ThunderAutoTrajectoryEditorState::TrajectorySelection::WAYPOINT,
        .selectionIndex = 0,
    };

    actionSelections.emplace(ThunderAutoTrajectoryPosition(0.0), startActionSelection);
  }

  const ThunderAutoPositionedTrajectoryItemList<ThunderAutoTrajectoryAction>& actionTargets =
      skeleton.actions();

  size_t actionIndex = 0;
  for (const auto& [position, action] : actionTargets) {
    PropertiesPage::TrajectoryItemSelection<std::string> actionSelection{
        .item = action.action,
        .editorLocked = action.editorLocked,
        .trajectorySelection = ThunderAutoTrajectoryEditorState::TrajectorySelection::ACTION,
        .selectionIndex = actionIndex,
    };

    actionSelections.emplace(position, actionSelection);

    actionIndex++;
  }

  if (skeleton.numPoints() > 2) {
    size_t waypointIndex = 1;
    for (auto waypointIt = std::next(skeleton.begin()); waypointIt != std::prev(skeleton.end());
         ++waypointIt, ++waypointIndex) {
      if (waypointIt->isStopped() && waypointIt->hasStopAction()) {
        PropertiesPage::TrajectoryItemSelection<std::string> acitionSelection{
            .item = "<stop> " + waypointIt->stopAction(),
            .editorLocked = waypointIt->isEditorLocked(),
            .trajectorySelection = ThunderAutoTrajectoryEditorState::TrajectorySelection::WAYPOINT,
            .selectionIndex = waypointIndex,
        };

        actionSelections.emplace(ThunderAutoTrajectoryPosition(static_cast<double>(waypointIndex)),
                                 acitionSelection);
      }
    }
  }

  if (skeleton.hasEndAction()) {
    PropertiesPage::TrajectoryItemSelection<std::string> endActionSelection{
        .item = "<end> " + skeleton.endAction(),
        .editorLocked = skeleton.back().isEditorLocked(),
        .trajectorySelection = ThunderAutoTrajectoryEditorState::TrajectorySelection::WAYPOINT,
        .selectionIndex = skeleton.numPoints() - 1,
    };

    actionSelections.emplace(ThunderAutoTrajectoryPosition(static_cast<double>(skeleton.numPoints() - 1)),
                             endActionSelection);
  }

  return actionSelections;
}

bool PropertiesPage::presentSlider(const char* id,
                                   double& valueDouble,
                                   const float speed,
                                   const char* format,
                                   bool* isFinished) {
  if (isFinished) {
    *isFinished = false;
  }

  float value = static_cast<float>(valueDouble);
  const float lastValue = value;
  bool active = ImGui::DragFloat(id, &value, speed, -1000.f, +1000.f, format, ImGuiSliderFlags_AlwaysClamp);
  valueDouble = static_cast<double>(value);

  if (value == lastValue) {
    active = false;
  }

  if (ImGui::IsItemActivated()) {
    m_history.startLongEdit();
  }

  if (ImGui::IsItemDeactivated()) {
    if (ImGui::IsItemDeactivatedAfterEdit()) {
      m_history.finishLongEdit();
      if (isFinished) {
        *isFinished = true;
      }
    } else {
      m_history.discardLongEdit();
    }
  }

  return active;
}

void PropertiesPage::presentColoredUnclickableButton(const char* label, ImVec2 size, const ImColor& color) {
  auto scopedFont = ImGui::Scoped::Font(FontLibrary::get().boldFont, 0.f);
  auto scopedNoNav = ImGui::Scoped::ItemFlag(ImGuiItemFlags_NoNav, true);

  const ImU32 colorU32 = color;
  auto scopedButtonColor = ImGui::Scoped::StyleColor(ImGuiCol_Button, colorU32);
  auto scopedButtonHoveredColor = ImGui::Scoped::StyleColor(ImGuiCol_ButtonHovered, colorU32);
  auto scopedButtonActiveColor = ImGui::Scoped::StyleColor(ImGuiCol_ButtonActive, colorU32);

  (void)ImGui::Button(label, size);
}

void PropertiesPage::presentSeparatorText(const char* text) {
  auto scopedFont = ImGui::Scoped::Font(FontLibrary::get().boldFont, 0.f);
  ImGui::SeparatorText(text);
}
