#include <popups/new_field.h>
#include <imgui_internal.h>
#include <thunder_auto.h>
#include <stb_image.h>
#include <glad/glad.h>
#include <iostream>
#include <cmath>

#ifdef THUNDER_AUTO_MACOS
# include <platform/macos/macos.h>
#elif THUNDER_AUTO_WINDOWS
# include <platform/windows/windows.h>
#elif THUNDER_AUTO_LINUX
# include <platform/linux/linux.h>
#endif

NewFieldPopup::NewFieldPopup() {
#ifdef THUNDER_AUTO_MACOS
  platform = PlatformMacOS::get();
#elif THUNDER_AUTO_WINDOWS
  platform = PlatformWindows::get();
#elif THUNDER_AUTO_LINUX
  platform = PlatformLinux::get();
#endif
}

NewFieldPopup::~NewFieldPopup() { }

void NewFieldPopup::present(bool* running) {
  if (!ImGui::BeginPopupModal(name.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    return;
  }
  
  if (selected_img) {
    present_field_setup();

    if (ImGui::Button("Ok")) {
      selected_img = false;
      glDeleteTextures(1, &field_tex);
      goto close;
    }
  }
  else {
    field = std::nullopt;

    static char img_path_buf[256] = "";
    
    if (ImGui::InputText("Field Image", img_path_buf, 256, ImGuiInputTextFlags_None)) {
      img_load_failed = false;
    }
    
    ImGui::SameLine();

    std::string img_path;
    if (ImGui::Button("Browse")) {
      img_path = platform->open_file_dialog(FileType::FILE, nullptr);
      memset(img_path_buf, 0, 256);
      strncpy(img_path_buf, img_path.c_str(), img_path.length());
    }

    if (ImGui::Button("Cancel")) {
      goto close;
    }

    ImGui::SameLine();

    if (ImGui::Button("Ok")) {
      int width, height, nr_channels;
      unsigned char* img_data = stbi_load(img_path_buf, &width, &height, &nr_channels, 0);
      if (img_data) {
        selected_img = true;

        int tex_channels = nr_channels == 3 ? GL_RGB : GL_RGBA;

        glGenTextures(1, &field_tex);
        glBindTexture(GL_TEXTURE_2D, field_tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, tex_channels, width, height, 0, tex_channels, GL_UNSIGNED_BYTE, img_data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(img_data);
        field_aspect_ratio = static_cast<float>(width) / static_cast<float>(height);

        field = { img_path_buf, ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f) };
      }
      else {
        img_load_failed = true;
      }
    }

    if (img_load_failed) {
      ImGui::Text("Failed to load image");
    }
  }

  
  ImGui::EndPopup();
  return;

close:
  ImGui::CloseCurrentPopup();
  *running = false;
  ImGui::EndPopup();
}

#define POINT_RADIUS 5

void NewFieldPopup::present_field_setup() {
  const ImGuiIO& io = ImGui::GetIO();
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  ImGuiWindow* win = ImGui::GetCurrentWindow();
  if (win->SkipItems) return;

  float dim_x = 500.0f,
        dim_y = 500.0f;

  if (field_aspect_ratio > 1.0f) {
    dim_y = 500.0f / field_aspect_ratio;
  }
  else {
    dim_x = 500.0f * field_aspect_ratio;
  }
  
  ImVec2 canvas(dim_x, dim_y);

  ImRect bb(win->DC.CursorPos, win->DC.CursorPos + canvas);
  ImGui::ItemSize(bb);
  if (!ImGui::ItemAdd(bb, 0)) return;

  const ImGuiID id = win->GetID("Field Setup");

  draw_list->AddImage(reinterpret_cast<void*>(static_cast<intptr_t>(field_tex)), bb.Min, bb.Max);

  ImVec2 mouse = io.MousePos;
  auto move_point = [&](ImVec2& pt) {
    pt.x = mouse.x;
    pt.y = mouse.y;

    pt.x = (pt.x - bb.Min.x) / (bb.Max.x - bb.Min.x);
    pt.y = 1 - (pt.y - bb.Min.y) / (bb.Max.y - bb.Min.y);
  };

  static enum { NONE = 0, MIN = 1, MAX = 2, } drag_pt = NONE;

  if (drag_pt && (ImGui::IsMouseDown(0) || ImGui::IsMouseDragging(0))) {
    move_point(drag_pt == MIN ? field->min : field->max);
  }
  else {
    drag_pt = NONE;

    if (ImGui::IsMouseDown(0) || ImGui::IsMouseDragging(0)) {
      auto check_dist = [&](ImVec2 pt) -> bool {
        ImVec2 pos = ImVec2(pt.x, 1 - pt.y) * (bb.Max - bb.Min) + bb.Min;
        float dist = std::hypotf(pos.x - mouse.x, pos.y - mouse.y);
        return dist < POINT_RADIUS * 4;
      };

      if (check_dist(field->min)) {
        drag_pt = MIN;
      }
      else if (check_dist(field->max)) {
        drag_pt = MAX;
      }
    }
  }

  ImVec2 min_pt = ImVec2(field->min.x, 1 - field->min.y) * (bb.Max - bb.Min) + bb.Min,
         max_pt = ImVec2(field->max.x, 1 - field->max.y) * (bb.Max - bb.Min) + bb.Min;

  draw_list->AddRect(min_pt, max_pt, ImColor(252, 186, 3, 255));

  draw_list->AddCircleFilled(min_pt, POINT_RADIUS, ImColor(252, 186, 3, 255));
  draw_list->AddCircleFilled(max_pt, POINT_RADIUS, ImColor(252, 186, 3, 255));
}

NewFieldPopup NewFieldPopup::instance {};
