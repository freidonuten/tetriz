#pragma once

#include <deque>

#include "server/room.hpp"


class RoomList
{
    auto get_room_iter(this auto& self, net::ConnectionWrapper client)
    {
        return std::ranges::find_if(self.rooms_, [client](const auto& room){ return room.has_member(client); });
    }

public:
    ~RoomList()
    {
        std::ranges::for_each(rooms_, &Room::stop);
    }
    
    auto has_room(net::ConnectionWrapper client) const
    {
        return get_room_iter(client) != rooms_.end();
    }

    auto create_room(size_t size) -> Room&
    {
        while (!rooms_.empty() && rooms_.front().empty())
            rooms_.pop_front();

        return rooms_.emplace_back(size);
    }

    auto get_room(net::ConnectionWrapper client) -> std::optional<std::reference_wrapper<Room>>
    {
        const auto iter = get_room_iter(client);
        if (iter != rooms_.end())
            return *iter;

        return std::nullopt;
    }

    [[nodiscard]]
    auto get_available_room(size_t size) -> Room&
    {
        const auto iter = std::ranges::find_if(rooms_, [size](const Room& room) {
            return room.size() == size && room.has_slot();
        });

        if (iter != rooms_.end())
            return *iter;

        return create_room(size);
    }

private:
    std::deque<Room> rooms_;

};
