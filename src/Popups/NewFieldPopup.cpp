#include <ThunderAuto/Popups/NewFieldPopup.hpp>

#include <ThunderAuto/Platform/Platform.hpp>
#include <ThunderAuto/ImGuiScopedField.hpp>
#include <ThunderAuto/Logger.hpp>
#include <imgui.h>
#include <imgui_raii.h>
#include <imgui_internal.h>
#include <fmt/format.h>
#include <stb_image.h>

static const ImColor kFieldBoundsWidgetColor = ImColor(252, 186, 3, 255);

void NewFieldPopup::present(bool* running) {
  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), false, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(
      ImVec2(GET_UISIZE(NEW_FIELD_POPUP_START_WIDTH), GET_UISIZE(NEW_FIELD_POPUP_START_HEIGHT)),
      ImGuiCond_FirstUseEver);
  auto scopedWin = ImGui::Scoped::PopupModal(name(), running);
  if (!scopedWin || !*running) {
    m_result = Result::CANCEL;
    return;
  }

  m_result = Result::NONE;

  if (m_isImageSelected) {
    if (ImGui::Button("Back")) {
      m_isImageSelected = false;
    }

    ImGui::SameLine();

    if (ImGui::Button("Done")) {
      m_result = Result::CREATE;

      Measurement2d size = m_field->fieldSize();
      Rect bounds = m_field->imageFieldBounds();
      ThunderAutoLogger::Info(
          "NewFieldPopup: Done. Info:\n"
          "\tField image: '{}'\n"
          "\tWidth: {:.3f} m\n"
          "\tLength: {:.3f} m\n"
          "\tBounds Min: ({:.3f}, {:.3f})\n"
          "\tBounds Max: ({:.3f}, {:.3f})",
          m_field->customImagePath().string(), size.width.value(), size.length.value(), bounds.min.x,
          bounds.min.y, bounds.max.x, bounds.max.y);
    }

    ImGui::Separator();

    presentFieldBoundsSetup();
  }
  if (!m_isImageSelected) {
    m_field = std::nullopt;

    std::filesystem::path imagePath;

    {
      auto scopedField = ImGui::ScopedField::Builder("Field Image").build();

      // Shrink the input text to make space for the browse button.
      {
        const ImGuiStyle& style = ImGui::GetStyle();
        float browseButtonWidth = ImGui::CalcTextSize("Browse").x + style.FramePadding.x * 2.f;
        ImGui::SetNextItemWidth(ImGui::CalcItemWidth() - style.ItemSpacing.x - browseButtonWidth);
      }

      ImGui::InputText("##Field Image Path", m_imagePathBuf, sizeof(m_imagePathBuf));
      imagePath = m_imagePathBuf;

      ImGui::SameLine();

      if (ImGui::Button("Browse")) {
        std::filesystem::path selectedImagePath = getPlatform().openFileDialog(FileType::FILE, {});

        if (!selectedImagePath.empty()) {
          imagePath = selectedImagePath;
          std::string imagePathStr = imagePath.string();
          strncpy(m_imagePathBuf, imagePathStr.c_str(), sizeof(m_imagePathBuf) - 1);
          m_imagePathBuf[sizeof(m_imagePathBuf) - 1] = '\0';
        }
      }
    }

    {
      auto scopedField = ImGui::ScopedField::Builder("Field Width").build();
      ImGui::DragFloat("##Field Width", &m_fieldWidth, 0.5f, 0.0f, 0.0f, "%.3f m");
      m_fieldWidth = std::max(m_fieldWidth, 0.0f);
    }
    {
      auto scopedField = ImGui::ScopedField::Builder("Field Length").build();
      ImGui::DragFloat("##Field Length", &m_fieldLength, 0.5f, 0.0f, 0.0f, "%.3f m");
      m_fieldLength = std::max(m_fieldLength, 0.0f);
    }

    if (ImGui::Button("Cancel")) {
      m_result = Result::CANCEL;
    }

    ImGui::SameLine();

    bool isContinueDisabled = false;
    std::string disabledTooltipText;

    if (imagePath.empty()) {
      isContinueDisabled = true;
      disabledTooltipText = "Image path not set";
    } else if (!std::filesystem::exists(imagePath) && std::filesystem::is_regular_file(imagePath)) {
      isContinueDisabled = true;
      disabledTooltipText = fmt::format("Image '{}' does not exist", imagePath.string());
    } else if (m_fieldWidth < 1e-9) {
      isContinueDisabled = true;
      disabledTooltipText = "Field width must be greater than 0 m";
    } else if (m_fieldLength < 1e-9) {
      isContinueDisabled = true;
      disabledTooltipText = "Field length must be greater than 0 m";
    }

    bool showDisabledTooltip = false;
    {
      auto scopedDisabled = ImGui::Scoped::Disabled(isContinueDisabled);

      if (ImGui::Button("Continue")) {
        m_fieldTexture = PlatformTexture::make(imagePath);
        if (m_fieldTexture) {
          m_isImageSelected = true;

          m_fieldAspectRatio =
              static_cast<double>(m_fieldTexture->width()) / static_cast<double>(m_fieldTexture->height());

          Rect rect{Vec2(0.0, 0.0), Vec2(1.0, 1.0)};
          Measurement2d size{units::meter_t(m_fieldWidth), units::meter_t(m_fieldLength)};

          m_field = ThunderAutoFieldImage(imagePath, rect, size);

          ThunderAutoLogger::Info("NewFieldPopup: Loaded custom field image from path '{}'",
                                  imagePath.string());

        } else {
          m_imageLoadFailed = true;
          ThunderAutoLogger::Error("NewFieldPopup: Failed to load image from path '{}'", imagePath.string());
        }
      }

      showDisabledTooltip =
          ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled) &&
          !disabledTooltipText.empty();
    }
    if (showDisabledTooltip) {
      ImGui::SetTooltip("%s", disabledTooltipText.c_str());
    }

    if (m_imageLoadFailed) {
      ImGui::Text("Failed to load image from path: %s", imagePath.string().c_str());
    }
  }

  if (m_result != Result::NONE) {
    *running = false;
  }
}

void NewFieldPopup::presentFieldBoundsSetup() {
  const ImGuiIO& io = ImGui::GetIO();
  ImDrawList* drawList = ImGui::GetWindowDrawList();
  ImGuiWindow* win = ImGui::GetCurrentWindow();
  if (win->SkipItems)
    return;

  const float pointRadius = GET_UISIZE(DRAG_POINT_RADIUS);

  ImVec2 windowSize = ImGui::GetWindowSize();

  float dimX = windowSize.x - ImGui::GetStyle().ScrollbarSize;
  float dimY = windowSize.y;

  // Fit within the window size while maintaining aspect ratio.
  if ((dimX / dimY) > m_fieldAspectRatio) {
    dimX = dimY * m_fieldAspectRatio;
  } else {
    dimY = dimX / m_fieldAspectRatio;
  }

  ImVec2 canvas(dimX, dimY);

  ImRect bb(win->DC.CursorPos, win->DC.CursorPos + canvas);
  ImGui::ItemSize(bb);
  if (!ImGui::ItemAdd(bb, 0))
    return;

  drawList->AddImage(m_fieldTexture->id(), bb.Min, bb.Max);

  ImVec2 mouse = io.MousePos;
  auto movePoint = [&](Vec2 pt) -> Vec2 {
    pt.x = mouse.x;
    pt.y = mouse.y;

    pt.x = (pt.x - bb.Min.x) / (bb.Max.x - bb.Min.x);
    pt.y = 1.0 - (pt.y - bb.Min.y) / (bb.Max.y - bb.Min.y);

    return pt;
  };

  static enum {
    NONE = 0,
    MIN = 1,
    MAX = 2,
  } dragPt = NONE;

  const Rect fieldBounds = m_field->imageFieldBounds();
  const bool mouseDown =
      (ImGui::IsMouseDown(ImGuiMouseButton_Left) || ImGui::IsMouseDragging(ImGuiMouseButton_Left));

  if (dragPt && mouseDown) {
    if (dragPt == MIN) {
      m_field->setImageFieldBoundMin(movePoint(fieldBounds.min));
    } else {
      m_field->setImageFieldBoundMax(movePoint(fieldBounds.max));
    }
  } else {
    dragPt = NONE;

    if (mouseDown) {
      auto checkDist = [&](Vec2 pt) -> bool {
        ImVec2 pos = ImVec2(pt.x, 1.f - pt.y) * (bb.Max - bb.Min) + bb.Min;
        float dist = std::hypotf(pos.x - mouse.x, pos.y - mouse.y);
        return dist < pointRadius * 4;
      };

      if (checkDist(fieldBounds.min)) {
        dragPt = MIN;
      } else if (checkDist(fieldBounds.max)) {
        dragPt = MAX;
      }
    }
  }

  ImVec2 minPt = ImVec2(fieldBounds.min.x, 1.0 - fieldBounds.min.y) * (bb.Max - bb.Min) + bb.Min,
         maxPt = ImVec2(fieldBounds.max.x, 1.0 - fieldBounds.max.y) * (bb.Max - bb.Min) + bb.Min;

  drawList->AddRect(minPt, maxPt, kFieldBoundsWidgetColor);

  drawList->AddCircleFilled(minPt, pointRadius, kFieldBoundsWidgetColor);
  drawList->AddCircleFilled(maxPt, pointRadius, kFieldBoundsWidgetColor);
}
