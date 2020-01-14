//
// Created by martin on 03.01.20.
//

#include "Player.h"

#include <utility>
#include <zconf.h>
#include <chrono>
#include <fcntl.h>
#include "constants.h"

Player::Player() {
   this->refreshTimestamp();
}

bool Player::operator<(const Player& player) const {
    return this->name < player.name;
}

bool Player::operator==(const Player& player) const {
    return this->name == player.name;
}

bool Player::operator==(const std::string& playerName) const {
    return this->name == playerName;
}

Player::Player(std::string name, int fileDescriptor) {
    this->refreshTimestamp();
    this->name = std::move(name);
    this->sockfd = fileDescriptor;
}

int Player::getFileDescriptor() const {
    return this->sockfd;
}

std::string Player::getName() const {
    return this->name;
}

std::weak_ptr<Room> Player::getRoom() const {
    return this->room;
}

void Player::setRoom(const std::shared_ptr<Room>& r) {
    this->room = r;
}

void Player::logout() {
    this->disconnect();
    if (auto spt = this->room.lock()){
        spt->kick(shared_from_this());
    }
}

void Player::disconnect() {
    this->sockfd = -1;
}

void Player::connect(int fd) {
    this->sockfd = fd;
    this->refreshTimestamp();
}

void Player::refreshTimestamp() {
    this->timestamp = std::chrono::high_resolution_clock::now();
}

long Player::getInactivityMs() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - this->timestamp).count();
}

bool Player::isDead() {
    return fcntl(this->sockfd, F_GETFD) == -1 || this->getInactivityMs() >= ACTIVE_TIMEOUT_MS;
}

