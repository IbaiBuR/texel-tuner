#pragma once

#define TAPERED 1

#include <string>

#include "../base.h"
#include "../external/chess.hpp"
#include "types.hpp"

namespace baryonyx
{
    class eval
    {
    public:
        static constexpr bool   includes_additional_score    = true;
        static constexpr bool   supports_external_chess_eval = false;
        static constexpr bool   retune_from_zero             = true;
        static constexpr tune_t preferred_k                  = 2.0;
        static constexpr i32    max_epoch                    = 5000;
        static constexpr bool   enable_qsearch               = false;
        static constexpr bool   filter_in_check              = false;
        static constexpr tune_t initial_learning_rate        = 1;
        static constexpr i32    learning_rate_drop_interval  = 10000;
        static constexpr tune_t learning_rate_drop_ratio     = 1;
        static constexpr bool   print_data_entries           = false;
        static constexpr i32    data_load_print_interval     = 10000;

        static parameters_t get_initial_parameters();

        static EvalResult get_fen_eval_result(const std::string& fen);

        static EvalResult get_external_eval_result(const chess::Board &board);

        static void print_parameters(const parameters_t& parameters);
    };
}
