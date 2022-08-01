#pragma once

#include <thunder_auto.h>

struct Field {
  enum class BuiltinImage : std::size_t {
    FIELD_2022,
  };

  std::variant<std::string, BuiltinImage> img;

  enum class ImageType {
    CUSTOM,
    BUILTIN,
  };

  ImageType img_type;

  ImVec2 min, max;

  Field() = default;

  inline Field(std::string img_path, ImVec2 min, ImVec2 max)
  : img(img_path), img_type(ImageType::CUSTOM), min(min), max(max) { }

  inline Field(BuiltinImage builtin_img, ImVec2 min, ImVec2 max)
  : img(builtin_img), img_type(ImageType::BUILTIN), min(min), max(max) { }

  ~Field() = default;
};
