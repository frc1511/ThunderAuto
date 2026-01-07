#include <ThunderAuto/Pages/PropertiesPage.hpp>

#include <ThunderAuto/ImGuiScopedField.hpp>
#include <ThunderAuto/FontLibrary.hpp>
#include <ThunderAuto/ColorPalette.hpp>
#include <ThunderAuto/Platform/Platform.hpp>
#include <ThunderAuto/App.hpp>
#include <IconsLucide.h>
#include <imgui_raii.h>
#include <imgui_internal.h>
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
          selectableTitle += fmt::format("  {}  {}", ICON_LC_ARROW_RIGHT, pointIt->linkName());
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

          if (ImGui::MenuItem(ICON_LC_TRASH "  Delete Point")) {
            state.currentTrajectoryDeleteSelectedItem();
            m_history.addState(state);
            m_editorPage.invalidateCachedTrajectory();
            m_editorPage.resetPlayback();
            break;
          }

          if (isPointLinked) {
            if (ImGui::MenuItem(ICON_LC_LINK "  Edit Link")) {
              m_event = Event::TRAJECTORY_POINT_LINK;
            }
            if (ImGui::MenuItem(ICON_LC_UNLINK "  Remove Link")) {
              pointIt->removeLink();
              m_history.addState(state);
            }
          } else {
            if (ImGui::MenuItem(ICON_LC_LINK "  Link")) {
              m_event = Event::TRAJECTORY_POINT_LINK;
            }
          }

          const char* lockedMenuItemText =
              isPointLocked ? ICON_LC_LOCK_OPEN "  Unlock in Editor" : ICON_LC_LOCK "  Lock in Editor";
          if (ImGui::MenuItem(lockedMenuItemText)) {
            state.currentTrajectoryToggleEditorLockedForSelectedItem();
            m_history.addState(state);
          }
        }

        if (isPointLocked) {
          ImGui::SameLine();

          const ImGuiStyle& style = ImGui::GetStyle();
          const float lockedButtonWidthNeeded = ImGui::CalcTextSize(ICON_LC_LOCK).x + style.ItemSpacing.x;
          const float lockedButtonCursorOffset = ImGui::GetContentRegionAvail().x - lockedButtonWidthNeeded;
          if (lockedButtonCursorOffset > 0) {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + lockedButtonCursorOffset);
          }

          auto scopedButtonColor = ImGui::Scoped::StyleColor(ImGuiCol_Button, 0);
          auto scopedButtonHoveredColor = ImGui::Scoped::StyleColor(ImGuiCol_ButtonHovered, 0);
          auto scopedButtonActiveColor = ImGui::Scoped::StyleColor(ImGuiCol_ButtonActive, 0);

          if (ImGui::SmallButton(ICON_LC_LOCK)) {
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
              "{}  Delete {}", ICON_LC_TRASH, TrajectorySelectionToString(selection.trajectorySelection));

          if (ImGui::MenuItem(deleteMenuItemText.c_str())) {
            state.currentTrajectoryDeleteSelectedItem();
            m_history.addState(state);
            m_editorPage.invalidateCachedTrajectory();
            m_editorPage.resetPlayback();
            break;
          }

          const bool locked = selection.editorLocked;
          const char* lockedMenuItemText =
              locked ? ICON_LC_LOCK_OPEN "  Unlock in Editor" : ICON_LC_LOCK "  Lock in Editor";
          if (ImGui::MenuItem(lockedMenuItemText)) {
            state.currentTrajectoryToggleEditorLockedForSelectedItem();
            m_history.addState(state);
          }
        }

        if (selection.editorLocked) {
          ImGui::SameLine();

          const ImGuiStyle& style = ImGui::GetStyle();
          const float lockedButtonWidthNeeded = ImGui::CalcTextSize(ICON_LC_LOCK).x + style.ItemSpacing.x;
          const float lockedButtonCursorOffset = ImGui::GetContentRegionAvail().x - lockedButtonWidthNeeded;
          if (lockedButtonCursorOffset > 0) {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + lockedButtonCursorOffset);
          }

          auto scopedButtonColor = ImGui::Scoped::StyleColor(ImGuiCol_Button, 0);
          auto scopedButtonHoveredColor = ImGui::Scoped::StyleColor(ImGuiCol_ButtonHovered, 0);
          auto scopedButtonActiveColor = ImGui::Scoped::StyleColor(ImGuiCol_ButtonActive, 0);

          if (ImGui::SmallButton(ICON_LC_LOCK)) {
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
                "{}  Delete {}", ICON_LC_TRASH, TrajectorySelectionToString(selection.trajectorySelection));

            if (ImGui::MenuItem(deleteMenuItemText.c_str())) {
              state.currentTrajectoryDeleteSelectedItem();
              m_history.addState(state);
              m_editorPage.invalidateCachedTrajectory();
              m_editorPage.resetPlayback();
              break;
            }

            const bool locked = selection.editorLocked;
            const char* lockedMenuItemText =
                locked ? ICON_LC_LOCK_OPEN "  Unlock in Editor" : ICON_LC_LOCK "  Lock in Editor";
            if (ImGui::MenuItem(lockedMenuItemText)) {
              state.currentTrajectoryToggleEditorLockedForSelectedItem();
              m_history.addState(state);
            }
          }

          if (selection.editorLocked) {
            ImGui::SameLine();

            const ImGuiStyle& style = ImGui::GetStyle();
            const float lockedButtonWidthNeeded = ImGui::CalcTextSize(ICON_LC_LOCK).x + style.ItemSpacing.x;
            const float lockedButtonCursorOffset = ImGui::GetContentRegionAvail().x - lockedButtonWidthNeeded;
            if (lockedButtonCursorOffset > 0) {
              ImGui::SetCursorPosX(ImGui::GetCursorPosX() + lockedButtonCursorOffset);
            }

            auto scopedButtonColor = ImGui::Scoped::StyleColor(ImGuiCol_Button, 0);
            auto scopedButtonHoveredColor = ImGui::Scoped::StyleColor(ImGuiCol_ButtonHovered, 0);
            auto scopedButtonActiveColor = ImGui::Scoped::StyleColor(ImGuiCol_ButtonActive, 0);

            if (ImGui::SmallButton(ICON_LC_LOCK)) {
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

    presentColoredUnclickableButton(ICON_LC_ARROW_LEFT, buttonSize, ThunderAutoColorPalette::kBlue);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
      ImGui::SetTooltip("Incoming Heading");
    }

    ImGui::SameLine();

    ImGui::SetNextItemWidth(sliderWidth);
    changed |= presentSlider("##Incoming Heading", incomingHeading, kAngleSliderSpeed, "%.2f°");

    ImGui::SameLine();

    presentColoredUnclickableButton(ICON_LC_ARROW_RIGHT, buttonSize, ThunderAutoColorPalette::kOrange);
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

    presentColoredUnclickableButton(ICON_LC_ARROW_LEFT, buttonSize, ThunderAutoColorPalette::kBlue);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
      ImGui::SetTooltip("Incoming Weight");
    }

    ImGui::SameLine();

    ImGui::SetNextItemWidth(sliderWidth);
    changed |= presentSlider("##Incoming Weight", incomingWeight, kFieldPositionSliderSpeed, "%.2f m");

    ImGui::SameLine();

    presentColoredUnclickableButton(ICON_LC_ARROW_RIGHT, buttonSize, ThunderAutoColorPalette::kOrange);
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
        ImGui::TextWrapped(ICON_LC_TRIANGLE_ALERT " Trajectory does not %s at 0 m/s",
                           isFirstPoint ? "start" : "end");
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

bool PropertiesPage::presentPointStopActionProperty(ThunderAutoTrajectorySkeletonWaypoint& point,
                                                    const ThunderAutoProjectState& state) {
  ThunderAutoAssert(point.isStopped());

  auto getActionName = [&]() -> const std::string& { return point.stopAction(); };
  auto setActionName = [&](const std::string& actionName) { point.setStopAction(actionName); };

  return presentActionProperty("Stop Action", "Action to perform before the robot resumes driving",
                               getActionName, setActionName, true, state);
}

bool PropertiesPage::presentTrajectoryStartActionProperty(ThunderAutoTrajectorySkeleton& skeleton,
                                                          const ThunderAutoProjectState& state) {
  auto getActionName = [&]() -> const std::string& { return skeleton.startAction(); };
  auto setActionName = [&](const std::string& actionName) { skeleton.setStartAction(actionName); };

  return presentActionProperty("Start Action", "Action to perform before the robot starts driving",
                               getActionName, setActionName, true, state);
}

bool PropertiesPage::presentTrajectoryEndActionProperty(ThunderAutoTrajectorySkeleton& skeleton,
                                                        const ThunderAutoProjectState& state) {
  auto getActionName = [&]() -> const std::string& { return skeleton.endAction(); };
  auto setActionName = [&](const std::string& actionName) { skeleton.setEndAction(actionName); };

  return presentActionProperty("End Action", "Action to perform after the robot has finished driving",
                               getActionName, setActionName, true, state);
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

  ThunderAutoModeStepDirectoryPath rootPath;

  {
    auto scopedChildWindow = ImGui::Scoped::ChildWindow(
        "Auto Mode Steps Child Window",
        ImVec2(0.f, GET_UISIZE(PROPERTIES_PAGE_AUTO_MODE_STEP_LIST_CHILD_WINDOW_START_SIZE_Y)),
        ImGuiChildFlags_ResizeY | ImGuiChildFlags_Borders);

    ThunderAutoMode& autoMode = state.currentAutoMode();

    ThunderAutoModeStepTrajectoryBehaviorTreeNode behaviorTree =
        autoMode.getTrajectoryBehaviorTree(state.trajectories);

    (void)drawAutoModeStepsTree(rootPath, autoMode.steps, behaviorTree, std::nullopt, true, true, state);
  }

  // Dropping trajectories and actions into the child window adds them as steps to the end of the auto mode.
  (void)autoModeStepDragDropTarget(rootPath, AutoModeStepDragDropInsertMethod::INTO, false, state);

  if (ImGui::Button("+ Add Step", ImVec2(ImGui::GetContentRegionAvail().x, 0.f))) {
    m_event = Event::AUTO_MODE_ADD_STEP;
  }
}

bool PropertiesPage::drawAutoModeStepTreeNode(
    const ThunderAutoModeStepPath& stepPath,
    std::unique_ptr<ThunderAutoModeStep>& step,
    const ThunderAutoModeStepTrajectoryBehaviorTreeNode& stepBehaviorTree,
    std::optional<frc::Pose2d> previousStepEndPose,
    bool isFirstTrajectoryStep,
    bool isLastTrajectoryStep,
    ThunderAutoProjectState& state) {
  // Space in between steps to allow for drag-and-drop.
  {
    auto scopedPadding = ImGui::Scoped::StyleVarY(ImGuiStyleVar_ItemSpacing, 0.f);

    const float spacingX = std::max(ImGui::GetContentRegionAvail().x, 1.f);
    const float spacingY = GET_UISIZE(SELECTABLE_LIST_ITEM_SPACING_Y) / 3.f;
    (void)ImGui::InvisibleButton("Drag Separator", ImVec2(spacingX, spacingY));

    bool shouldStop =
        autoModeStepDragDropTarget(stepPath, AutoModeStepDragDropInsertMethod::BEFORE, true, state);
    if (shouldStop) {
      return true;
    }
  }

  // Draw the tree node for the step.

  const ThunderAutoModeStepTrajectoryBehavior& stepBehavior = stepBehaviorTree.behavior;

  ImGuiTreeNodeFlags treeNodeFlags =
      ImGuiTreeNodeFlags_DrawLinesFull | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanFullWidth;
  std::string treeNodeLabel;
  bool warnNoItemSelected = false;

  switch (step->type()) {
    using enum ThunderAutoModeStepType;
    case ACTION: {
      const ThunderAutoModeActionStep& actionStep = reinterpret_cast<const ThunderAutoModeActionStep&>(*step);

      bool noActionSelected = warnNoItemSelected = actionStep.actionName.empty();
      treeNodeLabel =
          fmt::format(ICON_LC_PAPERCLIP "  {}", noActionSelected ? "<none>" : actionStep.actionName);
      treeNodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;
      break;
    }
    case TRAJECTORY: {
      const ThunderAutoModeTrajectoryStep& trajectoryStep =
          reinterpret_cast<const ThunderAutoModeTrajectoryStep&>(*step);

      bool noTrajectorySelected = warnNoItemSelected = trajectoryStep.trajectoryName.empty();
      treeNodeLabel =
          fmt::format(ICON_LC_ROUTE "  {}", noTrajectorySelected ? "<none>" : trajectoryStep.trajectoryName);
      treeNodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;
      break;
    }
    case BRANCH_BOOL: {
      const ThunderAutoModeBoolBranchStep& branchStep =
          reinterpret_cast<const ThunderAutoModeBoolBranchStep&>(*step);

      bool noConditionSelected = warnNoItemSelected = branchStep.conditionName.empty();
      treeNodeLabel =
          fmt::format(ICON_LC_TOGGLE_RIGHT "  {}", noConditionSelected ? "<none>" : branchStep.conditionName);
      break;
    }
    case BRANCH_SWITCH: {
      const ThunderAutoModeSwitchBranchStep& branchStep =
          reinterpret_cast<const ThunderAutoModeSwitchBranchStep&>(*step);

      bool noConditionSelected = warnNoItemSelected = branchStep.conditionName.empty();
      treeNodeLabel =
          fmt::format(ICON_LC_LIST_ORDERED "  {}", noConditionSelected ? "<none>" : branchStep.conditionName);
      break;
    }
    default:
      ThunderAutoUnreachable("Invalid auto mode step type");
  }

  ThunderAutoModeEditorState& editorState = state.editorState.autoModeEditorState;
  bool isStepSelected = (editorState.selectedStepPath == stepPath);

  bool noStartPose = false, noEndPose = false, startPoseMismatch = false;
  if (stepBehavior.runsTrajectory) {
    noStartPose = (!isFirstTrajectoryStep && !stepBehavior.startPose.has_value());
    noEndPose = (!isLastTrajectoryStep && !stepBehavior.endPose.has_value());
    startPoseMismatch = (!isFirstTrajectoryStep && stepBehavior.startPose != previousStepEndPose);
  }

  bool warn = warnNoItemSelected || stepBehavior.errorInfo || noStartPose || noEndPose || startPoseMismatch;

  if (warn) {
    ImGui::PushStyleColor(ImGuiCol_Text, (ImU32)ThunderAutoColorPalette::kYellow);
  }

  ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, 0.f);
  auto scopedTreeNode = ImGui::Scoped::TreeNodeEx(
      treeNodeLabel.c_str(), treeNodeFlags | (isStepSelected ? ImGuiTreeNodeFlags_Selected : 0));
  ImGui::PopStyleVar();

  if (warn) {
    ImGui::PopStyleColor();
  }

  if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal) && warn) {
    auto scopedTooltip = ImGui::Scoped::Tooltip();
    if (warnNoItemSelected) {
      ImGui::SetTooltip(ICON_LC_TRIANGLE_ALERT " No %s selected",
                        ThunderAutoModeStepTypeToString(step->type()));
    }
    if (stepBehavior.errorInfo.isTrajectoryMissing) {
      ImGui::Text(ICON_LC_TRIANGLE_ALERT " Contains one or more references to\nnon-existent trajectories");
    }
    if (stepBehavior.errorInfo.containsNonContinuousSequence) {
      ImGui::Text(ICON_LC_TRIANGLE_ALERT
                  " Contains one or more sequences of\nnon-continuous trajectory steps");
    }
    if (noStartPose) {
      ImGui::Text(
          ICON_LC_TRIANGLE_ALERT
          " Impossible to determine step start pose\nbecause branches do not share a common\nstart pose");
    }
    if (noEndPose) {
      ImGui::Text(ICON_LC_TRIANGLE_ALERT
                  " Impossible to determine step end pose\nbecause branches do not share a common\nend pose");
    }
    if (startPoseMismatch) {
      ImGui::Text(ICON_LC_TRIANGLE_ALERT
                  " Step start pose does not match the end\npose of the previous trajectory step");
    }
  }

  if (ImGui::IsItemActivated() && !isStepSelected) {
    editorState.selectedStepPath = stepPath;
    m_history.addState(state, false);
  }

  // Right-click the step.
  if (auto popup = ImGui::Scoped::PopupContextItem()) {
    if (ImGui::MenuItem(ICON_LC_TRASH "  Delete")) {
      ThunderAutoLogger::Info("Deleting auto mode step at \"{}\"", ThunderAutoModeStepPathToString(stepPath));
      state.currentAutoModeDeleteStep(stepPath);
      m_history.addState(state);
      return true;
    }

    // Cases can be added to switch branches.
    if (step->type() == ThunderAutoModeStepType::BRANCH_SWITCH) {
      ThunderAutoModeSwitchBranchStep& branchStep = reinterpret_cast<ThunderAutoModeSwitchBranchStep&>(*step);

      if (auto scopedMenu = ImGui::Scoped::Menu(ICON_LC_PLUS "  Add Case")) {
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
        if (isNewCaseValueUsed &&
            ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled)) {
          ImGui::SetTooltip(ICON_LC_TRIANGLE_ALERT " Case value already exists");
        }
      }
    }
  }

  // Steps can be dragged.
  if (auto scopedDragSource = ImGui::Scoped::DragDropSource()) {
    std::vector<uint8_t> payloadData = SerializeAutoModeStepPathForDragDrop(stepPath);
    ImGui::SetDragDropPayload("Auto Mode Step", payloadData.data(), payloadData.size());
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
        // True Branch
        {
          auto scopedTrueBranchTreeNode =
              ImGui::Scoped::TreeNodeEx("TRUE", treeNodeFlags | ImGuiTreeNodeFlags_AllowOverlap);

          const ThunderAutoModeStepDirectoryPath trueChildStepPath = stepPath.boolBranch(true);

          if (autoModeStepDragDropTarget(trueChildStepPath, AutoModeStepDragDropInsertMethod::INTO, true,
                                         state))
            return true;

          // Editor display/hide button.
          ImGui::SameLine();
          if (presentRightAlignedEyeButton(1, branchStep.editorDisplayTrueBranch)) {
            branchStep.editorDisplayTrueBranch = !branchStep.editorDisplayTrueBranch;
            m_history.addState(state, false);
          }

          // Branch Steps
          if (scopedTrueBranchTreeNode) {
            bool shouldStop = drawAutoModeStepsTree(
                trueChildStepPath, branchStep.trueBranch, stepBehaviorTree.childrenMap.at(true),
                previousStepEndPose, isFirstTrajectoryStep, isLastTrajectoryStep, state);
            if (shouldStop) {
              return true;
            }
          }
        }

        // False Branch
        {
          auto scopedFalseBranchTreeNode =
              ImGui::Scoped::TreeNodeEx("FALSE", treeNodeFlags | ImGuiTreeNodeFlags_AllowOverlap);

          const ThunderAutoModeStepDirectoryPath falseChildStepPath = stepPath.boolBranch(false);

          if (autoModeStepDragDropTarget(falseChildStepPath, AutoModeStepDragDropInsertMethod::INTO, true,
                                         state))
            return true;

          // Editor display/hide button.
          ImGui::SameLine();
          if (presentRightAlignedEyeButton(0, !branchStep.editorDisplayTrueBranch)) {
            branchStep.editorDisplayTrueBranch = !branchStep.editorDisplayTrueBranch;
            m_history.addState(state, false);
          }

          // Branch Steps
          if (scopedFalseBranchTreeNode) {
            bool shouldStop = drawAutoModeStepsTree(
                falseChildStepPath, branchStep.elseBranch, stepBehaviorTree.childrenMap.at(false),
                previousStepEndPose, isFirstTrajectoryStep, isLastTrajectoryStep, state);
            if (shouldStop) {
              return true;
            }
          }
        }
      }
      break;
    }
    case BRANCH_SWITCH: {
      ThunderAutoModeSwitchBranchStep& branchStep = reinterpret_cast<ThunderAutoModeSwitchBranchStep&>(*step);

      if (scopedTreeNode) {
        for (auto& [caseValue, caseBranch] : branchStep.caseBranches) {
          // Case Branch
          {
            std::string label = fmt::format("CASE {}", caseValue);

            auto scopedCaseBranchTreeNode =
                ImGui::Scoped::TreeNodeEx(label.c_str(), treeNodeFlags | ImGuiTreeNodeFlags_AllowOverlap);

            const ThunderAutoModeStepDirectoryPath caseChildStepPath = stepPath.switchBranchCase(caseValue);

            if (autoModeStepDragDropTarget(caseChildStepPath, AutoModeStepDragDropInsertMethod::INTO, true,
                                           state))
              return true;

            // Right-click
            if (auto popup = ImGui::Scoped::PopupContextItem()) {
              if (ImGui::MenuItem(ICON_LC_TRASH "  Delete Case")) {
                ThunderAutoLogger::Info("Deleting case {} from switch branch at \"{}\"", caseValue,
                                        ThunderAutoModeStepPathToString(stepPath));
                branchStep.caseBranches.erase(caseValue);
                if (!branchStep.editorDisplayDefaultBranch &&
                    branchStep.editorDisplayCaseBranch == caseValue) {
                  branchStep.editorDisplayDefaultBranch = true;
                }
                m_history.addState(state);
                break;
              }
            }

            // Editor display/hide button.
            ImGui::SameLine();
            if (presentRightAlignedEyeButton(caseValue,
                                             !branchStep.editorDisplayDefaultBranch &&
                                                 branchStep.editorDisplayCaseBranch == caseValue)) {
              if (branchStep.editorDisplayDefaultBranch || branchStep.editorDisplayCaseBranch != caseValue) {
                branchStep.editorDisplayCaseBranch = caseValue;
                branchStep.editorDisplayDefaultBranch = false;
                m_history.addState(state, false);
              }
            }

            // Branch Steps
            if (scopedCaseBranchTreeNode) {
              bool shouldStop = drawAutoModeStepsTree(
                  caseChildStepPath, caseBranch, stepBehaviorTree.childrenMap.at(caseValue),
                  previousStepEndPose, isFirstTrajectoryStep, isLastTrajectoryStep, state);
              if (shouldStop) {
                return true;
              }
            }
          }
        }

        // Default Branch
        {
          auto scopedDefaultBranchTreeNode =
              ImGui::Scoped::TreeNodeEx("DEFAULT", treeNodeFlags | ImGuiTreeNodeFlags_AllowOverlap);

          const ThunderAutoModeStepDirectoryPath defaultChildStepPath = stepPath.switchBranchDefault();

          if (autoModeStepDragDropTarget(defaultChildStepPath, AutoModeStepDragDropInsertMethod::INTO, true,
                                         state)) {
            return true;
          }

          // Editor display/hide button.
          ImGui::SameLine();
          {
            auto scopedID = ImGui::Scoped::ID("Default Branch Eye Button");
            if (presentRightAlignedEyeButton(0, branchStep.editorDisplayDefaultBranch)) {
              if (!branchStep.editorDisplayDefaultBranch) {
                branchStep.editorDisplayDefaultBranch = true;
              }
              m_history.addState(state, false);
            }
          }

          // Branch Steps
          if (scopedDefaultBranchTreeNode) {
            bool shouldStop = drawAutoModeStepsTree(defaultChildStepPath, branchStep.defaultBranch,
                                                    stepBehaviorTree.childrenVec.at(0), previousStepEndPose,
                                                    isFirstTrajectoryStep, isLastTrajectoryStep, state);
            if (shouldStop) {
              return true;
            }
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

bool PropertiesPage::drawAutoModeStepsTree(const ThunderAutoModeStepDirectoryPath& parentPath,
                                           std::list<std::unique_ptr<ThunderAutoModeStep>>& steps,
                                           const ThunderAutoModeStepTrajectoryBehaviorTreeNode& behaviorTree,
                                           std::optional<frc::Pose2d> originalPreviousStepEndPose,
                                           bool isFirstTrajectoryStep,
                                           bool isLastTrajectoryStep,
                                           ThunderAutoProjectState& state) {
  ThunderAutoAssert(steps.size() == behaviorTree.childrenVec.size());

  const ThunderAutoModeStepTrajectoryBehavior& behavior = behaviorTree.behavior;

  std::optional<frc::Pose2d> previousStepEndPose = originalPreviousStepEndPose;
  ThunderAutoModeStepPath stepPath(parentPath, 0);
  size_t stepIndex = 0;
  for (auto& step : steps) {
    stepPath.setStepIndex(stepIndex);

    auto scopedID = ImGui::Scoped::ID(step->getID());

    bool first = false, last = false;
    if (behavior.trajectoryStepRange.has_value()) {
      first = isFirstTrajectoryStep && (stepIndex == behavior.trajectoryStepRange->first);
      last = isLastTrajectoryStep && (stepIndex == behavior.trajectoryStepRange->second);
    }

    const ThunderAutoModeStepTrajectoryBehaviorTreeNode& stepBehaviorTree =
        behaviorTree.childrenVec.at(stepIndex);

    bool shouldStop =
        drawAutoModeStepTreeNode(stepPath, step, stepBehaviorTree, previousStepEndPose, first, last, state);
    if (shouldStop) {
      return true;
    }

    previousStepEndPose = stepBehaviorTree.behavior.endPose;
    stepIndex++;
  }

  // Space at the bottom to allow for drag-and-drop.
  {
    auto scopedID = ImGui::Scoped::ID("Bottom Drag Target");
    auto scopedPadding = ImGui::Scoped::StyleVarY(ImGuiStyleVar_ItemSpacing, 0.f);

    const float spacingX = std::max(ImGui::GetContentRegionAvail().x, 1.f);
    const float spacingY = GET_UISIZE(SELECTABLE_LIST_ITEM_SPACING_Y) / 3.f;
    (void)ImGui::InvisibleButton("Drag Separator", ImVec2(spacingX, spacingY));

    bool shouldStop;
    if (steps.empty()) {
      shouldStop =
          autoModeStepDragDropTarget(parentPath, AutoModeStepDragDropInsertMethod::INTO, true, state);
    } else {
      shouldStop = autoModeStepDragDropTarget(stepPath, AutoModeStepDragDropInsertMethod::AFTER, true, state);
    }

    if (shouldStop) {
      return true;
    }
  }

  return false;
}

bool PropertiesPage::autoModeStepDragDropTarget(
    std::variant<ThunderAutoModeStepPath, ThunderAutoModeStepDirectoryPath> closestStepOrDirectoryPath,
    AutoModeStepDragDropInsertMethod insertMethod,
    bool acceptAutoModeSteps,
    ThunderAutoProjectState& state) {
  using enum AutoModeStepDragDropInsertMethod;

  if (auto scopedDragTarget = ImGui::Scoped::DragDropTarget()) {
    const ImGuiPayload* payload = nullptr;
    if (acceptAutoModeSteps && (payload = ImGui::AcceptDragDropPayload("Auto Mode Step"))) {
      ThunderAutoModeStepPath payloadPath =
          DeserializeAutoModeStepPathFromDragDrop(payload->Data, payload->DataSize);

      // Move the step.

      bool moveWasSuccessful = false;

      if (insertMethod == INTO) {
        auto directoryPath = std::get<ThunderAutoModeStepDirectoryPath>(closestStepOrDirectoryPath);
        ThunderAutoLogger::Info("Drag and drop auto mode step from \"{}\" into \"{}\"",
                                ThunderAutoModeStepPathToString(payloadPath),
                                ThunderAutoModeStepDirectoryPathToString(directoryPath));

        moveWasSuccessful = state.currentAutoModeMoveStepIntoDirectory(payloadPath, directoryPath);
      } else {
        auto stepPath = std::get<ThunderAutoModeStepPath>(closestStepOrDirectoryPath);
        ThunderAutoLogger::Info("Drag and drop auto mode step from \"{}\" to {} \"{}\"",
                                ThunderAutoModeStepPathToString(payloadPath),
                                (insertMethod == BEFORE ? "before" : "after"),
                                ThunderAutoModeStepPathToString(stepPath));

        if (insertMethod == BEFORE) {
          moveWasSuccessful = state.currentAutoModeMoveStepBeforeOther(payloadPath, stepPath);
        } else if (insertMethod == AFTER) {
          moveWasSuccessful = state.currentAutoModeMoveStepAfterOther(payloadPath, stepPath);
        }
      }
      if (moveWasSuccessful) {
        m_history.addState(state);
      } else {
        ThunderAutoLogger::Warn("Failed to move auto mode step");
      }
      return moveWasSuccessful;

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
          auto directoryPath = std::get<ThunderAutoModeStepDirectoryPath>(closestStepOrDirectoryPath);
          state.currentAutoModeInsertStepInDirectory(directoryPath, std::move(newStep));
        } else {
          auto stepPath = std::get<ThunderAutoModeStepPath>(closestStepOrDirectoryPath);
          if (insertMethod == BEFORE) {
            state.currentAutoModeInsertStepBeforeOther(stepPath, std::move(newStep));
          } else if (insertMethod == AFTER) {
            state.currentAutoModeInsertStepAfterOther(stepPath, std::move(newStep));
          }
        }
        m_history.addState(state);
        return true;
      }
    }
  }

  return false;
}

std::vector<uint8_t> PropertiesPage::SerializeAutoModeStepPathForDragDrop(
    const ThunderAutoModeStepPath& path) {
  std::span<const ThunderAutoModeStepDirectoryPath::Node> directoryPath = path.directoryPath().path();
  size_t size = sizeof(size_t) + directoryPath.size() * sizeof(ThunderAutoModeStepDirectoryPath::Node);

  std::vector<uint8_t> dataVec(size);
  uint8_t* data = dataVec.data();

  size_t stepIndex = path.stepIndex();
  std::memcpy(data, &stepIndex, sizeof(size_t));
  data += sizeof(size_t);
  for (const auto& node : directoryPath) {
    std::memcpy(data, &node, sizeof(ThunderAutoModeStepDirectoryPath::Node));
    data += sizeof(ThunderAutoModeStepDirectoryPath::Node);
  }

  return dataVec;
}

ThunderAutoModeStepPath PropertiesPage::DeserializeAutoModeStepPathFromDragDrop(void* rawData, size_t size) {
  ThunderAutoAssert(size >= sizeof(size_t));

  char* data = reinterpret_cast<char*>(rawData);

  size_t stepIndex = *reinterpret_cast<size_t*>(data);
  size -= sizeof(size_t);
  data += sizeof(size_t);

  ThunderAutoAssert(size % sizeof(ThunderAutoModeStepDirectoryPath::Node) == 0);
  size_t numPathNodes = size / sizeof(ThunderAutoModeStepDirectoryPath::Node);

  std::vector<ThunderAutoModeStepDirectoryPath::Node> pathNodes;
  pathNodes.reserve(numPathNodes);
  for (size_t i = 0; i < numPathNodes; ++i) {
    auto& node = *reinterpret_cast<ThunderAutoModeStepDirectoryPath::Node*>(data);
    pathNodes.push_back(node);
    data += sizeof(ThunderAutoModeStepDirectoryPath::Node);
  }

  ThunderAutoModeStepDirectoryPath directoryPath(std::move(pathNodes));
  ThunderAutoModeStepPath path(std::move(directoryPath), stepIndex);
  return path;
}

void PropertiesPage::presentAutoModeSelectedStepProperties(ThunderAutoProjectState& state) {
  ThunderAutoModeEditorState& editorState = state.editorState.autoModeEditorState;
  ThunderAutoMode& mode = state.currentAutoMode();

  presentSeparatorText("Selected Auto Mode Step");

  auto scopedID = ImGui::Scoped::ID("Selected Step Properties");

  if (!editorState.selectedStepPath.has_value()) {
    ImGui::Text("No step selected");
    return;
  }

  ThunderAutoModeStep& step = mode.getStepAtPath(editorState.selectedStepPath.value());
  switch (step.type()) {
    using enum ThunderAutoModeStepType;
    case ACTION:
      presentAutoModeSelectedActionStepProperties(reinterpret_cast<ThunderAutoModeActionStep&>(step), state);
      break;
    case TRAJECTORY:
      presentAutoModeSelectedTrajectoryStepProperties(reinterpret_cast<ThunderAutoModeTrajectoryStep&>(step),
                                                      state);
      break;
    case BRANCH_BOOL:
      presentAutoModeSelectedBoolBranchStepProperties(reinterpret_cast<ThunderAutoModeBoolBranchStep&>(step),
                                                      state);
      break;
    case BRANCH_SWITCH:
      presentAutoModeSelectedSwitchBranchStepProperties(
          reinterpret_cast<ThunderAutoModeSwitchBranchStep&>(step), state);
      break;
    default:
      ThunderAutoUnreachable("Invalid auto mode step type");
  }
}

void PropertiesPage::presentAutoModeSelectedActionStepProperties(ThunderAutoModeActionStep& step,
                                                                 ThunderAutoProjectState& state) {
  bool changed = false;

  auto getActionName = [&]() -> const std::string& { return step.actionName; };
  auto setActionName = [&](const std::string& actionName) { step.actionName = actionName; };

  changed |= presentActionProperty("Action", nullptr, getActionName, setActionName, false, state);

  if (changed) {
    m_history.addState(state);
  }
}

void PropertiesPage::presentAutoModeSelectedTrajectoryStepProperties(ThunderAutoModeTrajectoryStep& step,
                                                                     ThunderAutoProjectState& state) {
  bool changed = false;

  auto getTrajectoryName = [&]() -> const std::string& { return step.trajectoryName; };
  auto setTrajectoryName = [&](const std::string& trajectoryName) { step.trajectoryName = trajectoryName; };

  changed |=
      presentTrajectoryProperty("Trajectory", nullptr, getTrajectoryName, setTrajectoryName, false, state);

  if (changed) {
    m_history.addState(state);
  }
}

void PropertiesPage::presentAutoModeSelectedBoolBranchStepProperties(ThunderAutoModeBoolBranchStep& step,
                                                                     ThunderAutoProjectState& state) {
  bool changed = false;

  {
    auto scopedField = ImGui::ScopedField::Builder("Condition")
                           .tooltip("Name of the registered boolean\ncondition to evaluate at this step")
                           .build();

    auto getConditionName = [&]() -> const std::string& { return step.conditionName; };
    auto setConditionName = [&](const std::string& conditionName) { step.conditionName = conditionName; };

    static bool isShowingInput = false;
    static char conditionNameInputBuffer[256] = "";

    changed |= presentInputText("##Bool Condition Name", getConditionName, setConditionName,
                                conditionNameInputBuffer, sizeof(conditionNameInputBuffer), isShowingInput);
  }

  if (changed) {
    m_history.addState(state);
  }
}

void PropertiesPage::presentAutoModeSelectedSwitchBranchStepProperties(ThunderAutoModeSwitchBranchStep& step,
                                                                       ThunderAutoProjectState& state) {
  bool changed = false;

  {
    auto scopedField = ImGui::ScopedField::Builder("Condition")
                           .tooltip("Name of the registered switch\ncondition to evaluate at this step")
                           .build();

    auto getConditionName = [&]() -> const std::string& { return step.conditionName; };
    auto setConditionName = [&](const std::string& conditionName) { step.conditionName = conditionName; };

    static bool isShowingInput = false;
    static char conditionNameInputBuffer[256] = "";

    changed |= presentInputText("##Switch Condition Name", getConditionName, setConditionName,
                                conditionNameInputBuffer, sizeof(conditionNameInputBuffer), isShowingInput);
  }

  if (changed) {
    m_history.addState(state);
  }
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

bool PropertiesPage::presentRightAlignedEyeButton(int id, bool isEyeOpen) {
  auto scopedID = ImGui::Scoped::ID(id);

  const ImGuiStyle& style = ImGui::GetStyle();

  const float buttonDimension = style.FontSizeBase + style.FramePadding.y * 2.0f;
  const ImVec2 buttonSize = ImVec2(buttonDimension, buttonDimension);

  const float regionAvailWidth = ImGui::GetContentRegionAvail().x;
  if (regionAvailWidth > buttonSize.x) {
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + regionAvailWidth - buttonSize.x);
  }

  ImVec4 buttonColor = style.Colors[ImGuiCol_Button];
  buttonColor.w = 0.f;
  auto scopedButtonColor = ImGui::Scoped::StyleColor(ImGuiCol_Button, buttonColor);
  auto scopedButtonHoverColor = ImGui::Scoped::StyleColor(ImGuiCol_ButtonHovered, buttonColor);
  auto scopedButtonActiveColor = ImGui::Scoped::StyleColor(ImGuiCol_ButtonActive, buttonColor);
  auto scopedBorderColor = ImGui::Scoped::StyleColor(ImGuiCol_Border, buttonColor);

  const char* buttonIcon = isEyeOpen ? ICON_LC_EYE : ICON_LC_EYE_OFF;

  if (!isEyeOpen) {  // Gray out the closed eye icon
    ImGui::PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
  }

  bool result = ImGui::Button(buttonIcon, buttonSize);

  if (!isEyeOpen) {
    ImGui::PopStyleColor();
  }

  return result;
}

bool PropertiesPage::presentActionProperty(const char* name,
                                           const char* tooltip,
                                           std::function<const std::string&()> getName,
                                           std::function<void(const std::string&)> setName,
                                           bool includeNoneOption,
                                           const ThunderAutoProjectState& state) {
  auto scopedField = ImGui::ScopedField::Builder(name).tooltip(tooltip).build();

  std::string currentName = getName();
  if (currentName.empty()) {
    currentName = "<none>";
  }
  bool changed = false;

  std::string comboID = fmt::format("##{} Name", name);
  if (auto scopedCombo = ImGui::Scoped::Combo(comboID.c_str(), currentName.c_str())) {
    if (includeNoneOption) {
      if (ImGui::Selectable("<none>", currentName.empty()) && !currentName.empty()) {
        setName("");
        changed = true;
      }

      if (!state.actionsOrder.empty()) {
        ImGui::Separator();
      }
    }

    for (const std::string& name : state.actionsOrder) {
      bool isNameSelected = (name == currentName);
      if (ImGui::Selectable(name.c_str(), isNameSelected) && !isNameSelected) {
        setName(name);
        changed = true;
        break;
      }
    }
  }

  return changed;
}

bool PropertiesPage::presentTrajectoryProperty(const char* name,
                                               const char* tooltip,
                                               std::function<const std::string&()> getName,
                                               std::function<void(const std::string&)> setName,
                                               bool includeNoneOption,
                                               const ThunderAutoProjectState& state) {
  auto scopedField = ImGui::ScopedField::Builder(name).tooltip(tooltip).build();

  std::string currentName = getName();
  if (currentName.empty()) {
    currentName = "<none>";
  }
  bool changed = false;

  std::string comboID = fmt::format("##{} Name", name);
  if (auto scopedCombo = ImGui::Scoped::Combo(comboID.c_str(), currentName.c_str())) {
    if (includeNoneOption) {
      if (ImGui::Selectable("<none>", currentName.empty()) && !currentName.empty()) {
        setName("");
        changed = true;
      }

      if (!state.trajectories.empty()) {
        ImGui::Separator();
      }
    }

    for (const auto& [name, trajectory] : state.trajectories) {
      bool isNameSelected = (name == currentName);
      if (ImGui::Selectable(name.c_str(), isNameSelected) && !isNameSelected) {
        setName(name);
        changed = true;
        break;
      }
    }
  }

  return changed;
}

bool PropertiesPage::presentInputText(const char* id,
                                      std::function<const std::string&()> getText,
                                      std::function<void(const std::string&)> setText,
                                      char* workingInputBuffer,
                                      size_t workingInputBufferSize,
                                      bool& isShowingInput) {
  auto scopedID = ImGui::Scoped::ID(id);

  bool changed = false;
  const std::string text = getText();

  const ImGuiStyle& style = ImGui::GetStyle();

  const float regionAvailWidth = ImGui::GetContentRegionAvail().x;
  ImGuiWindow* window = ImGui::GetCurrentWindow();
  const ImGuiID inputID = window->GetID("##Input Text");

  const ImVec2 posBefore = window->DC.CursorPos;

  const float buttonDimension = ImGui::GetStyle().FontSizeBase + style.FramePadding.y * 2.0f;
  const float nameAreaWidth = regionAvailWidth - buttonDimension - style.ItemSpacing.x;
  const ImVec2 buttonSize = ImVec2(buttonDimension, buttonDimension);

  bool wasShowingInput = isShowingInput;
  {
    ImVec4 buttonColor = style.Colors[ImGuiCol_Button];
    buttonColor.w = 0.f;
    auto scopedButtonColor = ImGui::Scoped::StyleColor(ImGuiCol_Button, buttonColor);
    auto scopedButtonHoverColor = ImGui::Scoped::StyleColor(ImGuiCol_ButtonHovered, buttonColor);
    auto scopedButtonActiveColor = ImGui::Scoped::StyleColor(ImGuiCol_ButtonActive, buttonColor);
    auto scopedButtonTextAlign = ImGui::Scoped::StyleVarX(ImGuiStyleVar_ButtonTextAlign, 0.0f);

    const std::string displayText = text.empty() ? "<none>" : text;
    {
      auto scopedButtonID = ImGui::Scoped::ID("##Display Button");
      (void)ImGui::Button(displayText.c_str(), ImVec2(nameAreaWidth, buttonSize.y));
    }
  }

  bool doubleClicked = (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left));

  ImGui::SameLine();
  if (ImGui::Button(ICON_LC_PENCIL, buttonSize) || doubleClicked) {
    isShowingInput = true;
    size_t textSize = std::min(workingInputBufferSize - 1, getText().size());
    std::strncpy(workingInputBuffer, text.c_str(), textSize);
    workingInputBuffer[textSize] = '\0';

    ImGui::SetActiveID(inputID, window);
    ImGui::SetKeyboardFocusHere();

    // Reload the ImGui user buffer for the InputText
    if (ImGuiInputTextState* state = ImGui::GetInputTextState(inputID)) {
      state->ReloadUserBufAndMoveToEnd();
    }
  }

  if (isShowingInput) {
    window->DC.CursorPos = posBefore;

    ImGuiInputTextCallback callback = [](ImGuiInputTextCallbackData* data) -> int {
      // [a-zA-Z0-9_ ]
      if (data->EventFlag == ImGuiInputTextFlags_CallbackCharFilter) {
        return std::isalnum(data->EventChar) || data->EventChar == '_' || data->EventChar == ' ' ? 0 : 1;
      }
      return 0;
    };

    ImGui::SetNextItemWidth(regionAvailWidth);
    ImGui::InputText("##Input Text", workingInputBuffer, workingInputBufferSize,
                     ImGuiInputTextFlags_CallbackCharFilter | ImGuiInputTextFlags_NoUndoRedo, callback);

    if (wasShowingInput) {  // Ignore the first frame to avoid immediate close.
      if (!ImGui::IsItemActive()) {
        isShowingInput = false;
      }

      if (ImGui::IsItemDeactivated()) {
        if (ImGui::IsItemDeactivatedAfterEdit()) {
          // Save input.
          std::string newText(workingInputBuffer);
          if (newText != text) {
            setText(newText);
            changed = true;
          }
        }
        isShowingInput = false;
      }
    }
  }

  return changed;
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
