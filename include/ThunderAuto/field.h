#pragma once

#include <ThunderAuto/thunder_auto.h>

class Field {
public:
  enum class BuiltinImage : std::size_t {
    FIELD_2024 = 0,
    FIELD_2023 = 1,
    FIELD_2022 = 2,
  };

  enum class ImageType {
    CUSTOM,
    BUILTIN,
  };

  inline Field()
    : Field(BuiltinImage::FIELD_2023) {}

  inline Field(std::string image_path, ImVec2 min, ImVec2 max)
    : Field(image_path, ImRect(min, max)) {}

  inline Field(std::string image_path, ImRect image_rect)
    : m_image(image_path),
      m_image_rect(image_rect) {}

  Field(BuiltinImage builtin_image);

  inline ImageType type() const {
    if (std::holds_alternative<std::string>(m_image)) {
      return ImageType::CUSTOM;
    }
    return ImageType::BUILTIN;
  }

  inline const std::string& custom_image_path() const {
    return std::get<std::string>(m_image);
  }

  inline BuiltinImage builtin_image() const {
    return std::get<BuiltinImage>(m_image);
  }

  inline const ImRect& image_rect() const { return m_image_rect; }

  inline void set_image_min(ImVec2 min) { m_image_rect.Min = min; }
  inline void set_image_max(ImVec2 max) { m_image_rect.Max = max; }

private:
  std::variant<std::string, BuiltinImage> m_image;

  ImRect m_image_rect;
};

void to_json(nlohmann::json& json, const Field& field);
void from_json(const nlohmann::json& json, Field& field);
