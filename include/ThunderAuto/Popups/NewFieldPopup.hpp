#pragma once

#include <ThunderAuto/Popups/Popup.hpp>
#include <ThunderAuto/Graphics/Texture.hpp>
#include <ThunderLibCore/Auto/ThunderAutoFieldImage.hpp>
#include <memory>
#include <optional>

using namespace thunder::core;

class NewFieldPopup : public Popup {
  // Result
  std::optional<ThunderAutoFieldImage> m_field;

  // Working variables
  bool m_isImageSelected;
  char m_imagePathBuf[256];
  std::unique_ptr<Texture> m_fieldTexture;
  double m_fieldAspectRatio;
  bool m_imageLoadFailed;
  float m_fieldWidth;
  float m_fieldLength;

 public:
  NewFieldPopup() { reset(); }

  void present(bool* running) override;
  const char* name() const noexcept override { return "New Field"; }

  void reset() {
    m_field = std::nullopt;
    m_isImageSelected = false;
    m_imagePathBuf[0] = '\0';
    m_fieldTexture = nullptr;
    m_fieldAspectRatio = 1;
    m_imageLoadFailed = false;
    m_fieldWidth = 8.0137;
    m_fieldLength = 16.54175;
  }

  const ThunderAutoFieldImage& field() const { return *m_field; }

  enum class Result {
    NONE,
    CREATE,
    CANCEL,
  };

  Result result() const { return m_result; }

 private:
  void presentFieldBoundsSetup();

 private:
  Result m_result = Result::NONE;
};
