#pragma once

#include <map>
#include <set>
#include <memory>
#include <functional>

class Player;
class Room;
class Protocol;

class Server
{
public:
    Server(int32_t roomLimit);

    auto create_room(uint32_t plimit) -> uint32_t;
    auto create_player(const std::string& name) -> bool;
    auto join_room(uint32_t room_id) -> bool;
    void notify(int32_t fileDescriptor);
    void clean_up_rooms();
    void set_close_function(std::function<void(const int32_t)> closeFunc);

    void foreach_room(const std::function<void(const std::shared_ptr<Room> &)> &consumer);
    void foreach_player(const std::function<void(const std::shared_ptr<Player> &)> &consumer);
    void close_active();


    [[nodiscard]] auto get_player_by_fd(int32_t file_descriptor) const -> std::shared_ptr<Player>;
    [[nodiscard]] auto get_player_by_name(const std::string& name) const -> std::shared_ptr<Player>;
    [[nodiscard]] auto get_active_player() const -> std::shared_ptr<Player>;

    [[nodiscard]] auto get_room_by_id(uint32_t room_id) const -> std::shared_ptr<Room>;
    [[nodiscard]] auto get_active_room() const -> std::shared_ptr<Room>;

private:
    mutable std::shared_ptr<Player> active;

    Protocol* protocol;
    std::set<std::shared_ptr<Player>> players;
    std::set<std::shared_ptr<Room>> rooms;
    std::function<void(const int32_t)> close;
    int32_t roomLimit;
    int32_t fd = -1;

};
