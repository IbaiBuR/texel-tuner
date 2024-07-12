#pragma once

#include <array>
#include <cctype>
#include <string>
#include <vector>

#include "bitboard.hpp"

namespace baryonyx
{
    namespace utils
    {
        inline std::vector<std::string> split_string(const std::string& str, const char delim)
        {
            std::vector<std::string> tokens;
            usize                    start = 0;
            usize                    end   = str.find(delim);

            while (end != std::string::npos)
            {
                tokens.emplace_back(str.substr(start, end - start));
                start = end + 1;
                end   = str.find(delim, start);
            }

            tokens.emplace_back(str.substr(start));

            return tokens;
        }

        constexpr piece char_to_piece(const char c)
        {
            switch (c)
            {
                case 'P':
                    return piece::w_pawn;
                case 'N':
                    return piece::w_knight;
                case 'B':
                    return piece::w_bishop;
                case 'R':
                    return piece::w_rook;
                case 'Q':
                    return piece::w_queen;
                case 'K':
                    return piece::w_king;
                case 'p':
                    return piece::b_pawn;
                case 'n':
                    return piece::b_knight;
                case 'b':
                    return piece::b_bishop;
                case 'r':
                    return piece::b_rook;
                case 'q':
                    return piece::b_queen;
                case 'k':
                    return piece::b_king;
                default:
                    return piece::none;
            }
        }

        constexpr piece_type piece_to_piece_type(const piece p)
        {
            switch (p)
            {
                case piece::w_pawn:
                case piece::b_pawn:
                    return piece_type::pawn;
                case piece::w_knight:
                case piece::b_knight:
                    return piece_type::knight;
                case piece::w_bishop:
                case piece::b_bishop:
                    return piece_type::bishop;
                case piece::w_rook:
                case piece::b_rook:
                    return piece_type::rook;
                case piece::w_queen:
                case piece::b_queen:
                    return piece_type::queen;
                case piece::w_king:
                case piece::b_king:
                    return piece_type::king;
                default:
                    return piece_type::none;
            }
        }

        constexpr color piece_color(const piece p)
        {
            switch (p)
            {
                case piece::w_pawn:
                case piece::w_knight:
                case piece::w_bishop:
                case piece::w_rook:
                case piece::w_queen:
                case piece::w_king:
                    return color::white;
                case piece::b_pawn:
                case piece::b_knight:
                case piece::b_bishop:
                case piece::b_rook:
                case piece::b_queen:
                case piece::b_king:
                    return color::black;
                default:
                    return color::none;
            }
        }
    }

    class castling_rights
    {
    public:
        enum class castling_flag : u8
        {
            none,
            wk  = 1,
            wq  = 2,
            bk  = 4,
            bq  = 8,
            all = 15
        };

        constexpr castling_rights() : m_flags(castling_flag::none) {}

        constexpr explicit castling_rights(const castling_flag flag) : m_flags(flag) {}

        constexpr explicit castling_rights(const std::string& flags) : m_flags(castling_flag::none)
        {
            for (const char c: flags)
            {
                switch (c)
                {
                    case 'K':
                        *this |= castling_rights(castling_flag::wk);
                        break;
                    case 'Q':
                        *this |= castling_rights(castling_flag::wq);
                        break;
                    case 'k':
                        *this |= castling_rights(castling_flag::bk);
                        break;
                    case 'q':
                        *this |= castling_rights(castling_flag::bq);
                        break;
                    default:
                        break;
                }
            }
        }

        constexpr bool operator==(const castling_rights& other) const
        {
            return static_cast<u8>(m_flags) == static_cast<u8>(other.m_flags);
        }

        constexpr bool operator!=(const castling_rights& other) const
        {
            return static_cast<u8>(m_flags) != static_cast<u8>(other.m_flags);
        }

        constexpr castling_rights operator|(const castling_rights& other) const
        {
            return castling_rights(static_cast<castling_flag>(static_cast<u8>(m_flags)
                                                              | static_cast<u8>(other.m_flags)));
        }

        constexpr castling_rights& operator|=(const castling_rights& other)
        {
            m_flags = static_cast<castling_flag>(static_cast<u8>(m_flags)
                                                 | static_cast<u8>(other.m_flags));
            return *this;
        }

        constexpr castling_rights operator&(const int update) const
        {
            return castling_rights(
                static_cast<castling_flag>(static_cast<u8>(m_flags) & update));
        }

        constexpr castling_rights& operator&=(const int update)
        {
            m_flags = static_cast<castling_flag>(static_cast<u8>(m_flags) & update);
            return *this;
        }

        [[nodiscard]] constexpr u8 as_u8() const { return static_cast<u8>(m_flags); }

        template<color C>
        [[nodiscard]] bool king_side_available() const
        {
            if constexpr (C == color::white)
                return static_cast<u8>(m_flags) & static_cast<u8>(castling_flag::wk);
            else
                return static_cast<u8>(m_flags) & static_cast<u8>(castling_flag::bk);
        }

        template<color C>
        [[nodiscard]] bool queen_side_available() const
        {
            if constexpr (C == color::white)
                return static_cast<u8>(m_flags) & static_cast<u8>(castling_flag::wq);
            else
                return static_cast<u8>(m_flags) & static_cast<u8>(castling_flag::bq);
        }

        [[nodiscard]] std::string to_string() const
        {
            std::string result;

            if (king_side_available<color::white>())
                result += 'K';
            if (queen_side_available<color::white>())
                result += 'Q';
            if (king_side_available<color::black>())
                result += 'k';
            if (queen_side_available<color::black>())
                result += 'q';

            if (result.empty())
                result = '-';

            return result;
        }

    private:
        castling_flag m_flags;
    };

    class position
    {
    public:
        position(): m_pieces(), m_full_move_number(1), m_stm(color::white), m_ep_sq(square::none), m_half_move_clock(0)
        {
            m_pieces.fill(piece::none);
        }

        explicit position(const std::string& fen): m_pieces()
        {
            m_pieces.fill(piece::none);

            const auto tokens = utils::split_string(fen, ' ');
            const auto ranks  = utils::split_string(tokens[0], '/');

            int rank_index = constants::num_ranks - 1;

            for (const auto& rank: ranks)
            {
                u8 file_index = 0;

                for (const auto c: rank)
                {
                    if (std::isdigit(c))
                        file_index += c - '0';
                    else
                    {
                        const square sq    = square_of(file_index, rank_index);
                        const piece  piece = utils::char_to_piece(c);

                        set_piece(piece, sq);
                        ++file_index;
                    }
                }
                --rank_index;
            }

            m_stm      = tokens[1] == "w" ? color::white : color::black;
            m_castling = castling_rights(tokens[2]);

            const auto& en_passant = tokens[3];
            m_ep_sq = en_passant == "-" ? square::none : square_of(en_passant[0] - 'a', en_passant[1] - 1 - '0');

            m_half_move_clock  = std::stoi(tokens[4]);
            m_full_move_number = std::stoi(tokens[5]);
        }

        [[nodiscard]] color           side_to_move() const { return m_stm; }
        [[nodiscard]] square          ep_square() const { return m_ep_sq; }
        [[nodiscard]] castling_rights castling() const { return m_castling; }
        [[nodiscard]] u8              fifty_move_rule() const { return m_half_move_clock; }
        [[nodiscard]] u16             full_moves() const { return m_full_move_number; }

        [[nodiscard]] piece piece_on(const square sq) const
        {
            return m_pieces[static_cast<u8>(sq)];
        }

        [[nodiscard]] bitboard occupancies(const color c) const
        {
            return m_occupied_bb[static_cast<u8>(c)];
        }

        [[nodiscard]] bitboard piece_type_bb(const piece_type pt) const
        {
            return m_piece_bb[static_cast<u8>(pt)];
        }

        template<color C>
        [[nodiscard]] int piece_count(const piece_type pt) const
        {
            if constexpr (C == color::white)
                return (piece_type_bb(pt) & occupancies(color::white)).bit_count();
            else
                return (piece_type_bb(pt) & occupancies(color::black)).bit_count();
        }

        void set_piece(const piece p, const square sq)
        {
            m_pieces[static_cast<u8>(sq)] = p;

            bitboard::set_bit(m_piece_bb[static_cast<u8>(utils::piece_to_piece_type(p))], sq);
            bitboard::set_bit(m_occupied_bb[static_cast<u8>(utils::piece_color(p))], sq);
        }

    private:
        std::array<piece, constants::num_squares>        m_pieces;
        std::array<bitboard, constants::num_piece_types> m_piece_bb;
        std::array<bitboard, constants::num_colors>      m_occupied_bb;
        u16                                              m_full_move_number;
        color                                            m_stm;
        square                                           m_ep_sq;
        castling_rights                                  m_castling;
        u8                                               m_half_move_clock;
    };
}
