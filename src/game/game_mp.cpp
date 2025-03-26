#include <ftxui/screen/screen.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>

#include "game/renderer.hpp"
#include "networking_socket.hpp"
#include "proto/protocol.hpp"

using namespace ftxui;
using namespace std::chrono_literals;
using namespace tetriz::proto;


auto main(int argc, char** argv) -> int
{
    auto sock = net::ClientSocket();
    sock.set_nodelay();
    sock.connect("127.0.0.1", 6666);

    const auto timeout = timeval{
        .tv_sec = 0,
        .tv_usec = 10'000
    };

    setsockopt(sock.descriptor(), SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    auto game = tetriz::proto::DatagramSync{};
    auto game_renderer = make_board_renderer(game);

    auto event_listener = CatchEvent(game_renderer, [&](const Event& e) {
        using tetriz::proto::Move;

        auto notify_move = [&sock](Move move) {
            sock.write(serialize_move(move));
        };

        if (e == Event::ArrowLeft || e == Event::Character('h'))
            notify_move(Move::Left);
        else if (e == Event::ArrowRight || e == Event::Character('l'))
            notify_move(Move::Right);
        else if (e == Event::ArrowDown || e == Event::Character('j'))
            notify_move(Move::Down);
        else if (e == Event::ArrowUp || e == Event::Character('k'))
            notify_move(Move::Rotate);
        else if (e == Event::Character('c'))
            notify_move(Move::Swap);
        else if (e == Event::Character(' '))
            notify_move(Move::Drop);
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
            if (const auto message = sock.read(); message)
            {
                const auto msg = deserialize(*message);
                if (msg.has_value() && msg->type == MessageType::Sync)
                    game = std::get<DatagramSync>(msg->payload);
            }

            screen.PostEvent(Event::Custom);
        }
    });

    screen.Loop(event_listener);
    running = false;

    return 0;
}

