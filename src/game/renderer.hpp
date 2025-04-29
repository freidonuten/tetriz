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

constexpr auto render(const tetriz::Board& board, const tetriz::Tetromino& tetromino)
{
    return canvas(10*4, 20*4, [&](Canvas& canvas) {
        const auto board_raw = project_on_board(board, tetromino);

        for (int r = 0; r < 20; r++)
            for (int c = 0; c < 10; c++)
                canvas.DrawText(c * 4, r * 4, " ┘", [b=board_raw[r+2][c]](Pixel &p) {
                    p.foreground_color = Color::GrayDark;
                    if (b != tetriz::Block::Void)
                        p.background_color = block_to_color(b);
                });
    });
}

constexpr auto render(std::ranges::range auto bag)
{
    return canvas(4*4, 12*4, [bag](Canvas& canvas) {
        for (const auto [n, shape] : bag | std::views::enumerate)
        {
            const auto& bb = tetriz::bounding_boxes[shape][tetriz::TetrominoRotation::Base];
            for (const auto [o, row_raw] : bb | std::views::take(3) | std::views::enumerate)
                for (const auto [x, block] : row_raw | std::views::enumerate)
                    canvas.DrawText(x * 4, (n*3+o)*4, " ┘", [block, shape](Pixel& p) {
                        p.foreground_color = Color::GrayDark;
                        if (block)
                            p.background_color = block_to_color(static_cast<tetriz::Block>(shape));
                    });
        }
    });
}

constexpr auto render(const std::optional<tetriz::TetrominoShape>& tetromino)
{
    return canvas(4*4, 3*4, [&](Canvas& canvas) {
        if (tetromino)
        {
            const auto& bb = tetriz::bounding_boxes[*tetromino][tetriz::TetrominoRotation::Base];
            for (const auto [y, row_raw] : bb | std::views::take(3) | std::views::enumerate)
                for (const auto [x, block] : row_raw | std::views::enumerate)
                    canvas.DrawText(x * 4, y * 4, " ┘", [block, shape=*tetromino](Pixel& p) {
                        p.foreground_color = Color::GrayDark;
                        if (block)
                            p.background_color = block_to_color(static_cast<tetriz::Block>(shape));
                    });
        }
        else
        {
            canvas.DrawText(0, 0, " ┘ ┘ ┘ ┘", Color::GrayDark);
            canvas.DrawText(0, 4, " ┘ ┘ ┘ ┘", Color::GrayDark);
            canvas.DrawText(0, 8, " ┘ ┘ ┘ ┘", Color::GrayDark);
        }
    });
}

constexpr auto render(uint64_t score)
{
    return text(std::to_string(score)) | align_right | bold;
}

constexpr auto render(Duration timestamp)
{
    return text(std::format("{:%M:%S}", timestamp)) | align_right;
}

constexpr auto render(const auto& board, const auto& bag, const auto& swap, const auto& score, const auto& time) -> Elements
{
    constexpr auto side_panel_width = 10;
    return {
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
    };
}

inline
auto make_board_renderer(const tetriz::Game& game, const Duration& game_time) -> Component
{
    return Renderer([&] {
        const auto board = render(game.board(), game.current());
        const auto bag = render(game.bag().peek<4>());
        const auto swap = render(game.swapped());
        const auto score = render(game.score());
        const auto time = render(game_time);

        static constexpr auto labels = magic_enum::containers::array<tetriz::State, std::string_view>{
            "",
            "·▄▄▄▄         ▐ ▄ ▄▄▄ .\n"
            "██▪ ██ ▪     •█▌▐█▀▄.▀·\n"
            "▐█· ▐█▌ ▄█▀▄ ▐█▐▐▌▐▀▀▪▄\n"
            "██. ██ ▐█▌.▐▌██▐█▌▐█▄▄▌\n"
            "▀▀▀▀▀•  ▀█▄▀▪▀▀ █▪ ▀▀▀ \n",
            " ▄▄    ▄▄▄    ▌   ▄  ▄▄▄\n"
            "▐█ ▀   █  █  ██ ▐███ ▀▄ ▀\n"
            "▄█ ▀█▄ █▀▀█ ▐█ ▌▐▌▐█ ▐▀▀ ▄\n"
            "▐█▄ ▐█ █  ▌ ██ ██▌▐█▌▐█▄▄▌\n"
            " ▀▀▀▀  ▀  ▀ ▀▀  █ ▀▀▀ ▀▀▀\n"
            "            ▌ ▐ ▄▄▄  ▄▄▄\n"
            "      ▄█▀▄  █ █▌▀▄ ▀ ▀▄ █\n"
            "     ▐█▌ ▐▌▐█▐█ ▐▀▀ ▄▐▀▀▄\n"
            "      ▀█▄▀  ███ ▐█▄▄▌▐█ █▌\n"
            "             ▀   ▀▀▀  ▀  ▀\n",
        };

        switch (game.state()) {
        case tetriz::State::Playing:
            return hbox(render(board, bag, swap, score, time));
        case tetriz::State::GameOver:
            return dbox(
                hbox(render(board, bag, swap, score, time)),
                paragraph(std::string(labels[game.state()])) | color(Color::Red3) | border | bgcolor(Color::RGBA(0,0,0,230)) | center
            );
        case tetriz::State::Finished:
            return dbox(
                hbox(render(board, bag, swap, score, time)),
                paragraph(std::string(labels[game.state()])) | color(Color::LightGreen) | border | bgcolor(Color::RGBA(0,0,0,230)) | center
            );
        }
    });
}

inline
auto render(const tetriz::proto::DatagramGame& game, const tetriz::proto::DatagramTime& time) -> Elements
{
    const auto board = render(game.board, game.current);
    const auto bag = render(game.bag);
    const auto swap = render(game.swap);
    const auto score = render(game.score);
    const auto timestamp = render(time.timestamp);

    return render(board, bag, swap, score, timestamp);
}

inline
auto make_boards_renderer(std::span<const tetriz::proto::DatagramGame> games, const tetriz::proto::DatagramTime& time) -> Component
{
    return Renderer([games, &time] {
        return ftxui::hbox(games
                | std::views::transform([&time](const auto& game) { return render(game, time); })
                | std::views::join
                | std::ranges::to<std::vector>());
    });
}
