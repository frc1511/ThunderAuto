#include <ThunderAuto/macro_util.hpp>

void replace_macro(std::string& str,
                   std::string_view name,
                   std::string_view value) {
  std::size_t pos;
  std::string macro = "${" + std::string(name) + "}";

  while (pos = str.find(macro), pos != std::string::npos) {
    str.replace(pos, macro.length(), value);
  }
}

