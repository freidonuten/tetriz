#pragma once

#include <string>
#include <vector>
#include <functional>
#include <map>
#include <tuple>
#include <memory>
#include <chrono>


class Player;

class Room : public std::enable_shared_from_this<Room> {
public:
    enum class State;

    Room() = default; // FIXME do I need a default constructor?
                      // it's kinda breaking preconditions
    Room(uint32_t plimit, std::shared_ptr<Player> owner);

    void kick(const std::shared_ptr<Player>& player);
    auto join(std::shared_ptr<Player> player) -> bool;
    void foreach_player(const std::function<void(const std::shared_ptr<Player>&)> &consumer);

    auto get_seed() const -> uint32_t;
    auto get_id() const -> uint32_t;
    auto get_player_count() const -> uint32_t;
    auto get_player_limit() const -> uint32_t;
    auto get_delta_t() const -> int64_t;
    auto get_owner() const -> std::shared_ptr<Player>;
    auto get_move(const std::shared_ptr<Player>& player, int32_t i) -> std::string&;
    void add_move(const std::shared_ptr<Player>& player, std::string move);
    auto get_move_count(const std::shared_ptr<Player>& player) -> int32_t;
    auto is_active() -> bool;

    auto operator<(const Room& room) const -> bool;

    auto get_move_timestamp(const std::shared_ptr<Player> &player, int32_t i) -> int64_t;

private:
    static uint32_t last_id;

    std::vector<std::shared_ptr<Player>> players;
    std::map<
        std::shared_ptr<Player>,
        std::vector<std::tuple<std::string, int64_t>>
    > moves;
    uint32_t room_id = ++last_id;
    uint32_t player_limit{};
    uint32_t seed{};
    std::unique_ptr<std::chrono::high_resolution_clock::time_point> startTime;

};
