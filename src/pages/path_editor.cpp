#include <pages/path_editor.h>
  

#include <cmath>
#include <utility>
#include <vector>
#include <iostream>
  
PathEditorPage::PathEditorPage() { }

PathEditorPage::~PathEditorPage() { }

struct SplinePoint {
  float x;
  float y;
  float m;
};

void PathEditorPage::present(bool* running) {
  ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin("Path Editor", running,
                ImGuiWindowFlags_NoCollapse
              | ImGuiWindowFlags_UnsavedDocument * unsaved)) {
    ImGui::End();
    return;
  }

  focused = ImGui::IsWindowFocused();
  ImGui::Text("%d\n", focused);

  float samples = 100.0f;

  ImGui::PlotLines("Lines", [](void*, int i) -> float {
    static const std::vector<SplinePoint> points {
      { 0.0f, 0.0f, 0.0f },
      { 0.3f, 0.9f, 0.0f },
      { 0.5f, 0.1f, 0.0f },
      { 0.8f, 0.6f, 0.0f },
      { 1.0f, 1.0f, 0.0f },
    };

    // 0-1 value.
    float x = i * (1.0f/100.0f);

    decltype(points)::const_iterator pt_iter = points.end();

    for (auto it = points.begin(); it != points.end(); ++it) {
      if (it->x >= x && (it == points.begin() || (it - 1)->x < x)) {
        pt_iter = it - 1;
        break;
      }
    }

    if (pt_iter == points.end()) {
      return 0.0f;
    }

    float px0 = pt_iter->x, py0 = pt_iter->y, m0 = pt_iter->m,
          px1 = (pt_iter + 1)->x, py1 = (pt_iter + 1)->y, m1 = (pt_iter + 1)->m;

    float t = (x - px0) / (px1 - px0);

    // The hermite base functions.
    auto h00 = [](float t) -> float { return 2.0f * std::powf(t, 3) - 3.0f * std::powf(t, 2) + 1.0f; };
    auto h10 = [](float t) -> float { return std::powf(t, 3) - 2.0f * (std::powf(t, 2)) + t; };
    auto h01 = [](float t) -> float { return -2.0f * std::powf(t, 3) + 3.0f * std::powf(t, 2); };
    auto h11 = [](float t) -> float { return std::powf(t, 3) - std::powf(t, 2); };

    float a = py0 * h00(t);
    float b = py1 * h01(t);
    float c = h10(t) * (px1 - px0) * m0;
    float d = h11(t) * (px1 - px0) * m1;

    float y = a + b + c + d;

    static int high = -1;
    if (i > high) {
      high = i;
      std::cout << "y = " << y << '\n';
    }

    return y;
  }, nullptr, samples, 0, nullptr, 0.0f, 1.0f, ImVec2(500, 500));
  
  ImGui::End();
}

PathEditorPage PathEditorPage::instance {};
