#include <ThunderAuto/popups/new_field_popup.h>

#include <glad/glad.h>

#include <ThunderAuto/file_types.h>
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
      glDeleteTextures(1, &m_field_texture);
      m_result = Result::CREATE;
    }
  } else {
    m_field = std::nullopt;

    static char img_path_buf[256] = "";

    if (ImGui::InputText("Field Image", img_path_buf, 256,
                         ImGuiInputTextFlags_None)) {
      m_image_load_failed = false;
    }

    ImGui::SameLine();

    std::string img_path;
    if (ImGui::Button("Browse")) {
      img_path = m_platform_manager.open_file_dialog(FileType::FILE, {});
      memset(img_path_buf, 0, 256);
      strncpy(img_path_buf, img_path.c_str(), img_path.length());
    }

    if (ImGui::Button("Cancel")) {
      m_result = Result::CANCEL;
    }

    ImGui::SameLine();

    if (ImGui::Button("Ok")) {
      int width, height, nr_channels;
      unsigned char* img_data =
          stbi_load(img_path_buf, &width, &height, &nr_channels, 0);
      if (img_data) {
        m_selected_image = true;

        int tex_channels = nr_channels == 3 ? GL_RGB : GL_RGBA;

        glGenTextures(1, &m_field_texture);
        glBindTexture(GL_TEXTURE_2D, m_field_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, tex_channels, width, height, 0,
                     tex_channels, GL_UNSIGNED_BYTE, img_data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(img_data);
        m_field_aspect_ratio =
            static_cast<float>(width) / static_cast<float>(height);

        m_field = Field(img_path_buf, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f));
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

  draw_list->AddImage(
      reinterpret_cast<void*>(static_cast<intptr_t>(m_field_texture)), bb.Min,
      bb.Max);

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
