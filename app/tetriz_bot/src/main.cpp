#include <ftxui/screen/screen.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>

#include "argparse/argparse.hpp"
#include "engine/game.hpp"
#include "tui/renderer.hpp"
#include "util/time.hpp"
#include "solver/solver.hpp"
#include "logging/logger.hpp"

using namespace ftxui;
using namespace std::chrono_literals;

struct Configuration
{
    uint64_t seed = Clock::now().time_since_epoch().count();
    uint32_t goal;
    uint32_t tick_length;
    uint32_t frame_length;
};

auto parse(int argc, char** argv)
{
    auto program = argparse::ArgumentParser("tetriz", "0.0.0");
    auto configuration = Configuration{};

    program.add_argument("--seed")
        .help("game seed, ommit for random seed")
        .scan<'i', uint64_t>()
        .store_into(configuration.seed);

    program.add_argument("--goal")
        .help("number of lines to clear to win the game")
        .default_value<uint32_t>(40)
        .scan<'i', uint32_t>()
        .nargs(1)
        .store_into(configuration.goal);

    program.add_argument("--tick-length")
        .help("length of each tick in milliseconds")
        .default_value<uint32_t>(1000)
        .scan<'i', uint32_t>()
        .nargs(1)
        .store_into(configuration.tick_length);

    program.add_argument("--frame-length")
        .help("maximum length of a single frame")
        .default_value<uint32_t>(33)
        .scan<'i', uint32_t>()
        .nargs(1)
        .store_into(configuration.frame_length);

    program.parse_args(argc, argv);

    return configuration;
}

auto main(int argc, char** argv) -> int
{
    const auto config = parse(argc, argv);
    const auto time_begin = Clock::now();

    set_log_stream(std::cerr);
    set_log_level(Severity::Trace);

    auto time = std::chrono::duration_cast<Duration>(Clock::now() - time_begin);
    auto game = tetriz::Game(config.seed, config.goal);
    auto game_renderer = make_board_renderer(game, time);
    auto played_sequence = std::optional<tetriz::MoveSequence>{};
    auto player_thread = std::jthread([&played_sequence, &game](std::stop_token stoken){
        while (!stoken.stop_requested())
        {
            if (!played_sequence)
            {
                std::this_thread::sleep_for(10ms);
                continue;
            }

            for (auto move : *played_sequence)
            {
                game.play(move);
                log_trace("Playing: {}", magic_enum::enum_name(move));
            }

            game.drop();
            played_sequence = std::nullopt;
        }
    });
    auto event_listener = CatchEvent(game_renderer, [&](const Event& e) {
        if (game.state() != tetriz::State::Playing)
            return false;

        if (e == Event::Custom)
        {
            played_sequence = [&]{
                const auto main_seq = tetriz::select_optimal_move_sequence(game.board(), game.current());
                const auto alternative = spawn(game.board(), game.swapped().value_or(game.bag().peek<1>().front()));
                if (!alternative)
                    return main_seq.sequence;

                const auto alt_seq = tetriz::select_optimal_move_sequence(game.board(), *alternative);

                if (alt_seq.score > main_seq.score)
                {
                    game.swap();
                    return alt_seq.sequence;
                }

                return main_seq.sequence;
            }();
        }

        return true;
    });

    auto screen = ScreenInteractive::FitComponent();
    screen.SetCursor(ftxui::Screen::Cursor(0, 0, Screen::Cursor::Hidden));

    auto running = std::atomic<bool>(true);

    const auto tick_ms = std::chrono::milliseconds(config.tick_length);
    const auto frame_ms = std::chrono::milliseconds(config.frame_length);
    const auto game_worker = std::jthread([&] {
        auto next_tick = Clock::now() + tick_ms;
        while (running && game.state() == tetriz::State::Playing)
        {
            const auto time_now = Clock::now();
            time = normalize_duration(time_now - time_begin);

            if (time_now >= next_tick)
            {
                game.tick();
                next_tick += tick_ms;

                screen.PostEvent(Event::Custom);
            }

            const auto next_frame = time_now + frame_ms;
            std::this_thread::sleep_until(std::min(next_frame, next_tick));
        }

        screen.Exit();
    });

    screen.Loop(event_listener);
    running = false;
    player_thread.request_stop();

    return 0;
}
