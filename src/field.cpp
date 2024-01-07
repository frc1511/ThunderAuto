#include <ThunderAuto/field.h>

static const ImRect field_rects[] {
    // 2024
    ImRect(ImVec2(0.080078125f, 0.07861328125f),
           ImVec2(0.919921875f, 0.90625f)),

    // 2023
    ImRect(ImVec2(0.02800000086426735f, 0.018844246864318848f),
           ImVec2(0.9739999771118164f, 0.9825640916824341f)),

    // 2022
    ImRect(ImVec2(0.12f, 0.16f), ImVec2(0.88f, 0.84f)),
};

static const char* field_names[] {
    "2024",
    "2023",
    "2022",
};

Field::Field(BuiltinImage builtin_image)
  : m_image(builtin_image),
    m_image_rect(field_rects[static_cast<std::size_t>(builtin_image)]) {}

void to_json(nlohmann::json& json, const Field& field) {
  std::string image_str;
  Field::ImageType type = field.type();

  if (type == Field::ImageType::CUSTOM) {
    image_str = field.custom_image_path();
  } else {
    image_str = field_names[static_cast<std::size_t>(field.builtin_image())];
  }

  ImRect image_rect = field.image_rect();

  json = nlohmann::json {
      {"img_type", static_cast<std::size_t>(type)},
      {"img", image_str},
  };

  if (type == Field::ImageType::CUSTOM) {
    json["min_x"] = image_rect.Min.x;
    json["min_y"] = image_rect.Min.y;
    json["max_x"] = image_rect.Max.x;
    json["max_y"] = image_rect.Max.y;
  }
}

void from_json(const nlohmann::json& json, Field& field) {
  auto type =
      static_cast<Field::ImageType>(json.at("img_type").get<std::size_t>());

  std::string image_str = json.at("img").get<std::string>();

  if (type == Field::ImageType::CUSTOM) {
    ImRect image_rect;
    image_rect.Min.x = json.at("min_x").get<float>();
    image_rect.Min.y = json.at("min_y").get<float>();
    image_rect.Max.x = json.at("max_x").get<float>();
    image_rect.Max.y = json.at("max_y").get<float>();

    field = Field(image_str, image_rect);
  } else {
    int image_index = -1;
    for (int i = 0; i < 3; ++i) {
      if (image_str == field_names[i]) {
        image_index = i;
        break;
      }
    }
    assert(image_index != -1);

    field = Field(static_cast<Field::BuiltinImage>(image_index));
  }
}

