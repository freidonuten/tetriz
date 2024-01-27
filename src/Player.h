#pragma once

#include <string>
#include <memory>
#include <chrono>

class Room;

class Player : public std::enable_shared_from_this<Player>
{
public:
    Player();
    Player(std::string name, int32_t file_descriptor);

    void logout();
    void connect(int32_t file_descriptor);
    void disconnect();
    void refresh_timestamp();
    void set_room(const std::shared_ptr<Room>& room);

    [[nodiscard]] auto get_inactivity_ms() const -> int64_t;
    [[nodiscard]] auto get_file_descriptor() const -> int32_t;
    [[nodiscard]] auto get_name() const -> std::string;
    [[nodiscard]] auto get_room() const -> std::weak_ptr<Room>;
    [[nodiscard]] auto is_dead() const -> bool;

    auto operator<(const Player& player) const -> bool;
    auto operator==(const Player& player) const -> bool;
    auto operator==(const std::string& name) const -> bool;

private:
    int32_t sockfd = 0;
    std::chrono::high_resolution_clock::time_point timestamp;
    std::string name;
    std::weak_ptr<Room> room;
};
