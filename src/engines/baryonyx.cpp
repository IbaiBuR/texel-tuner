#include "baryonyx.hpp"

#include <cmath>
#include <format>
#include <iostream>

#include "position.hpp"

namespace baryonyx
{
    template<typename T>
    using piece_table = std::array<T, constants::num_piece_types>;

    template<typename T>
    using piece_square_table = piece_table<std::array<T, constants::num_squares>>;

    using trace_type = std::array<i16, constants::num_colors>;

    struct eval_trace
    {
        i32 score{};

        piece_table<trace_type>        piece_values{};
        piece_square_table<trace_type> all_psqt{};
        trace_type                     tempo{};
    };

    #define TRACE_INCREMENT(term, color)  trace.term[color]++
    #define TRACE_ADD(term, color, count) trace.term[color] += count

    const std::array game_phase_increments = {0, 1, 1, 2, 4, 0};
    const int        max_game_phase        = 24;

    const piece_table<score> piece_values = {
        S(82, 94), S(337, 281), S(365, 297),
        S(477, 512), S(1025, 936), S(0, 0)
    };

    const std::array pawn_table = {
        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
        S(0, 0), S(98, 178), S(134, 173), S(61, 158), S(95, 134), S(68, 147), S(126, 132),
        S(34, 165), S(-11, 187), S(-6, 94), S(7, 100), S(26, 85), S(31, 67), S(65, 56),
        S(56, 53), S(25, 82), S(-20, 84), S(-14, 32), S(13, 24), S(6, 13), S(21, 5),
        S(23, -2), S(12, 4), S(17, 17), S(-23, 17), S(-27, 13), S(-2, 9), S(-5, -3),
        S(12, -7), S(17, -7), S(6, -8), S(10, 3), S(-25, -1), S(-26, 4), S(-4, 7),
        S(-4, -6), S(-10, 1), S(3, 0), S(3, -5), S(33, -1), S(-12, -8), S(-35, 13),
        S(-1, 8), S(-20, 8), S(-23, 10), S(-15, 13), S(24, 0), S(38, 2), S(-22, -7),
        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
        S(0, 0)
    };

    const std::array knight_table = {
        S(-167, -58), S(-89, -38), S(-34, -13), S(-49, -28), S(61, -31), S(-97, -27), S(-15, -63),
        S(-107, -99), S(-73, -25), S(-41, -8), S(72, -25), S(36, -2), S(23, -9), S(62, -25),
        S(7, -24), S(-17, -52), S(-47, -24), S(60, -20), S(37, 10), S(65, 9), S(84, -1),
        S(129, -9), S(73, -19), S(44, -41), S(-9, -17), S(17, 3), S(19, 22), S(53, 22),
        S(37, 22), S(69, 11), S(18, 8), S(22, -18), S(-13, -18), S(4, -6), S(16, 16),
        S(13, 25), S(28, 16), S(19, 17), S(21, 4), S(-8, -18), S(-23, -23), S(-9, -3),
        S(12, -1), S(10, 15), S(19, 10), S(17, -3), S(25, -20), S(-16, -22), S(-29, -42),
        S(-53, -20), S(-12, -10), S(-3, -5), S(-1, -2), S(18, -20), S(-14, -23), S(-19, -44),
        S(-105, -29), S(-21, -51), S(-58, -23), S(-33, -15), S(-17, -22), S(-28, -18), S(-19, -50),
        S(-23, -64)
    };

    const std::array bishop_table = {
        S(-29, -14), S(4, -21), S(-82, -11), S(-37, -8), S(-25, -7), S(-42, -9), S(7, -17),
        S(-8, -24), S(-26, -8), S(16, -4), S(-18, 7), S(-13, -12), S(30, -3), S(59, -13),
        S(18, -4), S(-47, -14), S(-16, 2), S(37, -8), S(43, 0), S(40, -1), S(35, -2),
        S(50, 6), S(37, 0), S(-2, 4), S(-4, -3), S(5, 9), S(19, 12), S(50, 9),
        S(37, 14), S(37, 10), S(7, 3), S(-2, 2), S(-6, -6), S(13, 3), S(13, 13),
        S(26, 19), S(34, 7), S(12, 10), S(10, -3), S(4, -9), S(0, -12), S(15, -3),
        S(15, 8), S(15, 10), S(14, 13), S(27, 3), S(18, -7), S(10, -15), S(4, -14),
        S(15, -18), S(16, -7), S(0, -1), S(7, 4), S(21, -9), S(33, -15), S(1, -27),
        S(-33, -23), S(-3, -9), S(-14, -23), S(-21, -5), S(-13, -9), S(-12, -16), S(-39, -5),
        S(-21, -17)
    };

    const std::array rook_table = {
        S(32, 13), S(42, 10), S(32, 18), S(51, 15), S(63, 12), S(9, 12), S(31, 8), S(43, 5),
        S(27, 11), S(32, 13), S(58, 13), S(62, 11), S(80, -3), S(67, 3), S(26, 8), S(44, 3),
        S(-5, 7), S(19, 7), S(26, 7), S(36, 5), S(17, 4), S(45, -3), S(61, -5), S(16, -3),
        S(-24, 4), S(-11, 3), S(7, 13), S(26, 1), S(24, 2), S(35, 1), S(-8, -1), S(-20, 2),
        S(-36, 3), S(-26, 5), S(-12, 8), S(-1, 4), S(9, -5), S(-7, -6), S(6, -8), S(-23, -11),
        S(-45, -4), S(-25, 0), S(-16, -5), S(-17, -1), S(3, -7), S(0, -12), S(-5, -8), S(-33, -16),
        S(-44, -6), S(-16, -6), S(-20, 0), S(-9, 2), S(-1, -9), S(11, -9), S(-6, -11), S(-71, -3),
        S(-19, -9), S(-13, 2), S(1, 3), S(17, -1), S(16, -5), S(7, -13), S(-37, 4), S(-26, -20)
    };

    const std::array queen_table = {
        S(-28, -9), S(0, 22), S(29, 22), S(12, 27), S(59, 27), S(44, 19), S(43, 10),
        S(45, 20), S(-24, -17), S(-39, 20), S(-5, 32), S(1, 41), S(-16, 58), S(57, 25),
        S(28, 30), S(54, 0), S(-13, -20), S(-17, 6), S(7, 9), S(8, 49), S(29, 47),
        S(56, 35), S(47, 19), S(57, 9), S(-27, 3), S(-27, 22), S(-16, 24), S(-16, 45),
        S(-1, 57), S(17, 40), S(-2, 57), S(1, 36), S(-9, -18), S(-26, 28), S(-9, 19),
        S(-10, 47), S(-2, 31), S(-4, 34), S(3, 39), S(-3, 23), S(-14, -16), S(2, -27),
        S(-11, 15), S(-2, 6), S(-5, 9), S(2, 17), S(14, 10), S(5, 5), S(-35, -22),
        S(-8, -23), S(11, -30), S(2, -16), S(8, -16), S(15, -23), S(-3, -36), S(1, -32),
        S(-1, -33), S(-18, -28), S(-9, -22), S(10, -43), S(-15, -5), S(-25, -32), S(-31, -20),
        S(-50, -41)
    };

    const std::array king_table = {
        S(-65, -74), S(23, -35), S(16, -18), S(-15, -18), S(-56, -11), S(-34, 15), S(2, 4),
        S(13, -17), S(29, -12), S(-1, 17), S(-20, 14), S(-7, 17), S(-8, 17), S(-4, 38),
        S(-38, 23), S(-29, 11), S(-9, 10), S(24, 17), S(2, 23), S(-16, 15), S(-20, 20),
        S(6, 45), S(22, 44), S(-22, 13), S(-17, -8), S(-20, 22), S(-12, 24), S(-27, 27),
        S(-30, 26), S(-25, 33), S(-14, 26), S(-36, 3), S(-49, -18), S(-1, -4), S(-27, 21),
        S(-39, 24), S(-46, 27), S(-44, 23), S(-33, 9), S(-51, -11), S(-14, -19), S(-14, -3),
        S(-22, 11), S(-46, 21), S(-44, 23), S(-30, 16), S(-15, 7), S(-27, -9), S(1, -27),
        S(7, -11), S(-8, 4), S(-64, 13), S(-43, 14), S(-16, 4), S(9, -5), S(8, -17),
        S(-15, -53), S(36, -34), S(12, -21), S(-54, -11), S(8, -28), S(-28, -14), S(24, -24),
        S(14, -43)
    };

    const piece_square_table<score> all_psqt = {
        pawn_table, knight_table, bishop_table, rook_table, queen_table, king_table
    };

    const score tempo = S(0, 0);

    int get_game_phase(const position& pos)
    {
        const int game_phase = game_phase_increments[static_cast<u8>(piece_type::knight)]
                               * pos.piece_type_bb(piece_type::knight).bit_count()
                               + game_phase_increments[static_cast<u8>(piece_type::bishop)]
                               * pos.piece_type_bb(piece_type::bishop).bit_count()
                               + game_phase_increments[static_cast<u8>(piece_type::rook)]
                               * pos.piece_type_bb(piece_type::rook).bit_count()
                               + game_phase_increments[static_cast<u8>(piece_type::queen)]
                               * pos.piece_type_bb(piece_type::queen).bit_count();

        return std::min(game_phase, max_game_phase);
    }

    template<color Us>
    score evaluate_material(const position& pos, eval_trace& trace)
    {
        constexpr color them = ~Us;

        const int our_pawns   = pos.piece_count<Us>(piece_type::pawn);
        const int our_knights = pos.piece_count<Us>(piece_type::knight);
        const int our_bishops = pos.piece_count<Us>(piece_type::bishop);
        const int our_rooks   = pos.piece_count<Us>(piece_type::rook);
        const int our_queens  = pos.piece_count<Us>(piece_type::queen);

        const int their_pawns   = pos.piece_count<them>(piece_type::pawn);
        const int their_knights = pos.piece_count<them>(piece_type::knight);
        const int their_bishops = pos.piece_count<them>(piece_type::bishop);
        const int their_rooks   = pos.piece_count<them>(piece_type::rook);
        const int their_queens  = pos.piece_count<them>(piece_type::queen);

        const int pawn_count   = our_pawns - their_pawns;
        const int knight_count = our_knights - their_knights;
        const int bishop_count = our_bishops - their_bishops;
        const int rook_count   = our_rooks - their_rooks;
        const int queen_count  = our_queens - their_queens;

        score material_score{};

        material_score += piece_values[static_cast<u8>(piece_type::pawn)] * pawn_count;
        material_score += piece_values[static_cast<u8>(piece_type::knight)] * knight_count;
        material_score += piece_values[static_cast<u8>(piece_type::bishop)] * bishop_count;
        material_score += piece_values[static_cast<u8>(piece_type::rook)] * rook_count;
        material_score += piece_values[static_cast<u8>(piece_type::queen)] * queen_count;

        TRACE_ADD(piece_values[static_cast<u8>(piece_type::pawn)], static_cast<u8>(Us), our_pawns);
        TRACE_ADD(piece_values[static_cast<u8>(piece_type::pawn)], static_cast<u8>(them), their_pawns);
        TRACE_ADD(piece_values[static_cast<u8>(piece_type::knight)], static_cast<u8>(Us), our_knights);
        TRACE_ADD(piece_values[static_cast<u8>(piece_type::knight)], static_cast<u8>(them), their_knights);
        TRACE_ADD(piece_values[static_cast<u8>(piece_type::bishop)], static_cast<u8>(Us), our_bishops);
        TRACE_ADD(piece_values[static_cast<u8>(piece_type::bishop)], static_cast<u8>(them), their_bishops);
        TRACE_ADD(piece_values[static_cast<u8>(piece_type::rook)], static_cast<u8>(Us), our_rooks);
        TRACE_ADD(piece_values[static_cast<u8>(piece_type::rook)], static_cast<u8>(them), their_rooks);
        TRACE_ADD(piece_values[static_cast<u8>(piece_type::queen)], static_cast<u8>(Us), our_queens);
        TRACE_ADD(piece_values[static_cast<u8>(piece_type::queen)], static_cast<u8>(them), their_queens);

        return material_score;
    }

    template<color Us>
    score evaluate_psqt(const position& pos, eval_trace& trace)
    {
        constexpr color them = ~Us;

        auto our_pieces   = pos.occupancies(Us);
        auto their_pieces = pos.occupancies(them);

        score psqt_score{};


        while (!our_pieces.empty())
        {
            const auto       sq       = static_cast<square>(our_pieces.pop_lsb());
            const piece_type pt       = utils::piece_to_piece_type(pos.piece_on(sq));
            const u8         pt_as_u8 = static_cast<u8>(pt);
            const u8         sq_as_u8 = static_cast<u8>(relative_square<Us>(sq));
            psqt_score += all_psqt[pt_as_u8][sq_as_u8];
            TRACE_INCREMENT(all_psqt[pt_as_u8][sq_as_u8], static_cast<u8>(Us));
        }

        while (!their_pieces.empty())
        {
            const auto       sq       = static_cast<square>(their_pieces.pop_lsb());
            const piece_type pt       = utils::piece_to_piece_type(pos.piece_on(sq));
            const u8         pt_as_u8 = static_cast<u8>(pt);
            const u8         sq_as_u8 = static_cast<u8>(relative_square<them>(sq));
            psqt_score -= all_psqt[pt_as_u8][sq_as_u8];
            TRACE_INCREMENT(all_psqt[pt_as_u8][sq_as_u8], static_cast<u8>(them));
        }

        return psqt_score;
    }

    template<color Us>
    eval_trace evaluate(const position& pos)
    {
        eval_trace  trace{};
        const score total_score = evaluate_material<Us>(pos, trace) + evaluate_psqt<Us>(pos, trace) + tempo;
        TRACE_INCREMENT(tempo, static_cast<u8>(Us));

        const int   game_phase = get_game_phase(pos);
        const score eval = (mg_score(total_score) * game_phase + eg_score(total_score) * (max_game_phase - game_phase))
                           / max_game_phase;

        trace.score = Us == color::white ? eval : -eval;

        return trace;
    }

    eval_trace evaluate(const position& pos)
    {
        return pos.side_to_move() == color::white ? evaluate<color::white>(pos) : evaluate<color::black>(pos);
    }

    static i32 round_value(const tune_t value)
    {
        return static_cast<i32>(std::round(value));
    }

    static void print_parameter(const pair_t parameter)
    {
        const auto mg = round_value(parameter[static_cast<i32>(PhaseStages::Midgame)]);
        const auto eg = round_value(parameter[static_cast<i32>(PhaseStages::Endgame)]);

        std::cout << std::format("S({}, {})", mg, eg);
    }

    static void print_single(const parameters_t& parameters, int& index, const std::string& name)
    {
        std::cout << std::format("constexpr packed_score {} = ", name);

        print_parameter(parameters[index]);
        index++;

        std::cout << ";" << std::endl;
    }

    static void print_array(const parameters_t& parameters, int& index, const std::string& name, const int count)
    {
        std::cout << std::format("constexpr std::array {} = {}", name, '{');

        for (auto i = 0; i < count; i++)
        {
            print_parameter(parameters[index]);
            index++;

            if (i != count - 1)
                std::cout << ", ";
        }
        std::cout << "};" << std::endl;
    }

    static void print_array_2d(const parameters_t& parameters, int& index, const std::string& name, const int count1,
                               const int           count2)
    {
        std::cout << std::format("constexpr std::array<std::array<packed_score, {}>, {}> {} = {}{}", count2, count1, name, '{', '{');

        for (auto i = 0; i < count1; i++)
        {
            std::cout << "    {";
            for (auto j = 0; j < count2; j++)
            {
                print_parameter(parameters[index]);
                index++;

                if (j != count2 - 1)
                {
                    std::cout << ", ";
                }
            }
            std::cout << "},\n";
        }
        std::cout << "}};\n";
    }

    void eval::print_parameters(const parameters_t& parameters)
    {
        int index = 0;

        print_array(parameters, index, "piece_values", constants::num_piece_types);
        print_array_2d(parameters, index, "all_psqt", constants::num_piece_types, constants::num_squares);
        print_single(parameters, index, "tempo");
    }

    parameters_t eval::get_initial_parameters()
    {
        parameters_t parameters;

        get_initial_parameter_array(parameters, piece_values, constants::num_piece_types);
        get_initial_parameter_array_2d(parameters, all_psqt, constants::num_piece_types, constants::num_squares);
        get_initial_parameter_single(parameters, tempo);

        return parameters;
    }

    static coefficients_t get_coefficients(const eval_trace& trace)
    {
        coefficients_t coefficients;

        get_coefficient_array(coefficients, trace.piece_values, constants::num_piece_types);
        get_coefficient_array_2d(coefficients, trace.all_psqt, constants::num_piece_types, constants::num_squares);
        get_coefficient_single(coefficients, trace.tempo);

        return coefficients;
    }

    EvalResult eval::get_fen_eval_result(const std::string& fen)
    {
        const position pos(fen);
        EvalResult     result;
        const auto     trace = evaluate(pos);
        result.score         = trace.score;
        result.coefficients  = get_coefficients(trace);

        return result;
    }
}
