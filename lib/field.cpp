#include <ThunderAuto/field.h>

void to_json(nlohmann::json& json, const Field& field) {
  std::string img;
  if (field.img_type == Field::ImageType::CUSTOM) {
    img = std::get<std::string>(field.img);
  }
  else {
    img = fmt::format("{}", static_cast<std::size_t>(std::get<Field::BuiltinImage>(field.img)));
  }
  
  json = nlohmann::json {
    { "img_type", static_cast<std::size_t>(field.img_type) },
    { "img", img },
    { "min_x", field.min.x },
    { "min_y", field.min.y },
    { "max_x", field.max.x },
    { "max_y", field.max.y }
  };
}

void from_json(const nlohmann::json& json, Field& field) {
  field.img_type = static_cast<Field::ImageType>(json.at("img_type").get<std::size_t>());
  std::string img = json.at("img").get<std::string>();
  if (field.img_type == Field::ImageType::CUSTOM) {
    field.img = img;
  }
  else {
    field.img = static_cast<Field::BuiltinImage>(std::stoul(img));
  }

  field.min.x = json.at("min_x").get<float>();
  field.min.y = json.at("min_y").get<float>();
  field.max.x = json.at("max_x").get<float>();
  field.max.y = json.at("max_y").get<float>();
}
