#include <ThunderAuto/Pages/PropertiesPage.hpp>

#include <ThunderAuto/ImGuiScopedField.hpp>
#include <ThunderAuto/FontLibrary.hpp>
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
      // processAutoModeProperties(state);
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
  presentTrajectorySpeedConstraintProperties(state);
}

void PropertiesPage::presentTrajectoryItemList(ThunderAutoProjectState& state) {
  if (!ImGui::CollapsingHeader("Trajectory Items", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
    return;

  if (auto scopedTabBar = ImGui::Scoped::TabBar("TrajectoryItems")) {
    const char* const childWindowName = "TrajectoryItemsChildWindow";

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

  if (!ImGui::CollapsingHeader("Selected Trajectory Item", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
    return;

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

  // Start/end rotations.

  if (isFirstPoint) {
    ImGui::Separator();
    changed |= presentTrajectoryStartRotationProperty(skeleton);
  } else if (isLastPoint) {
    ImGui::Separator();
    changed |= presentTrajectoryEndRotationProperty(skeleton);
  }

  // Actions
  if (isFirstPoint) {
    ImGui::Separator();
    changed |= presentTrajectoryStartActionsProperty(skeleton, state);
  } else if (isLastPoint) {
    ImGui::Separator();
    changed |= presentTrajectoryEndActionsProperty(skeleton, state);
  }

  ImGui::Separator();

  // Max velocity override.
  changed |= presentPointVelocityOverrideProperty(point, isFirstPoint, isLastPoint,
                                                  skeleton.settings().maxLinearVelocity);

  if (!isFirstPoint && !isLastPoint && point.isStopped()) {
    changed |= presentPointStopRotationProperty(point);
    changed |= presentPointStopActionsProperty(point, state);
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

    {
      auto scopedFont = ImGui::Scoped::Font(FontLibrary::get().boldFont, 0.f);
      auto scopedNoNav = ImGui::Scoped::ItemFlag(ImGuiItemFlags_NoNav, true);
      (void)ImGui::Button("X", buttonSize);
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
      ImGui::SetTooltip("X Position");
    }

    ImGui::SameLine();

    ImGui::SetNextItemWidth(sliderWidth);
    changed |= presentSlider("##X Position", x, kFieldPositionSliderSpeed, "%.2f m");

    ImGui::SameLine();

    {
      auto scopedFont = ImGui::Scoped::Font(FontLibrary::get().boldFont, 0.f);
      auto scopedNoNav = ImGui::Scoped::ItemFlag(ImGuiItemFlags_NoNav, true);
      (void)ImGui::Button("Y", buttonSize);
    }
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

    {
      auto scopedFont = ImGui::Scoped::Font(FontLibrary::get().boldFont, 0.f);
      auto scopedNoNav = ImGui::Scoped::ItemFlag(ImGuiItemFlags_NoNav, true);
      (void)ImGui::Button(ICON_FA_ARROW_LEFT, buttonSize);
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
      ImGui::SetTooltip("Incoming Heading");
    }

    ImGui::SameLine();

    ImGui::SetNextItemWidth(sliderWidth);
    changed |= presentSlider("##Incoming Heading", incomingHeading, kAngleSliderSpeed, "%.2f°");

    ImGui::SameLine();

    {
      auto scopedFont = ImGui::Scoped::Font(FontLibrary::get().boldFont, 0.f);
      auto scopedNoNav = ImGui::Scoped::ItemFlag(ImGuiItemFlags_NoNav, true);
      (void)ImGui::Button(ICON_FA_ARROW_RIGHT, buttonSize);
    }
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

    {
      auto scopedFont = ImGui::Scoped::Font(FontLibrary::get().boldFont, 0.f);
      auto scopedNoNav = ImGui::Scoped::ItemFlag(ImGuiItemFlags_NoNav, true);
      (void)ImGui::Button(ICON_FA_ARROW_LEFT, buttonSize);
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
      ImGui::SetTooltip("Incoming Weight");
    }

    ImGui::SameLine();

    ImGui::SetNextItemWidth(sliderWidth);
    changed |= presentSlider("##Incoming Weight", incomingWeight, kFieldPositionSliderSpeed, "%.2f m");

    ImGui::SameLine();

    {
      auto scopedFont = ImGui::Scoped::Font(FontLibrary::get().boldFont, 0.f);
      auto scopedNoNav = ImGui::Scoped::ItemFlag(ImGuiItemFlags_NoNav, true);
      (void)ImGui::Button(ICON_FA_ARROW_RIGHT, buttonSize);
    }
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

bool PropertiesPage::presentActionsProperty(const char* name,
                                            const char* tooltip,
                                            const std::unordered_set<std::string>& actions,
                                            std::function<bool(const std::string&)> removeAction,
                                            std::function<bool(const std::string&)> addAction,
                                            std::span<const std::string> availableActions) {
  bool changed = false;

  auto scopedField = ImGui::ScopedField::Builder(name).tooltip(tooltip).build();

  {
    auto scopedChildWindow = ImGui::Scoped::ChildWindow(
        "Actions", ImVec2(0.f, GET_UISIZE(PROPERTIES_PAGE_ACTIONS_LIST_CHILD_WINDOW_START_SIZE_Y)),
        ImGuiChildFlags_ResizeY | ImGuiChildFlags_Borders);

    auto scopedPadding =
        ImGui::Scoped::StyleVarY(ImGuiStyleVar_ItemSpacing, GET_UISIZE(SELECTABLE_LIST_ITEM_SPACING_Y));

    size_t i = 0;
    for (const std::string& action : actions) {
      auto scopedID = ImGui::Scoped::ID(i++);

      (void)ImGui::Selectable(action.c_str(), false, ImGuiSelectableFlags_AllowOverlap);

      ImGui::SameLine();

      const ImGuiStyle& style = ImGui::GetStyle();
      const float removeButtonWidthNeeded = ImGui::CalcTextSize(ICON_FA_TRASH).x + style.ItemSpacing.x;
      const float removeButtonCursorOffset = ImGui::GetContentRegionAvail().x - removeButtonWidthNeeded;
      if (removeButtonCursorOffset > 0) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + removeButtonCursorOffset);
      }

      auto scopedButtonColor = ImGui::Scoped::StyleColor(ImGuiCol_Button, 0);
      auto scopedButtonHoveredColor = ImGui::Scoped::StyleColor(ImGuiCol_ButtonHovered, 0);
      auto scopedButtonActiveColor = ImGui::Scoped::StyleColor(ImGuiCol_ButtonActive, 0);

      if (ImGui::SmallButton(ICON_FA_TRASH)) {
        changed |= removeAction(action);
        break;
      }
    }
  }
  if (auto scopedDragTarget = ImGui::Scoped::DragDropTarget()) {
    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Action")) {
      std::string payloadActionName = reinterpret_cast<const char*>(payload->Data);
      changed |= addAction(payloadActionName);
    }
  }

  std::string addActionPopupName = fmt::format("Add Action ({})", name);

  ImVec2 buttonSize(ImGui::GetContentRegionAvail().x, 0.f);
  if (ImGui::Button("+ Add Action", buttonSize)) {
    ImGui::OpenPopup(addActionPopupName.c_str());
  }
  if (auto scopedPopup = ImGui::Scoped::Popup(addActionPopupName.c_str())) {
    std::string selectedAction;

    if (auto scopedCombo = ImGui::Scoped::Combo("##Action Name", "")) {
      for (const std::string& action : availableActions) {
        auto scopedDisabled = ImGui::Scoped::Disabled(actions.contains(action));

        if (ImGui::Selectable(action.c_str(), false)) {
          selectedAction = action;
          break;
        }
      }
    }

    if (!selectedAction.empty()) {
      changed |= addAction(selectedAction);
      ImGui::CloseCurrentPopup();
    }
  }

  return changed;
}

bool PropertiesPage::presentPointStopActionsProperty(ThunderAutoTrajectorySkeletonWaypoint& point,
                                                     const ThunderAutoProjectState& state) {
  ThunderAutoAssert(point.isStopped());

  auto removeAction = [&](const std::string& actionName) { return point.removeStopAction(actionName); };
  auto addAction = [&](const std::string& actionName) { return point.addStopAction(actionName); };

  return presentActionsProperty("Stop Actions",
                                "Actions to perform (concurrently) before the robot resumes driving",
                                point.stopActions(), removeAction, addAction, state.actionsOrder);
}

bool PropertiesPage::presentTrajectoryStartActionsProperty(ThunderAutoTrajectorySkeleton& skeleton,
                                                           const ThunderAutoProjectState& state) {
  auto removeAction = [&](const std::string& actionName) { return skeleton.removeStartAction(actionName); };
  auto addAction = [&](const std::string& actionName) { return skeleton.addStartAction(actionName); };

  return presentActionsProperty("Actions",
                                "Actions to perform (concurrently) before the robot starts driving",
                                skeleton.startActions(), removeAction, addAction, state.actionsOrder);
}

bool PropertiesPage::presentTrajectoryEndActionsProperty(ThunderAutoTrajectorySkeleton& skeleton,
                                                         const ThunderAutoProjectState& state) {
  auto removeAction = [&](const std::string& actionName) { return skeleton.removeEndAction(actionName); };
  auto addAction = [&](const std::string& actionName) { return skeleton.addEndAction(actionName); };

  return presentActionsProperty("Actions",
                                "Actions to perform (concurrently) after the robot has finished driving",
                                skeleton.endActions(), removeAction, addAction, state.actionsOrder);
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

void PropertiesPage::presentTrajectorySpeedConstraintProperties(ThunderAutoProjectState& state) {
  if (!ImGui::CollapsingHeader("Trajectory Speed Constraints", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
    return;

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

// void PropertiesPage::presentAutoModeProperties(ThunderAutoProjectState& state) {}

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

  if (skeleton.hasStartActions()) {
    PropertiesPage::TrajectoryItemSelection<std::string> startActionSelection{
        .item = "... <start actions> ...",
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
      if (waypointIt->isStopped() && waypointIt->hasStopActions()) {
        PropertiesPage::TrajectoryItemSelection<std::string> acitionSelection{
            .item = "... <stop actions> ...",
            .editorLocked = waypointIt->isEditorLocked(),
            .trajectorySelection = ThunderAutoTrajectoryEditorState::TrajectorySelection::WAYPOINT,
            .selectionIndex = waypointIndex,
        };

        actionSelections.emplace(ThunderAutoTrajectoryPosition(static_cast<double>(waypointIndex)),
                                 acitionSelection);
      }
    }
  }

  if (skeleton.hasEndActions()) {
    PropertiesPage::TrajectoryItemSelection<std::string> startActionSelection{
        .item = "... <end actions> ...",
        .editorLocked = skeleton.back().isEditorLocked(),
        .trajectorySelection = ThunderAutoTrajectoryEditorState::TrajectorySelection::WAYPOINT,
        .selectionIndex = skeleton.numPoints() - 1,
    };

    actionSelections.emplace(ThunderAutoTrajectoryPosition(static_cast<double>(skeleton.numPoints() - 1)),
                             startActionSelection);
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
