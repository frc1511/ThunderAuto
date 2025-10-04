#pragma once

#include <imgui.h>

inline bool IsCtrlDown() {
  const ImGuiIO& io = ImGui::GetIO();
  return io.KeyCtrl && !io.KeyAlt && !io.KeyShift;
}

inline bool IsCtrlShiftDown() {
  const ImGuiIO& io = ImGui::GetIO();
  return io.KeyCtrl && !io.KeyAlt && io.KeyShift;
}

inline bool IsKeyPressed(ImGuiKey key) {
  return ImGui::IsKeyPressed(key);
}

inline bool IsKeyPressedOrRepeat(ImGuiKey key) {
  return ImGui::IsKeyPressed(key, true);
}
