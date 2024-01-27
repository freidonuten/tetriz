//
// Created by martin on 03.01.20.
//

#include <chrono>
#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>
#include <ctime>
#include <climits>
#include "Room.h"
#include "Player.h"
#include "constants.h"

uint32_t Room::last_id = 0;


Room::Room(uint32_t plimit, std::shared_ptr<Player> owner)
    : room_id(++Room::last_id)
    , player_limit(plimit)
    , seed((srand(time(nullptr)), rand()))
    , startTime(nullptr)
{
    players.emplace_back(std::move(owner));
}

void Room::kick(const std::shared_ptr<Player>& player)
{
    player->set_room(nullptr);

    auto player_iter = std::find(players.begin(), players.end(), player);

    if (player_iter != players.end())
    {
        players.erase(player_iter);
    }
}

auto Room::join(std::shared_ptr<Player> player) -> bool
{
    if (get_player_count() < get_player_limit() && !player->get_room().lock())
    {
        player->set_room(shared_from_this());
        players.push_back(std::move(player));

        if (players.size() == player_limit)
        {
            startTime = std::make_unique<std::chrono::high_resolution_clock::time_point>(
                    std::chrono::high_resolution_clock::now()
                    + std::chrono::duration<int32_t, std::milli>(GAME_START_DELAY)
            );
//            startTime = millis_after(GAME_START_DELAY);
            for(const auto& p : players)
            {
                moves[p] = std::vector<std::tuple<std::string, int64_t>>();
            }
        }

        return true;
    }

    return false;
}

void Room::foreach_player(const std::function<void(const std::shared_ptr<Player>&)> &consumer)
{
    for(const auto& player : players)
    {
       consumer(player);
    }
}

auto Room::get_id() const -> uint32_t
{
    return room_id;
}

auto Room::get_player_count() const -> uint32_t
{
    return players.size();
}

auto Room::get_player_limit() const -> uint32_t
{
    return player_limit;
}

auto Room::get_owner() const -> std::shared_ptr<Player>
{
    return players.front();
}

auto Room::operator<(const Room& room) const -> bool
{
    return get_id() < room.get_id();
}

auto Room::get_delta_t() const -> int64_t
{
    if (!startTime)
    {
        return std::numeric_limits<int64_t>::max();
    }

    return std::chrono::duration_cast<std::chrono::milliseconds>(
        *startTime - std::chrono::high_resolution_clock::now()
    ).count();
}

void Room::add_move(const std::shared_ptr<Player>& player, std::string move)
{
    moves.at(player).emplace_back(std::move(move), -get_delta_t());
}

auto Room::get_move(const std::shared_ptr<Player>& player, int32_t index) -> std::string&
{
    return std::get<0>(moves.at(player).at(index));
}

auto Room::get_move_timestamp(const std::shared_ptr<Player>& player, int32_t index) -> int64_t
{
    return std::get<1>(moves.at(player).at(index));
}

auto Room::get_move_count(const std::shared_ptr<Player> &player) -> int32_t
{
    return moves.at(player).size();
}

auto Room::is_active() -> bool
{
    const auto is_alive = [](const auto& player){ return !player->is_dead(); };
    const auto player_iter = std::find_if(players.begin(), players.end(), is_alive);

    return get_player_count() > 0 && player_iter != players.end();
}

auto Room::get_seed() const -> uint32_t
{
    return seed;
}
