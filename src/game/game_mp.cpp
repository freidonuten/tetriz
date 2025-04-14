#include <span>

#include <ftxui/screen/screen.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>

#include "argparse/argparse.hpp"
#include "game/renderer.hpp"
#include "networking_socket.hpp"
#include "proto/protocol.hpp"

using namespace ftxui;
using namespace std::chrono_literals;
using namespace tetriz::proto;

struct Configuration
{
    std::string host;
    uint16_t port;
};

auto parse(int argc, char** argv)
{
    auto program = argparse::ArgumentParser("tetriz", "0.0.0");
    auto configuration = Configuration{};

    program.add_argument("host")
        .help("ipv4 address to bind")
        .metavar("ADDRESS")
        .default_value("127.0.0.1")
        .store_into(configuration.host);

    program.add_argument("port")
        .help("port number to bind")
        .metavar("PORT")
        .default_value<uint16_t>(4444)
        .scan<'i', uint16_t>()
        .store_into(configuration.port);

    program.parse_args(argc, argv);

    return configuration;
}

auto main(int argc, char** argv) -> int
{
    const auto config = parse(argc, argv);
    auto sock = net::ClientSocket();
    sock.set_nodelay();
    sock.connect(config.host, config.port);

    const auto timeout = timeval{
        .tv_sec = 0,
        .tv_usec = 10'000
    };

    setsockopt(sock.descriptor(), SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    auto game = tetriz::proto::DatagramGame{};
    auto time = tetriz::proto::DatagramTime{};
    auto game_renderer = make_board_renderer(game, time);

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

    sock.write(serialize_hola());

    auto conn_worker = std::jthread([&]{
        while (running)
        {
            if (const auto message = sock.read(); message)
            {
                auto processable = std::span<const uint8_t>(*message);

                while (processable.size() > 0)
                {
                    if (const auto msg = deserialize(processable); !msg)
                        break;
                    else if (msg->type == MessageType::Game)
                    {
                        game = std::get<DatagramGame>(msg->payload);
                        processable = processable.subspan(sizeof(decltype(serialize_game({}))));
                    }
                    else if (msg->type == MessageType::Time)
                    {
                        time = std::get<DatagramTime>(msg->payload);
                        processable = processable.subspan(sizeof(decltype(serialize_time({}))));
                    }
                }
            }

            screen.PostEvent(Event::Custom);
        }
    });

    screen.Loop(event_listener);
    running = false;

    return 0;
}

