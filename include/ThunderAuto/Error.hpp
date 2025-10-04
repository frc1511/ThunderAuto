#pragma once

#include <ThunderAuto/Logger.hpp>
#include <ThunderLibCore/Error.hpp>

#define ThunderAutoAssert(condition, ...) TAssert(ThunderAutoLogger, condition, ##__VA_ARGS__)
#define ThunderAutoUnreachable(...) TUnreachable(ThunderAutoLogger, ##__VA_ARGS__)
