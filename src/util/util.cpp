#include <ThunderAuto/thunder_auto.h>

void replace_macro(std::string& str, std::string_view macro, std::string_view value) {
  std::size_t pos;
  while (pos = str.find(fmt::format("${{{}}}", macro)), pos != std::string::npos) {
    str.replace(pos, macro.length() + 3, value);
  }
}