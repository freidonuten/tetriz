#include <boost/di.hpp>
#include <argparse/argparse.hpp>

#include "Server.h"
#include "logger.hpp"


auto parse(int argc, char** argv)
{
    auto program = argparse::ArgumentParser("tetrizd", "0.0.0");
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

    program.add_argument("--log-level")
        .help("log level, from 1 to 5")
        .default_value<uint32_t>(2)
        .choices<uint32_t>(1, 2, 3, 4, 5)
        .scan<'i', uint32_t>()
        .metavar("N")
        .store_into(configuration.log_level);

    program.parse_args(argc, argv);

    return configuration;
}

auto main(int argc, char** argv) -> int
{
    namespace di = boost::di;

    const auto config = parse(argc, argv);
    const auto injector = di::make_injector(
        di::bind<Configuration>.to(config)
    );

    log_level = static_cast<Severity>(config.log_level);

    injector
        .create<Server>()
        .start();

    return 0;
}
