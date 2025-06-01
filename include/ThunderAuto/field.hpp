#pragma once

#include <ThunderAuto/thunder_auto.hpp>

class Field {
public:
  enum class BuiltinImage : std::size_t {
    FIELD_2025,
    FIELD_2024,
    FIELD_2023,
    FIELD_2022,
  };

  enum class ImageType {
    CUSTOM,
    BUILTIN,
  };

  inline Field()
    : Field(BuiltinImage::FIELD_2025) {}

  inline Field(const std::string& image_path, ImVec2 min, ImVec2 max, ImVec2 size)
    : Field(image_path, ImRect(min, max), size) {}

  inline Field(const std::string& image_path, ImRect image_rect, ImVec2 size)
    : m_image(image_path),
      m_image_rect(image_rect),
      m_size(size) {}

  explicit Field(BuiltinImage builtin_image);

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

  inline const ImVec2& size() const { return m_size; }

  inline void set_image_min(ImVec2 min) { m_image_rect.Min = min; }
  inline void set_image_max(ImVec2 max) { m_image_rect.Max = max; }

private:
  std::variant<std::string, BuiltinImage> m_image;
  ImRect m_image_rect;
  ImVec2 m_size; // meters
};

void to_json(wpi::json& json, const Field& field);
void from_json(const wpi::json& json, Field& field);
