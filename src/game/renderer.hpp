#pragma once

#include <ranges>

#include <ftxui/screen/screen.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>

#include "engine/game.hpp"
#include "proto/protocol.hpp"

using namespace ftxui;


constexpr auto block_to_color(tetriz::Block block) -> Color
{
    switch (block)
    {
        case tetriz::Block::Void:    return Color::Default;
        case tetriz::Block::Cyan:    return Color::Cyan;
        case tetriz::Block::Blue:    return Color::Blue;
        case tetriz::Block::Orange:  return Color::Orange1;
        case tetriz::Block::Yellow:  return Color::Yellow;
        case tetriz::Block::Lime:    return Color::GreenLight;
        case tetriz::Block::Magenta: return Color::Magenta;
        case tetriz::Block::Red:     return Color::Red;
    }
}

constexpr auto render(const tetriz::Board& board, const tetriz::Tetromino tetromino)
{
    auto result = Elements{};
    const auto board_raw = project_on_board(board, tetromino);

    for (const auto& row_raw : board_raw | std::views::drop(2))
    {
        auto row = Elements{};
        for (const auto block : row_raw)
            row.push_back(text(" ┘") | bgcolor(block_to_color(block)) | color(Color::GrayDark));

        result.push_back(hbox(std::move(row)));
    }

    return vbox(result);
}

constexpr auto render(const std::array<tetriz::TetrominoShape, 4>& bag)
{
    auto result = Elements{};
    for (const auto shape : bag)
    {
        for (const auto row_raw : tetriz::bounding_boxes[shape][tetriz::TetrominoRotation::Base] | std::views::take(3))
        {
            auto to_block = [&](bool block){
                return block ? static_cast<tetriz::Block>(shape) : tetriz::Block::Void; };

            auto row = Elements{};
            for (const auto block : row_raw | std::views::transform(to_block))
                row.push_back(text(" ┘") | bgcolor(block_to_color(block)) | color(Color::GrayDark));

            result.push_back(hbox(std::move(row)));
        }
    }

    return vbox(result);
}

constexpr auto render(const std::optional<tetriz::TetrominoShape> tetromino)
{
    auto result = Elements{};
    if (tetromino)
    {
        auto to_block = [&](bool block){
            return block ? static_cast<tetriz::Block>(*tetromino) : tetriz::Block::Void; };

        for (const auto row_raw : tetriz::bounding_boxes[*tetromino][tetriz::TetrominoRotation::Base] | std::views::take(3))
        {
            auto row = Elements{};
            for (const auto block : row_raw | std::views::transform(to_block))
                row.push_back(text(" ┘") | bgcolor(block_to_color(block)) | color(Color::GrayDark));

            result.push_back(hbox(std::move(row)));
        }
    }
    else
    {
        for (const auto _ : std::views::iota(0, 3))
        {
            auto row = Elements{};
            for (const auto _ : std::views::iota(0, 4))
                row.push_back(text(" ┘") | color(Color::GrayDark));

            result.push_back(hbox(std::move(row)));
        }
    }

    return vbox(result);
}

constexpr auto render(uint64_t score)
{
    return text(std::to_string(score)) | align_right | bold;
}

constexpr auto render(Duration timestamp)
{
    return text(std::format("{:%M:%S}", timestamp)) | center;
}

constexpr auto compose(const auto& board, const auto& bag, const auto& swap, const auto& score, const auto& time) -> Element
{
    constexpr auto side_panel_width = 10;
    return hflow(
        window(time, board),
        vbox(
            window(text("score"), score)
                | size(WIDTH, EQUAL, side_panel_width),
            window(text("next"), bag)
                | size(WIDTH, EQUAL, side_panel_width)
                | size(HEIGHT, EQUAL, 14),
            window(text("swap"), swap)
                | size(WIDTH, EQUAL, side_panel_width)
                | size(HEIGHT, EQUAL, 5)
        )
    ) | color(Color::Default);
}

inline
auto make_board_renderer(const tetriz::Game& game, const auto& time_source) -> Component
{
    return Renderer([&] {
        const auto board = render(game.board(), game.current());
        const auto bag = render(game.bag().peek<4>());
        const auto swap = render(game.swapped());
        const auto score = render(game.score());
        const auto time = render(time_source());

        return compose(board, bag, swap, score, time);
    });
}

inline
auto make_board_renderer(const tetriz::proto::DatagramGame& game, const tetriz::proto::DatagramTime& time) -> Element
{
    const auto board = render(game.board, game.current);
    const auto bag = render(game.bag);
    const auto swap = render(game.swap);
    const auto score = render(game.score);
    const auto timestamp = render(time.timestamp);

    return compose(board, bag, swap, score, timestamp);
}

inline
auto make_boards_renderer(std::span<const tetriz::proto::DatagramGame> games, const tetriz::proto::DatagramTime& time) -> Component
{
    return Renderer([games, &time] {
        return ftxui::hbox(games
                | std::views::transform([&time](const auto& game) { return make_board_renderer(game, time); })
                | std::ranges::to<std::vector>());
    });
}
