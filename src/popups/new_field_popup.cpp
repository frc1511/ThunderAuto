#include <ThunderAuto/popups/new_field_popup.hpp>

#include <ThunderAuto/file_types.hpp>
#include <ThunderAuto/imgui_util.hpp>
#include <stb_image.h>

void NewFieldPopup::present(bool* running) {
  ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), false,
                          ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize(ImVec2(370, 250), ImGuiCond_FirstUseEver);
  if (!ImGui::BeginPopupModal(m_name, nullptr, ImGuiWindowFlags_None)) {
    return;
  }

  m_result = Result::NONE;

  if (m_selected_image) {
    present_field_setup();

    if (ImGui::Button("Ok")) {
      m_selected_image = false;
      m_result = Result::CREATE;
    }
  } else {
    m_field = std::nullopt;

    static char img_path_buf[256] = "";
    std::string img_path;

    {
      ImGuiScopedField field("Field Image", 100);

      if (ImGui::InputText("##field_image", img_path_buf, 256,
                           ImGuiInputTextFlags_None)) {
        m_image_load_failed = false;
      }

      ImGui::SameLine();

      if (ImGui::Button("Browse")) {
        img_path = m_platform_manager.open_file_dialog(FileType::FILE, {});
        memset(img_path_buf, 0, 256);
        strncpy(img_path_buf, img_path.c_str(), img_path.length());
      }
    }

    {
      ImGuiScopedField field("Field Width", 100);
      ImGui::InputFloat("##field_width", &m_field_size.x, 0.f, 0.f, "%0.3f m");
    }
    {
      ImGuiScopedField field("Field Height", 100);
      ImGui::InputFloat("##field_height", &m_field_size.y, 0.f, 0.f, "%0.3f m");
    }

    if (ImGui::Button("Cancel")) {
      m_result = Result::CANCEL;
    }

    ImGui::SameLine();

    if (ImGui::Button("Ok")) {
      m_field_texture.load_from_file(img_path_buf);
      if (m_field_texture) {
        m_selected_image = true;

        m_field_aspect_ratio = static_cast<float>(m_field_texture.width()) /
                               static_cast<float>(m_field_texture.height());

        m_field = Field(img_path_buf, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f),
                        m_field_size);
      } else {
        m_image_load_failed = true;
      }
    }

    if (m_image_load_failed) {
      ImGui::Text("Failed to load image");
    }
  }

  ImGui::EndPopup();

  if (m_result != Result::NONE) {
    *running = false;
  }
}

#define POINT_RADIUS 5

void NewFieldPopup::present_field_setup() {
  const ImGuiIO& io = ImGui::GetIO();
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  ImGuiWindow* win = ImGui::GetCurrentWindow();
  if (win->SkipItems) return;

  ImVec2 win_size(ImGui::GetWindowSize());

  float dim_x(win_size.x - ImGui::GetStyle().ScrollbarSize),
      dim_y(win_size.y * 0.8f);

  // Fit within the window size while maintaining aspect ratio.
  if ((dim_x / dim_y) > m_field_aspect_ratio) {
    dim_x = dim_y * m_field_aspect_ratio;
  } else {
    dim_y = dim_x / m_field_aspect_ratio;
  }

  ImVec2 canvas(dim_x, dim_y);

  ImRect bb(win->DC.CursorPos, win->DC.CursorPos + canvas);
  ImGui::ItemSize(bb);
  if (!ImGui::ItemAdd(bb, 0)) return;

  draw_list->AddImage(m_field_texture.id(), bb.Min, bb.Max);

  ImVec2 mouse = io.MousePos;
  auto move_point = [&](ImVec2 pt) -> ImVec2 {
    pt.x = mouse.x;
    pt.y = mouse.y;

    pt.x = (pt.x - bb.Min.x) / (bb.Max.x - bb.Min.x);
    pt.y = 1 - (pt.y - bb.Min.y) / (bb.Max.y - bb.Min.y);

    return pt;
  };

  static enum {
    NONE = 0,
    MIN = 1,
    MAX = 2,
  } drag_pt = NONE;

  const ImRect field_rect = m_field->image_rect();

  if (drag_pt && (ImGui::IsMouseDown(0) || ImGui::IsMouseDragging(0))) {
    if (drag_pt == MIN) {
      m_field->set_image_min(move_point(field_rect.Min));
    } else {
      m_field->set_image_max(move_point(field_rect.Max));
    }
  } else {
    drag_pt = NONE;

    if (ImGui::IsMouseDown(0) || ImGui::IsMouseDragging(0)) {
      auto check_dist = [&](ImVec2 pt) -> bool {
        ImVec2 pos = ImVec2(pt.x, 1 - pt.y) * (bb.Max - bb.Min) + bb.Min;
        float dist = std::hypotf(pos.x - mouse.x, pos.y - mouse.y);
        return dist < POINT_RADIUS * 4;
      };

      if (check_dist(field_rect.Min)) {
        drag_pt = MIN;
      } else if (check_dist(field_rect.Max)) {
        drag_pt = MAX;
      }
    }
  }

  ImVec2 min_pt = ImVec2(field_rect.Min.x, 1 - field_rect.Min.y) *
                      (bb.Max - bb.Min) +
                  bb.Min,
         max_pt = ImVec2(field_rect.Max.x, 1 - field_rect.Max.y) *
                      (bb.Max - bb.Min) +
                  bb.Min;

  draw_list->AddRect(min_pt, max_pt, ImColor(252, 186, 3, 255));

  draw_list->AddCircleFilled(min_pt, POINT_RADIUS, ImColor(252, 186, 3, 255));
  draw_list->AddCircleFilled(max_pt, POINT_RADIUS, ImColor(252, 186, 3, 255));
}

