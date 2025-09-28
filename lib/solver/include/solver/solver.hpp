#pragma once

#include <generator>
#include <stack>
#include <unordered_set>
#include <ranges>

#include "engine/game_board.hpp"
#include "engine/game.hpp"
#include "util/algorithm.hpp"


namespace tetriz
{
    using MoveSequence = std::vector<Move>;
    struct StateExpansion
    {
        Tetromino tetromino;
        MoveSequence sequence;
    };

    inline
    auto expand_state(const Board& board, Tetromino initial) -> std::generator<const StateExpansion&>
    {
        auto stack = std::stack<StateExpansion>{{{initial, {}}}};
        auto visited = std::unordered_set<Tetromino>{};

        while (!stack.empty())
        {
            co_yield stack.top();

            auto [tetromino, sequence] = stack.top();
                                         stack.pop();

            if (sequence.size() == sequence.max_size())
                continue;

            for (const auto move : { Move::Left, Move::Right, Move::Rotate })
            {
                auto candidate = tetromino;

                switch (move)
                {
                    case Move::Left: candidate += { -1,  0}; break;
                    case Move::Right: candidate += { +1,  0}; break;
                    case Move::Rotate: candidate = rotate(board, tetromino).value_or(tetromino); break;
                    default: std::unreachable();
                }

                if (!visited.emplace(candidate).second)
                    continue;

                if (!projects(board, candidate))
                    continue;

                sequence.push_back(move);
                stack.emplace(candidate, sequence);
                sequence.pop_back();
            }
        }

        co_return;
    }

    template <typename T, size_t Dim1, size_t Dim2>
    [[nodiscard]] constexpr
    auto transpose(const std::array<std::array<T, Dim2>, Dim1>& matrix)
        -> std::array<std::array<T, Dim1>, Dim2>
    {
        const auto ys = std::views::iota(0ul, Dim1);
        const auto xs = std::views::iota(0ul, Dim2);
        auto result = std::array<std::array<T, Dim1>, Dim2>{};

        for (const auto [x, y] : std::views::cartesian_product(xs, ys))
            result[x][y] = matrix[y][x];

        return result;
    }

    constexpr
    auto heuristic_height(const Board& board) -> double
    {
        constexpr auto aggregate_max = board_width * board_height;
        const auto board_t = transpose(board);
        const auto heights = board_t | std::views::transform([](const auto& col) {
            return std::distance(col.begin(), utl::ranges::find_not(col, Block::Void));
        });

        return aggregate_max - *std::ranges::fold_left_first(heights, std::plus{});
    }

    constexpr
    auto heuristic_holes(const Board& board) -> double
    {
        const auto board_t = transpose(board);
        const auto holes = board_t | std::views::transform([](const auto& col){
            return std::count(utl::ranges::find_not(col, Block::Void), col.end(), Block::Void);
        });

        return *std::ranges::fold_left_first(holes, std::plus{});
    }

    constexpr
    auto heuristic_complete_lines(const Board& board) -> double
    {
        static constexpr auto is_complete = [](const auto& row){
            return !std::ranges::contains(row, Block::Void);
        };

        return std::ranges::count_if(board, is_complete);
    }

    constexpr
    auto heuristic_bumpyness(const Board& board) -> double
    {
        const auto board_t = transpose(board);
        const auto height_differences = board_t
            | std::views::transform([](const auto& col) { return std::distance(col.begin(), utl::ranges::find_not(col, Block::Void)); })
            | std::views::adjacent_transform<2>([](auto lhs, auto rhs) { return std::abs(lhs - rhs); });

        return *std::ranges::fold_left_first(height_differences, std::plus{});
    }

    template <typename Fn>
    struct WeightedHeuristic
    {
        double w;
        Fn fn;
    };

    template <typename... Hs>
    constexpr
    auto meta_heuristic(const Board& board, Hs... hs) -> double
    {
        return ((hs.w * hs.fn(board)) + ...);
    }

    constexpr
    auto fitness_func(const Board& board, Tetromino tetromino) -> double
    {
        return meta_heuristic(
            project(board, drop(board, tetromino)),
            WeightedHeuristic{ .w = -0.51, .fn = heuristic_height },
            WeightedHeuristic{ .w = -0.18, .fn = heuristic_bumpyness },
            WeightedHeuristic{ .w = +0.76, .fn = heuristic_complete_lines },
            WeightedHeuristic{ .w = -0.36, .fn = heuristic_holes }
        );
    }

    struct ScoredSequence
    {
        double score;
        MoveSequence sequence;
    };

    constexpr
    auto select_optimal_move_sequence(const Board& board, Tetromino tetromino) -> ScoredSequence
    {
        constexpr auto cmp = [](const auto& lhs, const auto& rhs) { return lhs.score < rhs.score; };
        const  auto proj = [&board](auto& expansion){
            auto& [t, s] = expansion;
            return ScoredSequence{
                .score = fitness_func(board, t),
                .sequence = std::move(s)
            };
        };

        return std::ranges::max(expand_state(board, tetromino) | std::views::transform(proj), cmp);
    }
}
