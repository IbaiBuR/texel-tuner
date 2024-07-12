#ifndef CONFIG_H
#define CONFIG_H 1

#include <cstdint>

#include "engines/baryonyx.hpp"

using TuneEval = baryonyx::eval;

constexpr int32_t data_load_thread_count = 4;
constexpr int32_t thread_count = 12;

#endif // CONFIG_H