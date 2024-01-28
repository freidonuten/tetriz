#pragma once

#include "CorruptedRequestException.h"
#include "MessageTokenizer.h"
#include "protocol_serialization.hpp"
#include <cstdint>
#include <format>
#include <string>
#include <variant>
#include <boost/pfr.hpp>


namespace protocol
{
    enum class State : uint8_t
    {
        ANONYMOUS,
        SIGNED
    };

    template<typename T>
    concept Message = requires {
        { T::symbol } -> std::same_as<const char>;
        { T::state } -> std::same_as<const State>;
    };

    inline constexpr struct login_t
    {
        constexpr static auto symbol = 'L';
        constexpr static auto state = State::ANONYMOUS;
        std::string user_name;
    } login;

    inline constexpr struct ping_t
    {
        constexpr static auto symbol = '!';
        constexpr static auto state = State::ANONYMOUS;
    } ping;

    inline constexpr struct player_list_t
    {
        constexpr static auto symbol = 'P';
        constexpr static auto state = State::SIGNED;
        int32_t room_id{};
    } player_list;

    inline constexpr struct room_list_t
    {
        constexpr static auto symbol = 'C';
        constexpr static auto state = State::SIGNED;
    } room_list;

    inline constexpr struct room_read_t
    {
        constexpr static auto symbol = 'R';
        constexpr static auto state = State::SIGNED;
        int32_t room_id{};
    } room_read;

    inline constexpr struct room_time_t
    {
        constexpr static auto symbol = 'O';
        constexpr static auto state = State::SIGNED;
    } room_time;

    inline constexpr struct room_join_t
    {
        constexpr static auto symbol = 'J';
        constexpr static auto state = State::SIGNED;
        int32_t room_id{};
    } room_join;

    inline constexpr struct room_leave_t
    {
        constexpr static auto symbol = 'V';
        constexpr static auto state = State::SIGNED;
    } room_leave;

    inline constexpr struct room_seed_t
    {
        constexpr static auto symbol = 'S';
        constexpr static auto state = State::SIGNED;
    } room_seed;

    inline constexpr struct room_new_t
    {
        constexpr static auto symbol = 'N';
        constexpr static auto state = State::SIGNED;
        int32_t player_limit{};
    } room_new;

    inline constexpr struct room_active_t
    {
        constexpr static auto symbol = '?';
        constexpr static auto state = State::SIGNED;
    } room_active;

    inline constexpr struct room_delta_t
    {
        constexpr static auto symbol = 'T';
        constexpr static auto state = State::SIGNED;
    } room_delta;

    inline constexpr struct move_play_t
    {
        constexpr static auto symbol = 'M';
        constexpr static auto state = State::SIGNED;
        std::string move{};
    } move_play;

    inline constexpr struct move_get_t
    {
        constexpr static auto symbol = 'G';
        constexpr static auto state = State::SIGNED;
        std::string player{};
        int32_t index{};
    } move_get;

    inline constexpr struct move_last_timestamp_t
    {
        constexpr static auto symbol = 'X';
        constexpr static auto state = State::SIGNED;
    } move_last_timestamp;

    using MessageVariant = std::variant<
        login_t,
        ping_t,
        player_list_t,
        room_list_t,
        room_read_t,
        room_time_t,
        room_join_t,
        room_leave_t,
        room_seed_t,
        room_new_t,
        room_active_t,
        room_delta_t,
        move_play_t,
        move_get_t,
        move_last_timestamp_t
    >;

    constexpr
    auto deserialize(std::string_view message) -> MessageVariant
    {
        return deserialize<MessageVariant>(message);
    }
}
