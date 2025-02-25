#pragma once

#include "logger.hpp"
#include <cstdint>
#include <optional>
#include <ranges>
#include <string>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <map>

using Clock = std::chrono::high_resolution_clock;
using Timestamp = Clock::time_point;
using ObjectId = uint64_t;

template <typename T>
struct optional_ref : public std::optional<std::reference_wrapper<T>>
{
    using Super = std::optional<std::reference_wrapper<T>>;
    using std::optional<std::reference_wrapper<T>>::optional;

    explicit optional_ref(T& obj) : Super(std::ref(obj)) {}

    T* operator->() { return &Super::value().get(); }
    const T* operator->() const { return &Super::value().get(); }

    T& operator*() { return Super::value().get(); }
    const T& operator*() const { return Super::value().get(); }
};

enum class GameMoveType
{
    MoveLeft,
    MoveRight,
    MoveDown,
    Drop,
    Rotate,
    Swap,
};

constexpr auto to_move_type(std::string_view val) -> std::optional<GameMoveType>
{
    if (val == "L") return GameMoveType::MoveLeft;
    if (val == "R") return GameMoveType::MoveRight;
    if (val == "D") return GameMoveType::MoveDown;
    if (val == "SWP") return GameMoveType::Swap;
    if (val == "ROT") return GameMoveType::Rotate;
    if (val == "DRP") return GameMoveType::Drop;
    return std::nullopt;
}

constexpr auto from_move_type(GameMoveType val) -> std::string_view
{
    switch (val)
    {
    case GameMoveType::MoveLeft: return "L";
    case GameMoveType::MoveRight: return "R";
    case GameMoveType::MoveDown: return "D";
    case GameMoveType::Drop: return "DRP";
    case GameMoveType::Rotate: return "ROT";
    case GameMoveType::Swap: return "SWP";
    }
    return "???";
}

struct GameMove
{
    GameMoveType type;
    Timestamp timestamp;
};

struct Room;

struct Player
{
    std::string name = "<unknown>";
    Timestamp timestamp = Clock::now();
    std::optional<ObjectId> room_oid{};
};

struct Room
{
    std::unordered_map<ObjectId, std::vector<GameMove>> players{};
    ObjectId owner;
    uint32_t player_limit{};
    uint32_t seed{};
    std::optional<Timestamp> start_time{};

    auto delta_t() const -> int64_t
    {
        using namespace std::chrono;

        return start_time
            .transform([](auto ts){ return duration_cast<milliseconds>(ts - Clock::now()); })
            .transform([](auto duration){ return duration.count(); })
            .value_or(std::numeric_limits<int64_t>::max());
    }

    constexpr auto is_waiting() const -> bool
    {
        return start_time
            .transform([](const auto ts){ return ts > Clock::now(); })
            .value_or(true);
    }

    constexpr auto is_full() const -> bool
    {
        return players.size() >= player_limit;
    }

    constexpr auto is_joinable() const -> bool
    {
        return is_waiting() && !is_full();
    }
};

inline auto oid_mapper = std::map<uint32_t, ObjectId>{};

class Model
{
public:
    auto room(ObjectId id) -> optional_ref<Room>
    {
        if (const auto room_iter = rooms_.find(id); room_iter != rooms_.end())
            return room_iter->second;
        else
            return std::nullopt;
    }

    auto room(ObjectId id) const -> optional_ref<const Room>
    {
        if (const auto room_iter = rooms_.find(id); room_iter != rooms_.end())
            return room_iter->second;
        else
            return std::nullopt;
    }

    auto player(ObjectId id) -> optional_ref<Player>
    {
        if (const auto player_iter = players_.find(id); player_iter != players_.end())
            return player_iter->second;
        else
            return std::nullopt;
    }

    auto player(ObjectId id) const -> optional_ref<const Player>
    {
        if (const auto player_iter = players_.find(id); player_iter != players_.end())
            return player_iter->second;
        else
            return std::nullopt;
    }

    auto rooms() const { return std::views::all(rooms_); }
    auto players() const { return std::views::all(players_); }

    auto room_create(ObjectId owner_id, uint32_t player_limit) -> std::optional<ObjectId>
    {
        auto player = this->player(owner_id);

        if (!player)
            return std::nullopt;

        auto [room, success] = rooms_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(next_id++),
            std::forward_as_tuple(
                std::unordered_map<ObjectId, std::vector<GameMove>>{ {owner_id, {}} },
                owner_id,
                player_limit
            )
        );

        if (!success)
            return std::nullopt;

        player->room_oid = room->first;

        return room->first;
    }

    auto player_create(const std::string& name) -> std::optional<ObjectId>
    {
        const auto [iter, success] = players_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(next_id++),
            std::forward_as_tuple(name)
        );

        if (success)
            return iter->first;

        return std::nullopt;
    }

    auto player_join_room(ObjectId player_id, ObjectId room_id) -> bool
    {
        using namespace std::chrono_literals;

        if (auto room = this->room(room_id); room && room->is_joinable())
        {
            room->players.emplace(player_id, std::vector<GameMove>{});
            players_.at(player_id).room_oid = room_id;

            if (room->players.size() == room->player_limit)
            {
                room->start_time = Clock::now() + 10s;
            }

            return true;
        }

        return false;
    }

    auto player_leave_room(ObjectId player_id) -> bool
    {
        auto player = this->player(player_id);

        if (!player || !player->room_oid)
            return false;

        auto room = this->room(*player->room_oid);
        room->players.erase(player_id);

        // rebing owner or collect the room if we're the last player
        if (room->owner == player_id)
        {
            if (room->players.size() > 0)
                room->owner = room->players.begin()->first;
            else
                rooms_.erase(*player->room_oid);
        }

        player->room_oid = std::nullopt;

        return true;
    }

private:
    std::unordered_map<ObjectId, Player> players_;
    std::unordered_map<ObjectId, Room> rooms_;

    static inline ObjectId next_id = 1;
};
