#include <ftxui/screen/screen.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>

#include "engine/game.hpp"
#include "game/renderer.hpp"

using namespace ftxui;
using namespace std::chrono_literals;


auto main(int argc, char** argv) -> int
{
    const auto time = [start_time = Clock::now()]{
        return std::chrono::duration_cast<Duration>(
            std::chrono::high_resolution_clock::now() - start_time);
    };

    auto game = tetriz::Game{};
    auto game_renderer = make_board_renderer(game, time);

    auto event_listener = CatchEvent(game_renderer, [&](const Event& e) {
        if (e == Event::ArrowLeft || e == Event::Character('h'))
            game.move(tetriz::Direction::Left);
        else if (e == Event::ArrowRight || e == Event::Character('l'))
            game.move(tetriz::Direction::Right);
        else if (e == Event::ArrowDown || e == Event::Character('j'))
            game.move(tetriz::Direction::Down);
        else if (e == Event::ArrowUp || e == Event::Character('k'))
            game.rotate();
        else if (e == Event::Character('c'))
            game.swap();
        else if (e == Event::Character(' '))
            game.drop();
        else
            return false;

        return true;
    });

    auto screen = ScreenInteractive::FitComponent();
    screen.SetCursor(ftxui::Screen::Cursor(0, 0, Screen::Cursor::Hidden));

    auto running = true;
    auto game_worker = std::jthread([&]{
        auto next_tick = std::chrono::steady_clock::now();
        while (running)
        {
            if (std::chrono::steady_clock::now() >= next_tick)
            {
                game.move(tetriz::Direction::Down);
                next_tick += 1s;
            }

            screen.PostEvent(Event::Custom);
            std::this_thread::sleep_for(73ms);
        }
    });

    screen.Loop(event_listener);
    running = false;

    return 0;
}
