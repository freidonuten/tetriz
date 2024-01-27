#include <utility>
#include <zconf.h>
#include <chrono>
#include <fcntl.h>

#include "constants.h"
#include "Player.h"
#include "Room.h"


Player::Player()
{
   refresh_timestamp();
}

Player::Player(std::string name, int file_descriptor)
    : sockfd(file_descriptor)
    , name(std::move(name))
{
    refresh_timestamp();
}

auto Player::operator<(const Player& player) const -> bool
{
    return name < player.name;
}

auto Player::operator==(const Player& player) const -> bool
{
    return name == player.name;
}

auto Player::operator==(const std::string& playerName) const -> bool
{
    return name == playerName;
}

auto Player::get_file_descriptor() const -> int32_t
{
    return sockfd;
}

auto Player::get_name() const -> std::string
{
    return name;
}

auto Player::get_room() const -> std::weak_ptr<Room>
{
    return room;
}

void Player::set_room(const std::shared_ptr<Room>& room)
{
    this->room = room;
}

void Player::logout()
{
    disconnect();

    if (auto spt = room.lock())
    {
        spt->kick(shared_from_this());
    }
}

void Player::disconnect()
{
    sockfd = -1;
}

void Player::connect(int file_descriptor)
{
    sockfd = file_descriptor;
    refresh_timestamp();
}

void Player::refresh_timestamp()
{
    timestamp = std::chrono::high_resolution_clock::now();
}

auto Player::get_inactivity_ms() const -> int64_t
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - timestamp
    ).count();
}

auto Player::is_dead() const -> bool
{
    return fcntl(sockfd, F_GETFD) == -1;
}

