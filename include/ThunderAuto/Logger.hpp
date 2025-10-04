#pragma once

#include <ThunderLibCore/Logger.hpp>

using namespace thunder::core;

class ThunderAutoLogger {
 public:
  static spdlog::logger* get();
};
