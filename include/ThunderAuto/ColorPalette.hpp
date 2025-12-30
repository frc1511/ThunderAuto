#pragma once

#include <imgui.h>

class ThunderAutoColorPalette {
 private:
  // Use this to get VSCode color picker.
  static ImColor rgba(int r, int g, int b, float a) { return ImColor(r, g, b, static_cast<int>(a * 255)); };

 public:
  static inline const ImColor kRedHigh = rgba(237, 28, 36, 1);
  static inline const ImColor kRedMid = rgba(194, 32, 32, 1);
  static inline const ImColor kRedLow = rgba(153, 32, 27, 1);
  static inline const ImColor kRedVeryLow = rgba(76, 25, 18, 1);

  static inline const ImColor kOrangeHigh = rgba(245, 126, 37, 1);
  static inline const ImColor kOrangeMid = rgba(218, 100, 30, 1);
  static inline const ImColor kOrangeLow = rgba(179, 82, 24, 1);

  static inline const ImColor kYellowHigh = rgba(255, 242, 0, 1);
  static inline const ImColor kYellowMid = rgba(208, 197, 20, 1);
  static inline const ImColor kYellowLow = rgba(163, 154, 26, 1);

  static inline const ImColor kGreenHigh = rgba(0, 166, 81, 1);
  static inline const ImColor kGreenMid = rgba(0, 130, 64, 1);
  static inline const ImColor kGreenLow = rgba(0, 97, 48, 1);

  static inline const ImColor kBlueHigh = rgba(0, 156, 215, 1);
  static inline const ImColor kBlueMid = rgba(0, 102, 179, 1);
  static inline const ImColor kBlueLow = rgba(0, 61, 119, 1);

  static inline const ImColor kPurpleHigh = rgba(163, 73, 164, 1);
  static inline const ImColor kPurpleMid = rgba(102, 45, 145, 1);
  static inline const ImColor kPurpleLow = rgba(69, 30, 98, 1);

  static inline const ImColor kGrayHigh = rgba(55, 61, 72, 1);
  static inline const ImColor kGrayMid = rgba(41, 46, 54, 1);
  static inline const ImColor kGrayLow = rgba(27, 33, 39, 1);

  static inline const ImColor kBackground = rgba(13, 18, 23, 1);

  static inline const ImColor kRed = kRedHigh;
  static inline const ImColor kOrange = kOrangeHigh;
  static inline const ImColor kYellow = kYellowHigh;
  static inline const ImColor kGreen = kGreenHigh;
  static inline const ImColor kBlue = kBlueMid;
  static inline const ImColor kPurple = kPurpleMid;
  static inline const ImColor kWhite = rgba(255, 255, 255, 1);

  static inline const ImColor kInvisible = rgba(0, 0, 0, 0);
};
